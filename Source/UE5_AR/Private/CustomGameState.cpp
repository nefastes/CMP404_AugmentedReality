// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameState.h"


ACustomGameState::ACustomGameState() :
	Score(50)
{

}

void ACustomGameState::UpdateScore()
{
	// This function gets called when a ball interacts with the trigger in the hoop
	// For now, just add 50.
	Score += 50;
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Added 50 points!"));
}
