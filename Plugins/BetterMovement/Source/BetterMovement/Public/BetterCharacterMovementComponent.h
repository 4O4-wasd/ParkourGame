// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BetterCharacterMovementComponent.generated.h"

#pragma region Enums

UENUM(BlueprintType)
enum ECustomMovementMode
{
	Walking UMETA(DisplayName = "Walking"),
	Sprinting UMETA(DisplayName = "Sprinting"),
	Crouching UMETA(DisplayName = "Crouching"),
	Sliding UMETA(DisplayName = "Sliding"),
	Vaulting UMETA(DisplayName = "Vaulting"),
	WallRunning UMETA(DisplayName = "WallRunning"),
	Dashing UMETA(DisplayName = "Dashing"),
};

UENUM(BlueprintType)
enum ECustomMovementWallRunSide
{
	WallRunNone UMETA(DisplayName = "None"),
	WallRunLeft UMETA(DisplayName = "Left"),
	WallRunRight UMETA(DisplayName = "Right"),
};

#pragma endregion Enums

#pragma region Structs

USTRUCT(BlueprintType)
struct FMovementTypeSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Deceleration;
};

USTRUCT()
struct FSlideFMovementTypeSetting : public FMovementTypeSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxSlideSpeed = 3000;
};

USTRUCT()
struct FWallRunFMovementTypeSetting : public FMovementTypeSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinRunHeight = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GravityScale = .5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float JumpStrength = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float JumpUpStrength = 800.f;
};

USTRUCT()
struct FDashMovementTypeSetting : public FMovementTypeSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Distance = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CooldownTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Time = .2f;
};

USTRUCT(BlueprintType)
struct FMovementSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMovementTypeSetting Walk = {600, 2500, 2000};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMovementTypeSetting Sprint = {1000, 2800, 2300};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMovementTypeSetting Crouch = {550, 2500, 1800};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSlideFMovementTypeSetting Slide = {500, 5000, 700};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWallRunFMovementTypeSetting WallRun = {1000, 2800, 2300};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDashMovementTypeSetting Dash = {0, 2500, 2000};
};

USTRUCT(BlueprintType)
struct FMovementEnabledSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool Sprint = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool Crouch = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool Slide = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool Vault = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool WallRun = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool Dash = true;
};

#pragma endregion Structs

UCLASS(ClassGroup = (BetterMovement), meta = (BlueprintSpawnableComponent))
class BETTERMOVEMENT_API UBetterCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UBetterCharacterMovementComponent();

#pragma region Public Functions

	UFUNCTION(BlueprintCallable)
	void SprintPressed();

	UFUNCTION(BlueprintCallable)
	void SprintReleased();

	UFUNCTION(BlueprintCallable)
	void CrouchPressed();

	UFUNCTION(BlueprintCallable)
	void CrouchReleased();

	UFUNCTION(BlueprintCallable)
	void DashPressed();

	UFUNCTION(BlueprintCallable)
	void StopWallRun(float CoolDown = .5f);

	UFUNCTION(BlueprintCallable)
	void Vault();

	UFUNCTION(BlueprintCallable)
	bool CanVault(FVector& EndingLocation) const;

	UFUNCTION(BlueprintCallable)
	void WallJump();

	UFUNCTION(BlueprintCallable)
	void SetCustomMovementMode(ECustomMovementMode NewMovementMode);

	UFUNCTION(BlueprintCallable)
	bool IsCustomMovementModeEnabled(ECustomMovementMode TheMovementMode) const;

	// Getters / Setters
	FORCEINLINE void SetMovementInput(FVector2D NewInput) { MovementInput = NewInput; }
	FORCEINLINE auto GetCurrentCustomMovementMode() const { return CurrentCustomMovementMode; }

#pragma endregion Public Functions

protected:
#pragma region Lifecycle

	virtual void InitializeComponent() override;

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

#pragma endregion Lifecycle

#pragma region Movement Logic

	virtual void CustomMovementUpdate();
	void SpeedControl();
	void ResolveMovement();
	void CameraTilt() const;

	bool CanStand() const;
	bool CanSprint() const;

	virtual void BeginCrouch();
	virtual void EndCrouch();

	virtual void Dash();
	virtual void DashUpdate();

	virtual void BeginSlide();
	virtual void EndSlide();
	virtual void SlideUpdate();

	virtual void VaultUpdate();

	void StartWallRun(const FVector& WallNormal, bool bIsRightSide);
	void UpdateWallRun();

	virtual void DiagonalMove();
	static FVector CalculateFloorInfluence(const FVector& FloorNormal);
	void OnCustomMovementModeChanged(ECustomMovementMode PrevMovementMode);

	bool CanWallRun() const;
	bool CanFindAWall(FVector& OutWallNormal, bool& bIsRightSide) const;

	void ResetWallTimer(float ResetTime);

	bool CanVaultToHit(const UCapsuleComponent* Capsule, const FHitResult& Hit, FVector& EndingLocation) const;
	bool CheckCapsuleCollision(const FVector& Center, float HalfHeight, float Radius) const;

	FMovementTypeSetting GetCustomMovementSetting(ECustomMovementMode CustomMovement) const;

#pragma endregion Movement Logic

#pragma region Private Variables

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	FMovementSetting MovementSetting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	FMovementEnabledSettings MovementEnabledSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	float SlideCameraTilt = 7;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	float VaultCameraTilt = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	float WallRunCameraTilt = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	bool AutoSprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run", meta = (AllowPrivateAccess = "true"))
	float WallRunCooldownTime = 0.5f;

#pragma endregion Private Variables

#pragma region State Variables

protected:
	bool bIsSprintKeyDown = false;
	bool bIsCrouchKeyDown = false;
	bool bCanWallRun = true;
	bool bCanDash = true;
	bool IsExitingWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	FVector2D MovementInput;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Movement", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECustomMovementMode> CurrentCustomMovementMode;

	FVector CurrentWallNormal;
	float WallRunDuration = 0.f;
	float VaultProgress = .0f;
	float DashProgress = .0f;

	float StandingCapsuleHalfHeight;

	FTimerHandle SlideTimerHandler;
	FTimerHandle EndCrouchTimerHandler;
	FTimerHandle DashTimerHandler;

	FVector JumpTargetLocation;
	FVector StartingVaultLocation;
	FVector EndingVaultLocation;

	FVector StartingDashLocation;
	FVector EndingDashLocation;

#pragma endregion State Variables
};
