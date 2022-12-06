// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CustomGameState.generated.h"

#define THREE_POINTER_DISTANCE 7.239f //~23 feet & 9 inches, NBA standards: https://www.usab.com/youth/news/2011/06/the-history-of-the-3-pointer.aspx#:~:text=Though%20the%20distances%20differ%20between,foot%2C%206%2Dinch%20line.

/**
 * 
 */
UCLASS()
class UE5_AR_API ACustomGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	ACustomGameState();
	~ACustomGameState() = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Score;

	UFUNCTION(BlueprintCallable, Category="UpdateScore")
	float ScoreGoal(float shotDistance);
};
