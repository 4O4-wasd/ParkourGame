#include "MyUtils.h"

float UMyUtils::GetAngleBetweenVectors(const FVector& VectorA, const FVector& VectorB)
{
	// Normalize both vectors
	const FVector NormVector1 = VectorA.GetSafeNormal();
	const FVector NormVector2 = VectorB.GetSafeNormal();

	// Get the dot product of the two normalized vectors
	float DotProduct = FVector::DotProduct(NormVector1, NormVector2);

	// Clamp the dot product to ensure it's within the valid range for acos [-1, 1]
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);

	// Calculate the angle in radians using the inverse cosine (acos)
	const float AngleInRadians = FMath::Acos(DotProduct);

	// Convert the angle to degrees
	float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);

	// Now we need to use the cross product to determine the sign of the angle
	const FVector CrossProduct = FVector::CrossProduct(NormVector1, NormVector2);

	// If the cross product is in the negative Z direction, the angle should be subtracted
	// if (CrossProduct.Z < 0.0f)
	// {
	// 	AngleInDegrees = 360.0f - AngleInDegrees;
	// }

	return AngleInDegrees;
}

FRotator UMyUtils::VectorToRotator(const FVector& Direction)
{
	// Normalize the direction vector to ensure it's a unit vector
	FVector NormalizedDirection = Direction.GetSafeNormal();

	// Convert the normalized direction vector to a rotator
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

// Overload for FVector - Converts FVector to a string and prints to the screen
void UMyUtils::PrintToScreen(const FVector& Vector, const float TimeToDisplay, const FColor TextColor)
{
	const FString VectorString = Vector.ToString(); // Converts FVector to a string
	PrintToScreen(VectorString, TimeToDisplay, TextColor);
}

// Overload for FRotator - Converts FRotator to a string and prints to the screen
void UMyUtils::PrintToScreen(const FRotator& Rotator, const float TimeToDisplay, const FColor TextColor)
{
	const FString RotatorString = Rotator.ToString(); // Converts FRotator to a string
	PrintToScreen(RotatorString, TimeToDisplay, TextColor);
}

// Overload for FName - Converts FName to a string and prints to the screen
void UMyUtils::PrintToScreen(const FName& Name, const float TimeToDisplay, const FColor TextColor)
{
	const FString NameString = Name.ToString(); // Converts FName to a string
	PrintToScreen(NameString, TimeToDisplay, TextColor);
}

// Function to print a message to the log (FString version)
void UMyUtils::PrintToLog(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}

// Overload for FVector - Converts FVector to a string and logs to the output
void UMyUtils::PrintToLog(const FVector& Vector)
{
	const FString VectorString = Vector.ToString(); // Converts FVector to a string
	PrintToLog(VectorString);
}

// Overload for FRotator - Converts FRotator to a string and logs to the output
void UMyUtils::PrintToLog(const FRotator& Rotator)
{
	const FString RotatorString = Rotator.ToString(); // Converts FRotator to a string
	PrintToLog(RotatorString);
}

// Overload for FName - Converts FName to a string and logs to the output
void UMyUtils::PrintToLog(const FName& Name)
{
	const FString NameString = Name.ToString(); // Converts FName to a string
	PrintToLog(NameString);
}
