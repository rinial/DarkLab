// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Darkness.generated.h"

// Tracking type
UENUM(BlueprintType)
enum class ETrackingEnum : uint8
{
	VE_None 	UMETA(DisplayName = "None"),
	VE_Actor 	UMETA(DisplayName = "Actor"),
	VE_Location	UMETA(DisplayName = "Location")
};

// Darkness states
UENUM(BlueprintType)
enum class EDarkStateEnum : uint8
{
	VE_Passive 	UMETA(DisplayName = "Passive"),
	VE_Hunting 	UMETA(DisplayName = "Hunting"),
	VE_Retreating	UMETA(DisplayName = "Retreating")
};

// The darkness that hunts the player
UCLASS(Blueprintable)
class DARKLAB_API ADarkness : public APawn
{
	GENERATED_BODY()

public:
	// Movement 
	void Move(const FVector direction);
	void MoveToLocation(FVector location);
	void MoveToActor(AActor* actor);
	void Stop();
	// When light is too strong goes backwards and returns true. Great for some situations and will look weird in others. Returns false if light ain't too strong
	bool RetreatFromLight();

protected:
	// Tracks something
	void Tracking();

	// Tracking parameters
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Tracking")
	ETrackingEnum TrackingType = ETrackingEnum::VE_None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Tracking")
	AActor* TrackedActor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Tracking")
	FVector TrackedLocation;

	// The amount of light the darkness is in
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	float Luminosity = 0.0f;
	// The amount of light the darkness is in
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	FVector BrightestLightLocation;
	// The darkness's resistance to light
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	float LightResistance = 0.0f;
	// The speed of resistance rising
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	float LightResSpeed = 0.01f;
	// The darkness's resistance to light
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Luminosity")
	float LightFearK = 10.0f;

	// Current state of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: State")
	EDarkStateEnum State = EDarkStateEnum::VE_Passive;

	// The particle system, forming the main body of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Components")
	class UParticleSystemComponent* DarkParticles;
	// The spherical collision of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Components")
	class USphereComponent* Collision;
	// The movement component of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness: Components")
	class UFloatingPawnMovement* Movement;

private:
	// A reference to the game mode
	UPROPERTY()
	class AMainGameMode* GameMode;

public:
	// Used for the collision overlaps
	UFUNCTION(BlueprintCallable, Category = "Darkness: Overlap")
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION(BlueprintCallable, Category = "Darkness: Overlap")
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	// Sets default values
	ADarkness();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};