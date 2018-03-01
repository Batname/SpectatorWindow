// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/RHI/Public/RHI.h"
#include "Runtime/SlateCore/Public/Widgets/SWindow.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "MV_SpectatorWindowActor.generated.h"

class UTexture2D;

class UMV_UserWidget;

UCLASS()
class MULTIWINDOW_API AMV_SpectatorWindowActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMV_SpectatorWindowActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called before destroying the objec
	virtual void BeginDestroy() override;

	// called whenever this actor is being removed from a level
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// update class instances in the editor if changes are made to their properties
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called after the actors components have been initialized
	virtual void PostInitializeComponents() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Dynamic texture ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MV")
	int32 DynamicTextureWidth = 2560;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MV")
	int32 DynamicTextureHeight = 1440;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MV")
	int32 DynamicTextureBytesPerPixel = 4;

private:
	uint8* DynamicTextureBuffer;
	int32 DynamicTextureBufferSize;
	int32 DynamicTextureBufferSizeSqrt;

	FUpdateTextureRegion2D* DynamicTextureUpdateTextureRegion;

	UTexture2D* DynamicTexture = nullptr;

	void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);

	void CreateTexture(bool argForceMake);
	void UpdateTexture();

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ UMG Widget ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
protected:
	UMV_UserWidget* MV_UserWidget;

	/** Reference to blueprint class*/
	UPROPERTY(EditDefaultsOnly, Category = "MV")
	TSubclassOf<UMV_UserWidget> MV_UserWidgetBP;


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Copy from frame buffer  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
protected:
	bool bIsBufferReady = false;

	FVector2D ViewportPositionGeometry = FVector2D::ZeroVector;
	FVector2D ViewportSizeGeometry = FVector2D::ZeroVector;

	bool FindViewportGeometry(TSharedPtr<SWindow> WindowWidget, FGeometry& OutGeometry) const;
	bool FindViewportGeometryInternal(const FGeometry& Geometry, TSharedPtr<SWidget> Widget, FGeometry& OutGeometry) const;

	void OnSlateRendered(class SWindow& SlateWindow, void* ViewportRHIPtr);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Miscellaneous ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
private:
	APlayerController* PlayerController;
};
