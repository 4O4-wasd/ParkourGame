// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BetterCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "ParkourCharacter.generated.h"

class UWeaponControllerComponent;
struct FInputActionValue;

UENUM()
enum EWallRunSide
{
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UENUM()
enum EWallRunEndReason
{
	FallOffWall UMETA(DisplayName = "FallOffWall"),
	JumpedOffWall UMETA(DisplayName = "JumpedOffWall")
};


DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class PARKOURGAME_API AParkourCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

#pragma region Weapon

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponParent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponSway;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponEquip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponHolder;

#pragma endregion Weapon

#pragma region SecondWeapon

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	USceneComponent* SecondWeaponParent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	USceneComponent* SecondWeaponSway;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	USceneComponent* SecondWeaponEquip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	USceneComponent* SecondWeaponHolder;

#pragma endregion

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlideAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* UnEquipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SecondAttackAction;

	FRotator CurrentCameraRotation;

	FRotator TargetCameraRotation;

public:
	// Sets default values for this character's properties
	AParkourCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void JumpVault();

	void Move(const FInputActionValue& Value);
	void StopMove();

	void Look(const FInputActionValue& Value);

	void DynamicFieldOfView();

	void WeaponsSway();

	void DashCustom();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sway, meta = (AllowPrivateAccess = "true"))
	float MaxSwayDegree = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sway, meta = (AllowPrivateAccess = "true"))
	float SwayDegree = 2.5;

	FVector2D MoveVector;
	FVector2D LookVector;

	FVector InitialWeaponPosition; // Initial position of the weapon mesh
	FRotator InitialWeaponRotation; // Initial rotation of the weapon mesh

	FVector WeaponOffset; // Current weapon offset
	FVector LastVelocity; // Last velocity of the character

	float BobbingSpeed = 5.f; // Speed of the bobbing
	float BobbingAmount = 10.f; // Amount of bobbing

protected:
	FTimerHandle FireTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	UBetterCharacterMovementComponent* BetterCharacterMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UWeaponController* WeaponController;

	UPROPERTY(BlueprintReadWrite, Category = Camera)
	float FieldOfView = 120;

public:
	FORCEINLINE void ChangeCameraLag(const bool bIsEnabled) const { CameraBoom->bEnableCameraLag = bIsEnabled; }
	FORCEINLINE auto GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE auto GetBetterCharacterMovement() const { return BetterCharacterMovement; }

	bool bLockMovement;
};
