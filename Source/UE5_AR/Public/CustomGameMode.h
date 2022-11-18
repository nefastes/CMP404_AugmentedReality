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
private:

	FTimerHandle Ticker;
	AHelloARManager* pARManager;
	AGameManager* pGameManager;

public:
	ACustomGameMode();
	virtual ~ACustomGameMode() = default;


	virtual void StartPlay() override;

	AHelloARManager* GetARManager() const;

	UFUNCTION(BlueprintCallable)
	AGameManager* GetGameManager() const;

	/**
	 * @brief An example of an event as generally seen within the Event Graph of Blueprints
	 *  You can have different implementations of this event which will be called when you call this as a delegate.
	 *	See the CPP For more information on the implementation
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GameModeBase", DisplayName = "Start Play")
		void StartPlayEvent();

	// /**
	//  * @brief A Dummy event for spawning ACustomActor.
	//  */
	// UFUNCTION(BlueprintCallable, Category="Spawn")
	// 	virtual void SpawnCube();
	
	virtual void Tick(float DeltaSeconds) override;
	

	/**
	 * @brief This function is virtual - you can inherit this class to override this function
	 * Each level can have their own unique spawned actors but with similar base qualities using inheritance
	 */
	virtual void SpawnInitialActors();

	UPROPERTY(Category="Game Manager To Spawn",EditAnywhere,BlueprintReadWrite)
	TSubclassOf<AGameManager> GameManagerToSpawn;
};
