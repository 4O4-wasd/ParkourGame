// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MyUtils.generated.h"

/**
 * 
 */
UCLASS()
class PARKOURGAME_API UMyUtils : public UObject
{
	GENERATED_BODY()

public:
	// Static function to calculate the angle between two vectors in degrees
	UFUNCTION(BlueprintCallable, Category = "Utils")
	static float GetAngleBetweenVectors(const FVector& VectorA, const FVector& VectorB);

	UFUNCTION(BlueprintCallable, Category = "Utils")
	static FRotator VectorToRotator(const FVector& Direction);

	// Static function to print a message to the screen (like PrintString)
	UFUNCTION(BlueprintCallable, Category = "Utils")
	static void PrintToScreen(const FString& Message, float TimeToDisplay = 2.0f, FColor TextColor = FColor::White);

	// Overload for FVector
	static void PrintToScreen(const FVector& Vector, float TimeToDisplay = 2.0f, FColor TextColor = FColor::White);

	// Overload for FRotator
	static void PrintToScreen(const FRotator& Rotator, float TimeToDisplay = 2.0f, FColor TextColor = FColor::White);

	// Overload for FString (just for clarity and reusability)
	static void PrintToScreen(const FName& Name, float TimeToDisplay = 2.0f, FColor TextColor = FColor::White);

	// Static function to print a message to the log
	UFUNCTION(BlueprintCallable, Category = "Utils")
	static void PrintToLog(const FString& Message);

	// Overload for FVector
	static void PrintToLog(const FVector& Vector);

	// Overload for FRotator
	static void PrintToLog(const FRotator& Rotator);

	// Overload for FName
	static void PrintToLog(const FName& Name);
};
