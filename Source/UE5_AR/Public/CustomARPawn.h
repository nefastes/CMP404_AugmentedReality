// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "CustomARPawn.generated.h"

class UCameraComponent;
class APlaceableActor;

UCLASS()
class UE5_AR_API ACustomARPawn : public APawn
{
	GENERATED_BODY()

private:
	FVector2D mPreviousTouch;
	APlaceableActor* pDraggedActor;

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

	void MoveForward(float Value);
	void MoveRight(float Value);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SceneComponent;

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		UCameraComponent* CameraComponent;
};
