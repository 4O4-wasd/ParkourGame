// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponPickupMaster.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "WeaponActor.generated.h"

UENUM()
enum EFireModeType
{
    Automatic UMETA(DisplayName = "Automatic"),
    Single UMETA(DisplayName = "Single")
};

/**
 * 
 */
UCLASS()
class PARKOURGAME_API AWeaponActor : public AWeaponPickupMaster
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    float Damage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    float Range;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    int BulletsPerShot = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    FString WeaponName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    FString FireMuzzleSocketName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    FVector WeaponDefaultRelativeLocation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    FVector WeaponAimRelativeLocation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    FVector WeaponPushbackRecoil;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    FVector WeaponRotationRecoil;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    FVector WeaponCameraRecoil;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    float RecoilSnappiness;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    float RecoilReturnSpeed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    float MaxRandomSpreadAngle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USoundBase *FireSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    TEnumAsByte<EFireModeType> FireModeType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    float WeaponFireDelay;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    UAnimationAsset *ReloadAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    int DefaultMagazineSize;

public:
    UPROPERTY(BlueprintReadWrite, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent *PlayerCamera;

    ///////////////////////////////////////////////////////////

    FORCEINLINE auto GetCanShoot() { return bCanShoot; }
    FORCEINLINE void SetCanShoot(bool canShoot) { bCanShoot = canShoot; }

    FORCEINLINE auto GetIsReloading() { return bIsReloading; }

    FORCEINLINE auto GetDamage() { return Damage; }

    FORCEINLINE auto GetRange() { return Range; }
    
    FORCEINLINE auto GetWeaponName() { return WeaponName; }

    FORCEINLINE auto GetFireMuzzleSocketName() { return FireMuzzleSocketName; }

    FORCEINLINE auto GetWeaponDefaultRelativeLocation() { return WeaponDefaultRelativeLocation; }

    FORCEINLINE auto GetWeaponAimRelativeLocation() { return WeaponAimRelativeLocation; }

    FORCEINLINE auto GetWeaponRecoil() { return WeaponCameraRecoil; }

    FORCEINLINE auto GetRecoilSnappiness() { return RecoilSnappiness; }

    FORCEINLINE auto GetRecoilReturnSpeed() { return RecoilReturnSpeed; }

    FORCEINLINE auto GetMaxRandomSpreadAngle() { return MaxRandomSpreadAngle; }

    FORCEINLINE auto GetFireSound() { return FireSound; }

    FORCEINLINE auto GetFireModeType() { return FireModeType; }

    FORCEINLINE auto GetWeaponFireDelay() { return WeaponFireDelay; }

    FORCEINLINE auto GetReloadAnimation() { return ReloadAnimation; }

    // FORCEINLINE auto GetPlayerCamera() { return PlayerCamera; }

private:

protected:
    FRotator CurrentCameraRotation;

    FRotator TargetCameraRotation;

    FVector CurrentRelativeLocation;

    FVector TargetRelativeLocation;

    FRotator CurrentRelativeRotation;

    FRotator TargetRelativeRotation;

    FTimerHandle FireTimerHandle;

    UFUNCTION(BlueprintCallable)
    void Recoil();

    UPROPERTY(BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    bool bIsReloading;

    int CurrentAmmo;

    UPROPERTY(BlueprintReadWrite, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    bool bIsCurrentlyHeld;

    UPROPERTY(BlueprintReadWrite, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    bool JustForTest;

    FTimerHandle FireDelayTimerHandle;

    void StopFire();

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void WhenWeaponFire();

    virtual void WhenWeaponFire_Implementation();

    virtual void OnWeaponUnequip_Implementation();

public:
    // Sets default values for this character's properties
    AWeaponActor();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void FireWeapon(bool IsPressed);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void ReloadWeapon();

    virtual void ResetFire();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnWeaponUnequip();

    bool bCanShoot = true;
};