// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "CustomGameMode.generated.h"

//Forward Declarations
class APlaceableActor;
class AHelloARManager;
class AGameManager;

/**
 * 
 */

UCLASS()
class UE5_AR_API ACustomGameMode : public AGameModeBase
{
	GENERATED_BODY()
	AHelloARManager* pARManager;
	AGameManager* pGameManager;

public:
	ACustomGameMode();
	virtual ~ACustomGameMode() = default;
	virtual void StartPlay() final override;
	virtual void Tick(float DeltaSeconds) final override;

	UPROPERTY(Category="Game Manager To Spawn",EditAnywhere,BlueprintReadWrite)
	TSubclassOf<AGameManager> GameManagerToSpawn;

	UFUNCTION(BlueprintCallable)
	AHelloARManager* GetARManager() const;

	UFUNCTION(BlueprintCallable)
	AGameManager* GetGameManager() const;

	UFUNCTION(BlueprintCallable, Category="UFunctions")
	void ResetMode();
};
