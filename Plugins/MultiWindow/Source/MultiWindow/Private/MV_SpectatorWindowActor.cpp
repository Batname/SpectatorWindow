// Fill out your copyright notice in the Description page of Project Settings.

#include "MV_SpectatorWindowActor.h"
#include "MV_UserWidget.h"

#include "Runtime/Engine/Classes/Components/SphereComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"


// Sets default values
AMV_SpectatorWindowActor::AMV_SpectatorWindowActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create just for visualisation
	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(40.0f);

	// Init Default Wariables

}

// Called when the game starts or when spawned
void AMV_SpectatorWindowActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("AMV_SpectatorWindowActor::BeginPlay"));

	// Create UMG widget
	if (MV_UserWidgetBP != nullptr)
	{
		MV_UserWidget = CreateWidget<UMV_UserWidget>(GetWorld(), MV_UserWidgetBP);
	}

	// Set reference to player controller
	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	// Create Window, only if widget exists
	if (MV_UserWidget != nullptr)
	{
		MV_UserWidget->CreateSpectatorWindow();
	}
}

void AMV_SpectatorWindowActor::BeginDestroy()
{
	Super::BeginDestroy();

	// Destroy Window, only if widget exists
	if (MV_UserWidget != nullptr)
	{
		MV_UserWidget->DestroySpectatorWindow();
	}
}

void AMV_SpectatorWindowActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UE_LOG(LogTemp, Warning, TEXT("AMV_SpectatorWindowActor::EndPlay"));
}

// Called every frame
void AMV_SpectatorWindowActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UE_LOG(LogTemp, Warning, TEXT("AMV_SpectatorWindowActor::Tick"));

}

void AMV_SpectatorWindowActor::UpdateTextureRegions(UTexture2D * Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D * Regions, uint32 SrcPitch, uint32 SrcBpp, uint8 * SrcData, bool bFreeData)
{
}

bool AMV_SpectatorWindowActor::FindViewportGeometry(TSharedPtr<SWindow> WindowWidget, FGeometry & OutGeometry) const
{
	return false;
}

bool AMV_SpectatorWindowActor::FindViewportGeometryInternal(const FGeometry & Geometry, TSharedPtr<SWidget> Widget, FGeometry & OutGeometry) const
{
	return false;
}

void AMV_SpectatorWindowActor::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);
}

void AMV_SpectatorWindowActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

