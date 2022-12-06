// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "GameFramework/Actor.h"
#include "PlaceableActor.generated.h"

class UARPin;

UCLASS()
class UE5_AR_API APlaceableActor : public AActor
{
	GENERATED_BODY()
	bool bSelected;
	UMaterial* pMaterial;
	UMaterialInstanceDynamic* pDynamicMaterial;
	
public:	
	// Sets default values for this actor's properties
	APlaceableActor();

	void SetSelected(bool s) { bSelected = s; }
	bool IsSelected() { return bSelected; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SceneComponent;

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* StaticMeshComponent;

	UARPin* PinComponent;

	// I do not know how to find the trigger in the world, so I am calling this event to retrieve the location from BP_PlaceableActor
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlaceableActor", DisplayName = "Get Trigger Position")
	FVector GetTriggerPosition();

};
