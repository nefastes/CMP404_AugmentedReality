// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomARPawn.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "ARBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "CustomGameMode.h"

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
}

// Called to bind functionality to input
void ACustomARPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Bind various player inputs to functions
	// There are a few types - BindTouch, BindAxis, and BindEvent.  
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACustomARPawn::OnScreenTouch);
}

void ACustomARPawn::OnScreenTouch(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("ScreenTouch Reached"));
	if(GM)
	{
		// Add call to the line-trace here from the Custom Game Mode
	}
}
