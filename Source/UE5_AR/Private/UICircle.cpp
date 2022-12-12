// Fill out your copyright notice in the Description page of Project Settings.


#include "UICircle.h"

#include "Components/BillboardComponent.h"

// Sets default values
AUICircle::AUICircle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SceneComponent);

	const auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Assets/Meshes/2DCircle.2DCircle'"));
	StaticMeshComponent->SetStaticMesh(MeshAsset.Object);
	StaticMeshComponent->SetSimulatePhysics(false);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Load materials
	static ConstructorHelpers::FObjectFinder<UMaterial> HoopMaterialAsset(TEXT("Material'/Game/Assets/Materials/DynamicCircle_Mat.DynamicCircle_Mat'"));
	pMaterial = HoopMaterialAsset.Object;
}

AUICircle::~AUICircle()
{
	vCustomUpdates.clear();
}

// Called when the game starts or when spawned
void AUICircle::BeginPlay()
{
	// Assign dynamic materials
	pDynamicMaterial = UMaterialInstanceDynamic::Create(pMaterial, this);
	StaticMeshComponent->SetMaterial(0, pDynamicMaterial);
	
	Super::BeginPlay();
}

// Called every frame
void AUICircle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Perform any given custom update
	for(const auto& function : vCustomUpdates)
		function(this);
}

void AUICircle::SetColour(const FVector& colour) const
{
	pDynamicMaterial->SetVectorParameterValue(TEXT("Circle Colour"), colour);
}

void AUICircle::AddCustomUpdate(std::function<void(AUICircle* thisPtr)> function)
{
	vCustomUpdates.push_back(function);
}

