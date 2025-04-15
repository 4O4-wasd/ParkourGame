// Fill out your copyright notice in the Description page of Project Settings.


#include "SwingWeapon.h"

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
			                                             WebAttachPoint, DeltaSeconds, 6);

			DrawDebugLine(
				GetWorld(),
				Mesh->GetSocketLocation(FName(*FireMuzzleSocketName)),
				DebugLineEnd,
				FColor::Black,
				false,
				.0f,
				0,
				3.0f
			);
		}
		else
		{
			DebugLineEnd = Mesh->GetSocketLocation(FName(*FireMuzzleSocketName));
		}
		PreviousLocation = WeaponController->CharacterOwner->GetActorLocation();
		RotateTowardsAttachPoint();
	}

	// Store current location for next frame
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

	FVector HitLocation;
	if (GetWebTargetPoint(HitLocation))
	{
		WebAttachPoint = HitLocation;
		CurrentWebLength = FVector::Distance(WeaponController->CharacterOwner->GetActorLocation(), WebAttachPoint);

		// Cap the web length
		CurrentWebLength = FMath::Clamp(CurrentWebLength, MinWebLength, MaxWebLength);

		// Enable swinging mode and adjust character movement
		bIsSwinging = true;

		// Calculate initial swing plane
		UpdateSwingPlane();

		// Configure character movement for swinging
		WeaponController->CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		WeaponController->CharacterOwner->GetCharacterMovement()->GravityScale = SwingGravityScale;

		// UGameplayStatics::PlaySoundAtLocation(this, WebAttachSound, Mesh->GetSocketLocation(*FireMuzzleSocketName));
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
		// Add a boost in the direction of travel when releasing web
		const FVector CurrentVelocity = WeaponController->CharacterOwner->GetCharacterMovement()->Velocity;
		const FVector NormalizedVelocity = CurrentVelocity.GetSafeNormal();

		// Apply boost in current direction
		WeaponController->CharacterOwner->GetCharacterMovement()->
		                  AddImpulse(NormalizedVelocity * WebReleaseBoost, true);

		// Reset state
		bIsSwinging = false;
		SwingInputDirection = FVector::ZeroVector;

		// Reset movement settings
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

	// Get camera view point
	const FVector CameraLocation = WeaponController->GetFollowCamera()->GetComponentLocation();
	const FRotator CameraRotation = WeaponController->GetFollowCamera()->GetComponentRotation();

	const FVector LineEnd = CameraLocation + (CameraRotation.Vector() * WebShootRange);;

	const FVector BoxExtent(100.0f, 100.0f, 100.0f);
	// Perform line trace
	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		CameraLocation,
		LineEnd,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeBox(BoxExtent)
	);

	// DrawDebugBox(GetWorld(), (LineStart + LineEnd) / 2, BoxExtent, FColor::Red, false, 10.0f);

	// If we hit something, use that as the web attach point
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

	// Calculate the swing plane normal (perpendicular to the direction of movement and web)
	const FVector ToAttachPoint = WebAttachPoint - WeaponController->CharacterOwner->GetActorLocation();
	FVector MovementDirection = WeaponController->CharacterOwner->GetCharacterMovement()->Velocity.GetSafeNormal();

	// If not moving, use camera right vector as direction
	if (MovementDirection.IsNearlyZero())
	{
		if (WeaponController->CharacterOwner->Controller)
		{
			const FRotator CameraRotation = WeaponController->CharacterOwner->Controller->GetControlRotation();
			MovementDirection = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::X);
		}
	}

	// Calculate normal to the swing plane
	SwingPlaneNormal = FVector::CrossProduct(ToAttachPoint, MovementDirection).GetSafeNormal();

	// If we couldn't determine a valid swing plane, default to perpendicular to forward
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
	// SetActorRotation(FRotator(NewRotation.Yaw, NewRotation.Pitch, NewRotation.Roll));
	const auto a = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), WebAttachPoint);
	const auto b = UKismetMathLibrary::RInterpTo(Mesh->GetRelativeRotation(), FRotator(
		                                             0, a.Yaw - WeaponController->GetFollowCamera()->
		                                             GetComponentRotation().Yaw,
		                                             -a.Pitch), GetWorld()->GetDeltaSeconds(), WeaponRotationTime);
	Mesh->SetRelativeRotation(b);
}

void ASwingWeapon::ApplySwingPhysics(const float DeltaTime)
{
	if (!WeaponController->CharacterOwner)
	{
		return;
	}
	// Update swing plane if necessary
	if (SwingPlaneNormal.IsNearlyZero() || FVector::DotProduct(SwingInputDirection, SwingInputDirection) > 0.1f)
	{
		UpdateSwingPlane();
	}

	// Calculate key vectors
	const FVector ToAttachPoint = WebAttachPoint - WeaponController->CharacterOwner->GetActorLocation();
	const float DistanceToAttachPoint = ToAttachPoint.Size();
	const FVector NormalizedToAttach = ToAttachPoint / DistanceToAttachPoint;

	// Apply centripetal force to keep the swing moving in an arc
	const FVector CurrentVelocity = WeaponController->CharacterOwner->GetCharacterMovement()->Velocity;
	const float CurrentSpeed = CurrentVelocity.Size();

	// Calculate tangent direction (perpendicular to both web direction and swing plane normal)
	FVector TangentDirection = FVector::CrossProduct(NormalizedToAttach, SwingPlaneNormal).GetSafeNormal();

	// Calculate direction of swing based on current velocity
	const float SwingDirectionSign = FVector::DotProduct(TangentDirection, CurrentVelocity) >= 0 ? 1.0f : -1.0f;
	TangentDirection *= SwingDirectionSign;

	// Project current velocity onto the swing plane
	const FVector VelocityOnPlane = CurrentVelocity - FVector::DotProduct(CurrentVelocity, SwingPlaneNormal) *
		SwingPlaneNormal;

	// Apply forces
	FVector ResultantForce = FVector::ZeroVector;

	// Apply tension force to keep the web taut
	if (DistanceToAttachPoint > CurrentWebLength)
	{
		// Strong tension force to enforce web length
		const FVector TensionForce = NormalizedToAttach * (DistanceToAttachPoint - CurrentWebLength) * 200000.f;
		ResultantForce += TensionForce;

		// Set the scale of the static mesh based on the distance
		const FVector NewScale = FVector(.05f, DistanceToAttachPoint * .009, .05f);
		// Assuming you want the mesh to stretch along the Z-axis
		// Apply constraint to velocity as well
		const FVector VelocityAlongWeb = FVector::DotProduct(CurrentVelocity, NormalizedToAttach) * NormalizedToAttach;
		if (DistanceToAttachPoint > CurrentWebLength && (!((WeaponController->CharacterOwner->GetActorLocation() -
				WebAttachPoint).Z > 200))
			&& FVector::DotProduct(VelocityAlongWeb, NormalizedToAttach) > 0)
		{
			// Cancel out velocity component moving away from center
			WeaponController->CharacterOwner->GetCharacterMovement()->Velocity -= VelocityAlongWeb;
		}
	}

	// Calculate centripetal force (keeps the swing moving in an arc)
	const float CentripetalForceMagnitude = (CurrentSpeed * CurrentSpeed) / FMath::Max(DistanceToAttachPoint, 100.0f);
	const FVector CentripetalForce = NormalizedToAttach * CentripetalForceMagnitude * CentripetalForceFactor;
	ResultantForce += CentripetalForce;

	// Apply momentum force in the swing direction
	const FVector MomentumForce = TangentDirection * SwingMomentumForce;
	ResultantForce += MomentumForce;

	// Apply swing input force
	if (!SwingInputDirection.IsZero())
	{
		// Project input direction onto swing plane
		FVector InputOnPlane = SwingInputDirection - FVector::DotProduct(SwingInputDirection, SwingPlaneNormal) *
			SwingPlaneNormal;
		InputOnPlane = InputOnPlane.GetSafeNormal();

		// Add input force
		ResultantForce += InputOnPlane * SwingMomentumForce * 0.5f;
	}

	// Apply final force
	WeaponController->CharacterOwner->GetCharacterMovement()->AddForce(ResultantForce * DeltaTime);

	// Modify character orientation to face the swing direction
	if (CurrentSpeed > 100.0f)
	{
		const FRotator TargetRotation = VelocityOnPlane.Rotation();
		const FRotator CurrentRotation = WeaponController->CharacterOwner->GetActorRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f);
		NewRotation.Pitch = 0.0f; // Keep character level
		NewRotation.Roll = 0.0f;
		WeaponController->CharacterOwner->SetActorRotation(NewRotation);
	}
}
