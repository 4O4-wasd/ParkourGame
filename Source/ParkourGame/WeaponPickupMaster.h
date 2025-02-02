// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupMaster.h"
#include "WeaponPickupMaster.generated.h"

/**
 * 
 */
UCLASS()
class PARKOURGAME_API AWeaponPickupMaster : public APickupMaster
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AWeaponActor> WeaponToSpawn;

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(class UWeaponControllerComponent* Controller);

protected:
	virtual void Interact_Implementation(UWeaponControllerComponent* Controller);

public:
        UPROPERTY(BlueprintReadOnly, Category = Controller)
        UWeaponControllerComponent* WeaponController;
};
