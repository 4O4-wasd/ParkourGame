// 


#include "Item.h"

#include "WeaponController.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"


// Sets default values
AItem::AItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

// Called when the game starts or when spawned
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

// Called every frame
void AItem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}
