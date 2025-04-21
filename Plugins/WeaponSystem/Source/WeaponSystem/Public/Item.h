// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class WEAPONSYSTEM_API AItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AItem();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(class UWeaponController* Controller);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Interact_Implementation(UWeaponController* Controller);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Pickup)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Pickup)
	class USphereComponent* ItemUpSphere;

	UPROPERTY(BlueprintReadOnly, Category = Controller)
	UWeaponController* WeaponController;

public:
	FORCEINLINE auto GetItemMesh() const { return Mesh; }
	FORCEINLINE auto GetWeaponController() const { return WeaponController; }
};
