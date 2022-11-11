// Fill out your copyright notice in the Description page of Project Settings.


#include "BasketBall.h"

// Sets default values
ABasketBall::ABasketBall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Init the actor
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SceneComponent);

	const auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	StaticMeshComponent->SetStaticMesh(MeshAsset.Object);
}

// Called when the game starts or when spawned
void ABasketBall::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABasketBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

