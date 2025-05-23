// 


#include "Weapon.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"


// Sets default values
AWeapon::AWeapon():
	WeaponIcon(nullptr),
	WeaponDefaultRelativeLocation(0, 0, 0),
	WeaponDefaultRelativeRotation(0, 0, 0),
	PlayerCamera(nullptr)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetActorRelativeLocation(WeaponDefaultRelativeLocation);
	SetActorRelativeRotation(VectorToRotator(WeaponDefaultRelativeRotation));
}

void AWeapon::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AWeapon::Interact_Implementation(UWeaponController* Controller)
{
	Super::Interact_Implementation(Controller);
}

void AWeapon::OnWeaponEquip_Implementation()
{
}

void AWeapon::OnWeaponUnEquip_Implementation()
{
}

void AWeapon::AttackButtonPressed_Implementation()
{
}

void AWeapon::AttackButtonReleased_Implementation()
{
}

void AWeapon::EnableSettingsForEquipping() const
{
	CollisionComponent->SetSimulatePhysics(false);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ItemUpSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
}

void AWeapon::EnableSettingsForThrowing() const
{
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetSimulatePhysics(true);

	ItemUpSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
}
