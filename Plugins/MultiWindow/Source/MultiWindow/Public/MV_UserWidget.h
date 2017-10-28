// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MV_UserWidget.generated.h"

class FSceneViewport;
class SWindow;
class SOverlay;
class SViewport;

class UMaterialInstanceDynamic;


/**
 * 
 */
UCLASS()
class MULTIWINDOW_API UMV_UserWidget : public UUserWidget
{
	GENERATED_BODY()

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Window settings ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
public:
	UMV_UserWidget(const FObjectInitializer& ObjectInitializer);

	void CreateSpectatorWindow(UTexture2D* DynamicTex);

	void DestroySpectatorWindow();

	virtual void  NativeConstruct() override;

	virtual void NativeTick(const FGeometry & MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MV")
	int32 WindowSizeX = 1280;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MV")
	int32 WindowSizeY = 720;


	UPROPERTY(BlueprintReadWrite, Category = "MV")
	UTexture2D* DynamicTexture = nullptr;

	EWindowMode::Type WindowMode;

	TSharedPtr<FSceneViewport> SpectatorUISceneViewport = nullptr;
	TSharedPtr<SWindow> SpectatorWindow = nullptr;
	TSharedPtr<SOverlay> SpectatorOverlayWidget = nullptr;
	TSharedPtr<SViewport> SpectatorViewport = nullptr;

	UFUNCTION(BlueprintCallable, Category = "MV")
	void SetIsStandaloneGame(bool IsStandalone);


private:
	bool bIsStandaloneGame = false;
};
