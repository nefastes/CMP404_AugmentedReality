// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameMode.h"
#include "CustomARPawn.h"
#include "CustomGameState.h"
#include "CustomActor.h"
#include "HelloARManager.h"
#include "ARPin.h"
#include "ARBlueprintLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "PlaceableActor.h"

ACustomGameMode::ACustomGameMode():
	SpawnedActor(nullptr)
{
	// Add this line to your code if you wish to use the Tick() function
	PrimaryActorTick.bCanEverTick = true;

	// Set the default pawn and gamestate to be our custom pawn and gamestate programatically
	DefaultPawnClass = ACustomARPawn::StaticClass();
	GameStateClass = ACustomGameState::StaticClass();
	
	// Materials init
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("Material'/Game/Assets/Materials/Colour.Colour'"));
	pRedMaterial = MaterialAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset2(TEXT("Material'/Game/Assets/Materials/Blue.Blue'"));
	pBlueMaterial = MaterialAsset2.Object;
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset3(TEXT("Material'/Game/Assets/Materials/Selected.Selected'"));
	pSelectedMaterial = MaterialAsset3.Object;
}


void ACustomGameMode::StartPlay() 
{
	SpawnInitialActors();

	// This is called before BeginPlay
	StartPlayEvent();
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Current Score: %d"), GetScore()));

	// This function will transcend to call BeginPlay on all the actors 
	Super::StartPlay();
	

}

// An implementation of the StartPlayEvent which can be triggered by calling StartPlayEvent() 
void ACustomGameMode::StartPlayEvent_Implementation() 
{
	// Start a timer which will call the SpawnCube Function every 4 seconds
	GetWorldTimerManager().SetTimer(Ticker, this, &ACustomGameMode::SpawnCube, 4.0f, true, 0.0f);
}

int32 ACustomGameMode::GetScore()
{
	return GetGameState<ACustomGameState>()->Score;
}

void ACustomGameMode::SetScore(const int32 NewScore)
{
	GetGameState<ACustomGameState>()->Score = NewScore;

}

void ACustomGameMode::SpawnCube()
{
	const FActorSpawnParameters SpawnInfo;
	const FRotator MyRot(0, 0, 0);
	const FVector MyLoc(-300, 0, 0);
	ACustomActor* CustomActor = GetWorld()->SpawnActor<ACustomActor>(MyLoc, MyRot, SpawnInfo);
}

void ACustomGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (SpawnedActor)
	{
		if (!SpawnedActor->IsSelected())
		{
			// Change the colour of the actor based on the player distance
			auto controller = UGameplayStatics::GetPlayerController(this, 0);
			FVector cameraPos = controller->PlayerCameraManager->GetCameraLocation();
			FVector actorPos = SpawnedActor->GetActorLocation();
			FVector diff = actorPos - cameraPos;
			if (diff.GetAbs().Length() < 100.0)
				//pDynamicMaterial->SetVectorParameterValue(TEXT("InputColour"), FVector(0.f, 0.f, 1.f));
				SpawnedActor->StaticMeshComponent->SetMaterial(0, pRedMaterial);
			else
				//pDynamicMaterial->SetVectorParameterValue(TEXT("InputColour"), FVector(1.f, 0.f, 0.f));
				SpawnedActor->StaticMeshComponent->SetMaterial(0, pBlueMaterial);
		}
		else
		{
			SpawnedActor->StaticMeshComponent->SetMaterial(0, pSelectedMaterial);
		}
	}
}

void ACustomGameMode::SpawnInitialActors()
{

	// Spawn an instance of the HelloARManager class
	//pARManager = NewObject<AHelloARManager>();		// NO!
	pARManager = GetWorld()->SpawnActor<AHelloARManager>();
}


void ACustomGameMode::LineTraceSpawnActor(FVector ScreenPos)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Line Trace Reached"));

	//Basic variables for functionality
	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	FVector WorldPos;
	FVector WorldDir;

	//Gets the screen touch in world space and the tracked objects from a line trace from the touch
	UGameplayStatics::DeprojectScreenToWorld(playerController, FVector2D(ScreenPos), WorldPos, WorldDir);
	// Notice that this LineTrace is in the ARBluePrintLibrary - this means that it's exclusive only for objects tracked by ARKit/ARCore
	auto TraceResult = UARBlueprintLibrary::LineTraceTrackedObjects(FVector2D(ScreenPos), false, false, false, true);

	//Checks if the location is valid
	if (TraceResult.IsValidIndex(0))
	{
		// Get the first found object in the line trace - ignoring the rest of the array elements
		auto TrackedTF = TraceResult[0].GetLocalToWorldTransform();

		if (FVector::DotProduct(TrackedTF.GetRotation().GetUpVector(), WorldDir) < 0)
		{
			//Spawn the actor pin and get the transform
			UARPin* ActorPin = UARBlueprintLibrary::PinComponent(nullptr, TraceResult[0].GetLocalToWorldTransform(), TraceResult[0].GetTrackedGeometry());

			// Check if ARPins are available on your current device. ARPins are currently not supported locally by ARKit, so on iOS, this will always be "FALSE" 
			if (ActorPin)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("ARPin is valid"));
				//If the pin is valid 
				auto PinTF = ActorPin->GetLocalToWorldTransform();
				//Spawn a new Actor at the location if not done yet
				if (!SpawnedActor)
				{
					const FActorSpawnParameters SpawnInfo;
					const FRotator MyRot(0, 0, 0);
					const FVector MyLoc(0, 0, 0);
					SpawnedActor = GetWorld()->SpawnActor<APlaceableActor>(PlacableToSpawn, MyLoc, MyRot, SpawnInfo);

					// Init dynamic materials
					/*if (!pDynamicMaterial)
					{
						pDynamicMaterial = UMaterialInstanceDynamic::Create(pRedMaterial, SpawnedActor->StaticMeshComponent);
						SpawnedActor->StaticMeshComponent->SetMaterial(0, pDynamicMaterial);
					}*/
					SpawnedActor->StaticMeshComponent->SetMaterial(0, pBlueMaterial);
				}
				
				


				// Set the spawned actor location based on the Pin. Have a look at the code for Placeable Object to see how it handles the AR PIN passed on
				SpawnedActor->SetActorTransform(PinTF);
				SpawnedActor->PinComponent = ActorPin;
			
			}
			// IF ARPins are Not supported locally (for iOS Devices) We will spawn the object in the location where the line trace has hit
			else{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("ARPin is invalid"));
				//Spawn a new Actor at the location if not done yet
				if (!SpawnedActor)
				{
					const FActorSpawnParameters SpawnInfo;
					const FRotator MyRot(0, 0, 0);
					const FVector MyLoc(0, 0, 0);
					SpawnedActor = GetWorld()->SpawnActor<APlaceableActor>(PlacableToSpawn, MyLoc, MyRot, SpawnInfo);
					// Init dynamic materials
					/*if (!pDynamicMaterial)
					{
						pDynamicMaterial = UMaterialInstanceDynamic::Create(pRedMaterial, SpawnedActor->StaticMeshComponent);
						SpawnedActor->StaticMeshComponent->SetMaterial(0, pDynamicMaterial);
					}*/
					SpawnedActor->StaticMeshComponent->SetMaterial(0, pBlueMaterial);
				}
				SpawnedActor->SetActorTransform(TrackedTF);
				SpawnedActor->SetActorScale3D(FVector(0.2, 0.2, 0.2));


			}
		}
	}
}

