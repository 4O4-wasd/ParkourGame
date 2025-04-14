// 

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "RangeWeapon.generated.h"

class UInputAction;
class UNiagaraSystem;

UENUM()
enum EGunFireModeType
{
	Automatic UMETA(DisplayName = "Automatic"),
	Single UMETA(DisplayName = "Single")
};

UCLASS()
class WEAPONSYSTEM_API ARangeWeapon : public AWeapon
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category="Weapon Settings")
	bool IsGunWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Fire Settings")
	uint8 Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Fire Settings")
	float Range;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Fire Settings")
	uint8 BulletsPerShot = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Fire Settings")
	float MaxRandomSpreadAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Fire Settings")
	TEnumAsByte<EGunFireModeType> FireModeType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Fire Settings")
	float WeaponFireDelay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Fire Settings")
	uint8 DefaultMagazineSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Socket Names")
	FString FireMuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Socket Names")
	FString ShellEjectSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Recoil")
	FVector WeaponPushbackRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Recoil")
	FVector WeaponRotationRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Recoil")
	FVector WeaponCameraRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Recoil")
	float RecoilSnappiness;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Recoil")
	float RecoilReturnSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: SFX")
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: SFX")
	USoundBase* ShellEjectSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Animation")
	UAnimationAsset* ReloadAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Animation")
	UAnimationAsset* FireWeaponAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: VFX")
	UNiagaraSystem* MuzzleFlashSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: VFX")
	UNiagaraSystem* ShellEjectSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: VFX")
	UNiagaraSystem* ImpactPointSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: VFX")
	UNiagaraSystem* BulletTraceSystem;

private:
	FRotator CurrentCameraRotation;

	FRotator TargetCameraRotation;

	FVector CurrentRelativeLocation;

	FVector TargetRelativeLocation;

	FRotator CurrentRelativeRotation;

	FRotator TargetRelativeRotation;

	FTimerHandle FireTimerHandle;

	FTimerHandle FireDelayTimerHandle;

protected:
	UFUNCTION(BlueprintCallable)
	void Recoil();

	UPROPERTY(BlueprintReadOnly, Category = "Range Weapon: Properties")
	bool bIsReloading;

	UPROPERTY(BlueprintReadOnly, Category = "Range Weapon: Properties")
	bool bCanShoot = true;

	UPROPERTY(BlueprintReadOnly, Category = "Range Weapon: Properties")
	uint8 CurrentAmmo;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnWeaponFire();

	virtual void OnWeaponFire_Implementation();

	virtual void OnWeaponUnEquip_Implementation() override;

	void FireWeapon(bool IsPressed);

	virtual void ResetFire();

public:
	// Sets default values for this character's properties
	ARangeWeapon();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ReloadWeapon();

	static FRotator VectorToRotator(const FVector& Direction)
	{
		// Normalize the direction vector to ensure it's a unit vector
		const FVector NormalizedDirection = Direction.GetSafeNormal();

		// Convert the normalized direction vector to a rotator
		const FRotator Rotator = NormalizedDirection.ToOrientationRotator();

		return Rotator;
	}

	virtual void AttackButtonPressed_Implementation() override;
	virtual void AttackButtonReleased_Implementation() override;

public:
	///////////////////////////////////////////////////////////

	FORCEINLINE auto GetDamage() const { return Damage; }

	FORCEINLINE auto GetIsReloading() const { return bIsReloading; }

	FORCEINLINE auto GetRange() const { return Range; }

	FORCEINLINE auto GetFireMuzzleSocketName() { return FireMuzzleSocketName; }

	FORCEINLINE auto GetWeaponRecoil() const { return WeaponCameraRecoil; }

	FORCEINLINE auto GetRecoilSnappiness() const { return RecoilSnappiness; }

	FORCEINLINE auto GetRecoilReturnSpeed() const { return RecoilReturnSpeed; }

	FORCEINLINE auto GetMaxRandomSpreadAngle() const { return MaxRandomSpreadAngle; }

	FORCEINLINE auto GetFireSound() const { return FireSound; }

	FORCEINLINE auto GetFireModeType() const { return FireModeType; }

	FORCEINLINE auto GetWeaponFireDelay() const { return WeaponFireDelay; }

	FORCEINLINE auto GetReloadAnimation() const { return ReloadAnimation; }
};
