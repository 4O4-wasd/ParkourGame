#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "RangeWeapon.generated.h"

class UCurveVector;
class UInputAction;
class UNiagaraSystem;
class UTimelineComponent;

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
	TEnumAsByte<EGunFireModeType> FireModeType = Single;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Fire Settings")
	float WeaponFireDelay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Fire Settings")
	uint8 DefaultMagazineSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Socket Names")
	FString FireMuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Socket Names")
	FString ShellEjectSocketName;

	UTimelineComponent* PushbackRecoilTimelineComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Recoil")
	UCurveVector* PushbackRecoilCurve;

	UTimelineComponent* RotationRecoilTimelineComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range Weapon: Recoil")
	UCurveVector* RotationRecoilCurve;

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

	UPROPERTY(BlueprintReadOnly, Category = "Range Weapon: Properties")
	bool bIsReloading;

	UPROPERTY(BlueprintReadOnly, Category = "Range Weapon: Properties")
	bool bCanShoot = true;

	UPROPERTY(BlueprintReadOnly, Category = "Range Weapon: Properties")
	uint8 CurrentAmmo;

private:
	FRotator CurrentCameraRotation;

	FRotator TargetCameraRotation;

	FVector CurrentRelativeLocation;

	FVector TargetRelativeLocation;

	FRotator CurrentRelativeRotation;

	FRotator TargetRelativeRotation;

	FTimerHandle FireTimerHandle;

	FTimerHandle FireDelayTimerHandle;

	FTimerHandle ResetWeaponTransformTimerHandle;

	FVector BaseRelativeLocation;
	FRotator BaseRelativeRotation;

	bool bIsShooting = false;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void PushbackRecoil(const FVector& OutVector);

	UFUNCTION()
	void RotationRecoil(const FVector& OutVector);

	UFUNCTION()
	void RecoilTimelineFinished();

	void Recoil();

	virtual void Fire();

	virtual void OnWeaponUnEquip_Implementation() override;

	virtual void StartFiring(bool IsPressed);

	virtual void ResetFire();

	virtual void AttackButtonPressed_Implementation() override;

	virtual void AttackButtonReleased_Implementation() override;

public:
	ARangeWeapon();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ReloadWeapon();

	static FRotator ClampRotator(const FRotator& Input, const float MinPitch, const float MaxPitch, const float MinYaw,
	                             const float MaxYaw, const float MinRoll, const float MaxRoll)
	{
		FRotator Clamped = Input;

		Clamped.Pitch = FMath::Clamp(Clamped.Pitch, MinPitch, MaxPitch);
		Clamped.Yaw = FMath::Clamp(Clamped.Yaw, MinYaw, MaxYaw);
		Clamped.Roll = FMath::Clamp(Clamped.Roll, MinRoll, MaxRoll);

		Clamped.Normalize();

		return Clamped;
	}

	static float FInterpWithCurve(const float Current, const float Target, const float DeltaTime, float& ElapsedTime,
	                              const UCurveFloat* Curve)
	{
		if (!Curve)
		{
			return FMath::FInterpTo(Current, Target, DeltaTime, 5.0f);
		}
		const auto a = UCurveFloat();


		ElapsedTime += DeltaTime;

		float CurveDuration = 0.f;
		const TArray<FRichCurveKey>& Keys = Curve->FloatCurve.GetConstRefOfKeys();
		if (Keys.Num() > 0)
		{
			CurveDuration = Keys.Last().Time;
		}

		const float ClampedTime = FMath::Clamp(ElapsedTime, 0.f, CurveDuration);

		const float Alpha = Curve->GetFloatValue(ClampedTime);

		return FMath::Lerp(Current, Target, Alpha);
	}


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
