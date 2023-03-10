// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameMode.h"
#include "CustomARPawn.h"
#include "CustomGameState.h"
#include "HelloARManager.h"
#include "GameManager.h"
#include "ARBlueprintLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

ACustomGameMode::ACustomGameMode():
	pARManager(nullptr),
	pGameManager(nullptr)
{
	// Add this line to your code if you wish to use the Tick() function
	PrimaryActorTick.bCanEverTick = true;

	// Set the default pawn and gamestate to be our custom pawn and gamestate programatically
	DefaultPawnClass = ACustomARPawn::StaticClass();
	GameStateClass = ACustomGameState::StaticClass();
}


void ACustomGameMode::StartPlay() 
{
	// Spawn the main actors: AR Manager and Game Manager
	//pARManager = NewObject<AHelloARManager>();								// NO! Otherwise their StartPlay does not get called!
	pARManager = GetWorld()->SpawnActor<AHelloARManager>();						// In charge of all the AR side functionalities
	pGameManager = GetWorld()->SpawnActor<AGameManager>(GameManagerToSpawn);	// In charge of the game's logic

	// This function will transcend to call BeginPlay on all the actors 
	Super::StartPlay();
}

AHelloARManager* ACustomGameMode::GetARManager() const
{
	return pARManager;
}

AGameManager* ACustomGameMode::GetGameManager() const
{
	return pGameManager;
}

void ACustomGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ACustomGameMode::ResetMode()
{
	// Reset the game completely
	pGameManager->ResetGame();
}
