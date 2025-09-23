#include "MyUtils.h"

float UMyUtils::GetAngleBetweenVectors(const FVector& VectorA, const FVector& VectorB)
{
	const FVector NormVector1 = VectorA.GetSafeNormal();
	const FVector NormVector2 = VectorB.GetSafeNormal();

	float DotProduct = FVector::DotProduct(NormVector1, NormVector2);

	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);

	const float AngleInRadians = FMath::Acos(DotProduct);

	float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);

	const FVector CrossProduct = FVector::CrossProduct(NormVector1, NormVector2);

	return AngleInDegrees;
}

FRotator UMyUtils::VectorToRotator(const FVector& Direction)
{
	FVector NormalizedDirection = Direction.GetSafeNormal();

	FRotator Rotator = NormalizedDirection.ToOrientationRotator();

	return Rotator;
}

void UMyUtils::PrintToScreen(const FString& Message, float TimeToDisplay, FColor TextColor)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, TextColor, Message);
	}
}

void UMyUtils::PrintToScreen(const FVector& Vector, const float TimeToDisplay, const FColor TextColor)
{
	const FString VectorString = Vector.ToString();
	PrintToScreen(VectorString, TimeToDisplay, TextColor);
}

void UMyUtils::PrintToScreen(const FRotator& Rotator, const float TimeToDisplay, const FColor TextColor)
{
	const FString RotatorString = Rotator.ToString();
	PrintToScreen(RotatorString, TimeToDisplay, TextColor);
}

void UMyUtils::PrintToScreen(const FName& Name, const float TimeToDisplay, const FColor TextColor)
{
	const FString NameString = Name.ToString();
	PrintToScreen(NameString, TimeToDisplay, TextColor);
}

void UMyUtils::PrintToLog(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}

void UMyUtils::PrintToLog(const FVector& Vector)
{
	const FString VectorString = Vector.ToString();
	PrintToLog(VectorString);
}

void UMyUtils::PrintToLog(const FRotator& Rotator)
{
	const FString RotatorString = Rotator.ToString();
	PrintToLog(RotatorString);
}

void UMyUtils::PrintToLog(const FName& Name)
{
	const FString NameString = Name.ToString();
	PrintToLog(NameString);
}
