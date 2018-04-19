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

	// TODO delete later
	// This is only used for tests
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Darkness")
	bool bShouldTrack = true;

private:
	// Tracks something
	void Tracking();

	// Tracking parameters
	ETrackingEnum TrackingType = ETrackingEnum::VE_None;
	AActor* TrackedActor;
	FVector TrackedLocation;

	// The particle system, forming the main body of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness", meta = (AllowPrivateAccess = "true"))
	class UParticleSystemComponent* DarkParticles;

	// The spherical collision of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Collision;

	// The movement component of the darkness
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Darkness", meta = (AllowPrivateAccess = "true"))
	class UFloatingPawnMovement* Movement;

	// A reference to the game mode
	class AMainGameMode* GameMode;

public:
	// Used for the collision overlaps
	UFUNCTION(BlueprintCallable, Category = "Overlap")
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION(BlueprintCallable, Category = "Overlap")
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