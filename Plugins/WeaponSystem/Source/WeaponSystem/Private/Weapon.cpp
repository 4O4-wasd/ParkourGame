
#include "Weapon.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"


AWeapon::AWeapon():
	WeaponIcon(nullptr),
	WeaponDefaultRelativeLocation(0, 0, 0),
	WeaponDefaultRelativeRotation(0, 0, 0),
	PlayerCamera(nullptr)
{
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

void AWeapon::EnableSettingsForShowing() const
{
	CollisionComponent->SetSimulatePhysics(false);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ItemUpSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
