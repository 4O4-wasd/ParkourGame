// 


#include "HealthComponent.h"

#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UHealthComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UHealthComponent::Damage(const float DamagePoints = 0.f)
{
	Health -= DamagePoints;
	if (Health <= 0.f)
	{
		Health = 0.f;

		// Broadcast the death event
		OnDies.Broadcast();
	}
}
