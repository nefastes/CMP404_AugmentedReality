// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaceableActor.h"
#include "ARPin.h"
#include "ARBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APlaceableActor::APlaceableActor() : 
	bSelected(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SceneComponent);

	auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Assets/Meshes/hoop/hoop.hoop'"));
	StaticMeshComponent->SetStaticMesh(MeshAsset.Object);

	// Load materials
	static ConstructorHelpers::FObjectFinder<UMaterial> HoopMaterialAsset(TEXT("Material'/Game/Assets/Materials/Colour.Colour'"));
	pMaterial = HoopMaterialAsset.Object;
}

// Called when the game starts or when spawned
void APlaceableActor::BeginPlay()
{
	// Assign dynamic materials
	pDynamicMaterial = UMaterialInstanceDynamic::Create(pMaterial, this);
	StaticMeshComponent->SetMaterial(0, pDynamicMaterial);
	
	Super::BeginPlay();
}

// Called every frame
void APlaceableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Making sure the actor remains on the ARPin that has been found.
	if(PinComponent)
	{
		auto TrackingState = PinComponent->GetTrackingState();
		
		switch (TrackingState)
		{
		case EARTrackingState::Tracking:
			SceneComponent->SetVisibility(true);
			SetActorTransform(PinComponent->GetLocalToWorldTransform());
			break;

		case EARTrackingState::NotTracking:
			PinComponent = nullptr;
			break;
		}
	}

	// Change the color of the hoop based on the camera's proximity
	if (!bSelected)
	{
		// Change the colour of the actor based on the player distance
		auto controller = UGameplayStatics::GetPlayerController(this, 0);
		FVector cameraPos = controller->PlayerCameraManager->GetCameraLocation();
		FVector actorPos = GetActorLocation();
		FVector diff = actorPos - cameraPos;
		if (diff.GetAbs().Length() < 100.0)
			pDynamicMaterial->SetVectorParameterValue(TEXT("InputColour"), FVector(0.f, 0.f, 1.f));
		else
			pDynamicMaterial->SetVectorParameterValue(TEXT("InputColour"), FVector(1.f, 0.f, 0.f));
	}
	else pDynamicMaterial->SetVectorParameterValue(TEXT("InputColour"), FVector(1.f, 1.f, 0.f));

}

