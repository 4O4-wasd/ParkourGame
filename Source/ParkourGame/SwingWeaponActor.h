// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponActor.h"
#include "SwingWeaponActor.generated.h"

/**
 * 
 */
UCLASS()
class PARKOURGAME_API ASwingWeaponActor : public AWeaponActor
{
	GENERATED_BODY()

	ASwingWeaponActor();

	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void WhenWeaponFire_Implementation() override;
	virtual void OnWeaponUnequip_Implementation() override;
	virtual void FireWeapon_Implementation(bool IsPressed) override;
	virtual void OnWeaponEquip_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Swinging")
	bool ShootWeb();

	// Function to release web/grapple
	UFUNCTION(BlueprintCallable, Category = "Swinging")
	void ReleaseWeb();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swinging")
	float MaxWebLength = 1500.0f;

	// Min length of the web
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swinging")
	float MinWebLength = 100.0f;

	// Web shoot range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swinging")
	float WebShootRange = 2000.0f;

	// Forward momentum boost
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swinging")
	float SwingMomentumForce = 800.0f;

	// Centripetal force factor (keeps the swing taut)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swinging")
	float CentripetalForceFactor = 3.0f;

	// To show debug lines
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swinging")
	bool bShowDebugLines = true;

	// Web retraction speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swinging")
	float WebRetractionSpeed = 200.0f;

	// Web extension speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swinging")
	float WebExtensionSpeed = 200.0f;

	// Reduced gravity while swinging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swinging")
	float SwingingGravityScale = 0.4f;

	// Push force when releasing web
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swinging")
	float WebReleaseBoost = 500.0f;

	class AParkourCharacter* ParkourOwner;

	// Input functions
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void RetractWeb(float Value);

	// Point in world where web is attached
	FVector WebAttachPoint = FVector::Zero();

	// Current length of the web
	float CurrentWebLength;

	// Flag to check if player is swinging
	bool bIsSwinging;

	// Direction player is trying to swing
	FVector SwingInputDirection;

	// Previous position for momentum calculation
	FVector PreviousLocation;

	FVector DebugLineEnd;

	// Current swing angle
	float SwingAngle;

	// Direction of swing
	FVector SwingPlaneNormal;

	// Apply swing physics
	void ApplySwingPhysics(float DeltaTime);

	// Helper function to get target point for web shooting
	bool GetWebTargetPoint(FVector& OutHitLocation) const;

	// Calculate swing direction
	void UpdateSwingPlane();

	void RotateTowardsAttachPoint() const;
};
