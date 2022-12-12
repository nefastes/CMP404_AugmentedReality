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
ACustomARPawn::ACustomARPawn() : pDraggedActor(nullptr), ScreenTouchHoldTime(0.f), pUICircle(nullptr)
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
	// Spawn the UI Circle and hide it
	pUICircle = GetWorld()->SpawnActor<AUICircle>(UICircleToSpawn, FVector(0,0,0), FRotator(0,0,0));
	pUICircle->SetActorHiddenInGame(true);
	
	Super::BeginPlay();
}

// Called every frame
void ACustomARPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Keep track of how much time a touch was held on the screen
	// This must be done here as the input function IE_Repeat only triggers on drag
	APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);
	FVector2d touchPos;
	bool screenPressed;
	controller->GetInputTouchState(ETouchIndex::Touch1, touchPos.X, touchPos.Y, screenPressed);
	if(screenPressed)
	{
		// Move the circle in from of the camera
		FVector worldPosition, worldDirection;
		UGameplayStatics::DeprojectScreenToWorld(controller, touchPos, worldPosition, worldDirection);
		pUICircle->SetActorLocation(worldPosition + worldDirection);
		pUICircle->SetActorRotation(worldDirection.Rotation());
		pUICircle->SetActorScale3D(FMath::Lerp(FVector(0,0,0), FVector(1,1,1), FMath::Clamp(ScreenTouchHoldTime, 0.f, 1.f)));
		
		// Update the hold time
		ScreenTouchHoldTime += DeltaTime;
	}
}

// Called to bind functionality to input
void ACustomARPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Initialise the pawn on the Hoop placement behaviours
	SetInputState(InputState_::InputState_DragHoop);
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

	// Reset the hold time tracker for the new inputs
	ScreenTouchHoldTime = 0.f;
	
	// Bind the touches to the new behaviours
	switch (state)
	{
	case InputState_::InputState_DragHoop:
		PlayerInputComponent->BindTouch(IE_Pressed, this, &ACustomARPawn::OnHoopPressed);
		PlayerInputComponent->BindTouch(IE_Repeat, this, &ACustomARPawn::OnHoopDrag);
		PlayerInputComponent->BindTouch(IE_Released, this, &ACustomARPawn::OnHoopReleased);
		break;
	case InputState_::InputState_ShootBalls:
		PlayerInputComponent->BindTouch(IE_Pressed, this, &ACustomARPawn::OnShootPressed);
		PlayerInputComponent->BindTouch(IE_Repeat, this, &ACustomARPawn::OnShootDrag);
		PlayerInputComponent->BindTouch(IE_Released, this, &ACustomARPawn::OnShootReleased);
		break;
	default:
		break;
	}
}

bool ACustomARPawn::WorldHitTest(const FVector& InTouchPosition, FHitResult& OutResult) const
{
	APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);

	// Perform a deprojection, taking a 2d clicked point on the screen and finding a world position
	FVector worldPosition, worldDirection;
	const FVector2D touchPosition2D(InTouchPosition);
	bool success = UGameplayStatics::DeprojectScreenToWorld(controller, touchPosition2D, worldPosition, worldDirection);
	if (!success) return false;

	// Trace the input
	const FVector traceEndVector = worldPosition + worldDirection * 1000.f;
	success = GetWorld()->LineTraceSingleByChannel(OutResult, worldPosition, traceEndVector, ECollisionChannel::ECC_WorldDynamic);
	return success;
}

void ACustomARPawn::OnHoopPressed(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	const auto Temp = GetWorld()->GetAuthGameMode();
	const auto GameMode = Cast<ACustomGameMode>(Temp);
	const auto GameManager = GameMode->GetGameManager();
	const auto ARManager = GameMode->GetARManager();

	if (!GameManager || !ARManager)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at ACustomARPawn::OnHoopPressed: One of the managers was NULL"));
		return;
	}

	// Actor placement logic
	// Test if an actor was hit. If so, this actor must be dragged. Else, spawn the actor on the new location
	FHitResult hitResult;
	if (!WorldHitTest(ScreenPos, hitResult))
		GameManager->LineTraceSpawnActor(FVector2d(ScreenPos));
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

void ACustomARPawn::OnHoopDrag(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
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
		const auto Temp = GetWorld()->GetAuthGameMode();
		const auto GameMode = Cast<ACustomGameMode>(Temp);
		const auto GameManager = GameMode->GetGameManager();
		const auto ARManager = GameMode->GetARManager();
		
		pDraggedActor->SetSelected(false);
		pDraggedActor = nullptr;
		
		if (!GameManager || !ARManager)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at ACustomARPawn::OnHoopReleased: One of the managers was NULL"));
			return;
		}
		GameManager->LineTraceSpawnActor(FVector2d(ScreenPos));
		ARManager->AllowPlaneUpdate(true);
		ARManager->SetPlanesActive(true);
	}
}

void ACustomARPawn::OnShootPressed(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	// Reveal UI Circle to indicate the impulse
	pUICircle->SetActorHiddenInGame(false);
}

void ACustomARPawn::OnShootDrag(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	// Note to self: this function only triggers when the touch is dragged on the screen
	// For a 'hold' behaviour you need to use Update()
}

void ACustomARPawn::OnShootReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	// Shoot a ball on touch released
	const auto Temp = GetWorld()->GetAuthGameMode();
	const auto GameMode = Cast<ACustomGameMode>(Temp);
	const auto GameManager = GameMode->GetGameManager();

	if(!GameManager)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at ACustomARPawn::OnShootReleased: Game manager was NULL"));
		return;
	}
	
	// Spawn a ball at that location
	GameManager->SpawnBasketball(FVector2d(ScreenPos), ScreenTouchHoldTime);

	// Hide UI
	pUICircle->SetActorHiddenInGame(true);
	ScreenTouchHoldTime = 0.f;
}
