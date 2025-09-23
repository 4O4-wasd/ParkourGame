#pragma once

#include "BetterCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/MathFwd.h"
#include "ParkourGame/ParkourCharacter.h"

#define PRINT_TO_SCREEN(Message, Time, Color) \
if (GEngine) \
{ \
    GEngine->AddOnScreenDebugMessage(-1, Time, Color, Message); \
}

#pragma region Initialization Functions

void UBetterCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

UBetterCharacterMovementComponent::UBetterCharacterMovementComponent():
	AutoSprint(false),
	IsExitingWall(false),
	MovementInput(),
	CurrentCustomMovementMode(Walking),
	CurrentWallNormal(),
	VaultProgress(0),
	StandingCapsuleHalfHeight(0),
	JumpTargetLocation(),
	StartingVaultLocation(),
	EndingVaultLocation()
{
}

void UBetterCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	StandingCapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

#pragma endregion

#pragma region Main Update Functions

void UBetterCharacterMovementComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CustomMovementUpdate();
	CameraTilt();
}

void UBetterCharacterMovementComponent::CustomMovementUpdate()
{
	switch (CurrentCustomMovementMode)
	{
	case Sliding:
		SlideUpdate();
		break;
	case Vaulting:
		VaultUpdate();
		break;
	case WallRunning:
		UpdateWallRun();
		break;
	case Dashing:
		DashUpdate();
		break;
	case Crouching:
	case Walking:
	case Sprinting:
	default:
		break;
	}

#pragma region Simultaneous Operations

	SpeedControl();

	DiagonalMove();

	if (CanWallRun())
	{
		if (!GetWorld())
		{
			return;
		}
		bool bIsRightSide;
		if (FVector WallNormal; CanFindAWall(WallNormal, bIsRightSide))
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

#pragma endregion
}

#pragma endregion

#pragma region Movement State Management

void UBetterCharacterMovementComponent::SetCustomMovementMode(const ECustomMovementMode NewMovementMode)
{
	if (!IsCustomMovementModeEnabled(NewMovementMode))
	{
		return;
	}

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
		EndCrouch();
		break;
	case Vaulting:
	case Walking:
	case Sprinting:
		StopWallRun();
		EndCrouch();
		break;
	case Dashing:
		EndCrouch();
		StopWallRun();
		Dash();
		break;
	case Crouching:
		StopWallRun();
		EndSlide();
		BeginCrouch();
		break;
	case Sliding:
		StopWallRun();
		BeginCrouch();
		BeginSlide();
		break;
	default:
		PRINT_TO_SCREEN(FString::Printf(TEXT("UMM... THINK IT's A WRONG MOVEMENT")), 1000.f, FColor::Purple);
		break;
	}
}

bool UBetterCharacterMovementComponent::IsCustomMovementModeEnabled(const ECustomMovementMode TheMovementMode) const
{
	switch (TheMovementMode)
	{
	case Walking:
		return true;
	case Sprinting:
		return MovementEnabledSettings.Sprint;
	case Crouching:
		return MovementEnabledSettings.Crouch;
	case Sliding:
		return MovementEnabledSettings.Slide;
	case Dashing:
		return MovementEnabledSettings.Dash;
	case Vaulting:
		return MovementEnabledSettings.Vault;
	case WallRunning:
		return MovementEnabledSettings.WallRun;
	}

	return false;
}

void UBetterCharacterMovementComponent::OnCustomMovementModeChanged(const ECustomMovementMode PrevMovementMode)
{
	MaxWalkSpeed = GetCustomMovementSetting(CurrentCustomMovementMode).Speed;
	MaxAcceleration = GetCustomMovementSetting(CurrentCustomMovementMode).Acceleration;
	BrakingDecelerationWalking = GetCustomMovementSetting(CurrentCustomMovementMode).Deceleration;

	if (PrevMovementMode == Crouching)
	{
		CharacterOwner->SetActorLocation(CharacterOwner->GetActorLocation() + FVector(0, 0, 45));
	}

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

#pragma endregion

#pragma region Movement Utility Functions

void UBetterCharacterMovementComponent::SpeedControl()
{
	MaxWalkSpeed = GetCustomMovementSetting(CurrentCustomMovementMode).Speed;
	MaxAcceleration = GetCustomMovementSetting(CurrentCustomMovementMode).Acceleration;
	BrakingDecelerationWalking = GetCustomMovementSetting(CurrentCustomMovementMode).Deceleration;
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
		return static_cast<FMovementTypeSetting>(MovementSetting.Slide);
	case WallRunning:
		return FMovementTypeSetting(0, MovementSetting.WallRun.Acceleration, MovementSetting.WallRun.Deceleration);
	case Dashing:
		return static_cast<FMovementTypeSetting>(MovementSetting.Dash);
	default:
		return MovementSetting.Walk;
	}
}

#pragma endregion

#pragma region Camera Effects

void UBetterCharacterMovementComponent::CameraTilt() const
{
	if (!CharacterOwner)
	{
		return;
	}

	FVector WallNormal;
	bool bIsRightSide;
	CanFindAWall(WallNormal, bIsRightSide);

	const FRotator Rotation = UKismetMathLibrary::RInterpTo(
		GetController()->GetControlRotation(),
		FRotator(
			GetController()->GetControlRotation().Pitch,
			GetController()->GetControlRotation().Yaw,
			-1 * (
				CurrentCustomMovementMode == Sliding
					? SlideCameraTilt
					: (
						CurrentCustomMovementMode == Vaulting
							? VaultCameraTilt
							: (
								CurrentCustomMovementMode == WallRunning
									? (
										bIsRightSide
											? WallRunCameraTilt
											: -WallRunCameraTilt
									)
									: 0
							)
					)
			)
		),
		GetWorld()->GetDeltaSeconds(),
		5);

	if (CurrentCustomMovementMode == Sliding || CurrentCustomMovementMode == Vaulting || CurrentCustomMovementMode ==
		WallRunning)
	{
		GetController()->SetControlRotation(Rotation);
		return;
	}

	const FRotator BackToRotation = UKismetMathLibrary::RInterpTo(
		GetController()->GetControlRotation(),
		FRotator(
			GetController()->GetControlRotation().Pitch,
			GetController()->GetControlRotation().Yaw,
			0
		),
		GetWorld()->GetDeltaSeconds(),
		10
	);

	GetController()->SetControlRotation(BackToRotation);
}

#pragma endregion

#pragma region Crouch Functions

bool UBetterCharacterMovementComponent::CanStand() const
{
	//Re-initialize hit info
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);

	const FVector Start = CharacterOwner->GetActorLocation() - FVector(
		0, 0, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	const FVector End = Start + FVector(0, 0, StandingCapsuleHalfHeight * 2);

	return !GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
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

#pragma endregion

#pragma region Sprint Functions

bool UBetterCharacterMovementComponent::CanSprint() const
{
	if (!bIsSprintKeyDown)
	{
		return false;
	}

	return CanAttemptJump() && CanStand();
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

#pragma endregion

#pragma region Slide Functions

void UBetterCharacterMovementComponent::BeginSlide()
{
	Velocity += FVector(0, 0, -90.f);
}

void UBetterCharacterMovementComponent::EndSlide()
{
	GetWorld()->GetTimerManager().ClearTimer(SlideTimerHandler);
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


	const FVector AppliedForce = CalculateFloorInfluence(HitResult.ImpactNormal) * 1800000.0;
	AddForce(AppliedForce);

	if (Velocity.Length() > MovementSetting.Slide.MaxSlideSpeed)
	{
		Velocity = Velocity.GetSafeNormal() * MovementSetting.Slide.MaxSlideSpeed;
	}

	if (!IsFalling() && (Velocity.Length() <= MovementSetting.Slide.Speed))
	{
		ResolveMovement();
	}

	const auto Mesh = CharacterOwner->GetMesh();
	const float Roll = UKismetMathLibrary::MakeRotFromXZ(Mesh->GetForwardVector(), HitResult.ImpactNormal).Roll;
	const float Pitch = UKismetMathLibrary::MakeRotFromYZ(Mesh->GetRightVector(), HitResult.ImpactNormal).Pitch;
	const float Yaw = Mesh->GetComponentRotation().Yaw;
	const FRotator NewMeshRotation = UKismetMathLibrary::RInterpTo(
		Mesh->GetComponentRotation(),
		FRotator(Pitch, Yaw, Roll),
		GetWorld()->GetDeltaSeconds(),
		2.f
	);

	CharacterOwner->GetMesh()->SetWorldRotation(NewMeshRotation);
}

FVector UBetterCharacterMovementComponent::CalculateFloorInfluence(const FVector& FloorNormal)
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

#pragma endregion

#pragma region Dash Functions

void UBetterCharacterMovementComponent::DashPressed()
{
	if (CurrentCustomMovementMode == Walking || CurrentCustomMovementMode == Sprinting)
	{
		SetCustomMovementMode(Dashing);
	}
}

void UBetterCharacterMovementComponent::Dash()
{
	if (!bCanDash || (Velocity.Size2D() <= 0))
	{
		ResolveMovement();
		return;
	}
	DashProgress = 0;
	StartingDashLocation = CharacterOwner->GetActorLocation();
	const FVector MoveDirection = (CharacterOwner->GetActorForwardVector() * MovementInput.Y) + (CharacterOwner->
		GetActorRightVector() * MovementInput.X).GetClampedToMaxSize(1.f);
	EndingDashLocation = CharacterOwner->GetActorLocation() + (MoveDirection * MovementSetting.Dash.Distance);
	SetCustomMovementMode(Dashing);
}

void UBetterCharacterMovementComponent::DashUpdate()
{
	if (!bCanDash)
	{
		return;
	}
	if (DashProgress >= 1)
	{
		ResolveMovement();
		bCanDash = false;
		FTimerHandle _;
		GetWorld()->GetTimerManager().SetTimer(_, [&]()
		{
			bCanDash = true;
		}, MovementSetting.Dash.CooldownTime, false);
		return;
	}

	DashProgress = UKismetMathLibrary::FClamp(
		DashProgress + (GetWorld()->GetDeltaSeconds() / MovementSetting.Dash.Time), 0, 1
	);
	CharacterOwner->SetActorLocation(FMath::Lerp(StartingDashLocation, EndingDashLocation, DashProgress), true);
}

#pragma endregion

#pragma region Vault Functions

bool UBetterCharacterMovementComponent::CanVault(FVector& EndingLocation) const
{
	if (CurrentCustomMovementMode == Vaulting || CurrentCustomMovementMode == WallRunning)
	{
		return false;
	}

	if (Velocity.Size() < 1000)
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

	if (Hit.GetActor() && Hit.GetActor()->ActorHasTag("NotVaultable"))
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
	const bool IsInRange = UKismetMathLibrary::InRange_FloatFloat(
		Hit.Location.Z - Hit.TraceEnd.Z,
		50,
		170
	);

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
	if (CheckCapsuleCollision(Center, Capsule->GetScaledCapsuleHalfHeight(), Capsule->GetScaledCapsuleRadius()))
	{
		EndingLocation = FVector(0, 0, 0);
		return false;
	}

	return true;
}

bool UBetterCharacterMovementComponent::CheckCapsuleCollision(const FVector& Center, const float HalfHeight,
                                                              const float Radius) const
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Vehicle));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Destructible));

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(CharacterOwner);

	TArray<AActor*> OverlappingActors;

	return UKismetSystemLibrary::CapsuleOverlapActors(
		GetWorld(),
		Center,
		Radius,
		HalfHeight,
		ObjectTypes,
		AActor::StaticClass(),
		IgnoredActors,
		OverlappingActors
	);
}

void UBetterCharacterMovementComponent::Vault()
{
	if (!CanVault(EndingVaultLocation))x
	{
		return;
	}

	if (CurrentCustomMovementMode == WallRunning)
	{
		return;
	}

	VaultProgress = 0;
	StartingVaultLocation = CharacterOwner->GetActorLocation();
	SetCustomMovementMode(Vaulting);
}

void UBetterCharacterMovementComponent::VaultUpdate()
{
	if (VaultProgress >= 1)
	{
		ResolveMovement();
		return;
	}

	VaultProgress = UKismetMathLibrary::FClamp(VaultProgress + (GetWorld()->GetDeltaSeconds() / .2), 0, 1);
	CharacterOwner->SetActorLocation(FMath::Lerp(StartingVaultLocation, EndingVaultLocation, VaultProgress));
}

#pragma endregion

#pragma region Wall Run Functions

bool UBetterCharacterMovementComponent::CanWallRun() const
{
	FHitResult Hit;
	return bCanWallRun && !GetWorld()->LineTraceSingleByChannel(
		Hit, CharacterOwner->GetActorLocation(),
		CharacterOwner->GetActorLocation() + FVector(
			0, 0, -MovementSetting.WallRun.MinRunHeight),
		ECC_Visibility);
}

bool UBetterCharacterMovementComponent::CanFindAWall(FVector& OutWallNormal, bool& bIsRightSide) const
{
	const FVector Start = CharacterOwner->GetActorLocation();
	constexpr float TraceDistance = 60.f;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);

	auto PerformWallTrace = [&](const FVector& Direction, const bool RightSide)
	{
		FHitResult HitResult;
		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult, Start, Start + Direction * TraceDistance, ECC_Visibility, QueryParams
		);

		if (!bHit)
		{
			const bool bBottomHit = GetWorld()->LineTraceSingleByChannel(
				HitResult,
				Start - FVector(0, 0, 70),
				Start - FVector(0, 0, 70) + Direction * TraceDistance, ECC_Visibility, QueryParams
			);
			
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

	return PerformWallTrace(CharacterOwner->GetActorRightVector(), true) ||
		PerformWallTrace(-CharacterOwner->GetActorRightVector(), false);
}

void UBetterCharacterMovementComponent::StartWallRun(const FVector& WallNormal, const bool bIsRightSide)
{
	SetCustomMovementMode(WallRunning);
	CurrentWallNormal = WallNormal;
	WallRunDuration = 0.f;

	GravityScale = MovementSetting.WallRun.GravityScale;

	FVector WallRunDirection = FVector::CrossProduct(WallNormal, FVector::UpVector);

	if (!bIsRightSide)
	{
		WallRunDirection = -WallRunDirection;
	}

	if (FVector::DotProduct(WallRunDirection, CharacterOwner->GetActorForwardVector()) < 0.f)
	{
		WallRunDirection = -WallRunDirection;
	}

	WallRunDirection.Normalize();

	Velocity = WallRunDirection * MovementSetting.WallRun.Speed;


	UE_LOG(LogTemp, Log, TEXT("Wall Run Started"));
}

void UBetterCharacterMovementComponent::UpdateWallRun()
{
	WallRunDuration += GetWorld()->GetDeltaSeconds();

	const FVector Forward = CharacterOwner->GetActorForwardVector();
	FVector WallRunDirection = FVector::CrossProduct(CurrentWallNormal, FVector::UpVector);

	if (FVector::DotProduct(WallRunDirection, Forward) < 0)
	{
		WallRunDirection = -WallRunDirection;
	}
	WallRunDirection.Normalize();
	constexpr float WallPushForce = 1000.0f;
	AddForce(WallRunDirection * MovementSetting.WallRun.Speed);

	Velocity += -CurrentWallNormal * WallPushForce;

	if (!CanWallRun())
	{
		StopWallRun(1.f);
		Velocity += CurrentWallNormal * WallPushForce;
		return;
	}

	if (!(MovementInput.Y > 0))
	{
		StopWallRun(1.f);
		Velocity += CurrentWallNormal * WallPushForce;
		return;
	}

	Velocity = UKismetMathLibrary::Vector_ClampSizeMax(Velocity, 2000);

	bool bDummy;
	if (FVector DummyNormal; !CanFindAWall(DummyNormal, bDummy))
	{
		Velocity += CurrentWallNormal * WallPushForce;
		StopWallRun(1.f);
		return;
	}
}

void UBetterCharacterMovementComponent::WallJump()
{
	if (CurrentCustomMovementMode == WallRunning)
	{
		const FVector JumpOffDirection = (FVector::UpVector * 1 + CurrentWallNormal * 1);

		StopWallRun();
		Velocity = FVector::UpVector * MovementSetting.WallRun.JumpUpStrength +
			(CurrentWallNormal + CharacterOwner->GetActorForwardVector()) *
			MovementSetting.WallRun.JumpStrength;

		UE_LOG(LogTemp, Log, TEXT("Wall Jump executed: %s"), *JumpOffDirection.ToString());

		UE_LOG(LogTemp, Log, TEXT("Wall Jump executed: %s"), *JumpOffDirection.ToString());
	}
}

void UBetterCharacterMovementComponent::StopWallRun(const float CoolDown)
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

#pragma endregion
