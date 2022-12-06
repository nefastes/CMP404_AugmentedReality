// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CustomSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class UE5_AR_API UCustomSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 HighScore;
};
