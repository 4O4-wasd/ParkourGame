// Fill out your copyright notice in the Description page of Project Settings.


#include "BetterCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/MathFwd.h"

#define PRINT_TO_SCREEN(Message, Time, Color) \
if (GEngine) \
{ \
GEngine->AddOnScreenDebugMessage(-1, Time, Color, Message); \
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
	SpeedControl();
	DiagonalMove();
	VaultUpdate();
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

bool UBetterCharacterMovementComponent::CanStand() const
{
	if (bIsCrouchKeyDown)
	{
		return true;
	}

	//Re-initialize hit info
	FHitResult HitResult;

	const FVector Start = CharacterOwner->GetActorLocation() - FVector(
		0, 0, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	const FVector End = Start + FVector(0, 0, StandingCapsuleHalfHeight * 2.0f);

	return GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Pawn);
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
	if (CurrentCustomMovementMode == Vaulting)
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

	if (!DidHit)
	{
		return false;
	}
	const bool Result = CanVaultToHit(Capsule, Hit, EndingLocation);
	return Result;
}

bool UBetterCharacterMovementComponent::CanVaultToHit(UCapsuleComponent* Capsule, FHitResult Hit,
                                                      FVector& EndingLocation)
{
	const bool IsInRange = UKismetMathLibrary::InRange_FloatFloat(Hit.Location.Z - Hit.TraceEnd.Z, 50, 170);

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

bool UBetterCharacterMovementComponent::CheckCapsuleCollison(FVector Center, float HalfHeight, float Radius)
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

	return UKismetSystemLibrary::CapsuleOverlapActors(GetWorld(), Center, Radius, HalfHeight, ObjectTypes,
	                                                  AActor::StaticClass(), IgnoredActors, OverlappingActors);
}

void UBetterCharacterMovementComponent::Vault()
{
	VaultProgress = 0;
	StartingVaultLocation = CharacterOwner->GetActorLocation();
	SetCustomMovementMode(Vaulting);
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
	case Vaulting:
	case Walking:
	case Sprinting:
		EndSwing();
		EndCrouch();
		break;
	case Crouching:
		EndSlide();
		BeginCrouch();
		break;
	case Swinging:
		BeginSwing();
		break;
	case Sliding:
		EndSwing();
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

void UBetterCharacterMovementComponent::BeginCrouch()
{
	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(StandingCapsuleHalfHeight / 2);
	CharacterOwner->GetMesh()->SetRelativeLocation(FVector(0, 0, -45));
}

void UBetterCharacterMovementComponent::EndCrouch()
{
	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(StandingCapsuleHalfHeight);
	CharacterOwner->GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
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
	GetWorld()->GetTimerManager().SetTimer(SlideTimerHandler, this,
	                                       &UBetterCharacterMovementComponent::SlideUpdate,
	                                       GetWorld()->GetDeltaSeconds(), true);
}

void UBetterCharacterMovementComponent::EndSlide()
{
	GetWorld()->GetTimerManager().ClearTimer(SlideTimerHandler);
}

void UBetterCharacterMovementComponent::SlideUpdate()
{
	if (CurrentCustomMovementMode != Sliding)
	{
		return;
	}

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

void UBetterCharacterMovementComponent::BeginSwing()
{
}

void UBetterCharacterMovementComponent::EndSwing()
{
}

void UBetterCharacterMovementComponent::SwingUpdate()
{
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
	}
	if (CurrentCustomMovementMode == Sprinting)
	{
		SetCustomMovementMode(Sliding);
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

FMovementTypeSetting UBetterCharacterMovementComponent::GetCustomMovementSetting(
	ECustomMovementMode CustomMovement) const
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
	case Swinging:
		return MovementSetting.Swing;
	case Sliding:
		return MovementSetting.Slide;
	default:
		return MovementSetting.Walk;
	}
}
