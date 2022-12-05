// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameState.h"


ACustomGameState::ACustomGameState() :
	Score(0)
{
}

void ACustomGameState::UpdateScore(float shotDistance)
{
	// This function gets called when a ball interacts with the trigger in the hoop
	// For now, just add 50.
	const bool zone = shotDistance > THREE_POINTER_DISTANCE;
	const int32 points = zone * 3 + !zone; // Award 3 points if the shot distance was further away from the 3 points zone, one point otherwise
	Score += points;
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Awarded %d points!"), points));
}
