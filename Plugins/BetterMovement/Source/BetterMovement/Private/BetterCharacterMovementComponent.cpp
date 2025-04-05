// Fill out your copyright notice in the Description page of Project Settings.


#include "BetterCharacterMovementComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/MathFwd.h"
#include "ParkourGame/MyUtils.h"
#include "ParkourGame/ParkourCharacter.h"

#define PRINT_TO_SCREEN(Message, Time, Color) \
if (GEngine) \
{ \
GEngine->AddOnScreenDebugMessage(-1, Time, Color, Message); \
}

#define CHANGE_CAMERA_LAG(bIsEnabled) \
if (const auto CC = Cast<AParkourCharacter>(CharacterOwner)) \
{ \
\
}


void UBetterCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

UBetterCharacterMovementComponent::UBetterCharacterMovementComponent()
{
	// MovementSetting.Walk.Speed = 600;
	// MovementSetting.Walk.Acceleration = 2500;
	// MovementSetting.Slide.Deceleration = 2000;
	//
	// MovementSetting.Sprint.Speed = 1000;
	// MovementSetting.Sprint.Speed = 2800;
	// MovementSetting.Slide.Deceleration = 2300;
	//
	// MovementSetting.Crouch.Speed = 550;
	// MovementSetting.Crouch.Acceleration = 2500;
	// MovementSetting.Crouch.Deceleration = 1800;
	//
	// MovementSetting.Slide.Speed = 500;
	// MovementSetting.Slide.Acceleration = 5000;
	// MovementSetting.Slide.Deceleration = 700;
}

void UBetterCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CustomMovementUpdate();
	SpeedControl();
	DiagonalMove();
	CameraTilt();

	if (CurrentCustomMovementMode == WallRunning)
	{
		UpdateWallRun();
	}
	else if (CanWallRun())
	{
		if (!GetWorld())
		{
			return;
		}
		// Check for a valid wall on either side.
		FVector WallNormal;
		bool bIsRightSide;
		if (FindWall(WallNormal, bIsRightSide))
		{
			StartWallRun(WallNormal, bIsRightSide);
		}
	}

	if (AutoSprint && CurrentCustomMovementMode == Walking)
	{
		SetCustomMovementMode(Sprinting);
	}

	if (Velocity.Size2D() > 800)
	{
		JumpZVelocity = 700 + (FMath::Clamp(Velocity.Size2D(), 0, 3000) / 20);
	}
	else
	{
		JumpZVelocity = 700;
	}
	PRINT_TO_SCREEN(FString::Printf(TEXT("Input X: %f, Y: %f"), MovementInput.X, MovementInput.Y), 0.0f,
	                FColor::Orange);
	PRINT_TO_SCREEN(FString::Printf(TEXT("Velocity X: %f, Y: %f, Z: %f"), Velocity.X, Velocity.Y, Velocity.Z), 0.0f,
	                FColor::Orange);
	PRINT_TO_SCREEN(FString::Printf(TEXT("Velocity Size: %f"), Velocity.Length()), 0.0f, FColor::Green);
}

void UBetterCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	StandingCapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void UBetterCharacterMovementComponent::CustomMovementUpdate()
{
	switch (CurrentCustomMovementMode)
	{
	case Sliding:
		SlideUpdate();
		break;
	case Crouching:
	case Walking:
	case Sprinting:
	default:
		break;
	}
	VaultUpdate();
}

bool UBetterCharacterMovementComponent::CanStand() const
{
	//Re-initialize hit info
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);
	const FVector Start = CharacterOwner->GetActorLocation() - FVector(
		0, 0, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	const FVector End = Start + FVector(0, 0, StandingCapsuleHalfHeight * 2);
	DrawDebugLine(GetWorld(), Start, End, FColor::Emerald);

	return !GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
}

void UBetterCharacterMovementComponent::SpeedControl()
{
	MaxWalkSpeed = GetCustomMovementSetting(CurrentCustomMovementMode).Speed;
	MaxAcceleration = GetCustomMovementSetting(CurrentCustomMovementMode).Acceleration;
	BrakingDecelerationWalking = GetCustomMovementSetting(CurrentCustomMovementMode).Deceleration;
}


bool UBetterCharacterMovementComponent::CanSprint() const
{
	if (!bIsSprintKeyDown)
	{
		return false;
	}

	return CanAttemptJump() && CanStand();
}

bool UBetterCharacterMovementComponent::CanVault(FVector& EndingLocation)
{
	if (CurrentCustomMovementMode == Vaulting || CurrentCustomMovementMode == Walking || CurrentCustomMovementMode ==
		WallRunning)
	{
		return false;
	}

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(CharacterOwner);

	const auto Capsule = CharacterOwner->GetCapsuleComponent();

	const FVector Location = CharacterOwner->GetActorLocation() + (CharacterOwner->GetActorForwardVector() * 100);
	FHitResult Hit;
	const bool DidHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Location + FVector(0, 0, Capsule->GetScaledCapsuleHalfHeight()),
		Location - FVector(0, 0, Capsule->GetScaledCapsuleHalfHeight()),
		ECC_Visibility, CollisionParams);

	if (Hit.GetActor() && Hit.GetActor()->ActorHasTag("Non-Vaultable"))
	{
		return false;
	}

	if (!DidHit)
	{
		return false;
	}
	const bool Result = CanVaultToHit(Capsule, Hit, EndingLocation);
	return Result;
}

bool UBetterCharacterMovementComponent::CanVaultToHit(const UCapsuleComponent* Capsule, const FHitResult& Hit,
                                                      FVector& EndingLocation) const
{
	const bool IsInRange = UKismetMathLibrary::InRange_FloatFloat(Hit.Location.Z - Hit.TraceEnd.Z, 50,
	                                                              170);
	if (!IsInRange)
	{
		return false;
	}

	if (Hit.Normal.Z < GetWalkableFloorZ())
	{
		return false;
	}

	EndingLocation = Hit.Location + FVector(0, 0, Capsule->GetScaledCapsuleHalfHeight());

	const FVector Center = EndingLocation + FVector(0, 0, Capsule->GetScaledCapsuleRadius());
	if (CheckCapsuleCollison(Center, Capsule->GetScaledCapsuleHalfHeight(), Capsule->GetScaledCapsuleRadius()))
	{
		EndingLocation = FVector(0, 0, 0);
		return false;
	}


	return true;
}

bool UBetterCharacterMovementComponent::CheckCapsuleCollison(const FVector& Center, const float HalfHeight,
                                                             const float Radius) const
{
	DrawDebugCapsule(GetWorld(), Center, HalfHeight, Radius, FQuat::Identity,
	                 FColor::Green,
	                 false,
	                 0.f,
	                 0,
	                 2.0f);
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Vehicle));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Destructible));

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(CharacterOwner);

	TArray<AActor*> OverlappingActors;

	return UKismetSystemLibrary::CapsuleOverlapActors(GetWorld(), Center, Radius, HalfHeight, ObjectTypes,
	                                                  AActor::StaticClass(), IgnoredActors, OverlappingActors);
}

void UBetterCharacterMovementComponent::Vault()
{
	if (CurrentCustomMovementMode == WallRunning)
	{
		return;
	}
	VaultProgress = 0;
	StartingVaultLocation = CharacterOwner->GetActorLocation();
	SetCustomMovementMode(Vaulting);
	CHANGE_CAMERA_LAG(true);

	FTimerHandle VaultStartTimer;
	GetWorld()->GetTimerManager().SetTimer(VaultStartTimer, [&]()
	{
		CHANGE_CAMERA_LAG(false);
	}, .5, false);
}

void UBetterCharacterMovementComponent::SetCustomMovementMode(const ECustomMovementMode NewMovementMode)
{
	if (CurrentCustomMovementMode == NewMovementMode)
	{
		return;
	}

	const auto PrevCustomMovementMode = CurrentCustomMovementMode;
	CurrentCustomMovementMode = NewMovementMode;
	OnCustomMovementModeChanged(PrevCustomMovementMode);

	switch (CurrentCustomMovementMode)
	{
	case WallRunning:
	case Vaulting:
	case Walking:
	case Sprinting:
		EndCrouch();
		break;
	case Dashing:
		Dash();
		break;
	case Crouching:
		StopWallRun();
		EndSlide();
		BeginCrouch();
		break;
	case Sliding:
		BeginCrouch();
		BeginSlide();
		break;
	default:
		PRINT_TO_SCREEN(FString::Printf(TEXT("UMM... THINK IT's A WRONG MOVEMENT")), 1000.f, FColor::Purple);
		break;
	}
}

void UBetterCharacterMovementComponent::OnCustomMovementModeChanged(const ECustomMovementMode PrevMovementMode)
{
	MaxWalkSpeed = GetCustomMovementSetting(CurrentCustomMovementMode).Speed;
	MaxAcceleration = GetCustomMovementSetting(CurrentCustomMovementMode).Acceleration;
	BrakingDecelerationWalking = GetCustomMovementSetting(CurrentCustomMovementMode).Deceleration;

	if (CurrentCustomMovementMode == Sliding)
	{
		GroundFriction = 0;

		const float YInputAxis = FVector::DotProduct(Velocity, CharacterOwner->GetActorForwardVector());
		Velocity = CharacterOwner->GetActorForwardVector() * 1400.f *
			(YInputAxis > 0 ? 1 : YInputAxis < 0 ? -1 : 0);
	}

	if (PrevMovementMode == Sliding)
	{
		GroundFriction = 8;
	}
}

void UBetterCharacterMovementComponent::ResolveMovement()
{
	const auto ResolvedMovementMode =
		CanSprint() ? Sprinting : CanStand() ? Walking : Crouching;
	SetCustomMovementMode(ResolvedMovementMode);
}

void UBetterCharacterMovementComponent::CameraTilt() const
{
	if (!CharacterOwner)
	{
		return;
	}
	FVector WallNormal;
	bool bIsRightSide;
	FindWall(WallNormal, bIsRightSide);
	const FRotator Rotation = UKismetMathLibrary::RInterpTo(GetController()->GetControlRotation(),
	                                                        FRotator(GetController()->GetControlRotation().Pitch,
	                                                                 GetController()->GetControlRotation().Yaw,
	                                                                 -1 * (CurrentCustomMovementMode == Sliding
		                                                                       ? SlideCameraTilt
		                                                                       : CurrentCustomMovementMode == Vaulting
		                                                                       ? VaultCameraTilt
		                                                                       : CurrentCustomMovementMode ==
		                                                                       WallRunning
		                                                                       ? bIsRightSide
				                                                                       ? WallRunCameraTilt
				                                                                       : -WallRunCameraTilt
		                                                                       : 0)),
	                                                        GetWorld()->GetDeltaSeconds(),
	                                                        5);
	if (CurrentCustomMovementMode == Sliding || CurrentCustomMovementMode == Vaulting || CurrentCustomMovementMode ==
		WallRunning)
	{
		GetController()->SetControlRotation(Rotation);
		return;
	}


	const FRotator BackToRotation = UKismetMathLibrary::RInterpTo(GetController()->GetControlRotation(),
	                                                              FRotator(GetController()->GetControlRotation().Pitch,
	                                                                       GetController()->GetControlRotation().Yaw,
	                                                                       0),
	                                                              GetWorld()->GetDeltaSeconds(), 10);
	GetController()->SetControlRotation(BackToRotation);
}

void UBetterCharacterMovementComponent::BeginCrouch()
{
	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(StandingCapsuleHalfHeight / 2);
	CharacterOwner->GetMesh()->SetRelativeLocation(FVector(0, 0, -45));
	Velocity += FVector(0, 0, 100);
}

void UBetterCharacterMovementComponent::EndCrouch()
{
	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(StandingCapsuleHalfHeight);
	CharacterOwner->GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	CharacterOwner->SetActorLocation(CharacterOwner->GetActorLocation() + FVector(0, 0, 45));
}

void UBetterCharacterMovementComponent::Dash()
{
	if (!bCanDash)
	{
		ResolveMovement();
		return;
	}
	CharacterOwner->LaunchCharacter(Velocity * FVector(1, 1, 0) * DashSpeedMultiple, false, false);
	bCanDash = false;
	FTimerHandle _;
	GetWorld()->GetTimerManager().SetTimer(_, [&]()
	{
		bCanDash = true;
	}, DashCooldownTime, false);
	ResolveMovement();
}


FVector UBetterCharacterMovementComponent::CalculateFloorInfluence(FVector FloorNormal)
{
	if (FloorNormal.Equals(FVector::UpVector, .0001))
	{
		return FVector(0, 0, 0);
	}

	const FVector Cross1 = FVector::CrossProduct(FloorNormal, FVector::UpVector);
	const FVector Product1 = FVector::CrossProduct(FloorNormal, Cross1).GetSafeNormal(0.0001);
	const float Product2 = FMath::Clamp(1 - FVector::DotProduct(FloorNormal, FVector::UpVector), 0.f, 1.f);

	return Product1 * Product2;
}

void UBetterCharacterMovementComponent::BeginSlide()
{
	Velocity += FVector(0, 0, -90.f);
}

void UBetterCharacterMovementComponent::EndSlide()
{
	GetWorld()->GetTimerManager().ClearTimer(SlideTimerHandler);
}

void UBetterCharacterMovementComponent::ResetWallTimer(const float ResetTime)
{
	FTimerHandle TimerHandle;

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateLambda([&]
		{
			IsExitingWall = false;
		}),
		ResetTime,
		false
	);
}

void UBetterCharacterMovementComponent::SlideUpdate()
{
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(CharacterOwner);

	FHitResult HitResult;

	const FVector Start = CharacterOwner->GetActorLocation();
	const FVector End = CharacterOwner->GetActorLocation() - FVector(0, 0, 300);

	const auto bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);
	if (!bHit)
	{
		return;
	}

	if (IsFalling())
	{
		return;
	}

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 1);

	const FVector AppliedForce = CalculateFloorInfluence(HitResult.ImpactNormal) * 1800000.0;
	AddForce(AppliedForce);

	if (Velocity.Length() > MovementSetting.MaxSlideSpeed)
	{
		Velocity = Velocity.GetSafeNormal() * MovementSetting.MaxSlideSpeed;
	}

	if (!IsFalling() && (Velocity.Length() <= MovementSetting.Slide.Speed))
	{
		ResolveMovement();
	}

	const auto Mesh = CharacterOwner->GetMesh();
	const float Roll = UKismetMathLibrary::MakeRotFromXZ(Mesh->GetForwardVector(), HitResult.ImpactNormal).Roll;
	const float Pitch = UKismetMathLibrary::MakeRotFromYZ(Mesh->GetRightVector(), HitResult.ImpactNormal).Pitch;
	const float Yaw = Mesh->GetComponentRotation().Yaw;
	const FRotator NewMeshRotation = UKismetMathLibrary::RInterpTo(Mesh->GetComponentRotation(),
	                                                               FRotator(Pitch, Yaw, Roll),
	                                                               GetWorld()->GetDeltaSeconds(), 2.f);

	CharacterOwner->GetMesh()->SetWorldRotation(NewMeshRotation);
}

void UBetterCharacterMovementComponent::VaultUpdate()
{
	if (CurrentCustomMovementMode == Vaulting)
	{
		if (VaultProgress >= 1)
		{
			ResolveMovement();
			return;
		}
		VaultProgress = UKismetMathLibrary::FClamp(VaultProgress + (GetWorld()->GetDeltaSeconds() / .2), 0, 1);
		CharacterOwner->SetActorLocation(FMath::Lerp(StartingVaultLocation, EndingVaultLocation, VaultProgress));
	}

	if (!CanVault(EndingVaultLocation))
	{
		return;
	}
}

void UBetterCharacterMovementComponent::StartWallRun(const FVector& WallNormal, bool bIsRightSide)
{
	SetCustomMovementMode(WallRunning);
	CurrentWallNormal = WallNormal;
	WallRunDuration = 0.f;

	// Reduce gravity while wall running
	GravityScale = WallRunGravityScale;

	// Calculate the forward direction along the wall.
	// Cross the wall normal with up vector to get the wall run direction.
	FVector WallRunDirection = FVector::CrossProduct(WallNormal, FVector::UpVector);

	// Adjust based on which side the wall is on: if the wall is on the left, flip the direction.
	if (!bIsRightSide)
	{
		WallRunDirection = -WallRunDirection;
	}

	// Ensure the wall run direction aligns with the actor's forward vector.
	// If it doesn't, flip it so the character moves where they're looking.
	if (FVector::DotProduct(WallRunDirection, CharacterOwner->GetActorForwardVector()) < 0.f)
	{
		WallRunDirection = -WallRunDirection;
	}

	WallRunDirection.Normalize();

	// Set the fixed wall run speed.
	Velocity = WallRunDirection * WallRunSpeed;

	CHANGE_CAMERA_LAG(true);

	FTimerHandle WallRunStartTimer;
	GetWorld()->GetTimerManager().SetTimer(WallRunStartTimer, [&]()
	{
		CHANGE_CAMERA_LAG(false);
	}, .5, false);


	UE_LOG(LogTemp, Log, TEXT("Wall Run Started"));
}

void UBetterCharacterMovementComponent::UpdateWallRun()
{
	WallRunDuration += GetWorld()->GetDeltaSeconds();

	// Continuously update the wall run direction and enforce constant speed.
	FVector Forward = CharacterOwner->GetActorForwardVector();
	FVector WallRunDirection = FVector::CrossProduct(CurrentWallNormal, FVector::UpVector);

	// If WallRunDirection is opposite to where the player is looking, flip it
	if (FVector::DotProduct(WallRunDirection, Forward) < 0)
	{
		WallRunDirection = -WallRunDirection;
	}
	WallRunDirection.Normalize();
	float WallPushForce = 1000.0f;
	AddForce(WallRunDirection * WallRunSpeed);

	// Since CurrentWallNormal points out from the wall, we use -CurrentWallNormal to push toward it.
	Velocity += -CurrentWallNormal * WallPushForce;

	if (!CanWallRun())
	{
		Velocity += CurrentWallNormal * WallPushForce;
		StopWallRun(1.f);
		return;
	}

	if (!(MovementInput.Y > 0))
	{
		Velocity += CurrentWallNormal * WallPushForce;
		return;
	}

	Velocity = UKismetMathLibrary::Vector_ClampSizeMax(Velocity, 2000);

	// Optionally, check if conditions to stop wall run are met (e.g., player input, no wall, etc.)
	bool bDummy;
	if (FVector DummyNormal; !FindWall(DummyNormal, bDummy))
	{
		Velocity += CurrentWallNormal * WallPushForce;
		StopWallRun(1.f);
		return;
	}
}

void UBetterCharacterMovementComponent::WallJump()
{
	// Only execute if we're currently wall running.
	if (CurrentCustomMovementMode == WallRunning)
	{
		CHANGE_CAMERA_LAG(true);
		// Calculate the jump direction:
		// Combining upward with CurrentWallNormal pushes the character away from the wall.
		FVector JumpOffDirection = (FVector::UpVector * 1 + CurrentWallNormal * 1);

		StopWallRun();
		// Launch the character in that direction.
		Velocity = FVector::UpVector * WallUpJumpStrength + (CurrentWallNormal + CharacterOwner->
				GetActorForwardVector()) *
			WallJumpStrength;
		FTimerHandle WallJumpTimer;
		GetWorld()->GetTimerManager().SetTimer(WallJumpTimer, [&]()
		{
			CHANGE_CAMERA_LAG(false);
		}, .5, false);

		UE_LOG(LogTemp, Log, TEXT("Wall Jump executed: %s"), *JumpOffDirection.ToString());

		// Disable wall running temporarily to avoid immediate re-trigger.

		UE_LOG(LogTemp, Log, TEXT("Wall Jump executed: %s"), *JumpOffDirection.ToString());
	}
}

void UBetterCharacterMovementComponent::StopWallRun(float CoolDown)
{
	if (CurrentCustomMovementMode != WallRunning)
	{
		return;
	}
	GravityScale = 2.0f;
	bCanWallRun = false;
	ResolveMovement();
	FTimerHandle WallRunCooldownTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(WallRunCooldownTimerHandle, [&]()
	{
		bCanWallRun = true;
	}, CoolDown, false);
	PRINT_TO_SCREEN(FString::Printf(TEXT("Wall run stopped")), 100.0f,
	                FColor::Orange);
	UE_LOG(LogTemp, Log, TEXT("Wall Run Stopped"));
}

bool UBetterCharacterMovementComponent::CanWallRun() const
{
	FHitResult Hit;
	DrawDebugLine(GetWorld(), CharacterOwner->GetActorLocation(),
	              CharacterOwner->GetActorLocation() + FVector(
		              0, 0, -MinWallRunJumpHeight), FColor::Emerald);
	return bCanWallRun && !GetWorld()->LineTraceSingleByChannel(
		Hit, CharacterOwner->GetActorLocation(),
		CharacterOwner->GetActorLocation() + FVector(
			0, 0, -MinWallRunJumpHeight),
		ECC_Visibility);
}

bool UBetterCharacterMovementComponent::FindWall(FVector& OutWallNormal, bool& bIsRightSide) const
{
	const FVector Start = CharacterOwner->GetActorLocation();
	constexpr float TraceDistance = 60.f;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);

	// Lambda function for wall trace
	auto PerformWallTrace = [&](const FVector& Direction, const bool RightSide)
	{
		FHitResult HitResult;
		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult, Start, Start + Direction * TraceDistance, ECC_Visibility, QueryParams
		);

		DrawDebugLine(GetWorld(), Start, Start + Direction * TraceDistance, FColor::Blue, false, 0.1f);

		if (!bHit)
		{
			const bool bBottomHit = GetWorld()->LineTraceSingleByChannel(
				HitResult,
				Start - FVector(0, 0, 70),
				Start - FVector(0, 0, 70) + Direction * TraceDistance, ECC_Visibility, QueryParams
			);
			DrawDebugLine(
				GetWorld(),
				Start - FVector(0, 0, 70),
				Start - FVector(0, 0, 70) + Direction * TraceDistance, FColor::Blue, false, 0.1f);
			if (bBottomHit && HitResult.GetActor() && HitResult.GetActor()->ActorHasTag("WallRunnable") &&
				FMath::Abs(FVector::DotProduct(HitResult.Normal, FVector::UpVector)) < 0.5f)
			{
				OutWallNormal = HitResult.Normal;
				bIsRightSide = RightSide;
				return true;
			}
			return false;
		}

		if (bHit && HitResult.GetActor() && HitResult.GetActor()->ActorHasTag("WallRunnable") &&
			FMath::Abs(FVector::DotProduct(HitResult.Normal, FVector::UpVector)) < 0.5f)
		{
			OutWallNormal = HitResult.Normal;
			bIsRightSide = RightSide;
			return true;
		}
		return false;
	};

	// Perform traces for both right and left
	return PerformWallTrace(CharacterOwner->GetActorRightVector(), true) ||
		PerformWallTrace(-CharacterOwner->GetActorRightVector(), false);
}


void UBetterCharacterMovementComponent::DiagonalMove()
{
	if (MovementInput.X == 0 || MovementInput.Y == 0)
	{
		MaxWalkSpeed = GetCustomMovementSetting(CurrentCustomMovementMode).Speed;
		return;
	}

	if (CurrentCustomMovementMode == Sliding)
	{
		return;
	}

	MaxWalkSpeed = GetCustomMovementSetting(CurrentCustomMovementMode).Speed + 125.f;
}

void UBetterCharacterMovementComponent::SprintPressed()
{
	bIsSprintKeyDown = true;
	if (CurrentCustomMovementMode == Walking || Crouching)
	{
		SetCustomMovementMode(Sprinting);
	}
}

void UBetterCharacterMovementComponent::SprintReleased()
{
	bIsSprintKeyDown = false;
	if (CurrentCustomMovementMode == Sprinting)
	{
		ResolveMovement();
	}
}

void UBetterCharacterMovementComponent::CrouchPressed()
{
	bIsCrouchKeyDown = true;
	if (CurrentCustomMovementMode == Walking)
	{
		SetCustomMovementMode(Crouching);
		return;
	}
	if (CurrentCustomMovementMode == Sprinting)
	{
		SetCustomMovementMode(Sliding);
		return;
	}
}

void UBetterCharacterMovementComponent::CrouchReleased()
{
	bIsCrouchKeyDown = false;
	if (CurrentCustomMovementMode != Sliding)
	{
		ResolveMovement();
	}
}

void UBetterCharacterMovementComponent::DashPressed()
{
	if (CurrentCustomMovementMode == Walking || CurrentCustomMovementMode == Sprinting)
	{
		SetCustomMovementMode(Dashing);
	}
}

FMovementTypeSetting UBetterCharacterMovementComponent::GetCustomMovementSetting(
	const ECustomMovementMode CustomMovement) const
{
	switch (CustomMovement)
	{
	case Walking:
	case Vaulting:
		return MovementSetting.Walk;
	case Sprinting:
		return MovementSetting.Sprint;
	case Crouching:
		return MovementSetting.Crouch;
	case Sliding:
		return MovementSetting.Slide;
	case WallRunning:
		return MovementSetting.WallRun;
	default:
		return MovementSetting.Walk;
	}
}
