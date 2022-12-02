// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager.h"

#include <string>

#include "ARBlueprintLibrary.h"
#include "ARPin.h"
#include "CustomARPawn.h"
#include "CustomGameMode.h"
#include "CustomGameState.h"
#include "HelloARManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGameManager::AGameManager() : bValidCollision(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGameManager::BeginPlay()
{
	// This function will transcend to call BeginPlay on all the actors within this structure
	Super::BeginPlay();

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Game Manager Created and Initialised"));
}

// Called every frame
void AGameManager::Tick(float DeltaTime)
{
	// Update the Game Manager
	

	// Proceed to update all the components within this structure
	Super::Tick(DeltaTime);
}

void AGameManager::LineTraceSpawnActor(FVector ScreenPos)
{
	LineTraceSpawnActor(FVector2D(ScreenPos));
}

void AGameManager::LineTraceSpawnActor(FVector2D ScreenPos)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Line Trace Reached"));

	//Basic variables for functionality
	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	FVector CameraPos;
	FVector CameraDir;

	//Gets the screen touch in world space and the tracked objects from a line trace from the touch
	UGameplayStatics::DeprojectScreenToWorld(playerController, ScreenPos, CameraPos, CameraDir);
	// Notice that this LineTrace is in the ARBluePrintLibrary - this means that it's exclusive only for objects tracked by ARKit/ARCore
	auto TraceResult = UARBlueprintLibrary::LineTraceTrackedObjects(ScreenPos, false, false, false, true);

	//Checks if the location is valid
	if (TraceResult.IsValidIndex(0))
	{
		// Get the first found object in the line trace - ignoring the rest of the array elements
		auto TrackedTF = TraceResult[0].GetLocalToWorldTransform();

		if (FVector::DotProduct(TrackedTF.GetRotation().GetUpVector(), CameraDir) < 0)
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
				if (!pHoop)
				{
					const FActorSpawnParameters SpawnInfo;
					const FRotator MyRot(0, 0, 0);
					const FVector MyLoc(0, 0, 0);
					pHoop = GetWorld()->SpawnActor<APlaceableActor>(PlacableToSpawn, MyLoc, MyRot, SpawnInfo);
				}

				// Set the spawned actor location based on the Pin. Have a look at the code for Placeable Object to see how it handles the AR PIN passed on
				pHoop->SetActorTransform(PinTF);
				pHoop->PinComponent = ActorPin;

			}
			// IF ARPins are Not supported locally (for iOS Devices) We will spawn the object in the location where the line trace has hit
			else {
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("ARPin is invalid"));
				//Spawn a new Actor at the location if not done yet
				if (!pHoop)
				{
					const FActorSpawnParameters SpawnInfo;
					const FRotator MyRot(0, 0, 0);
					const FVector MyLoc(0, 0, 0);
					pHoop = GetWorld()->SpawnActor<APlaceableActor>(PlacableToSpawn, MyLoc, MyRot, SpawnInfo);
				}
				pHoop->SetActorTransform(TrackedTF);
				pHoop->SetActorScale3D(FVector(0.2, 0.2, 0.2));
			}
		}
	}
}

bool AGameManager::AcceptHoopAndStartGame()
{
	if(pHoop)
	{
		auto Temp = GetWorld()->GetAuthGameMode();
		auto GameMode = Cast<ACustomGameMode>(Temp);
		auto ARManager = GameMode->GetARManager();
		
		// Disable the tracking of the environment
		ARManager->AllowPlaneUpdate(false);
		ARManager->SetPlanesActive(false);

		// Disable hoop placement
		APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);
		ACustomARPawn* pawn = Cast<ACustomARPawn>(controller->GetPawn());
		if(!pawn)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error on start game: pawn cast faile"));
			return false;
		}
		pawn->SetInputState(InputState_::InputState_ShootBalls);
		
		return true;
	}
	return false;
}

void AGameManager::SpawnBasketball(FVector2D ScreenPos, float HoldTime)
{
	// Start by retrieving the spawn location and direction
	FVector worldPosition, worldDirection;
	if(!UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), ScreenPos, worldPosition, worldDirection))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at ACustomARPawn::OnShootReleased: Deprojection failed."));
		return;
	}

	// Spawn the Actor
	const FRotator rotation(0, 0, 0);
	AShootableActor* ball = GetWorld()->SpawnActor<AShootableActor>(ShootableToSpawn, worldPosition, rotation);

	// Apply an impulse, based on input time
	constexpr float minImpulse = 100.f;	// Evaluated at compile time
	constexpr float maxImpulse = 2000.f;
	const float inputTime = FMath::Clamp(HoldTime, 0.f, 1.f);
	const float impulseForce = FMath::Lerp(minImpulse, maxImpulse, inputTime);
	ball->StaticMeshComponent->AddImpulse(worldDirection * impulseForce);

	// Store the ball
	aBasketballs.Add(ball);
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString(TEXT("New ball spawned with impulse: ") + FString::SanitizeFloat(impulseForce)));
}

void AGameManager::RemoveBall(AShootableActor* ball)
{
	aBasketballs.Remove(ball);
}

void AGameManager::OnTriggerCollisionEnter(UPrimitiveComponent* trigger, UPrimitiveComponent* ball)
{
	const FVector& triggerPos = trigger->GetComponentLocation();
	const FVector& ballPos = ball->GetComponentLocation();

	// Z is up
	// The valid scenario is for the ball to enter from above the trigger
	bValidCollision = triggerPos.Z < ballPos.Z;
}

void AGameManager::OnTriggerCollisionExit(UPrimitiveComponent* trigger, UPrimitiveComponent* ball)
{
	const FVector& triggerPos = trigger->GetComponentLocation();
	const FVector& ballPos = ball->GetComponentLocation();

	// Z is up
	// The valid scenario is for the ball to exit from below
	// The & is checking if the entering collision was valid as well
	bValidCollision &= triggerPos.Z > ballPos.Z;

	// If the exit was valid, the ball went through the hoop and so the score should count
	if(bValidCollision)
		Cast<ACustomGameState>(GetWorld()->GetGameState())->UpdateScore();
}

