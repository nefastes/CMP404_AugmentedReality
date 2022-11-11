// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CustomActor.h"
#include "GameFramework/Pawn.h"
#include "CustomARPawn.generated.h"

class UCameraComponent;
class APlaceableActor;

UCLASS()
class UE5_AR_API ACustomARPawn : public APawn
{
	GENERATED_BODY()

private:
	APlaceableActor* pDraggedActor;
	ACustomActor* pGoghActor, *pEarthActor;

public:
	// Sets default values for this pawn's properties
	ACustomARPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FVector DeprojectToWorld(FVector2D screenTouch);
	bool WorldHitTest(const FVector& InTouchPosition, FHitResult& OutResult);

	virtual void OnScreenPressed(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	virtual void OnScreenReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SceneComponent;

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		UCameraComponent* CameraComponent;

	UPROPERTY(Category="Placeable",EditAnywhere,BlueprintReadWrite)
	TSubclassOf<ACustomActor> EarthActor;
	UPROPERTY(Category="Placeable",EditAnywhere,BlueprintReadWrite)
	TSubclassOf<ACustomActor> GoghActor;
};
