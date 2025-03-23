// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BetterCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	Walking UMETA(DisplayName = "Walking"),
	Sprinting UMETA(DisplayName = "Sprinting"),
	Crouching UMETA(DisplayName = "Crouching"),
	Sliding UMETA(DisplayName = "Sliding"),
	Vaulting UMETA(DisplayName = "Vaulting"),
	WallRunning UMETA(DisplayName = "WallRunning"),
};

USTRUCT(BlueprintType)
struct FMovementTypeSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Deceleration = 2600;
};

USTRUCT(BlueprintType)
struct FMovementSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMovementTypeSetting Walk;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMovementTypeSetting Sprint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMovementTypeSetting Crouch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMovementTypeSetting Slide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMovementTypeSetting WallRun;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxSlideSpeed;
};

UCLASS(ClassGroup=(BetterMovement), meta=(BlueprintSpawnableComponent))
class BETTERMOVEMENT_API UBetterCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	void SpeedControl();

protected:
	virtual void InitializeComponent() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void BeginPlay() override;

	virtual void CustomMovementUpdate();

	bool CanStand() const;

	bool CanSprint() const;

	bool CanVaultToHit(const UCapsuleComponent* Capsule, const FHitResult& Hit, FVector& EndingLocation) const;

	bool CheckCapsuleCollison(const FVector& Center, float HalfHeight, float Radius) const;

	void SetCustomMovementMode(ECustomMovementMode NewMovementMode);

	void OnCustomMovementModeChanged(ECustomMovementMode PrevMovementMode);

	void ResolveMovement();

	void CameraTilt() const;

	virtual void BeginCrouch();

	virtual void EndCrouch();

	virtual FVector CalculateFloorInfluence(FVector FloorNormal);

	virtual void BeginSlide();

	virtual void EndSlide();

	virtual void SlideUpdate();

	virtual void VaultUpdate();

	virtual void DiagonalMove();

	FMovementTypeSetting GetCustomMovementSetting(ECustomMovementMode CustomMovement) const;

public:
	UBetterCharacterMovementComponent();

	UFUNCTION(BlueprintCallable)
	void SprintPressed();

	UFUNCTION(BlueprintCallable)
	void SprintReleased();

	UFUNCTION(BlueprintCallable)
	void CrouchPressed();

	UFUNCTION(BlueprintCallable)
	void CrouchReleased();

	UFUNCTION(BlueprintCallable)
	void Vault();

	UFUNCTION(BlueprintCallable)
	bool CanVault(FVector& EndingLocation);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	FMovementSetting MovementSetting = {
		{
			600,
			2500,
			2000
		},
		{
			1000,
			2800,
			2300
		},
		{
			550,
			2500,
			1800
		},
		{
			500,
			5000,
			700
		},
		{
			850,
			2500,
			2000
		},
		7000
	};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	float SlideCameraTilt = 7;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	float VaultCameraTilt = 10;

	float StandingCapsuleHalfHeight;

	FTimerHandle SlideTimerHandler;

	FVector JumpTargetLocation;
	// FTimerHandle SwingTimerHandler;

	FVector StartingVaultLocation;
	FVector EndingVaultLocation;

	float VaultProgress;

protected:
	bool bIsSprintKeyDown = false;
	bool bIsCrouchKeyDown = false;

	FVector2D MovementInput;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECustomMovementMode> CurrentCustomMovementMode;

public:
	//Getters / Setters
	FORCEINLINE void SetMovementInput(FVector2D NewInput)
	{
		MovementInput = NewInput;
	}
};
