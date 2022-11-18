// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CustomActor.h"
#include "GameFramework/Pawn.h"
#include "CustomARPawn.generated.h"

class UCameraComponent;
class APlaceableActor;
enum class InputState_
{
	InputState_Undefined = 0,
	InputState_DragHoop,
	InputState_ShootBalls
};

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

	bool WorldHitTest(const FVector& InTouchPosition, FHitResult& OutResult);

	// The following functions determine the player inputs/behaviours
	virtual void OnHoopPressed(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	virtual void OnHoopHold(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	virtual void OnHoopReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);

	virtual void OnShootPressed(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	virtual void OnShootHold(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	virtual void OnShootReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called to change the kind of inputs the player does
	void SetInputState(InputState_ state);

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SceneComponent;

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		UCameraComponent* CameraComponent;

	UPROPERTY(Category="Placeable",EditAnywhere,BlueprintReadWrite)
	TSubclassOf<ACustomActor> EarthActor;
	UPROPERTY(Category="Placeable",EditAnywhere,BlueprintReadWrite)
	TSubclassOf<ACustomActor> GoghActor;
};
