// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomARPawn.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "ARBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "CustomGameMode.h"
#include "GameManager.h"
#include "PlaceableActor.h"
#include "GameFramework/PawnMovementComponent.h"
#include "HelloARManager.h"


// Sets default values
ACustomARPawn::ACustomARPawn() : pDraggedActor(nullptr), pGoghActor(nullptr), pEarthActor(nullptr)
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
}

// Called every frame
void ACustomARPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (pDraggedActor)
	{
		bool bIsScreenPressed;
		const auto controller = UGameplayStatics::GetPlayerController(this, 0);
		FVector2D newTouch;
		controller->GetInputTouchState(ETouchIndex::Touch1, newTouch.X, newTouch.Y, bIsScreenPressed);
		if (bIsScreenPressed)
		{
			const auto TraceResult = UARBlueprintLibrary::LineTraceTrackedObjects(newTouch, false, false, false, true);
			if (TraceResult.IsValidIndex(0)) pDraggedActor->SetActorTransform((TraceResult[0].GetLocalToWorldTransform()));
		}
	}

	// loop through all tracked images and see if van gogh was found
	TArray<UARTrackedImage*> images = UARBlueprintLibrary::GetAllGeometriesByClass<UARTrackedImage>();
	for(int32_t i = 0; i < images.Num(); ++i)
	{
		const UARCandidateImage* trackedImage = images[i]->GetDetectedImage();
		if(trackedImage)
		{
			if(trackedImage->GetFriendlyName().Equals(TEXT("Vgogh")))
			{
				// Store all the information to spawn a cube on this image
				if(!pGoghActor)
				{
					FActorSpawnParameters spawnParams;
					auto transform = images[i]->GetLocalToTrackingTransform();
					transform.SetScale3D(FVector(.1, .1, .1));
					pGoghActor = GetWorld()->SpawnActor<ACustomActor>(GoghActor, transform.GetLocation(), FRotator(transform.GetRotation()), spawnParams);
					// auto cubeSpawnTransform = images[i]->GetLocalToWorldTransform();
					// pGoghActor = GetWorld()->SpawnActor<APlaceableActor>(cubeSpawnTransform.GetLocation(), FRotator(cubeSpawnTransform.GetRotation()), spawnParams);
					pGoghActor->SetActorScale3D(transform.GetScale3D());
				}
				else
				{
					auto transform = images[i]->GetLocalToTrackingTransform();
					pGoghActor->StartLocation = transform.GetLocation();
				}
			}
			else if(trackedImage->GetFriendlyName().Equals(TEXT("Earth")))
			{
				// Store all the information to spawn a cube on this image
				if(!pEarthActor)
				{
					FActorSpawnParameters spawnParams;
					auto transform = images[i]->GetLocalToTrackingTransform();
					transform.SetScale3D(FVector(.1, .1, .1));
					pEarthActor = GetWorld()->SpawnActor<ACustomActor>(EarthActor, transform.GetLocation(), FRotator(transform.GetRotation()), spawnParams);
					pEarthActor->SetActorScale3D(transform.GetScale3D());
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Spawned earth actor at " + pEarthActor->GetActorLocation().ToString()));
				}
				else
				{
					auto transform = images[i]->GetLocalToTrackingTransform();
					pEarthActor->StartLocation = transform.GetLocation();
				}
			}	
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
	auto GameMode = Cast<ACustomGameMode>(Temp);
	auto GameManager = GameMode->GetGameManager();
	auto ARManager = GameMode->GetARManager();
	

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("ScreenTouch Reached"));

	FHitResult hitResult;
	if (!WorldHitTest(ScreenPos, hitResult) && GameManager)
	{
		GameManager->LineTraceSpawnActor(ScreenPos);
	}
	else
	{
		UClass* hitActorClass = UGameplayStatics::GetObjectClass(hitResult.GetActor());
		if (UKismetMathLibrary::ClassIsChildOf(hitActorClass, APlaceableActor::StaticClass()))
		{
			UKismetSystemLibrary::PrintString(this, FString(TEXT("\t\tActor pressed.")), true, true, FLinearColor::Red, 5);
			pDraggedActor = Cast<APlaceableActor>(hitResult.GetActor());
			UARBlueprintLibrary::RemovePin((pDraggedActor->PinComponent));
			pDraggedActor->PinComponent = nullptr;
			pDraggedActor->SetSelected(true);
			ARManager->AllowPlaneUpdate(false);
			ARManager->SetPlanesActive(false);
		}
	}
}

void ACustomARPawn::OnScreenReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	if (pDraggedActor)
	{
		auto Temp = GetWorld()->GetAuthGameMode();
		auto GameMode = Cast<ACustomGameMode>(Temp);
		auto GameManager = GameMode->GetGameManager();
		auto ARManager = GameMode->GetARManager();
		
		if (GameManager)
		{
			GameManager->LineTraceSpawnActor(ScreenPos);
		}
		pDraggedActor->SetSelected(false);
		pDraggedActor = nullptr;
		ARManager->AllowPlaneUpdate(true);
		ARManager->SetPlanesActive(true);
	}
}