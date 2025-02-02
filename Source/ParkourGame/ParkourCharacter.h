// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BetterCharacterMovementComponent.h"
#include "GameFramework/Character.h"
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
    class USpringArmComponent *CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent *FollowCamera;

#pragma region Weapon

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USceneComponent *WeaponParent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USceneComponent *WeaponFOV;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USceneComponent *WeaponSway;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USceneComponent *WeaponEquip;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USceneComponent *WeaponHolder;

#pragma endregion Weapon

#pragma region SecondWeapon

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USceneComponent *SecondWeaponParent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USceneComponent *SecondWeaponFOV;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USceneComponent *SecondWeaponSway;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USceneComponent *SecondWeaponEquip;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USceneComponent *SecondWeaponHolder;

#pragma endregion

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USkeletalMeshComponent *WeaponSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputMappingContext *DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction *JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction *MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction *LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction *SprintAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction *CrouchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction *SlideAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction *AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction *SecondAttackAction;

    FRotator CurrentCameraRotation;

    FRotator TargetCameraRotation;

public:
    // Sets default values for this character's properties
    AParkourCharacter(const FObjectInitializer &ObjectInitializer);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable)
    void CalculateCameraRotation(class AWeaponActor *Weapon);

    void MovingForward();
    void OnWall();
    void ConsumeJump();

    void Move(const FInputActionValue &Value);

    void Look(const FInputActionValue &Value);

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

private:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapons, meta = (AllowPrivateAccess = "true"))
    TArray<AWeaponActor *> Weapons;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapons, meta = (AllowPrivateAccess = "true"))
    AWeaponActor *CurrentWeapon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
    int32 CurrentWeaponIndex;

    // WallRun

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WallRun, meta = (AllowPrivateAccess = "true"))
    FVector WallRunDirection;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WallRun, meta = (AllowPrivateAccess = "true"))
    bool IsWallRunning;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WallRun, meta = (AllowPrivateAccess = "true"))
    int JumpLeft;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WallRun, meta = (AllowPrivateAccess = "true"))
    int MaxJumps = 2;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WallRun, meta = (AllowPrivateAccess = "true"))
    float RightAxis;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WallRun, meta = (AllowPrivateAccess = "true"))
    float ForwardAxis;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WallRun, meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<EWallRunSide> WallRunSide;

protected:
    FTimerHandle FireTimerHandle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
    UBetterCharacterMovementComponent *BetterCharacterMovement;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    UWeaponControllerComponent *WeaponController;

public:
    FORCEINLINE auto GetCurrentWeapon() { return CurrentWeapon; }

    bool bLockMovement;
};