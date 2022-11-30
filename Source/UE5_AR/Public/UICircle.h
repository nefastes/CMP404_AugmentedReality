// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UICircle.generated.h"

UCLASS()
class UE5_AR_API AUICircle : public AActor
{
	GENERATED_BODY()
	UMaterial* pMaterial;
	UMaterialInstanceDynamic* pDynamicMaterial;
	
public:	
	// Sets default values for this actor's properties
	AUICircle();

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
};
