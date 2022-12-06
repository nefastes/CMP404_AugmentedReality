// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameState.h"


ACustomGameState::ACustomGameState() :
	Score(0)
{
}

float ACustomGameState::ScoreGoal(float shotDistance)
{
	// This function gets called when a ball interacts with the trigger in the hoop
	const bool zone = shotDistance > THREE_POINTER_DISTANCE;
	const int32 points = zone * 3 + !zone; // Award 3 points if the shot distance was further away from the 3 points zone, one point otherwise
	Score += points;
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Awarded %d points!"), points));

	// Return the amount of time that should be added on the clock
	// 2 seconds for 1 point, 4 seconds for a 3 pointer
	return zone * 4.f + !zone * 2.f;
}
