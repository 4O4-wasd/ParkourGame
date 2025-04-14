// Fill out your copyright notice in the Description page of Project Settings.

#include "RangeWeapon.h"

#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "WeaponController.h"
#include "GameFramework/Character.h"

#define PRINT_TO_SCREEN(Message, Time, Color) \
if (GEngine) \
{ \
GEngine->AddOnScreenDebugMessage(-1, Time, Color, Message); \
}

ARangeWeapon::ARangeWeapon():
	Damage(10),
	Range(5000),
	MaxRandomSpreadAngle(2),
	FireModeType(Single),
	WeaponFireDelay(.5f),
	DefaultMagazineSize(10),
	WeaponPushbackRecoil(2, -10, 2),
	WeaponRotationRecoil(2, 2, -10),
	WeaponCameraRecoil(.7, 4, 4),
	RecoilSnappiness(6),
	RecoilReturnSpeed(2),
	FireSound(nullptr),
	ShellEjectSound(nullptr),
	ReloadAnimation(nullptr),
	FireWeaponAnimation(nullptr),
	MuzzleFlashSystem(nullptr),
	ShellEjectSystem(nullptr),
	ImpactPointSystem(nullptr),
	BulletTraceSystem(nullptr),
	CurrentCameraRotation(),
	TargetCameraRotation(),
	CurrentRelativeLocation(),
	TargetRelativeLocation(),
	CurrentRelativeRotation(),
	TargetRelativeRotation(),
	bIsReloading(false),
	CurrentAmmo(0)
{
	PrimaryActorTick.bCanEverTick = true;

	FireMuzzleSocketName = FString("Muzzle");
	ShellEjectSocketName = FString("ShellEject");
}

void ARangeWeapon::AttackButtonPressed_Implementation()
{
	FireWeapon(true);
}

void ARangeWeapon::AttackButtonReleased_Implementation()
{
	FireWeapon(false);
}

void ARangeWeapon::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = DefaultMagazineSize;
}

void ARangeWeapon::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (WeaponController)
	{
		if ((WeaponController->GetCurrentWeapon() == this) || (WeaponController->GetSecondCurrentWeapon() == this))
		{
			TargetCameraRotation = FMath::RInterpTo(TargetCameraRotation, FRotator::ZeroRotator,
			                                        GetWorld()->GetDeltaSeconds(), RecoilReturnSpeed);
			CurrentCameraRotation = FMath::RInterpTo(PlayerCamera->GetRelativeRotation(), TargetCameraRotation,
			                                         GetWorld()->GetDeltaSeconds(), RecoilSnappiness);

			if (PlayerCamera)
			{
				PlayerCamera->SetRelativeRotation(CurrentCameraRotation);
				UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->ViewPitchMax =
					90 - CurrentCameraRotation.Pitch;
			}

			TargetRelativeLocation = FMath::VInterpTo(TargetRelativeLocation,
			                                          (WeaponController->GetSecondCurrentWeapon() == this)
				                                          ? WeaponDefaultRelativeLocation * FVector(-1, 1, 1)
				                                          : WeaponDefaultRelativeLocation,
			                                          GetWorld()->GetDeltaSeconds(), RecoilSnappiness);
			CurrentRelativeLocation = FMath::VInterpTo(CurrentRelativeLocation, TargetRelativeLocation,
			                                           GetWorld()->GetDeltaSeconds(), RecoilSnappiness);
			SetActorRelativeLocation(CurrentRelativeLocation);

			TargetRelativeRotation = FMath::RInterpTo(TargetRelativeRotation,
			                                          VectorToRotator(
				                                          (WeaponController->GetSecondCurrentWeapon() == this)
					                                          ? WeaponDefaultRelativeRotation * FVector(-1, 1, 1)
					                                          : WeaponDefaultRelativeRotation),
			                                          GetWorld()->GetDeltaSeconds(), RecoilSnappiness);
			CurrentRelativeRotation = FMath::RInterpTo(CurrentRelativeRotation, TargetRelativeRotation,
			                                           GetWorld()->GetDeltaSeconds(), RecoilSnappiness);
			SetActorRelativeRotation(CurrentRelativeRotation);
		}
	}
}

void ARangeWeapon::FireWeapon(const bool IsPressed)
{
	if (WeaponController)
	{
		switch (FireModeType)
		{
		case Single:
			if (!bIsReloading && IsPressed && bCanShoot)
			{
				OnWeaponFire();
				bCanShoot = false;
				FTimerHandle UnusedTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(
					UnusedTimerHandle, [&]
					{
						bCanShoot = true;
					}, WeaponFireDelay, false);
			}
			break;
		case Automatic:
			if (bCanShoot && !bIsReloading && IsPressed)
			{
				OnWeaponFire();
				bCanShoot = false;
				if (!FireTimerHandle.IsValid())
				{
					GetWorld()->GetTimerManager().SetTimer(
						FireTimerHandle, [&]
						{
							OnWeaponFire();
						},
						WeaponFireDelay, true);
				}
				FTimerHandle UnusedTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(
					UnusedTimerHandle, [&]
					{
						bCanShoot = true;
					}, WeaponFireDelay, false);
			}
			else
			{
				ResetFire();
			}
		default:
			break;
		}
	}
}

void ARangeWeapon::Recoil()
{
	const auto NewRotation = FRotator(FMath::RandRange(WeaponCameraRecoil.Y / 3, WeaponCameraRecoil.Y),
	                                  FMath::RandRange(-WeaponCameraRecoil.X, WeaponCameraRecoil.X),
	                                  FMath::RandRange(-WeaponCameraRecoil.Z, WeaponCameraRecoil.Z));
	if (WeaponController)
	{
		// OwnerCharacter->WeaponCameraRecoil(NewRotation);
		TargetCameraRotation += NewRotation;
	}

	const auto NewRelativeLocation = FVector(FMath::RandRange(-WeaponPushbackRecoil.X, WeaponPushbackRecoil.X),
	                                         FMath::RandRange(WeaponPushbackRecoil.Y / 3, WeaponPushbackRecoil.Y),
	                                         FMath::RandRange(-WeaponPushbackRecoil.Z, WeaponPushbackRecoil.Z));

	if (WeaponController)
	{
		TargetRelativeLocation += NewRelativeLocation;
	}

	const auto NewRelativeRotation = FRotator(FMath::RandRange(-WeaponRotationRecoil.Y, WeaponRotationRecoil.Y),
	                                          FMath::RandRange(-WeaponRotationRecoil.X, WeaponRotationRecoil.X),
	                                          FMath::RandRange(WeaponRotationRecoil.Z / 3, WeaponRotationRecoil.Z));
	if (WeaponController)
	{
		// OwnerCharacter->WeaponCameraRecoil(NewRotation);
		TargetRelativeRotation += NewRelativeRotation;
	}
}

void ARangeWeapon::OnWeaponFire_Implementation()
{
	if (bIsReloading)
	{
		return;
	}

	if (!bCanShoot)
	{
		return;
	}

	if (CurrentAmmo == 0)
	{
		ReloadWeapon();
		return;
	}

	if (!WeaponController)
	{
		return;
	}

	if (!PlayerCamera)
	{
		return;
	}

	// FHitResult FireHitResult;
	const FVector MuzzleSocketLocation = PlayerCamera->GetComponentLocation();

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(WeaponController->CharacterOwner);

	for (int i = 0; i < BulletsPerShot; ++i)
	{
		const FVector End = MuzzleSocketLocation + (UKismetMathLibrary::RandomUnitVectorInConeInDegrees(
				PlayerCamera->GetForwardVector(), MaxRandomSpreadAngle) *
			Range);
		FHitResult FireHitResult;
		const bool bDidHit =
			GetWorld()->LineTraceSingleByChannel(FireHitResult, MuzzleSocketLocation, End, ECC_Pawn, CollisionParams);
		if (BulletTraceSystem)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BulletTraceSystem,
			                                               Mesh->GetSocketLocation(*FireMuzzleSocketName),
			                                               (End - Mesh->GetSocketLocation(
				                                               *FireMuzzleSocketName)).Rotation());
			// GetWorld()->SpawnActor<AActor>(BulletTraceSystem, Mesh->GetSocketLocation(*FireMuzzleSocketName),
			//                                (FireHitResult.ImpactPoint - Mesh->GetSocketLocation(
			// 	                               *FireMuzzleSocketName)).Rotation(), FActorSpawnParameters());
			// UNiagaraFunctionLibrary::SpawnSystemAttached(BulletTraceSystem, Mesh, NAME_None,
			//                                              Mesh->GetSocketTransform(*FireMuzzleSocketName,
			// 	                                                         RTS_Actor).
			//                                                          GetLocation(), FRotator::ZeroRotator,
			//                                              EAttachLocation::KeepRelativeOffset, true);
		}
		if (ImpactPointSystem && bDidHit)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactPointSystem, FireHitResult.ImpactPoint,
			                                               Mesh->GetComponentRotation() + FRotator(-90, 0, 0));
		}
	}

	if (FireWeaponAnimation)
	{
		Mesh->PlayAnimation(FireWeaponAnimation, false);
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Mesh->GetSocketLocation(*FireMuzzleSocketName));
	}

	if (ShellEjectSound)
	{
		FHitResult WeaponBottomHitResult;
		const bool bDidHit = GetWorld()->LineTraceSingleByChannel(
			WeaponBottomHitResult,
			Mesh->GetComponentLocation(),
			(Mesh->GetUpVector() * -1 * 1000) + Mesh->
			GetComponentLocation(), ECC_Visibility,
			CollisionParams
		);
		// DrawDebugLine(GetWorld(), Mesh->GetComponentLocation(),
		//               (Mesh->GetUpVector() * -1 * 1000) + Mesh->GetComponentLocation(), FColor::Magenta,
		//               false, 5);

		if (bDidHit)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ShellEjectSound, WeaponBottomHitResult.ImpactPoint);
		}
	}

	if (MuzzleFlashSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			MuzzleFlashSystem,
			Mesh,
			NAME_None,
			Mesh->GetSocketTransform(*FireMuzzleSocketName, RTS_Actor).GetLocation(),
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
		// UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, Mesh, NAME_None, Mesh->GetSocketTransform(*FireMuzzleSocketName, RTS_World).GetLocation());
	}

	if (ShellEjectSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			ShellEjectSystem,
			Mesh,
			NAME_None,
			Mesh->GetSocketTransform(*ShellEjectSocketName, RTS_Actor).GetLocation(), FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}
	Recoil();
	--CurrentAmmo;
}

void ARangeWeapon::ResetFire()
{
	if (FireTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	}
}

void ARangeWeapon::OnWeaponUnEquip_Implementation()
{
	ResetFire();
}

void ARangeWeapon::ReloadWeapon_Implementation()
{
	if (bIsReloading)
	{
		return;
	}

	if (CurrentAmmo == DefaultMagazineSize)
	{
		return;
	}

	if (!WeaponController)
	{
		return;
	}

	bIsReloading = true;

	Mesh->PlayAnimation(ReloadAnimation, false);
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
	{
		bIsReloading = false;
		CurrentAmmo = DefaultMagazineSize;
	}, ReloadAnimation->GetPlayLength(), false);
}
