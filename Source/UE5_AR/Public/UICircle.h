// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <vector>
#include "UICircle.generated.h"

UCLASS()
class UE5_AR_API AUICircle : public AActor
{
	GENERATED_BODY()
	UMaterial* pMaterial;
	UMaterialInstanceDynamic* pDynamicMaterial;
	std::vector<std::function<void(AUICircle* thisPtr)>> vCustomUpdates;
	
public:	
	// Sets default values for this actor's properties
	AUICircle();
	~AUICircle();

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

	// A function to change the colour of the circle
	void SetColour(const FVector& colour) const;

	// This is just to avoid having to redefine the class multiple times or to define booleans for different behaviours
	// Probably inefficient but handy
	void AddCustomUpdate(std::function<void(AUICircle* thisPtr)> function);
};
