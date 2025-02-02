// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupMaster.generated.h"

UCLASS()
class PARKOURGAME_API APickupMaster : public AActor
{
	GENERATED_BODY()

protected:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pickup, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pickup, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* WeaponPickUpSphere;

public:	
	//Sets default values for this actor's properties
	APickupMaster();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// virtual void Interact(class AParkourCharacter* CharacterThatWillPickItUp);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	virtual void EnableCollision();

	UFUNCTION(BlueprintCallable)
	virtual void DisableCollision();

private:

public:
	FORCEINLINE auto GetWeaponMesh() { return WeaponMesh; }
};
