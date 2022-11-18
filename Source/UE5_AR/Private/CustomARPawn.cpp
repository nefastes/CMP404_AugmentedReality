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
	
	// Initialise the pawn on the Hoop placement behaviours
#ifdef _WIN64
	SetInputState(InputState_::InputState_ShootBalls);
#else
	SetInputState(InputState_::InputState_DragHoop);
#endif
}

void ACustomARPawn::SetInputState(InputState_ state)
{
	UInputComponent* PlayerInputComponent = GetOwner()->InputComponent;
	if(!PlayerInputComponent)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at Pawn::SetInputState: PlayerInputComponent was NULL."));
		return;
	}

	// Unbind all previous touches
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Changing binds"));
	PlayerInputComponent->TouchBindings.Empty();
	
	// Bind the touches to the new behaviours
	switch (state)
	{
	case InputState_::InputState_DragHoop:
		PlayerInputComponent->BindTouch(IE_Pressed, this, &ACustomARPawn::OnHoopPressed);
		PlayerInputComponent->BindTouch(IE_Repeat, this, &ACustomARPawn::OnHoopHold);
		PlayerInputComponent->BindTouch(IE_Released, this, &ACustomARPawn::OnHoopReleased);
		break;
	case InputState_::InputState_ShootBalls:
		PlayerInputComponent->BindTouch(IE_Pressed, this, &ACustomARPawn::OnShootPressed);
		PlayerInputComponent->BindTouch(IE_Repeat, this, &ACustomARPawn::OnShootHold);
		PlayerInputComponent->BindTouch(IE_Released, this, &ACustomARPawn::OnShootReleased);
		break;
	default:
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at Pawn::SetInputState: trying to set inputs to undefined."));
		break;
	}
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

void ACustomARPawn::OnHoopPressed(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GameMode = Cast<ACustomGameMode>(Temp);
	auto GameManager = GameMode->GetGameManager();
	auto ARManager = GameMode->GetARManager();

	if (!GameManager || !ARManager)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at ACustomARPawn::OnHoopPressed: One of the managers was NULL"));
		return;
	}

	// Actor placement logic
	// Test if an actor was hit. If so, this actor must be dragged. Else, spawn the actor on the new location
	FHitResult hitResult;
	if (!WorldHitTest(ScreenPos, hitResult))
		GameManager->LineTraceSpawnActor(ScreenPos);
	else
	{
		UClass* hitActorClass = UGameplayStatics::GetObjectClass(hitResult.GetActor());
		if (UKismetMathLibrary::ClassIsChildOf(hitActorClass, APlaceableActor::StaticClass()))
		{
			pDraggedActor = Cast<APlaceableActor>(hitResult.GetActor());
			UARBlueprintLibrary::RemovePin((pDraggedActor->PinComponent));
			pDraggedActor->PinComponent = nullptr;
			pDraggedActor->SetSelected(true);
			ARManager->AllowPlaneUpdate(false);
			ARManager->SetPlanesActive(false);
		}
	}
}

void ACustomARPawn::OnHoopHold(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	// If an actor is waiting to be dragged, drag it to the new touch
	if (pDraggedActor)
	{
		const auto TraceResult = UARBlueprintLibrary::LineTraceTrackedObjects(FVector2d(ScreenPos), false, false, false, true);
		if (TraceResult.IsValidIndex(0)) pDraggedActor->SetActorTransform((TraceResult[0].GetLocalToWorldTransform()));
	}
}

void ACustomARPawn::OnHoopReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	if (pDraggedActor)
	{
		auto Temp = GetWorld()->GetAuthGameMode();
		auto GameMode = Cast<ACustomGameMode>(Temp);
		auto GameManager = GameMode->GetGameManager();
		auto ARManager = GameMode->GetARManager();
		
		pDraggedActor->SetSelected(false);
		pDraggedActor = nullptr;
		
		if (!GameManager || !ARManager)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at ACustomARPawn::OnHoopReleased: One of the managers was NULL"));
			return;
		}
		GameManager->LineTraceSpawnActor(ScreenPos);
		ARManager->AllowPlaneUpdate(true);
		ARManager->SetPlanesActive(true);
	}
}

void ACustomARPawn::OnShootPressed(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
}

void ACustomARPawn::OnShootHold(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
}

void ACustomARPawn::OnShootReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	// Shoot a ball on touch released

	auto Temp = GetWorld()->GetAuthGameMode();
	auto GameMode = Cast<ACustomGameMode>(Temp);
	auto GameManager = GameMode->GetGameManager();

	if(!GameManager)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at ACustomARPawn::OnShootReleased: Game manager was NULL"));
		return;
	}
	
	// Spawn a ball at that location
	GameManager->SpawnBasketball(FVector2d(ScreenPos));
}
