#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class UBoxComponent;

UCLASS()
class WEAPONSYSTEM_API AItem : public AActor
{
	GENERATED_BODY()

public:
	AItem();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(class UWeaponController* Controller);

protected:
	virtual void BeginPlay() override;

	virtual void Interact_Implementation(UWeaponController* Controller);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pickup)
	UBoxComponent* CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pickup)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Pickup)
	class USphereComponent* ItemUpSphere;

	UPROPERTY(BlueprintReadOnly, Category = Controller)
	UWeaponController* WeaponController;

public:
	FORCEINLINE auto GetItemMesh() const { return Mesh; }
	FORCEINLINE auto GetCollisionComponent() const { return CollisionComponent; }
	FORCEINLINE auto GetWeaponController() const { return WeaponController; }
	
	UFUNCTION(BlueprintCallable)
	void ClearItem();
};
