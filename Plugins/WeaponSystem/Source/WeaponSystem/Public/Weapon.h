// 

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

UCLASS()
class WEAPONSYSTEM_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeapon();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnWeaponEquip();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnWeaponUnEquip();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AttackButtonPressed();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AttackButtonReleased();

	void EnableSettingsForEquipping() const;

	void EnableSettingsForThrowing() const;


	static FRotator VectorToRotator(const FVector& Direction)
	{
		// Normalize the direction vector to ensure it's a unit vector
		const FVector NormalizedDirection = Direction.GetSafeNormal();

		// Convert the normalized direction vector to a rotator
		const FRotator Rotator = NormalizedDirection.ToOrientationRotator();

		return Rotator;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Interact_Implementation(UWeaponController* Controller) override;

	virtual void OnWeaponEquip_Implementation();

	virtual void OnWeaponUnEquip_Implementation();

	virtual void AttackButtonPressed_Implementation();

	virtual void AttackButtonReleased_Implementation();

protected:
	// Parameters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Settings", meta = (AllowPrivateAccess = "true"))
	FString WeaponName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Settings", meta = (AllowPrivateAccess = "true"))
	UTexture2D* WeaponIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Settings", meta = (AllowPrivateAccess = "true"))
	FVector WeaponDefaultRelativeLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Settings", meta = (AllowPrivateAccess = "true"))
	FVector WeaponDefaultRelativeRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Settings", meta = (AllowPrivateAccess = "true"))
	bool bCanBeEquippedInSecondHand = true;

	class UCameraComponent* PlayerCamera;

public:
	FORCEINLINE auto GetWeaponName() const { return WeaponName; }

	FORCEINLINE auto GetWeaponIcon() const { return WeaponIcon; }

	FORCEINLINE auto GetWeaponDefaultRelativeLocation() const { return WeaponDefaultRelativeLocation; }

	FORCEINLINE auto GetWeaponDefaultRelativeRotation() const { return WeaponDefaultRelativeRotation; }

	FORCEINLINE auto GetCanBeEquippedInSecondHand() const { return bCanBeEquippedInSecondHand; }

	FORCEINLINE void SetPlayerCamera(UCameraComponent* Camera) { PlayerCamera = Camera; }

	FORCEINLINE void ResetWeaponController() { WeaponController = nullptr; }
};
