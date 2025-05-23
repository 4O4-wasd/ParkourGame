// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponController.h"

#include "RangeWeapon.h"
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

#define PRINT_TO_SCREEN(Message, Time, Color) \
if (GEngine) \
{ \
GEngine->AddOnScreenDebugMessage(-1, Time, Color, Message); \
}

UWeaponController::UWeaponController():
	CurrentItem(nullptr),
	SecondCurrentItem(nullptr),
	FollowCamera(nullptr),
	WeaponHolder(nullptr),
	SecondWeaponHolder(nullptr),
	CharacterOwner(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentItemIndex = 0;

	Items.SetNum(4);
}

void UWeaponController::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = Cast<ACharacter>(GetOwner());
}

void UWeaponController::TickComponent(const float DeltaTime, const ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetWeaponVisibility();

	if (!CurrentItem && FollowCamera)
	{
		FollowCamera->SetRelativeRotation(
			FMath::RInterpTo(
				FollowCamera->GetRelativeRotation(),
				FRotator(0, 0, 0),
				GetWorld()->GetDeltaSeconds(),
				DefaultCameraInterpSpeed
			)
		);
	}

	if (Items[CurrentItemIndex])
	{
		PRINT_TO_SCREEN(Items[CurrentItemIndex]->GetName(), 0.0f, FColor::Green);
	}
}

void UWeaponController::PickUpAPickup()
{
	TArray<AActor*> Pickups;
	CharacterOwner->GetOverlappingActors(Pickups, AItem::StaticClass());

	if (Pickups.Num() != 0)
	{
		const auto Pickup = Cast<AItem>(Pickups[0]);
		Pickup->Interact(this);
	}
}

void UWeaponController::DropAnItem(AWeapon*& WeaponToDrop, const bool IsASecondWeapon)
{
	if (WeaponToDrop)
	{
		WeaponToDrop->DetachFromActor(FDetachmentTransformRules(
			EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
		WeaponToDrop->EnableSettingsForThrowing();
		WeaponToDrop->GetCollisionComponent()->AddForce(
			(FollowCamera->GetForwardVector() * 100) * 15000);
		WeaponToDrop->OnWeaponUnEquip();
		WeaponToDrop->SetPlayerCamera(nullptr);
		WeaponToDrop->ResetWeaponController();
		if (!IsASecondWeapon)
		{
			Items.Remove(WeaponToDrop);
		}
		WeaponToDrop = nullptr;
	}
}

void UWeaponController::DropCurrentItem()
{
	if (CurrentItem)
	{
		DropAnItem(CurrentItem);
		return;
	}

	if (SecondCurrentItem)
	{
		DropAnItem(SecondCurrentItem, true);
		return;
	}
}

void UWeaponController::ClearItem()
{
	CurrentItem->OnWeaponUnEquip();
	CurrentItem->SetPlayerCamera(nullptr);
	CurrentItem->ResetWeaponController();
	CurrentItem = nullptr;
}

void UWeaponController::InteractWithPickup_Implementation(AItem* Pickup)
{
	if (const auto Weapon = Cast<AWeapon>(Pickup))
	{
		if (CurrentItem)
		{
			DropAnItem(CurrentItem);
		}

		Items.Insert(Weapon, CurrentItemIndex);
		CurrentItem = Weapon;
		// SetArrayElem xent<AWeaponActor*>(GunWeapon, Weapons, CurrentWeaponIndex);
		Weapon->EnableSettingsForEquipping();
		Weapon->AttachToComponent(WeaponHolder, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
		// SetWeaponVisibility();
		if (FollowCamera)
		{
			Weapon->SetPlayerCamera(FollowCamera);
		}
		Weapon->GetRootComponent()->SetRelativeLocation(Weapon->GetWeaponDefaultRelativeLocation());

		EquipWeaponAnimation();
	}
}


void UWeaponController::EquipWeaponAnimation()
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

void UWeaponController::EquipSecondWeaponAnimation()
{
	if (!SecondWeaponHolder)
	{
		return;
	}
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

void UWeaponController::SetWeaponVisibility()
{
	for (const auto ElementWeapon : Items)
	{
		if (ElementWeapon)
		{
			if ((CurrentItem == ElementWeapon) || (SecondCurrentItem == ElementWeapon))
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

void UWeaponController::SetCurrentWeaponIndex(const int WeaponIndex)
{
	if (CurrentItemIndex == WeaponIndex)
	{
		return;
	}
	if (CurrentItem)
	{
		CurrentItem->OnWeaponUnEquip();
	}
	CurrentItemIndex = WeaponIndex;
	CurrentItem = Items[WeaponIndex];
	CurrentItem->OnWeaponEquip();

	EquipWeaponAnimation();
}

void UWeaponController::AttackPressed()
{
	PRINT_TO_SCREEN(FString::Printf(TEXT("Fire Function")), 0.0f, FColor::Emerald);

	if (!CurrentItem)
	{
		return;
	}

	// if (GetWorld()->GetTimerManager().IsTimerActive(WeaponEquipTimerHandle))
	// {
	// 	return;
	// }


	if (const auto Item = Cast<AWeapon>(CurrentItem))
	{
		Item->AttackButtonPressed();
	}
}

void UWeaponController::AttackReleased()
{
	if (!CurrentItem)
	{
		return;
	}

	if (const auto Item = Cast<AWeapon>(CurrentItem))
	{
		Item->AttackButtonReleased();
	}
}

void UWeaponController::SecondAttackPressed()
{
	if (!SecondCurrentItem)
	{
		return;
	}

	if (const auto Item = Cast<AWeapon>(SecondCurrentItem))
	{
		Item->AttackButtonPressed();
	}
}

void UWeaponController::SecondAttackReleased()
{
	if (!SecondCurrentItem)
	{
		return;
	}

	if (const auto Item = Cast<AWeapon>(SecondCurrentItem))
	{
		Item->AttackButtonReleased();
	}
}

void UWeaponController::ReloadWeapon() const
{
	if (CurrentItem)
	{
		if (const auto GunWeapon = Cast<ARangeWeapon>(CurrentItem))
		{
			GunWeapon->ReloadWeapon();
		}
	}

	if (SecondCurrentItem)
	{
		if (const auto GunWeapon = Cast<ARangeWeapon>(SecondCurrentItem))
		{
			GunWeapon->ReloadWeapon();
		}
	}
}

void UWeaponController::SwapItemSide()
{
	if (!SecondWeaponHolder)
	{
		return;
	}

	if (CurrentItem && !SecondCurrentItem)
	{
		if (!CurrentItem->GetCanBeEquippedInSecondHand())
		{
			return;
		}
		CurrentItem->OnWeaponUnEquip();
		SecondCurrentItem = CurrentItem;
		CurrentItem = nullptr;
		SecondCurrentItem->AttachToComponent(SecondWeaponHolder,
		                                     FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
		Items.RemoveAt(CurrentItemIndex);
		EquipSecondWeaponAnimation();
		SecondCurrentItem->OnWeaponEquip();
		return;
	}

	if (SecondCurrentItem && !CurrentItem)
	{
		if (!SecondCurrentItem->GetCanBeEquippedInSecondHand())
		{
			return;
		}
		SecondCurrentItem->OnWeaponUnEquip();
		CurrentItem = SecondCurrentItem;
		SecondCurrentItem = nullptr;
		CurrentItem->AttachToComponent(WeaponHolder, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
		Items.Insert(CurrentItem, CurrentItemIndex);
		SetWeaponVisibility();
		EquipWeaponAnimation();
		CurrentItem->OnWeaponEquip();
		return;
	}

	if (SecondCurrentItem && CurrentItem)
	{
		if (!CurrentItem->GetCanBeEquippedInSecondHand() || !SecondCurrentItem->GetCanBeEquippedInSecondHand())
		{
			return;
		}
		CurrentItem->OnWeaponUnEquip();
		SecondCurrentItem->OnWeaponUnEquip();
		const auto OldSecondWeapon = SecondCurrentItem;
		const auto OldWeapon = CurrentItem;
		CurrentItem = OldSecondWeapon;
		SecondCurrentItem = OldWeapon;
		SecondCurrentItem->AttachToComponent(SecondWeaponHolder,
		                                     FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
		CurrentItem->AttachToComponent(WeaponHolder, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));

		Items.RemoveAt(CurrentItemIndex);
		Items.Insert(OldSecondWeapon, CurrentItemIndex);

		EquipSecondWeaponAnimation();
		EquipWeaponAnimation();
		CurrentItem->OnWeaponEquip();
		SecondCurrentItem->OnWeaponEquip();
	}
}
