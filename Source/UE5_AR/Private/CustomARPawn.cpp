// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomARPawn.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "ARBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "CustomGameMode.h"
#include "PlaceableActor.h"


// Sets default values
ACustomARPawn::ACustomARPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);


	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	
	CameraComponent->SetupAttachment(SceneComponent);
}

// Called when the game starts or when spawned
void ACustomARPawn::BeginPlay()
{
	Super::BeginPlay();
	UARSessionConfig* Config = NewObject<UARSessionConfig>();
	UARBlueprintLibrary::StartARSession(Config);
}

// Called every frame
void ACustomARPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDragging)
	{
		bool bIsScreenPressed;
		auto controller = UGameplayStatics::GetPlayerController(this, 0);
		FVector2D newTouch;
		controller->GetInputTouchState(ETouchIndex::Touch1, newTouch.X, newTouch.Y, bIsScreenPressed);
		if (bIsScreenPressed)
		{
			//FVector2D drag = newTouch - mPreviousTouch;
			//pDraggedActor->AddActorWorldOffset(FVector(-drag.X, drag.Y, 0));
			///*FVector actorPos = pDraggedActor->GetActorLocation();
			//FVector drag = DeprojectDrag(mPreviousTouch, newTouch);
			//actorPos += drag;
			//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("DRAG: " + drag.ToString()));

			//FMatrix movementMatrix(FMatrix::Identity);
			//movementMatrix.M[3][0] = actorPos.X;
			//movementMatrix.M[3][1] = actorPos.Y;
			//movementMatrix.M[3][2] = actorPos.Z;
			//pDraggedActor->SetActorTransform(FTransform(movementMatrix));*/
			//mPreviousTouch = newTouch;

			FVector worldPos = DeprojectToWorld(newTouch);
			FVector actorPos = pDraggedActor->GetActorLocation();
			worldPos.Z = actorPos.Z;
			pDraggedActor->AddActorWorldOffset(worldPos - actorPos);
		}
	}
}

// Called to bind functionality to input
void ACustomARPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Bind various player inputs to functions
	// There are a few types - BindTouch, BindAxis, and BindEvent.  
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACustomARPawn::OnScreenPressed);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACustomARPawn::OnScreenReleased);
}

FVector ACustomARPawn::DeprojectDrag(FVector2D DragStart, FVector2D DragEnd)
{
	FVector worldPositionStart, worldPositionEnd, worldDirection;
	auto controller = UGameplayStatics::GetPlayerController(this, 0);
	UGameplayStatics::DeprojectScreenToWorld(controller, DragStart, worldPositionStart, worldDirection);
	UGameplayStatics::DeprojectScreenToWorld(controller, DragEnd, worldPositionEnd, worldDirection);
	return worldPositionEnd - worldPositionStart;
}

FVector ACustomARPawn::DeprojectToWorld(FVector2D ScreenTouch)
{
	FVector worldPosition, worldDirection;
	auto controller = UGameplayStatics::GetPlayerController(this, 0);
	UGameplayStatics::DeprojectScreenToWorld(controller, ScreenTouch, worldPosition, worldDirection);
	return worldPosition;
}

bool ACustomARPawn::WorldHitTest(const FVector& InTouchPosition, FHitResult& OutResult)
{
	APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);

	// Perform a deprojection, taking a 2d clicked point on the screen and finding a world position
	FVector worldPosition, worldDirection;
	FVector2D touchPosition2D(InTouchPosition);
	bool success = UGameplayStatics::DeprojectScreenToWorld(controller, touchPosition2D, worldPosition, worldDirection);
	if (!success) return false;

	// Trace the input
	FVector traceEndVector = worldPosition + worldDirection * 1000.f;
	success = GetWorld()->LineTraceSingleByChannel(OutResult, worldPosition, traceEndVector, ECollisionChannel::ECC_WorldDynamic);
	return success;
}

void ACustomARPawn::OnScreenPressed(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("ScreenTouch Reached"));

	FHitResult hitResult;
	if (!WorldHitTest(ScreenPos, hitResult) && GM)
	{
		GM->LineTraceSpawnActor(ScreenPos);
	}
	else
	{
		UClass* hitActorClass = UGameplayStatics::GetObjectClass(hitResult.GetActor());
		if (UKismetMathLibrary::ClassIsChildOf(hitActorClass, APlaceableActor::StaticClass()))
		{
			UKismetSystemLibrary::PrintString(this, FString(TEXT("\t\tActor pressed.")), true, true, FLinearColor::Red, 5);
			bIsDragging = true;
			mPreviousTouch = FVector2D(ScreenPos);
			pDraggedActor = Cast<APlaceableActor>(hitResult.GetActor());
			pDraggedActor->PinComponent = nullptr;
		}
	}
}

void ACustomARPawn::OnScreenReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	if (bIsDragging)
	{
		auto Temp = GetWorld()->GetAuthGameMode();
		auto GM = Cast<ACustomGameMode>(Temp);
		if (GM)
		{
			GM->LineTraceSpawnActor(ScreenPos);
		}
		bIsDragging = false;
		mPreviousTouch = FVector2D::ZeroVector;
		pDraggedActor = nullptr;
	}
}
