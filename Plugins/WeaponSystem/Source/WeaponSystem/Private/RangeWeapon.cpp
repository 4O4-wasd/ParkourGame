
#include "RangeWeapon.h"

#include "HealthComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "WeaponController.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveVector.h"
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
	bIsReloading(false),
	CurrentAmmo(0),
	CurrentCameraRotation(),
	TargetCameraRotation(),
	CurrentRelativeLocation(),
	TargetRelativeLocation(),
	CurrentRelativeRotation(),
	TargetRelativeRotation()
{
	PrimaryActorTick.bCanEverTick = true;

	FireMuzzleSocketName = FString("Muzzle");
	ShellEjectSocketName = FString("ShellEject");

	PushbackRecoilTimelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("RecoilTimelineComponent"));
	RotationRecoilTimelineComponent = CreateDefaultSubobject<UTimelineComponent>(
		TEXT("RotationRecoilTimelineComponent"));

	BaseRelativeLocation = WeaponDefaultRelativeLocation;
	BaseRelativeRotation = VectorToRotator(WeaponDefaultRelativeRotation);
}

void ARangeWeapon::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = DefaultMagazineSize;

	PushbackRecoilTimelineComponent->SetLooping(false);
	FOnTimelineVector PushbackRecoilTimelineVector;
	PushbackRecoilTimelineVector.BindUFunction(this, "PushbackRecoil");
	PushbackRecoilTimelineComponent->AddInterpVector(PushbackRecoilCurve, PushbackRecoilTimelineVector);
	PushbackRecoilTimelineComponent->SetTimelineLengthMode(TL_LastKeyFrame);

	RotationRecoilTimelineComponent->SetLooping(false);
	FOnTimelineVector RotationRecoilTimelineVector;
	RotationRecoilTimelineVector.BindUFunction(this, "RotationRecoil");
	RotationRecoilTimelineComponent->AddInterpVector(RotationRecoilCurve, RotationRecoilTimelineVector);
	RotationRecoilTimelineComponent->SetTimelineLengthMode(TL_LastKeyFrame);

	FOnTimelineEvent RecoilFinishedEvent;
	RecoilFinishedEvent.BindUFunction(this, "RecoilTimelineFinished");
	PushbackRecoilTimelineComponent->SetTimelineFinishedFunc(RecoilFinishedEvent);
	RotationRecoilTimelineComponent->SetTimelineFinishedFunc(RecoilFinishedEvent);
}


void ARangeWeapon::AttackButtonPressed_Implementation()
{
	StartFiring(true);
}

void ARangeWeapon::AttackButtonReleased_Implementation()
{
	StartFiring(false);
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

			TargetRelativeLocation = FMath::VInterpTo(
				TargetRelativeLocation,
				FVector::ZeroVector,
				GetWorld()->GetDeltaSeconds(),
				RecoilReturnSpeed
			);

			CurrentRelativeLocation = FMath::VInterpTo(
				CurrentRelativeLocation,
				TargetRelativeLocation,
				GetWorld()->GetDeltaSeconds(),
				RecoilSnappiness
			);

			TargetRelativeRotation = FMath::RInterpTo(
				TargetRelativeRotation,
				FRotator::ZeroRotator,
				GetWorld()->GetDeltaSeconds(),
				RecoilReturnSpeed
			);

			CurrentRelativeRotation = FMath::RInterpTo(
				CurrentRelativeRotation,
				TargetRelativeRotation,
				GetWorld()->GetDeltaSeconds(),
				RecoilSnappiness
			);
		}
	}
}

void ARangeWeapon::PushbackRecoil(const FVector& OutVector)
{
	if (WeaponController)
	{
		if ((WeaponController->GetCurrentWeapon() == this) || (WeaponController->GetSecondCurrentWeapon() == this))
		{
			WeaponController->GetWeaponHolder()->SetRelativeLocation(
				OutVector + CurrentRelativeLocation,
				false,
				nullptr,
				ETeleportType::TeleportPhysics
			);
			PRINT_TO_SCREEN(FString::Printf(TEXT("Hlo")), 0, FColor::Emerald);
		}
	}
}

void ARangeWeapon::RotationRecoil(const FVector& OutVector)
{
	if (WeaponController)
	{
		if ((WeaponController->GetCurrentWeapon() == this) || (WeaponController->GetSecondCurrentWeapon() == this))
		{
			const auto NewOut = FVector(
				FMath::RandRange(-OutVector.X, OutVector.X),
				FMath::RandRange(OutVector.Y / 3, OutVector.Y),
				FMath::RandRange(-OutVector.Z, OutVector.Z)
			);
			WeaponController->GetWeaponHolder()->SetRelativeRotation(
				FRotator(
					(OutVector).Y,
					(OutVector).Z,
					(OutVector).X
				) + CurrentRelativeRotation,
				false,
				nullptr,
				ETeleportType::TeleportPhysics
			);
		}
	}
}

void ARangeWeapon::RecoilTimelineFinished()
{
	PRINT_TO_SCREEN(FString::Printf(TEXT("Recoil Timeline finished")), 0, FColor::Orange);
}

void ARangeWeapon::StartFiring(const bool IsPressed)
{
	if (WeaponController)
	{
		switch (FireModeType)
		{
		case Single:
			if (!bIsReloading && IsPressed && bCanShoot)
			{
				Fire();
				bCanShoot = false;
				ResetFire();
			}
			break;
		case Automatic:
			if (bCanShoot && !bIsReloading && IsPressed)
			{
				bIsShooting = true;
				Fire();
				bCanShoot = false;
				if (!FireTimerHandle.IsValid())
				{
					GetWorld()->GetTimerManager().SetTimer(
						FireTimerHandle, [&]
						{
							Fire();
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
			break;
		default:
			break;
		}
	}
}

void ARangeWeapon::Recoil()
{
	TargetRelativeLocation = WeaponController->GetWeaponHolder()->GetRelativeLocation();
	TargetRelativeRotation = WeaponController->GetWeaponHolder()->GetRelativeRotation();
	PushbackRecoilTimelineComponent->PlayFromStart();
	RotationRecoilTimelineComponent->PlayFromStart();
	if (WeaponController && WeaponController->GetFollowCamera())
	{
		const auto NewRotation = FRotator(FMath::RandRange(WeaponCameraRecoil.Y / 3, WeaponCameraRecoil.Y),
		                                  FMath::RandRange(-WeaponCameraRecoil.X, WeaponCameraRecoil.X),
		                                  FMath::RandRange(-WeaponCameraRecoil.Z, WeaponCameraRecoil.Z));
		TargetCameraRotation += NewRotation;
	}
}

void ARangeWeapon::Fire()
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
		if (FireHitResult.GetActor())
		{
			if (const auto HealthComponent = FireHitResult.GetActor()->FindComponentByClass<UHealthComponent>())
			{
				HealthComponent->Damage(Damage);
			}
		}
		if (BulletTraceSystem)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BulletTraceSystem,
			                                               Mesh->GetSocketLocation(*FireMuzzleSocketName),
			                                               (End - Mesh->GetSocketLocation(
				                                               *FireMuzzleSocketName)).Rotation());
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
	switch (FireModeType)
	{
	case Single:
		{
			FTimerHandle UnusedTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				UnusedTimerHandle, [&]
				{
					bCanShoot = true;
				}, WeaponFireDelay, false);
			break;
		}
	case Automatic:
		if (FireTimerHandle.IsValid())
		{
			bIsShooting = false;
			GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
		}
		break;
	default:
		break;
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

	if (!ReloadAnimation)
	{
		bIsReloading = false;
		CurrentAmmo = DefaultMagazineSize;
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
