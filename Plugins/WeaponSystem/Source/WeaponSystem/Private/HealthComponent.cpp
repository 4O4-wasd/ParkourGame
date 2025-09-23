
#include "HealthComponent.h"

#include "Kismet/GameplayStatics.h"


UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}


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

		OnDies.Broadcast();
	}
}
