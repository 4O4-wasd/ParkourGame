// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponActor.h"

#include "MyUtils.h"
#include "Engine/World.h"
#include "ParkourCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "WeaponControllerComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#define PRINT_TO_SCREEN(Message, Time, Color) \
if (GEngine) \
{ \
GEngine->AddOnScreenDebugMessage(-1, Time, Color, Message); \
}

AWeaponActor::AWeaponActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = DefaultMagazineSize;
}

void AWeaponActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (WeaponController)
	{
		if ((WeaponController->GetCurrentWeapon() == this) || (WeaponController->GetSecondCurrentWeapon() == this))
		{
			const auto ReverseTargetRotation =
				FRotator(-TargetCameraRotation.Pitch, -TargetCameraRotation.Yaw, -TargetCameraRotation.Roll);
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
			                                          UMyUtils::VectorToRotator(
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

void AWeaponActor::FireWeapon_Implementation(const bool IsPressed)
{
	if (WeaponController)
	{
		switch (FireModeType)
		{
		case Single:
			if (!bIsReloading && IsPressed && bCanShoot)
			{
				WhenWeaponFire();
				bCanShoot = false;
				FTimerHandle UnusedTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(
					UnusedTimerHandle, [&] { bCanShoot = true; }, WeaponFireDelay, false);
			}
			break;
		case Automatic:
			if (bCanShoot && !bIsReloading && IsPressed)
			{
				WhenWeaponFire();
				bCanShoot = false;
				if (!FireTimerHandle.IsValid())
				{
					GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, [&] { WhenWeaponFire(); }, WeaponFireDelay,
					                                       true);
				}
				FTimerHandle UnusedTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(
					UnusedTimerHandle, [&] { bCanShoot = true; }, WeaponFireDelay, false);
			}
			else
			{
				ResetFire();
			}
		}
	}
}

void AWeaponActor::Recoil()
{
	auto NewRotation = FRotator(FMath::RandRange(WeaponCameraRecoil.Y / 3, WeaponCameraRecoil.Y),
	                            FMath::RandRange(-WeaponCameraRecoil.X, WeaponCameraRecoil.X),
	                            FMath::RandRange(-WeaponCameraRecoil.Z, WeaponCameraRecoil.Z));
	if (WeaponController)
	{
		// OwnerCharacter->WeaponCameraRecoil(NewRotation);
		TargetCameraRotation += NewRotation;
	}

	auto NewRelativeLocation = FVector(FMath::RandRange(-WeaponPushbackRecoil.X, WeaponPushbackRecoil.X),
	                                   FMath::RandRange(WeaponPushbackRecoil.Y / 3, WeaponPushbackRecoil.Y),
	                                   FMath::RandRange(-WeaponPushbackRecoil.Z, WeaponPushbackRecoil.Z));

	if (WeaponController)
	{
		TargetRelativeLocation += NewRelativeLocation;
	}

	auto NewRelativeRotation = FRotator(FMath::RandRange(-WeaponRotationRecoil.Y, WeaponRotationRecoil.Y),
	                                    FMath::RandRange(-WeaponRotationRecoil.X, WeaponRotationRecoil.X),
	                                    FMath::RandRange(WeaponRotationRecoil.Z / 3, WeaponRotationRecoil.Z));
	if (WeaponController)
	{
		// OwnerCharacter->WeaponCameraRecoil(NewRotation);
		TargetRelativeRotation += NewRelativeRotation;
	}
}

void AWeaponActor::WhenWeaponFire_Implementation()
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

	const FName TraceTag("MyTraceTag");

	GetWorld()->DebugDrawTraceTag = TraceTag;

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
			                                               WeaponMesh->GetSocketLocation(*FireMuzzleSocketName),
			                                               (End - WeaponMesh->GetSocketLocation(
				                                               *FireMuzzleSocketName)).Rotation());
			// GetWorld()->SpawnActor<AActor>(BulletTraceSystem, WeaponMesh->GetSocketLocation(*FireMuzzleSocketName),
			//                                (FireHitResult.ImpactPoint - WeaponMesh->GetSocketLocation(
			// 	                               *FireMuzzleSocketName)).Rotation(), FActorSpawnParameters());
			// UNiagaraFunctionLibrary::SpawnSystemAttached(BulletTraceSystem, WeaponMesh, NAME_None,
			//                                              WeaponMesh->GetSocketTransform(*FireMuzzleSocketName,
			// 	                                                         RTS_Actor).
			//                                                          GetLocation(), FRotator::ZeroRotator,
			//                                              EAttachLocation::KeepRelativeOffset, true);
		}
		if (ImpactPointSystem && bDidHit)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactPointSystem, FireHitResult.ImpactPoint,
			                                               WeaponMesh->GetComponentRotation() + FRotator(-90, 0, 0));
		}
	}

	if (FireWeaponAnimation)
	{
		WeaponMesh->PlayAnimation(FireWeaponAnimation, false);
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, WeaponMesh->GetSocketLocation(*FireMuzzleSocketName));
	}

	if (ShellEjectSound)
	{
		FHitResult WeaponBottomHitResult;
		const bool bDidHit = GetWorld()->LineTraceSingleByChannel(WeaponBottomHitResult,
		                                                          WeaponMesh->GetComponentLocation(),
		                                                          (WeaponMesh->GetUpVector() * -1 * 1000) + WeaponMesh->
		                                                          GetComponentLocation(), ECC_Visibility,
		                                                          CollisionParams);
		// DrawDebugLine(GetWorld(), WeaponMesh->GetComponentLocation(),
		//               (WeaponMesh->GetUpVector() * -1 * 1000) + WeaponMesh->GetComponentLocation(), FColor::Magenta,
		//               false, 5);

		if (bDidHit)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ShellEjectSound, WeaponBottomHitResult.ImpactPoint);
		}
	}

	if (MuzzleFlashSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlashSystem, WeaponMesh, NAME_None,
		                                             WeaponMesh->GetSocketTransform(*FireMuzzleSocketName, RTS_Actor).
		                                                         GetLocation(), FRotator::ZeroRotator,
		                                             EAttachLocation::KeepRelativeOffset, true);
		// UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, NAME_None, WeaponMesh->GetSocketTransform(*FireMuzzleSocketName, RTS_World).GetLocation());
	}

	if (ShellEjectSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(ShellEjectSystem, WeaponMesh, NAME_None,
		                                             WeaponMesh->GetSocketTransform(*ShellEjectSocketName, RTS_Actor).
		                                                         GetLocation(), FRotator::ZeroRotator,
		                                             EAttachLocation::KeepRelativeOffset, true);
	}
	Recoil();
	--CurrentAmmo;
}

void AWeaponActor::StopFire()
{
	if (FireModeType == Automatic)
	{
		ResetFire();
	}
}

void AWeaponActor::ResetFire()
{
	if (FireTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	}
}

void AWeaponActor::OnWeaponUnequip_Implementation()
{
	ResetFire();
	// do stuff
}

void AWeaponActor::ReloadWeapon_Implementation()
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

	WeaponMesh->PlayAnimation(ReloadAnimation, false);
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
	{
		bIsReloading = false;
		CurrentAmmo = DefaultMagazineSize;
	}, ReloadAnimation->GetPlayLength() - 1, false);
}
