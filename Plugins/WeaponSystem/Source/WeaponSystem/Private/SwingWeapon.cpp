

#include "SwingWeapon.h"

#include "MovieSceneTracksComponentTypes.h"
#include "WeaponController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#define PRINT_TO_SCREEN(Message, Time, Color) \
if (GEngine) \
{ \
GEngine->AddOnScreenDebugMessage(-1, Time, Color, Message); \
}

ASwingWeapon::ASwingWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	WebMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WebMesh"));
	WebMesh->SetupAttachment(Mesh, "Muzzle");
}

void ASwingWeapon::BeginPlay()
{
	Super::BeginPlay();
	DebugLineEnd = Mesh->GetSocketLocation(FName(*FireMuzzleSocketName));
}

void ASwingWeapon::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (WeaponController && WeaponController->CharacterOwner)
	{
		if (bIsSwinging)
		{
			ApplySwingPhysics(DeltaSeconds);

			DebugLineEnd = UKismetMathLibrary::VInterpTo(DebugLineEnd,
			                                             Mesh->GetSocketLocation(FName(*FireMuzzleSocketName)) -
			                                             WebAttachPoint, DeltaSeconds, 6);
		}
		else
		{
			DebugLineEnd = UKismetMathLibrary::VInterpTo(DebugLineEnd,
			                                             FVector::ZeroVector, DeltaSeconds, 40);
		}
		WebMesh->SetRelativeScale3D(
			FVector(
				(DebugLineEnd / 100).Size(),
				WebMesh->GetRelativeScale3D().Y,
				WebMesh->GetRelativeScale3D().Z
			)
		);
		PreviousLocation = WeaponController->CharacterOwner->GetActorLocation();
		RotateTowardsAttachPoint();
	}

}

void ASwingWeapon::AttackButtonPressed_Implementation()
{
	ShootWeb();
}

void ASwingWeapon::AttackButtonReleased_Implementation()
{
	ReleaseWeb();
}

void ASwingWeapon::OnWeaponUnEquip_Implementation()
{
	ReleaseWeb();
}

void ASwingWeapon::OnWeaponEquip_Implementation()
{
	Super::OnWeaponEquip_Implementation();

	if (WeaponController == nullptr || WeaponController->CharacterOwner == nullptr)
	{
		return;
	}
}

bool ASwingWeapon::ShootWeb()
{
	if (bIsSwinging)
	{
		return false;
	}
	if (WeaponController->CharacterOwner == nullptr)
	{
		return false;
	}

	if (FVector HitLocation; GetWebTargetPoint(HitLocation))
	{
		WebAttachPoint = HitLocation;
		CurrentWebLength = FVector::Distance(WeaponController->CharacterOwner->GetActorLocation(), WebAttachPoint);

		CurrentWebLength = FMath::Clamp(CurrentWebLength, MinWebLength, MaxWebLength);

		bIsSwinging = true;

		UpdateSwingPlane();

		WeaponController->CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		WeaponController->CharacterOwner->GetCharacterMovement()->GravityScale = SwingGravityScale;

		return true;
	}

	return false;
}

void ASwingWeapon::ReleaseWeb()
{
	if (WeaponController->CharacterOwner == nullptr)
	{
		return;
	}

	if (bIsSwinging)
	{
		const FVector CurrentVelocity = WeaponController->CharacterOwner->GetCharacterMovement()->Velocity;
		const FVector NormalizedVelocity = CurrentVelocity.GetSafeNormal();

		WeaponController->CharacterOwner->GetCharacterMovement()->
		                  AddImpulse(NormalizedVelocity * WebReleaseBoost, true);

		bIsSwinging = false;
		SwingInputDirection = FVector::ZeroVector;

		WeaponController->CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		WeaponController->CharacterOwner->GetCharacterMovement()->GravityScale = 2.0f;
		WebAttachPoint = FVector::Zero();
	}
}

bool ASwingWeapon::GetWebTargetPoint(FVector& OutHitLocation) const
{
	if (!WeaponController->CharacterOwner)
	{
		return false;
	}

	const FVector CameraLocation = WeaponController->GetFollowCamera()->GetComponentLocation();
	const FRotator CameraRotation = WeaponController->GetFollowCamera()->GetComponentRotation();

	const FVector LineEnd = CameraLocation + (CameraRotation.Vector() * WebShootRange);;

	const FVector BoxExtent(100.0f, 100.0f, 100.0f);
	FHitResult HitResult;
	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		CameraLocation,
		LineEnd,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeBox(BoxExtent)
	);

	if (!(HitResult.GetActor() && HitResult.GetActor()->ActorHasTag("Swingable")))
	{
		return false;
	}

	if (bHit)
	{
		OutHitLocation = HitResult.ImpactPoint;
		return true;
	}

	return false;
}

void ASwingWeapon::UpdateSwingPlane()
{
	if (!WeaponController->CharacterOwner)
	{
		return;
	}

	const FVector ToAttachPoint = WebAttachPoint - WeaponController->CharacterOwner->GetActorLocation();
	FVector MovementDirection = WeaponController->CharacterOwner->GetCharacterMovement()->Velocity.GetSafeNormal();

	if (MovementDirection.IsNearlyZero())
	{
		if (WeaponController->CharacterOwner->Controller)
		{
			const FRotator CameraRotation = WeaponController->CharacterOwner->Controller->GetControlRotation();
			MovementDirection = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::X);
		}
	}

	SwingPlaneNormal = FVector::CrossProduct(ToAttachPoint, MovementDirection).GetSafeNormal();

	if (SwingPlaneNormal.IsNearlyZero())
	{
		FVector ForwardVector = WeaponController->CharacterOwner->GetActorForwardVector();
		SwingPlaneNormal = FVector::CrossProduct(ToAttachPoint, ForwardVector).GetSafeNormal();
	}
}

void ASwingWeapon::RotateTowardsAttachPoint() const
{
	if (!WeaponController->CharacterOwner)
	{
		return;
	}

	if (!bIsSwinging)
	{
		const auto b = UKismetMathLibrary::RInterpTo(Mesh->GetRelativeRotation(),
		                                             WeaponDefaultRelativeRotation.GetSafeNormal().
		                                             ToOrientationRotator(), GetWorld()->GetDeltaSeconds(),
		                                             WeaponRotationTime);
		Mesh->SetRelativeRotation(b);
		return;
	}
	const auto a = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), WebAttachPoint);
	const auto rot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), WebAttachPoint);
	Mesh->SetWorldRotation(FRotator(
		a.Roll, (a.Yaw - 90),
		-a.Pitch));
}

void ASwingWeapon::ApplySwingPhysics(const float DeltaTime)
{
	if (!WeaponController->CharacterOwner)
	{
		return;
	}
	if (SwingPlaneNormal.IsNearlyZero() || FVector::DotProduct(SwingInputDirection, SwingInputDirection) > 0.1f)
	{
		UpdateSwingPlane();
	}

	const FVector ToAttachPoint = WebAttachPoint - WeaponController->CharacterOwner->GetActorLocation();
	const float DistanceToAttachPoint = ToAttachPoint.Size();
	const FVector NormalizedToAttach = ToAttachPoint / DistanceToAttachPoint;

	const FVector CurrentVelocity = WeaponController->CharacterOwner->GetCharacterMovement()->Velocity;
	const float CurrentSpeed = CurrentVelocity.Size();

	FVector TangentDirection = FVector::CrossProduct(NormalizedToAttach, SwingPlaneNormal).GetSafeNormal();

	const float SwingDirectionSign = FVector::DotProduct(TangentDirection, CurrentVelocity) >= 0 ? 1.0f : -1.0f;
	TangentDirection *= SwingDirectionSign;

	const FVector VelocityOnPlane = CurrentVelocity - FVector::DotProduct(CurrentVelocity, SwingPlaneNormal) *
		SwingPlaneNormal;

	FVector ResultantForce = FVector::ZeroVector;

	if (DistanceToAttachPoint > CurrentWebLength)
	{
		const FVector TensionForce = NormalizedToAttach * (DistanceToAttachPoint - CurrentWebLength) * 200000.f;
		ResultantForce += TensionForce;

		const FVector NewScale = FVector(.05f, DistanceToAttachPoint * .009, .05f);
		const FVector VelocityAlongWeb = FVector::DotProduct(CurrentVelocity, NormalizedToAttach) * NormalizedToAttach;
		if (DistanceToAttachPoint > CurrentWebLength && (!((WeaponController->CharacterOwner->GetActorLocation() -
				WebAttachPoint).Z > 200))
			&& FVector::DotProduct(VelocityAlongWeb, NormalizedToAttach) > 0)
		{
			WeaponController->CharacterOwner->GetCharacterMovement()->Velocity -= VelocityAlongWeb;
		}
	}

	const float CentripetalForceMagnitude = (CurrentSpeed * CurrentSpeed) / FMath::Max(DistanceToAttachPoint, 100.0f);
	const FVector CentripetalForce = NormalizedToAttach * CentripetalForceMagnitude * CentripetalForceFactor;
	ResultantForce += CentripetalForce;

	const FVector MomentumForce = TangentDirection * SwingMomentumForce;
	ResultantForce += MomentumForce;

	if (!SwingInputDirection.IsZero())
	{
		FVector InputOnPlane = SwingInputDirection - FVector::DotProduct(SwingInputDirection, SwingPlaneNormal) *
			SwingPlaneNormal;
		InputOnPlane = InputOnPlane.GetSafeNormal();

		ResultantForce += InputOnPlane * SwingMomentumForce * 0.5f;
	}

	WeaponController->CharacterOwner->GetCharacterMovement()->AddForce(ResultantForce * DeltaTime);

	if (CurrentSpeed > 100.0f)
	{
		const FRotator TargetRotation = VelocityOnPlane.Rotation();
		const FRotator CurrentRotation = WeaponController->CharacterOwner->GetActorRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f);
		NewRotation.Pitch = 0.0f;
		NewRotation.Roll = 0.0f;
		WeaponController->CharacterOwner->SetActorRotation(NewRotation);
	}
}
