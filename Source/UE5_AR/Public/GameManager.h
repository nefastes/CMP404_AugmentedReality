// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlaceableActor.h"
#include "ShootableActor.h"
#include "GameFramework/Actor.h"
#include "GameManager.generated.h"

UCLASS()
class UE5_AR_API AGameManager : public AActor
{
	GENERATED_BODY()
	APlaceableActor* pHoop;
	TArray<AShootableActor*> aBasketballs;
	bool bValidCollision;
	float GameTimeLeft;
	bool bGamePaused;
	FTimerHandle Timer;
	
public:	
	// Sets default values for this actor's properties
	AGameManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Functions to spawn a placeable actor given a screen location
	void LineTraceSpawnActor(FVector2D ScreenPos);

	// Global logic funcions
	void ResetGame();
	UFUNCTION(BlueprintCallable, Category="UFunctions")
	void TogglePause();

	// The actor to be spawned by the above functions, this should be setup as the hoop prefab in the blueprint
	UPROPERTY(Category="Actors",EditAnywhere,BlueprintReadWrite)
	TSubclassOf<APlaceableActor> PlacableToSpawn;

	UPROPERTY(Category="Actors",EditAnywhere,BlueprintReadWrite)
	TSubclassOf<AShootableActor> ShootableToSpawn;

	// Call this function when the user presses the button to accept the hoop placement
	UFUNCTION(BlueprintCallable, Category="UFunctions")
	bool AcceptHoopAndStartGame();

	// Functions to spawn a basketball from the touch position on the screen
	void SpawnBasketball(FVector2D ScreenPos, float HoldTime);
	void RemoveBall(AShootableActor* ball);

	// Scoring logic functions
	UFUNCTION(BlueprintCallable, Category="UFunctions")
	void OnTriggerCollisionEnter(UPrimitiveComponent* trigger, UPrimitiveComponent* ball);	// Returns true if the ball collides from above the trigger, false otherwise

	UFUNCTION(BlueprintCallable, Category="UFunctions")
	void OnTriggerCollisionExit(UPrimitiveComponent* trigger, UPrimitiveComponent* ball); // Returns true if the ball exits from the bottom, false otherwise

	// Game logic functions
	UFUNCTION(BlueprintCallable, Category="UFunctions")
	FString GetTimeString() const;
	UFUNCTION()
	void EndGame();
};
