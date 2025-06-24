// 

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "SwingWeapon.generated.h"

UCLASS()
class WEAPONSYSTEM_API ASwingWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASwingWeapon();
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category="Weapon Settings")
	bool IsSwingWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Weapon: Settings")
	float WeaponRotationTime = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Weapon: Web Settings")
	UStaticMeshComponent* WebMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Weapon: Web Settings")
	float MaxWebLength = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Weapon: Web Settings")
	float MinWebLength = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Weapon: Web Settings")
	float WebShootRange = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Weapon: Web Settings")
	float WebRetractionSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Weapon: Web Settings")
	float WebExtensionSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Weapon: Web Settings")
	float WebReleaseBoost = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Swing Weapon: Socket Names")
	FString FireMuzzleSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Weapon: Physics Settings")
	float SwingMomentumForce = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Weapon: Physics Settings")
	float CentripetalForceFactor = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Weapon: Physics Settings")
	float SwingGravityScale = 0.4f;

	virtual void BeginPlay() override;

	// Input functions
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void RetractWeb(float Value);
	bool ShootWeb();
	void ReleaseWeb();

	virtual void AttackButtonPressed_Implementation() override;
	virtual void AttackButtonReleased_Implementation() override;

	virtual void OnWeaponEquip_Implementation() override;
	virtual void OnWeaponUnEquip_Implementation() override;


	FVector WebAttachPoint = FVector::Zero();
	float CurrentWebLength;
	bool bIsSwinging;
	FVector SwingInputDirection;
	FVector PreviousLocation;
	FVector DebugLineEnd;
	float SwingAngle;
	FVector SwingPlaneNormal;

	// Apply swing physics
	void ApplySwingPhysics(float DeltaTime);

	// Helper function to get target point for web shooting
	bool GetWebTargetPoint(FVector& OutHitLocation) const;

	// Calculate swing direction
	void UpdateSwingPlane();

	void RotateTowardsAttachPoint() const;
};
