// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponPickupMaster.h"
#include "WeaponControllerComponent.h"

void AWeaponPickupMaster::Interact_Implementation(UWeaponControllerComponent* Controller)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("This is an on screen message!"));
        WeaponController = Controller;
	WeaponController->SpawnWeapon(this);
}
