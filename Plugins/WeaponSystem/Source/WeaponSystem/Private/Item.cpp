#include "Item.h"

#include "WeaponController.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"


AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Component"));
	RootComponent = (CollisionComponent);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	ItemUpSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickUp Sphere"));
	ItemUpSphere->SetupAttachment(Mesh);

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->BodyInstance.bOverrideMass = true;
	CollisionComponent->BodyInstance.SetMassOverride(50.f);
	
	ItemUpSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
	ItemUpSphere->InitSphereRadius(150);
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
}

void AItem::Interact_Implementation(UWeaponController* Controller)
{
	if (Controller)
	{
		WeaponController = Controller;
		Controller->InteractWithPickup(this);
	}
}

void AItem::ClearItem()
{
	WeaponController->ClearItem();
}

void AItem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}
