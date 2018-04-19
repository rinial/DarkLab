// Fill out your copyright notice in the Description page of Project Settings.

#include "MainGameMode.h"
#include "EngineUtils.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "UObject/UObjectIterator.h"
#include "DrawDebugHelpers.h"

// Returns the light level on the position
float AMainGameMode::GetLightingAmount(const AActor* actor, const bool sixPoints, const float sixPointsRadius)
{
	if (!actor)
		return 0.0f;
	return GetLightingAmount(actor, actor->GetActorLocation(), sixPoints, sixPointsRadius);
}
float AMainGameMode::GetLightingAmount(const FVector location, const bool sixPoints, const float sixPointsRadius)
{
	return GetLightingAmount(nullptr, location, sixPoints, sixPointsRadius);
}
float AMainGameMode::GetLightingAmount(const TArray<FVector> locations)
{
	return GetLightingAmount(nullptr, locations);
}
float AMainGameMode::GetLightingAmount(const AActor* actor, const FVector location, const bool sixPoints, const float sixPointsRadius)
{
	TArray<FVector> locations;
	locations.Add(location);
	if(sixPoints)
	{
		// We add six locations around the point
		locations.Add(location + FVector::UpVector * sixPointsRadius);
		locations.Add(location - FVector::UpVector * sixPointsRadius);
		locations.Add(location + FVector::RightVector * sixPointsRadius);
		locations.Add(location - FVector::RightVector * sixPointsRadius);
		locations.Add(location + FVector::ForwardVector * sixPointsRadius);
		locations.Add(location - FVector::ForwardVector * sixPointsRadius);
	}
	return GetLightingAmount(actor, locations);
}
float AMainGameMode::GetLightingAmount(const AActor* actor, const TArray<FVector> locations)
{
	FCollisionQueryParams params = FCollisionQueryParams(FName(TEXT("LightTrace")), true);
	// Add actor and all of its components as ignored
	if (actor)
	{		
		params.AddIgnoredActor(actor);
		TInlineComponentArray<UActorComponent*> components;
		actor->GetComponents(components, true);
		for (UActorComponent* component : components)
			params.AddIgnoredComponent(Cast<UPrimitiveComponent>(component));
	}

	float result = 0.0f;

	// First we find all the point lights (spot lights count as point lights)
	TArray<UPointLightComponent*> pointLights;
	UWorld* gameWorld = GetWorld();
	for (TObjectIterator<UPointLightComponent> Itr; Itr; ++Itr)
	{
		// World Check
		if (Itr->GetWorld() != gameWorld)
			continue;

		pointLights.Add(*Itr);
	}

	// We find local results for all locations
	for (FVector location : locations)
	{
		DrawDebugPoint(gameWorld, location, 5, FColor::Red);

		// This will be used for the spot lights
		FBoxSphereBounds bounds = FBoxSphereBounds(location, FVector(1, 1, 1), 1);

		float localResult = 0.0f;

		// We take the highest local result among lights
		for (UPointLightComponent* lightComp : pointLights)
		{
			// We don't care about invisible lights
			if (!lightComp->IsVisible())
				continue;

			// First we check if it's a spotlight and whether location is in the cone
			USpotLightComponent* spotLight = Cast<USpotLightComponent>(lightComp);
			if (spotLight && !spotLight->AffectsBounds(bounds))
				continue;
			
			FVector lightLocation = lightComp->GetComponentLocation();
			float distance = FVector::Dist(location, lightLocation);
			float lightRadius = lightComp->AttenuationRadius;
			// UE_LOG(LogTemp, Warning, TEXT("dist: %f, rad: %f"), distance, lightRadius);
			if (distance > lightRadius)
				continue;
			bool bHit = GetWorld()->LineTraceTestByChannel(lightLocation, location, ECC_Visibility, params);

			// If location could be lit
			if (!bHit)
			{
				DrawDebugLine(gameWorld, location, lightLocation, FColor::Cyan);

				// 1 if near the edge of light, 0 if in center
				float temp = distance / lightRadius;
				// We take into account the inverse squared falloff
				temp = FMath::Pow(temp, 0.5f);
				// Now it's 0 near the edge and 1 in center
				temp = 1 - temp;

				// UE_LOG(LogTemp, Warning, TEXT("%f"), temp);
				// It always counts the brightest light
				if (temp > localResult)
					localResult = temp;
			}
		}
		
		// UE_LOG(LogTemp, Warning, TEXT("%f"), localResult);
		// We take the maximum value of the positions
		if (localResult > result)
			result = localResult;
	}

	// UE_LOG(LogTemp, Warning, TEXT("Final %f"), result);
	return result;
}

// Sets default values
AMainGameMode::AMainGameMode()
{
	// TODO
}