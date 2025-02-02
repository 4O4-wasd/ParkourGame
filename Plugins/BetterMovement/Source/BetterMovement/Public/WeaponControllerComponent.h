// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponControllerComponent.generated.h"


class UCameraComponent;
class AWeaponActor;
class AWeaponPickupMaster;

UCLASS(ClassGroup=(BetterMovement), meta=(BlueprintSpawnableComponent))
class BETTERMOVEMENT_API UWeaponControllerComponent : public UActorComponent
{
    GENERATED_BODY()
#pragma region Functions

private:
    void EquipWeaponAnimation();
    void EquipSecondWeaponAnimation();

protected:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable)
    virtual void PickUpWeapon();

    UFUNCTION(BlueprintCallable)
    virtual void DropCurrentWeapon();

    virtual void SetWeaponVisibility();

    virtual void ResetFire();

public:
    // Sets default values for this component's properties
    UWeaponControllerComponent();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SpawnWeapon(AWeaponPickupMaster *WeaponToSpawn);

    virtual void WeaponRecoil(FRotator NewCameraRotation);

    UFUNCTION(BlueprintCallable)
    virtual void SetCurrentWeaponIndex(int WeaponIndex);

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction *ThisTickFunction) override;

    UFUNCTION(BlueprintCallable)
    void ReloadWeapon();

    UFUNCTION(BlueprintCallable)
    void FirePressed();

    UFUNCTION(BlueprintCallable)
    void FireReleased();

    UFUNCTION(BlueprintCallable)
    void SecondFirePressed();

    UFUNCTION(BlueprintCallable)
    void SecondFireReleased();

    UFUNCTION(BlueprintCallable)
    void SwapWeaponSide();

    UFUNCTION(BlueprintCallable)
    void SetLockMovement(const bool bLockMovement);


#pragma endregion Functions

#pragma region Variables

private:
    FTimerHandle FireTimerHandle;

    FTimerHandle WeaponEquipTimerHandle;

    FTimerHandle SecondWeaponEquipTimerHandle;

    FRotator CurrentCameraRotation;

    FRotator TargetCameraRotation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float DefaultCameraInterpSpeed = 8.f;

protected:
    UPROPERTY(BlueprintReadOnly, Category = Weapons)
    TArray<AWeaponActor *> Weapons;

    UPROPERTY(BlueprintReadOnly, Category = Weapons)
    AWeaponActor *CurrentWeapon;

    UPROPERTY(BlueprintReadOnly, Category = Weapons)
    AWeaponActor *SecondCurrentWeapon;

    UPROPERTY(BlueprintReadWrite, Category = Weapons)
    int32 CurrentWeaponIndex;

    UCameraComponent *FollowCamera;

    USceneComponent *WeaponHolder;
    USceneComponent *SecondWeaponHolder;

public:
    UPROPERTY(BlueprintReadOnly, Category = Character)
    ACharacter *CharacterOwner;

    FORCEINLINE void InitializeValues(UCameraComponent *NewFollowCamera, USceneComponent *NewWeaponHolder)
    {
        FollowCamera = NewFollowCamera;
        WeaponHolder = NewWeaponHolder;
    }

    FORCEINLINE void InitializeValues(UCameraComponent *NewFollowCamera, USceneComponent *NewWeaponHolder,
                                      USceneComponent *NewSecondWeaponHolder)
    {
        FollowCamera = NewFollowCamera;
        WeaponHolder = NewWeaponHolder;
        SecondWeaponHolder = NewSecondWeaponHolder;
    }

    FORCEINLINE auto GetCurrentWeapon() const { return CurrentWeapon; }
    FORCEINLINE auto GetSecondCurrentWeapon() const { return SecondCurrentWeapon; }

#pragma endregion Variables
};