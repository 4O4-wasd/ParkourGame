// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupMaster.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
APickupMaster::APickupMaster()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// WeaponCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Object"));
	// WeaponCollisionComponent->SetupAttachment(RootComponent);
	//

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PickUp Mesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	WeaponPickUpSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickUp Radius"));
	WeaponPickUpSphere->SetupAttachment(WeaponMesh);
}

void APickupMaster::EnableCollision()
{
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetSimulatePhysics(true);

	WeaponPickUpSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
	WeaponMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
}

void APickupMaster::DisableCollision()
{
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::ProbeOnly);
	WeaponMesh->SetSimulatePhysics(false);

	WeaponPickUpSphere->SetCollisionEnabled(ECollisionEnabled::ProbeOnly);
	WeaponMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
}

// Called when the game starts or when spawned
void APickupMaster::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APickupMaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
