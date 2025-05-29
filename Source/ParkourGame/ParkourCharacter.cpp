// Fill out your copyright notice in the Description page of Project Settings.

#include "ParkourCharacter.h"

#include "BetterCharacterMovementComponent.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyUtils.h"
#include "WeaponController.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

// Sets default values
AParkourCharacter::AParkourCharacter(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer.SetDefaultSubobjectClass<UBetterCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	BetterCharacterMovement = Cast<UBetterCharacterMovementComponent>(GetCharacterMovement());
	WeaponController = CreateDefaultSubobject<UWeaponController>(TEXT("Weapon Controller"));

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 500.0f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 0.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagMaxDistance = 40;
	CameraBoom->CameraLagSpeed = 20;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

#pragma region Weapon

	WeaponParent = CreateDefaultSubobject<USceneComponent>("Weapon Parent");
	WeaponParent->SetupAttachment(FollowCamera);

	WeaponSway = CreateDefaultSubobject<USceneComponent>("Weapon Sway");
	WeaponSway->SetupAttachment(WeaponParent);

	WeaponEquip = CreateDefaultSubobject<USceneComponent>("Weapon Equip");
	WeaponEquip->SetupAttachment(WeaponSway);

	WeaponHolder = CreateDefaultSubobject<USceneComponent>("Weapon Holder");
	WeaponHolder->SetupAttachment(WeaponEquip);

#pragma endregion

#pragma region SecondWeapon

	SecondWeaponParent = CreateDefaultSubobject<USceneComponent>("Second Weapon Parent");
	SecondWeaponParent->SetupAttachment(FollowCamera);

	SecondWeaponSway = CreateDefaultSubobject<USceneComponent>("Second Weapon Sway");
	SecondWeaponSway->SetupAttachment(SecondWeaponParent);

	SecondWeaponEquip = CreateDefaultSubobject<USceneComponent>("Second Weapon Equip");
	SecondWeaponEquip->SetupAttachment(SecondWeaponSway);

	SecondWeaponHolder = CreateDefaultSubobject<USceneComponent>("Second Weapon Holder");
	SecondWeaponHolder->SetupAttachment(SecondWeaponEquip);

#pragma endregion

	WeaponSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	WeaponSkeletalMesh->SetupAttachment(GetMesh(), FName("WeaponSocket"));

	WeaponController->InitializeValues(FollowCamera, WeaponHolder, SecondWeaponHolder);
}

// Called when the game starts or when spawned
void AParkourCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const auto PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AParkourCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!WeaponController)
	{
		UMyUtils::PrintToScreen(FString("WeaponController is not valid"));
	}

	WeaponsSway();

	// CurrentWeapon = Weapons[CurrentWeaponIndex];
	// SetWeaponVisibilty();
	//
	// if (!CurrentWeapon)
	// {
	//     FollowCamera->SetRelativeRotation(FMath::RInterpTo(FollowCamera->GetRelativeRotation(), TargetCameraRotation,
	//         GetWorld()->GetDeltaSeconds(), 8.f));
	// }
}

void AParkourCharacter::JumpVault()
{
	if (BetterCharacterMovement->GetCurrentCustomMovementMode() == WallRunning)
	{
		BetterCharacterMovement->WallJump();
		return;
	}
	FVector _;
	if (!BetterCharacterMovement->CanVault(_))
	{
		Jump();
		return;
	}

	BetterCharacterMovement->Vault();
}

void AParkourCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const auto MovementVector = Value.Get<FVector2D>();

	if (bLockMovement)
	{
		return;
	}

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
		BetterCharacterMovement->SetMovementInput(MovementVector);
		MoveVector = MovementVector;
	}
}

void AParkourCharacter::StopMove()
{
	BetterCharacterMovement->SetMovementInput(FVector2d::Zero());
	MoveVector = FVector2D::Zero();
}

void AParkourCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	const auto LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
		LookVector = LookAxisVector;
	}
}

void AParkourCharacter::WeaponsSway()
{
	constexpr auto InitRotation = FRotator();

	const auto FinalRotation = FRotator(
		LookVector.X * SwayDegree,
		LookVector.Y * SwayDegree,
		LookVector.Y * SwayDegree
	);

	const auto NewRotation = UKismetMathLibrary::RInterpTo(
		WeaponSway->GetRelativeRotation(),
		FRotator(
			InitRotation.Pitch - FinalRotation.Pitch,
			InitRotation.Yaw - FinalRotation.Yaw,
			InitRotation.Roll + FinalRotation.Roll
		),
		GetWorld()->GetDeltaSeconds(),
		2.5
	);
	WeaponSway->SetRelativeRotation(
		FRotator(
			UKismetMathLibrary::FClamp(NewRotation.Pitch, -MaxSwayDegree, MaxSwayDegree),
			UKismetMathLibrary::FClamp(NewRotation.Yaw, -MaxSwayDegree, MaxSwayDegree),
			UKismetMathLibrary::FClamp(NewRotation.Roll, -MaxSwayDegree, MaxSwayDegree)
		)
	);
}

// Called to bind functionality to input
void AParkourCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AParkourCharacter::JumpVault);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AParkourCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AParkourCharacter::StopMove);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AParkourCharacter::Look);

		// Sprinting
		EnhancedInputComponent->BindAction(
			SprintAction,
			ETriggerEvent::Started, BetterCharacterMovement,
			&UBetterCharacterMovementComponent::SprintPressed
		);
		EnhancedInputComponent->BindAction(
			SprintAction,
			ETriggerEvent::Completed,
			BetterCharacterMovement,
			&UBetterCharacterMovementComponent::SprintReleased
		);

		// Crouching
		EnhancedInputComponent->BindAction(
			CrouchAction,
			ETriggerEvent::Started,
			BetterCharacterMovement,
			&UBetterCharacterMovementComponent::CrouchPressed
		);
		EnhancedInputComponent->BindAction(
			CrouchAction,
			ETriggerEvent::Completed,
			BetterCharacterMovement,
			&UBetterCharacterMovementComponent::CrouchReleased
		);

		// Dashing
		EnhancedInputComponent->BindAction(
			DashAction,
			ETriggerEvent::Started,
			BetterCharacterMovement,
			&UBetterCharacterMovementComponent::DashPressed
		);

		EnhancedInputComponent->BindAction(
			EquipAction,
			ETriggerEvent::Started,
			WeaponController,
			&UWeaponController::PickUpAPickup
		);

		EnhancedInputComponent->BindAction(
			UnEquipAction,
			ETriggerEvent::Started,
			WeaponController,
			&UWeaponController::DropCurrentItem
		);

		// Firing
		EnhancedInputComponent->BindAction(
			AttackAction,
			ETriggerEvent::Started,
			WeaponController,
			&UWeaponController::AttackPressed
		);
		EnhancedInputComponent->BindAction(
			AttackAction,
			ETriggerEvent::Completed,
			WeaponController,
			&UWeaponController::AttackReleased
		);

		EnhancedInputComponent->BindAction(
			SecondAttackAction,
			ETriggerEvent::Started,
			WeaponController,
			&UWeaponController::SecondAttackPressed
		);
		EnhancedInputComponent->BindAction(
			SecondAttackAction,
			ETriggerEvent::Completed,
			WeaponController,
			&UWeaponController::SecondAttackReleased
		);
	}
	else
	{
		UE_LOG(
			LogTemplateCharacter,
			Error,
			TEXT(
				"'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
			), *GetNameSafe(this)
		);
	}
}
