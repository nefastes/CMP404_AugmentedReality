// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "ARTextures.h"
#include "GameFramework/Actor.h"
#include "HelloARManager.generated.h"


class UARSessionConfig;
class AARPlaneActor;
class UARPlaneGeometry;

UCLASS()
class UE5_AR_API AHelloARManager : public AActor
{
	GENERATED_BODY()
	
	bool bAllowPlaneUpdate;

public:	
	// Sets default values for this actor's properties
	AHelloARManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AllowPlaneUpdate(bool allow);
	void SetPlanesActive(bool active);

	//Create a default Scene Component
	UPROPERTY(Category = "SceneComp", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SceneComponent;

	const UARTexture* GetDepthMap() const; 

protected:
	
	// Updates the plane actors on every frame as long as the AR Session is running
	void UpdatePlaneActors();

	AARPlaneActor* SpawnPlaneActor();
	FColor GetPlaneColor(int Index);
	void ResetARCoreSession();
	// Configuration file for AR Session
	UARSessionConfig* Config;

	//Base plane actor for geometry detection
	AARPlaneActor* PlaneActor;


	//Map of geometry planes
	TMap<UARPlaneGeometry*, AARPlaneActor*> PlaneActors;

	//Index for plane colours adn array of colours
	int PlaneIndex = 0;
	TArray<FColor> PlaneColors;

	// Store the depth map on update
	UARTexture* pDepthMap, *pCameraMap;

};
