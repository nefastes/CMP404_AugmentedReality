// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlaceableActor.h"
#include "UICircle.h"
#include "ShootableActor.h"
#include "CustomSaveGame.h"
#include "GameFramework/Actor.h"
#include "GameManager.generated.h"

UCLASS()
class UE5_AR_API AGameManager : public AActor
{
	GENERATED_BODY()
	APlaceableActor* pHoop;
	AUICircle* pThreePointerZone;
	TArray<AShootableActor*> aBasketballs;
	bool bValidCollision;
	bool bGamePaused;
	FTimerHandle Timer;
	UPROPERTY() UCustomSaveGame* saveFile;
	
public:	
	// Sets default values for this actor's properties
	AGameManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() final override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) final override;

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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameManager", DisplayName = "End Game Event")
	void EndGame();

	// Sounds
	UPROPERTY(Category="Sounds",EditAnywhere,BlueprintReadWrite)
	TArray<USoundBase*> aSoundsTwoPointers;
	UPROPERTY(Category="Sounds",EditAnywhere,BlueprintReadWrite)
	TArray<USoundBase*> aSoundsThreePointers;
	UPROPERTY(Category="Sounds",EditAnywhere,BlueprintReadWrite)
	TArray<USoundBase*> aSoundsBallBounces;
	UPROPERTY(Category="Sounds",EditAnywhere,BlueprintReadWrite)
	TArray<USoundBase*> aSoundsCommentatorEndGame;
	UPROPERTY(Category="Sounds",EditAnywhere,BlueprintReadWrite)
	USoundBase* SoundStartGame;
	UPROPERTY(Category="Sounds",EditAnywhere,BlueprintReadWrite)
	USoundBase* SoundEndGame;
	UPROPERTY(Category="Sounds",EditAnywhere,BlueprintReadWrite)
	USoundBase* MusicAmbient;
	void PlayRandomAudioFrom(const TArray<USoundBase*>& soundBank, const float volumeMultiplier) const;
	UFUNCTION(BlueprintCallable, Category = "UFunctions")
		void OnBallAnyCollision(const FVector& impactNormal, const FVector& impactLocation);
};
