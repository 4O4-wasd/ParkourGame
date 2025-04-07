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
	UTexture2D* WeaponIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	FString FireMuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	FString ShellEjectSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	FVector WeaponDefaultRelativeLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	FVector WeaponDefaultRelativeRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	FVector WeaponPushbackRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	FVector WeaponRotationRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	FVector WeaponCameraRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	int DefaultMagazineSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	float RecoilSnappiness;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	float RecoilReturnSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	float MaxRandomSpreadAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	bool bCanBeEquipedInSecondHand = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	USoundBase* ShellEjectSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EFireModeType> FireModeType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	float WeaponFireDelay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UAnimationAsset* ReloadAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UAnimationAsset* FireWeaponAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* MuzzleFlashSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* ShellEjectSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* ImpactPointSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* BulletTraceSystem;

public:
	UPROPERTY(BlueprintReadWrite, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* PlayerCamera;

	///////////////////////////////////////////////////////////

	FORCEINLINE auto GetCanShoot() const { return bCanShoot; }

	FORCEINLINE auto GetCanBeEquipedInSecondHand() const { return bCanBeEquipedInSecondHand; }

	FORCEINLINE void SetCanShoot(bool canShoot) { bCanShoot = canShoot; }

	FORCEINLINE auto GetIsReloading() const { return bIsReloading; }

	FORCEINLINE auto GetDamage() const { return Damage; }

	FORCEINLINE auto GetRange() const { return Range; }

	FORCEINLINE auto GetWeaponName() { return WeaponName; }

	FORCEINLINE auto GetFireMuzzleSocketName() { return FireMuzzleSocketName; }

	FORCEINLINE auto GetWeaponDefaultRelativeLocation() const { return WeaponDefaultRelativeLocation; }

	FORCEINLINE auto GetWeaponDefaultRelativeRotation() const { return WeaponDefaultRelativeRotation; }

	FORCEINLINE auto GetWeaponRecoil() const { return WeaponCameraRecoil; }

	FORCEINLINE auto GetRecoilSnappiness() const { return RecoilSnappiness; }

	FORCEINLINE auto GetRecoilReturnSpeed() const { return RecoilReturnSpeed; }

	FORCEINLINE auto GetMaxRandomSpreadAngle() const { return MaxRandomSpreadAngle; }

	FORCEINLINE auto GetFireSound() const { return FireSound; }

	FORCEINLINE auto GetFireModeType() const { return FireModeType; }

	FORCEINLINE auto GetWeaponFireDelay() const { return WeaponFireDelay; }

	FORCEINLINE auto GetReloadAnimation() const { return ReloadAnimation; }

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

	UPROPERTY(BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
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
