// Fill out your copyright notice in the Description page of Project Settings.


#include "SwingWeaponActor.h"

#include "MyUtils.h"
#include "ParkourCharacter.h"
#include "WeaponControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#define PRINT_TO_SCREEN(Message, Time, Color) \
if (GEngine) \
{ \
GEngine->AddOnScreenDebugMessage(-1, Time, Color, Message); \
}

ASwingWeaponActor::ASwingWeaponActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASwingWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	DebugLineEnd = WeaponMesh->GetSocketLocation(FName(*FireMuzzleSocketName));
}

void ASwingWeaponActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (ParkourOwner)
	{
		if (bIsSwinging)
		{
			ApplySwingPhysics(DeltaSeconds);

			DebugLineEnd = UKismetMathLibrary::VInterpTo(DebugLineEnd,
			                                             WebAttachPoint, DeltaSeconds, 6);

			DrawDebugLine(
				GetWorld(),
				WeaponMesh->GetSocketLocation(FName(*FireMuzzleSocketName)),
				DebugLineEnd,
				FColor::Black,
				false,
				.0f,
				0,
				3.0f
			);

			// Debug visualization
			if (bShowDebugLines)
			{
				// Draw swing plane normal
				DrawDebugLine(
					GetWorld(),
					ParkourOwner->GetActorLocation(),
					ParkourOwner->GetActorLocation() + SwingPlaneNormal * 100.0f,
					FColor::Green,
					false,
					.0f,
					0,
					2.0f
				);
			}
		}
		else
		{
			DebugLineEnd = WeaponMesh->GetSocketLocation(FName(*FireMuzzleSocketName));
		}
		PreviousLocation = ParkourOwner->GetActorLocation();
		RotateTowardsAttachPoint();
		UMyUtils::PrintToScreen(WebAttachPoint, 0, FColor::Yellow);
	}

	// Store current location for next frame
}

void ASwingWeaponActor::WhenWeaponFire_Implementation()
{
	ShootWeb();
}

void ASwingWeaponActor::OnWeaponUnequip_Implementation()
{
	Super::OnWeaponUnequip_Implementation();
	ReleaseWeb();
}

void ASwingWeaponActor::FireWeapon_Implementation(const bool IsPressed)
{
	Super::FireWeapon_Implementation(IsPressed);
	if (!IsPressed)
	{
		ReleaseWeb();
	}
}

void ASwingWeaponActor::OnWeaponEquip_Implementation()
{
	Super::OnWeaponEquip_Implementation();

	if (WeaponController == nullptr || WeaponController->CharacterOwner == nullptr)
	{
		return;
	}
	ParkourOwner = Cast<AParkourCharacter>(WeaponController->CharacterOwner);
}

bool ASwingWeaponActor::ShootWeb()
{
	if (bIsSwinging)
	{
		return false;
	}
	if (ParkourOwner == nullptr)
	{
		return false;
	}

	FVector HitLocation;
	if (GetWebTargetPoint(HitLocation))
	{
		WebAttachPoint = HitLocation;
		CurrentWebLength = FVector::Distance(ParkourOwner->GetActorLocation(), WebAttachPoint);

		// Cap the web length
		CurrentWebLength = FMath::Clamp(CurrentWebLength, MinWebLength, MaxWebLength);

		// Enable swinging mode and adjust character movement
		bIsSwinging = true;

		// Calculate initial swing plane
		UpdateSwingPlane();

		// Configure character movement for swinging
		ParkourOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		ParkourOwner->GetCharacterMovement()->GravityScale = SwingingGravityScale;
		ParkourOwner->GetBetterCharacterMovement()->SetCustomMovementMode(Sprinting);

		UGameplayStatics::PlaySoundAtLocation(this, FireSound, WeaponMesh->GetSocketLocation(*FireMuzzleSocketName));
		return true;
	}

	return false;
}

void ASwingWeaponActor::ReleaseWeb()
{
	if (ParkourOwner == nullptr)
	{
		return;
	}

	if (bIsSwinging)
	{
		// Add a boost in the direction of travel when releasing web
		FVector CurrentVelocity = ParkourOwner->GetCharacterMovement()->Velocity;
		FVector NormalizedVelocity = CurrentVelocity.GetSafeNormal();

		// Apply boost in current direction
		ParkourOwner->GetCharacterMovement()->AddImpulse(NormalizedVelocity * WebReleaseBoost, true);

		// Reset state
		bIsSwinging = false;
		SwingInputDirection = FVector::ZeroVector;

		// Reset movement settings
		ParkourOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		ParkourOwner->GetCharacterMovement()->GravityScale = 2.0f;
		WebAttachPoint = FVector::Zero();
	}
}

bool ASwingWeaponActor::GetWebTargetPoint(FVector& OutHitLocation) const
{
	if (!ParkourOwner)
	{
		return false;
	}

	// Get camera view point
	FVector CameraLocation = ParkourOwner->GetFollowCamera()->GetComponentLocation();
	FRotator CameraRotation = ParkourOwner->GetFollowCamera()->GetComponentRotation();

	// Calculate the start and end points for line trace
	FVector LineStart = CameraLocation;
	FVector LineEnd = LineStart + (CameraRotation.Vector() * WebShootRange);

	// Set up collision parameters
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(ParkourOwner);

	const FVector BoxExtent(100.0f, 100.0f, 100.0f);
	// Perform line trace
	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		LineStart,
		LineEnd,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeBox(BoxExtent),
		QueryParams
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

void ASwingWeaponActor::UpdateSwingPlane()
{
	if (!ParkourOwner)
	{
		return;
	}

	// Calculate the swing plane normal (perpendicular to the direction of movement and web)
	const FVector ToAttachPoint = WebAttachPoint - ParkourOwner->GetActorLocation();
	FVector MovementDirection = ParkourOwner->GetCharacterMovement()->Velocity.GetSafeNormal();

	// If not moving, use camera right vector as direction
	if (MovementDirection.IsNearlyZero())
	{
		if (ParkourOwner->Controller)
		{
			const FRotator CameraRotation = ParkourOwner->Controller->GetControlRotation();
			MovementDirection = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::X);
		}
	}

	// Calculate normal to the swing plane
	SwingPlaneNormal = FVector::CrossProduct(ToAttachPoint, MovementDirection).GetSafeNormal();

	// If we couldn't determine a valid swing plane, default to perpendicular to forward
	if (SwingPlaneNormal.IsNearlyZero())
	{
		FVector ForwardVector = ParkourOwner->GetActorForwardVector();
		SwingPlaneNormal = FVector::CrossProduct(ToAttachPoint, ForwardVector).GetSafeNormal();
	}
}

void ASwingWeaponActor::RotateTowardsAttachPoint() const
{
	if (!ParkourOwner)
	{
		return;
	}

	if (WebAttachPoint == FVector::Zero())
	{
		return;
	}
	// SetActorRotation(FRotator(NewRotation.Yaw, NewRotation.Pitch, NewRotation.Roll));
	const auto a = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), WebAttachPoint);
	WeaponMesh->SetRelativeRotation(FRotator(
		0, a.Yaw - ParkourOwner->GetFollowCamera()->GetComponentRotation().Yaw,
		-a.Pitch));
}

void ASwingWeaponActor::ApplySwingPhysics(const float DeltaTime)
{
	if (!ParkourOwner)
	{
		return;
	}
	// Update swing plane if necessary
	if (SwingPlaneNormal.IsNearlyZero() || FVector::DotProduct(SwingInputDirection, SwingInputDirection) > 0.1f)
	{
		UpdateSwingPlane();
	}

	// Calculate key vectors
	const FVector ToAttachPoint = WebAttachPoint - ParkourOwner->GetActorLocation();
	const float DistanceToAttachPoint = ToAttachPoint.Size();
	const FVector NormalizedToAttach = ToAttachPoint / DistanceToAttachPoint;

	// Apply centripetal force to keep the swing moving in an arc
	const FVector CurrentVelocity = ParkourOwner->GetCharacterMovement()->Velocity;
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
		if (DistanceToAttachPoint > CurrentWebLength && (!((ParkourOwner->GetActorLocation() - WebAttachPoint).Z > 200))
			&& FVector::DotProduct(VelocityAlongWeb, NormalizedToAttach) > 0)
		{
			// Cancel out velocity component moving away from center
			ParkourOwner->GetCharacterMovement()->Velocity -= VelocityAlongWeb;
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
	ParkourOwner->GetCharacterMovement()->AddForce(ResultantForce * DeltaTime);

	// Modify character orientation to face the swing direction
	if (CurrentSpeed > 100.0f)
	{
		const FRotator TargetRotation = VelocityOnPlane.Rotation();
		const FRotator CurrentRotation = ParkourOwner->GetActorRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f);
		NewRotation.Pitch = 0.0f; // Keep character level
		NewRotation.Roll = 0.0f;
		ParkourOwner->SetActorRotation(NewRotation);
	}
}
