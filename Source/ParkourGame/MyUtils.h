#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MyUtils.generated.h"

UCLASS()
class PARKOURGAME_API UMyUtils : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Utils")
	static float GetAngleBetweenVectors(const FVector& VectorA, const FVector& VectorB);

	UFUNCTION(BlueprintCallable, Category = "Utils")
	static FRotator VectorToRotator(const FVector& Direction);

	UFUNCTION(BlueprintCallable, Category = "Utils")
	static void PrintToScreen(const FString& Message, float TimeToDisplay = 2.0f, FColor TextColor = FColor::White);

	static void PrintToScreen(const FVector& Vector, float TimeToDisplay = 2.0f, FColor TextColor = FColor::White);

	static void PrintToScreen(const FRotator& Rotator, float TimeToDisplay = 2.0f, FColor TextColor = FColor::White);

	static void PrintToScreen(const FName& Name, float TimeToDisplay = 2.0f, FColor TextColor = FColor::White);

	UFUNCTION(BlueprintCallable, Category = "Utils")
	static void PrintToLog(const FString& Message);

	static void PrintToLog(const FVector& Vector);

	static void PrintToLog(const FRotator& Rotator);

	static void PrintToLog(const FName& Name);
};
