// Fill out your copyright notice in the Description page of Project Settings.

#include "MV_GameViewportClient.h"

void UMV_GameViewportClient::Draw(FViewport * Viewport, FCanvas * SceneCanvas)
{
	Super::Draw(Viewport, SceneCanvas);
}

void UMV_GameViewportClient::ProcessScreenShots(FViewport * Viewport)
{
	Super::ProcessScreenShots(Viewport);

	if (GIsDumpingMovie || FScreenshotRequest::IsScreenshotRequested() || GIsHighResScreenshot)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMV_GameViewportClient::ProcessScreenShots"));
	}
}
