// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootableActor.h"

#include "CustomGameMode.h"
#include "GameManager.h"

// Sets default values
AShootableActor::AShootableActor() : DeathClock(0.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SceneComponent);

	const auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Assets/Meshes/basketball/Basketball_size6_SF.Basketball_size6_SF'"));
	StaticMeshComponent->SetStaticMesh(MeshAsset.Object);

	// Enable physics
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->WakeRigidBody();
}

// Called when the game starts or when spawned
void AShootableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShootableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DeathClock += DeltaTime;
	if(DeathClock > 10.f)
	{
		auto Temp = GetWorld()->GetAuthGameMode();
		auto GameMode = Cast<ACustomGameMode>(Temp);
		auto GameManager = GameMode->GetGameManager();

		if(!GameManager)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Error at ACustomARPawn::OnShootReleased: Game manager was NULL"));
			return;
		}
		StaticMeshComponent->PutRigidBodyToSleep();
		StaticMeshComponent->SetSimulatePhysics(false);
		this->Destroy();
		GameManager->RemoveBall(this);
	}
}

