// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "UICircle.h"
#include "CustomARPawn.generated.h"

class UCameraComponent;
class APlaceableActor;

UENUM()
enum InputState_
{
	InputState_None = 0,
	InputState_DragHoop,
	InputState_ShootBalls
};

UCLASS()
class UE5_AR_API ACustomARPawn : public APawn
{
	GENERATED_BODY()
	APlaceableActor* pDraggedActor;
	float ScreenTouchHoldTime;
	AUICircle* pUICircle;

public:
	// Sets default values for this pawn's properties
	ACustomARPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Function to test if any object is being hit from a screen touch
	bool WorldHitTest(const FVector& InTouchPosition, FHitResult& OutResult) const;

	// The following functions determine the player inputs/behaviours
	void OnHoopPressed(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	void OnHoopDrag(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	void OnHoopReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);

	void OnShootPressed(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	void OnShootDrag(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	void OnShootReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) final override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) final override;

	// Called to change the kind of inputs the player does
	UFUNCTION(BlueprintCallable, Category="UFunctions")
	void SetInputState(InputState_ state);

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SceneComponent;

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		UCameraComponent* CameraComponent;

	UPROPERTY(Category="Actors",EditAnywhere,BlueprintReadWrite)
	TSubclassOf<AUICircle> UICircleToSpawn;
};
