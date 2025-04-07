// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponControllerComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "ParkourGame/ParkourCharacter.h"
#include "ParkourGame/WeaponActor.h"
#include "ParkourGame/WeaponPickupMaster.h"

#define PRINT_TO_SCREEN(Message, Time, Color) \
if (GEngine) \
{ \
GEngine->AddOnScreenDebugMessage(-1, Time, Color, Message); \
}

UWeaponControllerComponent::UWeaponControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentWeaponIndex = 0;

	Weapons.SetNum(36);
}

void UWeaponControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = Cast<ACharacter>(GetOwner());
}

void UWeaponControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetWeaponVisibility();

	if (!CurrentWeapon)
	{
		FollowCamera->SetRelativeRotation(FMath::RInterpTo(FollowCamera->GetRelativeRotation(), TargetCameraRotation,
		                                                   GetWorld()->GetDeltaSeconds(), DefaultCameraInterpSpeed));
	}

	if (Weapons[CurrentWeaponIndex])
	{
		PRINT_TO_SCREEN(Weapons[CurrentWeaponIndex]->GetName(), 0.0f, FColor::Green);
	}
}

void UWeaponControllerComponent::WeaponRecoil(const FRotator NewCameraRotation)
{
	TargetCameraRotation += NewCameraRotation;
}

void UWeaponControllerComponent::PickUpWeapon()
{
	TArray<AActor*> Pickups;
	CharacterOwner->GetOverlappingActors(Pickups, AWeaponPickupMaster::StaticClass());

	if (Pickups.Num() != 0)
	{
		AWeaponPickupMaster* NewWeapon = Cast<AWeaponPickupMaster>(Pickups[0]);
		NewWeapon->Interact(this);
		NewWeapon->OnWeaponEquip();
	}
}

void UWeaponControllerComponent::DropAWeapon(AWeaponActor*& WeaponToDrop, const bool IsASecondWeapon)
{
	if (WeaponToDrop)
	{
		WeaponToDrop->DetachFromActor(FDetachmentTransformRules(
			EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, EDetachmentRule::KeepWorld, true));
		WeaponToDrop->EnableCollision();
		WeaponToDrop->SetActorLocation(WeaponToDrop->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 100,
		                               false, nullptr, ETeleportType::TeleportPhysics);
		WeaponToDrop->OnWeaponUnequip();
		WeaponToDrop->PlayerCamera = nullptr;
		WeaponToDrop->WeaponController = nullptr;
		if (!IsASecondWeapon)
		{
			Weapons.Remove(WeaponToDrop);
		}
		WeaponToDrop = nullptr;
	}
}

void UWeaponControllerComponent::DropCurrentWeapon()
{
	if (CurrentWeapon)
	{
		DropAWeapon(CurrentWeapon);
		return;
	}

	if (SecondCurrentWeapon)
	{
		DropAWeapon(SecondCurrentWeapon, true);
		return;
	}
}

void UWeaponControllerComponent::SpawnWeapon_Implementation(AWeaponPickupMaster* WeaponToSpawn)
{
	const auto GunWeapon = Cast<AWeaponActor>(WeaponToSpawn);
	if (CurrentWeapon)
	{
		DropAWeapon(CurrentWeapon);
	}

	Weapons.Insert(GunWeapon, CurrentWeaponIndex);
	CurrentWeapon = GunWeapon;
	// SetArrayElement<AWeaponActor*>(GunWeapon, Weapons, CurrentWeaponIndex);
	GunWeapon->DisableCollision();
	GunWeapon->AttachToComponent(WeaponHolder, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
	// SetWeaponVisibility();
	GunWeapon->PlayerCamera = FollowCamera;
	GunWeapon->GetRootComponent()->SetRelativeLocation(GunWeapon->GetWeaponDefaultRelativeLocation());

	EquipWeaponAnimation();
}


void UWeaponControllerComponent::EquipWeaponAnimation()
{
	WeaponHolder->SetRelativeLocation(FVector(0, -20, -50));
	GetWorld()->GetTimerManager().SetTimer(WeaponEquipTimerHandle, [&]()
	{
		const auto NewLocation = UKismetMathLibrary::VInterpTo(WeaponHolder->GetRelativeLocation(), FVector::ZeroVector,
		                                                       GetWorld()->GetDeltaSeconds(), 1);

		WeaponHolder->SetRelativeLocation(NewLocation);
		if (WeaponHolder->GetRelativeLocation() == FVector::ZeroVector)
		{
			GetWorld()->GetTimerManager().ClearTimer(WeaponEquipTimerHandle);
		}
	}, 0.001f, true);
}

void UWeaponControllerComponent::EquipSecondWeaponAnimation()
{
	SecondWeaponHolder->SetRelativeLocation(FVector(0, -20, -50));
	GetWorld()->GetTimerManager().SetTimer(SecondWeaponEquipTimerHandle, [&]()
	{
		const auto NewLocation = UKismetMathLibrary::VInterpTo(SecondWeaponHolder->GetRelativeLocation(),
		                                                       FVector::ZeroVector, GetWorld()->GetDeltaSeconds(), 1);

		SecondWeaponHolder->SetRelativeLocation(NewLocation);
		if (SecondWeaponHolder->GetRelativeLocation() == FVector::ZeroVector)
		{
			GetWorld()->GetTimerManager().ClearTimer(SecondWeaponEquipTimerHandle);
		}
	}, 0.001f, true);
}

void UWeaponControllerComponent::SetWeaponVisibility()
{
	for (const auto ElementWeapon : Weapons)
	{
		if (ElementWeapon)
		{
			if ((CurrentWeapon == ElementWeapon) || (SecondCurrentWeapon == ElementWeapon))
			{
				ElementWeapon->GetRootComponent()->SetVisibility(true);
			}
			else
			{
				ElementWeapon->GetRootComponent()->SetVisibility(false);
			}
		}
	}
}

void UWeaponControllerComponent::SetCurrentWeaponIndex(int WeaponIndex)
{
	if (CurrentWeaponIndex == WeaponIndex)
	{
		return;
	}
	if (CurrentWeapon)
	{
		CurrentWeapon->ResetFire();
		CurrentWeapon->OnWeaponUnequip();
	}
	CurrentWeaponIndex = WeaponIndex;
	CurrentWeapon = Weapons[WeaponIndex];

	EquipWeaponAnimation();
}

void UWeaponControllerComponent::FirePressed()
{
	if (!CurrentWeapon)
	{
		return;
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(WeaponEquipTimerHandle))
	{
		return;
	}
	CurrentWeapon->FireWeapon(true);
}

void UWeaponControllerComponent::FireReleased()
{
	if (!CurrentWeapon)
	{
		return;
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(WeaponEquipTimerHandle))
	{
		return;
	}
	CurrentWeapon->FireWeapon(false);
}

void UWeaponControllerComponent::SecondFirePressed()
{
	if (!SecondCurrentWeapon)
	{
		return;
	}
	SecondCurrentWeapon->FireWeapon(true);
}

void UWeaponControllerComponent::SecondFireReleased()
{
	if (!SecondCurrentWeapon)
	{
		return;
	}
	SecondCurrentWeapon->FireWeapon(false);
}

void UWeaponControllerComponent::ResetFire()
{
	if (FireTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	}
}

void UWeaponControllerComponent::ReloadWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->ReloadWeapon();
	}

	if (SecondCurrentWeapon)
	{
		SecondCurrentWeapon->ReloadWeapon();
	}
}

void UWeaponControllerComponent::SwapWeaponSide()
{
	if (!SecondWeaponHolder)
	{
		return;
	}

	if (CurrentWeapon && !SecondCurrentWeapon)
	{
		if (!CurrentWeapon->GetCanBeEquipedInSecondHand())
		{
			return;
		}
		CurrentWeapon->ResetFire();
		SecondCurrentWeapon = CurrentWeapon;
		CurrentWeapon = nullptr;
		SecondCurrentWeapon->AttachToComponent(SecondWeaponHolder,
		                                       FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
		Weapons.RemoveAt(CurrentWeaponIndex);
		EquipSecondWeaponAnimation();
		return;
	}

	if (SecondCurrentWeapon && !CurrentWeapon)
	{
		if (!SecondCurrentWeapon->GetCanBeEquipedInSecondHand())
		{
			return;
		}
		SecondCurrentWeapon->ResetFire();
		CurrentWeapon = SecondCurrentWeapon;
		SecondCurrentWeapon = nullptr;
		CurrentWeapon->AttachToComponent(WeaponHolder, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
		Weapons.Insert(CurrentWeapon, CurrentWeaponIndex);
		SetWeaponVisibility();
		EquipWeaponAnimation();
		return;
	}

	if (SecondCurrentWeapon && CurrentWeapon)
	{
		if (!CurrentWeapon->GetCanBeEquipedInSecondHand() || !SecondCurrentWeapon->GetCanBeEquipedInSecondHand())
		{
			return;
		}
		CurrentWeapon->ResetFire();
		SecondCurrentWeapon->ResetFire();
		const auto OldSecondWeapon = SecondCurrentWeapon;
		const auto OldWeapon = CurrentWeapon;
		CurrentWeapon = OldSecondWeapon;
		SecondCurrentWeapon = OldWeapon;
		SecondCurrentWeapon->AttachToComponent(SecondWeaponHolder,
		                                       FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
		CurrentWeapon->AttachToComponent(WeaponHolder, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));

		Weapons.RemoveAt(CurrentWeaponIndex);
		Weapons.Insert(OldSecondWeapon, CurrentWeaponIndex);

		EquipSecondWeaponAnimation();
		EquipWeaponAnimation();
		return;
	}
}

void UWeaponControllerComponent::SetLockMovement(const bool bLockMovement)
{
	if (!CharacterOwner)
	{
		return;
	}
	if (const auto pkc = Cast<AParkourCharacter>(CharacterOwner))
	{
		pkc->bLockMovement = bLockMovement;
	}
}

void UWeaponControllerComponent::WeaponSway()
{
}
