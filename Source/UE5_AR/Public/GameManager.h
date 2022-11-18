// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlaceableActor.h"
#include "ShootableActor.h"
#include "GameFramework/Actor.h"
#include "VectorField/VectorField.h"
#include "GameManager.generated.h"

UCLASS()
class UE5_AR_API AGameManager : public AActor
{
	GENERATED_BODY()
	APlaceableActor* pHoop;
	UMaterial* pHoopMaterial;
	UMaterialInstanceDynamic* pHoopDynamicMaterial;
	TArray<AShootableActor*> aBasketballs;
	
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
	void LineTraceSpawnActor(FVector ScreenPos);
	void LineTraceSpawnActor(FVector2D ScreenPos);

	// The actor to be spawned by the above functions, this should be setup as the hoop prefab in the blueprint
	UPROPERTY(Category="Actors",EditAnywhere,BlueprintReadWrite)
	TSubclassOf<APlaceableActor> PlacableToSpawn;

	UPROPERTY(Category="Actors",EditAnywhere,BlueprintReadWrite)
	TSubclassOf<AShootableActor> ShootableToSpawn;

	// Call this function when the user presses the button to accept the hoop placement
	UFUNCTION(BlueprintCallable, Category="StartGame")
	bool AcceptHoopAndStartGame();

	// Functions to spawn a basketball from the touch position on the screen
	void SpawnBasketball(FVector2D ScreenPos);
	void RemoveBall(AShootableActor* ball);
};
