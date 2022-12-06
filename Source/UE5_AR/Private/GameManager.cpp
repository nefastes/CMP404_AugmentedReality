// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager.h"
#include "ARBlueprintLibrary.h"
#include "ARPin.h"
#include "CustomARPawn.h"
#include "CustomGameMode.h"
#include "CustomGameState.h"
#include "HelloARManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGameManager::AGameManager() : pHoop(nullptr), pThreePointerZone(nullptr), bValidCollision(false), bGamePaused(true), saveFile(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGameManager::BeginPlay()
{
	// This function will transcend to call BeginPlay on all the actors within this structure
	Super::BeginPlay();
	
	// Check if a save file exists on this device, if so retrieve the previous high score
	ACustomGameState* gs = Cast<ACustomGameState>(GetWorld()->GetGameState());
	saveFile = Cast<UCustomSaveGame>(UGameplayStatics::LoadGameFromSlot(FString("score"), 0));
	if(!saveFile)
	{
		gs->HighScore = 0;
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No save file found, loaded highscore is zero."));
		saveFile = Cast<UCustomSaveGame>(UGameplayStatics::CreateSaveGameObject(UCustomSaveGame::StaticClass()));
	}
	else gs->HighScore = saveFile->HighScore;

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Game Manager Created and Initialised"));
}

// Called every frame
void AGameManager::Tick(float DeltaTime)
{
	// Update the Game Manager

	// Proceed to update all the components within this structure
	Super::Tick(DeltaTime);
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
					const FRotator MyRot(0, 0, 0);
					const FVector MyLoc(0, 0, 0);
					pHoop = GetWorld()->SpawnActor<APlaceableActor>(PlacableToSpawn, MyLoc, MyRot);
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
					const FRotator MyRot(0, 0, 0);
					const FVector MyLoc(0, 0, 0);
					pHoop = GetWorld()->SpawnActor<APlaceableActor>(PlacableToSpawn, MyLoc, MyRot);
				}
				pHoop->SetActorTransform(TrackedTF);
			}
		}
	}
}

void AGameManager::ResetGame()
{
	const auto Temp = GetWorld()->GetAuthGameMode();
	const auto GameMode = Cast<ACustomGameMode>(Temp);
	const auto ARManager = GameMode->GetARManager();

	// Re-allow to capture new planes from the camera
	ARManager->AllowPlaneUpdate(true);
	ARManager->SetPlanesActive(true);

	// Enable hoop placement
	const APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);
	ACustomARPawn* pawn = Cast<ACustomARPawn>(controller->GetPawn());
	if(!pawn)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at AGameManager::ResetGame: pawn cast failed"));
		return;
	}
	pawn->SetInputState(InputState_::InputState_DragHoop);

	// Reset the score
	Cast<ACustomGameState>(GetWorld()->GetGameState())->Score = 0;

	// Reset the member variables
	bGamePaused = true;
	GetWorldTimerManager().ClearTimer(Timer);
	
	// Hide the three pointer zone
	pThreePointerZone->SetActorHiddenInGame(true);

	// Kill All basketballs
	for(AShootableActor* item : aBasketballs)
		item->Destroy();
	aBasketballs.Empty();
}

void AGameManager::TogglePause()
{
	bGamePaused = !bGamePaused;
	ACustomARPawn* pawn = Cast<ACustomARPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());

	if(bGamePaused)
	{
		GetWorldTimerManager().PauseTimer(Timer);
		pawn->SetInputState(InputState_None);
	}
	else
	{
		GetWorldTimerManager().UnPauseTimer(Timer);
		pawn->SetInputState(InputState_ShootBalls);
	}
}

bool AGameManager::AcceptHoopAndStartGame()
{
#ifdef _WIN64
	TArray<AActor*> ActorsToFind;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlaceableActor::StaticClass(), ActorsToFind);
	pHoop = Cast<APlaceableActor>(ActorsToFind[0]);
#endif
	
	if(pHoop)
	{
		const auto Temp = GetWorld()->GetAuthGameMode();
		const auto GameMode = Cast<ACustomGameMode>(Temp);
		const auto ARManager = GameMode->GetARManager();
		
		// Disable the tracking of the environment
		ARManager->AllowPlaneUpdate(false);
		ARManager->SetPlanesActive(false);

		// Disable hoop placement
		const APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);
		ACustomARPawn* pawn = Cast<ACustomARPawn>(controller->GetPawn());
		if(!pawn)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at AGameManager::AcceptHoopAndStartGame: pawn cast failed"));
			return false;
		}
		pawn->SetInputState(InputState_::InputState_ShootBalls);

		// Spawn UI circle to indicate the three pointer zone
		const FVector& hoopPos = pHoop->GetActorLocation();
		FVector circlePos = pHoop->GetTriggerPosition();
		circlePos.Z = hoopPos.Z;
		if(!pThreePointerZone)
		{
			pThreePointerZone = GetWorld()->SpawnActor<AUICircle>(circlePos, FRotator(0,0,0));
			pThreePointerZone->SetActorScale3D(FVector(14.478f, 14.478f, 14.478f)); // Originally half a meter, scaled to three pointer distance
			pThreePointerZone->AddCustomUpdate([](AUICircle* thisPtr)
			{
				const auto controller = UGameplayStatics::GetPlayerController(thisPtr, 0);
				const FVector cameraPos = controller->PlayerCameraManager->GetCameraLocation();
				const FVector actorPos = thisPtr->GetActorLocation();
				const FVector horizontalDiff = FVector(actorPos.X - cameraPos.X, actorPos.Y - cameraPos.Y, 0.f);
				const float diff = horizontalDiff.Length() / 100.f; // Divide by 100 to convert to meters
				if (diff < THREE_POINTER_DISTANCE)
					thisPtr->SetColour(FVector(1.f, 1.f, 1.f));
				else
					thisPtr->SetColour(FVector(0.f, 1.f, 0.f));
			});
		}
		else
		{
			pThreePointerZone->SetActorHiddenInGame(false);
			pThreePointerZone->SetActorLocation(circlePos);
		}
		
		// Assign the time limit in seconds
		GetWorldTimerManager().SetTimer(Timer, this, &AGameManager::EndGame, 60.f, false);
		bGamePaused = false;
		return true;
	}
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Please tap the screen to place a hoop before starting the game!"));
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
	constexpr float maxImpulse = 1500.f;
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
	{
		// Give points
		const auto controller = UGameplayStatics::GetPlayerController(this, 0);
		const FVector cameraPos = controller->PlayerCameraManager->GetCameraLocation();
		const FVector distance = FVector(triggerPos.X - cameraPos.X, triggerPos.Y - cameraPos.Y, 0.f);
		const float shotDistance = distance.Length() / 100.f;				// In meters
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Shot Distance: %f"), shotDistance));
		const float timeToAdd = Cast<ACustomGameState>(GetWorld()->GetGameState())->ScoreGoal(shotDistance);

		// Add the time given by the scoring on the clock
		const float timeLeft = GetWorldTimerManager().GetTimerRemaining(Timer);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Time Left: %f"), timeLeft));
		GetWorldTimerManager().ClearTimer(Timer);
		GetWorldTimerManager().SetTimer(Timer, this, &AGameManager::EndGame, timeLeft + timeToAdd, false);
	}
}

FString AGameManager::GetTimeString() const
{
	// This function is really bad but it will have to do
	if(!GetWorldTimerManager().TimerExists(Timer)) return FString("0:00:000");
	
	const float timeLeft = GetWorldTimerManager().GetTimerRemaining(Timer);
	const int32 minutes = timeLeft / 60.f;
	const int32 seconds = fmodf(timeLeft, 60.f);
	const int32 milliseconds = fmodf(timeLeft * 1000.f, 1000.f);

	const FString m = FString::FromInt(minutes);
	FString s = FString::FromInt(seconds);
	if(s.Len() < 2) s.InsertAt(0, TEXT('0'));
	FString mm = FString::FromInt(milliseconds);
	if(mm.Len() < 2) mm.InsertAt(0, FString(TEXT("00")));
	else if(mm.Len() < 3) mm.InsertAt(0, TEXT('0'));

	FString time(TEXT("::"));
	time.Append(mm);
	time.InsertAt(1, s);
	time.InsertAt(0, m);

	return time;
}

void AGameManager::EndGame_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("End Game Reached!"));
	
	// Disable inputs
	const APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);
	ACustomARPawn* pawn = Cast<ACustomARPawn>(controller->GetPawn());
	if(!pawn)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at AGameManager::EndGame_Implementation: pawn cast failed"));
		return;
	}
	pawn->SetInputState(InputState_::InputState_None);

	// Hide the three pointer zone
	pThreePointerZone->SetActorHiddenInGame(true);

	// Write if highscore
	ACustomGameState* gs = Cast<ACustomGameState>(GetWorld()->GetGameState());
	if(gs->Score > gs->HighScore)
	{
		gs->HighScore = gs->Score;
		if(IsValid(saveFile))
		{
			saveFile->HighScore = gs->HighScore;
			const FString target("score");
			UGameplayStatics::SaveGameToSlot(saveFile, target, 0);
		}
		else GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, TEXT("Error at AGameManager::EndGame_Implementation: save file could not open for writing."));
	}
}

