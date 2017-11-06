// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "MV_GameViewportClient.generated.h"

/**
 * 
 */
UCLASS()
class MULTIWINDOW_API UMV_GameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
	
	virtual void Draw(FViewport* Viewport, FCanvas* SceneCanvas) override;
	virtual void ProcessScreenShots(FViewport* Viewport) override;
	
};
