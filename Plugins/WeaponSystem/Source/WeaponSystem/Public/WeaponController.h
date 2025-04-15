// 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponController.generated.h"


class UCameraComponent;
class AWeapon;
class AItem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WEAPONSYSTEM_API UWeaponController : public UActorComponent
{
	GENERATED_BODY()
#pragma region Functions

private:
	void EquipWeaponAnimation();
	void EquipSecondWeaponAnimation();

protected:
	virtual void BeginPlay() override;

	void DropAnItem(AWeapon*& WeaponToDrop, const bool IsASecondWeapon = false);

	virtual void SetWeaponVisibility();

public:
	// Sets default values for this component's properties
	UWeaponController();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InteractWithPickup(AItem* Pickup);

	UFUNCTION(BlueprintCallable)
	virtual void SetCurrentWeaponIndex(int WeaponIndex);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void ReloadWeapon() const;

	UFUNCTION(BlueprintCallable)
	void AttackPressed();

	UFUNCTION(BlueprintCallable)
	void AttackReleased();

	UFUNCTION(BlueprintCallable)
	void SecondAttackPressed();

	UFUNCTION(BlueprintCallable)
	void SecondAttackReleased();

	UFUNCTION(BlueprintCallable)
	void SwapItemSide();

	UFUNCTION(BlueprintCallable)
	virtual void PickUpAPickup();

	UFUNCTION(BlueprintCallable)
	virtual void DropCurrentItem();


#pragma endregion Functions

#pragma region Variables

private:
	FTimerHandle WeaponEquipTimerHandle;

	FTimerHandle WeaponUnEquipTimerHandle;

	FTimerHandle SecondWeaponEquipTimerHandle;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
	float DefaultCameraInterpSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
	float MaxSwayDegree = 2.5;

	UPROPERTY(BlueprintReadOnly, Category = Weapons)
	TArray<AWeapon*> Items;

	UPROPERTY(BlueprintReadOnly, Category = Weapons)
	AWeapon* CurrentItem;

	UPROPERTY(BlueprintReadOnly, Category = Weapons)
	AWeapon* SecondCurrentItem;

	UPROPERTY(BlueprintReadWrite, Category = Weapons)
	uint8 CurrentItemIndex;

	UCameraComponent* FollowCamera;

	USceneComponent* WeaponHolder;
	USceneComponent* SecondWeaponHolder;

	USceneComponent* WeaponSwayHolder;
	USceneComponent* SecondWeaponSwayHolder;

public:
	UPROPERTY(BlueprintReadOnly, Category = Character)
	ACharacter* CharacterOwner;

	FORCEINLINE void InitializeValues(UCameraComponent* NewFollowCamera, USceneComponent* NewWeaponHolder)
	{
		FollowCamera = NewFollowCamera;
		WeaponHolder = NewWeaponHolder;
	}

	FORCEINLINE void InitializeValues(UCameraComponent* NewFollowCamera, USceneComponent* NewWeaponHolder,
	                                  USceneComponent* NewSecondWeaponHolder)
	{
		FollowCamera = NewFollowCamera;
		WeaponHolder = NewWeaponHolder;
		SecondWeaponHolder = NewSecondWeaponHolder;
	}

	FORCEINLINE auto GetCurrentWeapon() const { return CurrentItem; }
	FORCEINLINE auto GetSecondCurrentWeapon() const { return SecondCurrentItem; }
	FORCEINLINE auto GetFollowCamera() const { return FollowCamera; }

#pragma endregion Variables
};
