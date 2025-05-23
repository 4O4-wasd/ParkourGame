// 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDies);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WEAPONSYSTEM_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Properties, meta = (AllowPrivateAccess = "true"))
	float Health = 100.f;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnDies OnDies;

	UFUNCTION(BlueprintCallable)
	void Damage(float DamagePoints);
};
