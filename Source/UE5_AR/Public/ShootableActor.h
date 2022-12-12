// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShootableActor.generated.h"

UCLASS()
class UE5_AR_API AShootableActor : public AActor
{
	GENERATED_BODY()
	float DeathClock;
	
public:	
	// Sets default values for this actor's properties
	AShootableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() final override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) final override;

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* SceneComponent;

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* StaticMeshComponent;
};
