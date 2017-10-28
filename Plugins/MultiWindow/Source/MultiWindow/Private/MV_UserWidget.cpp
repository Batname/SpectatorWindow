// Fill out your copyright notice in the Description page of Project Settings.

#include "MV_UserWidget.h"

#include "Runtime/Engine/Public/Slate/SceneViewport.h"
#include "Runtime/SlateCore/Public/Widgets/SWindow.h"
#include "Runtime/Slate/Public/Widgets/SViewport.h"
#include "Runtime/SlateCore/Public/Widgets/SOverlay.h"
#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "Runtime/Slate/Public/Widgets/Layout/SConstraintCanvas.h"

UMV_UserWidget::UMV_UserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	// Set Default Window mode
	WindowMode = EWindowMode::Windowed;
}


void UMV_UserWidget::SetIsStandaloneGame(bool IsStandalone)
{
	bIsStandaloneGame = IsStandalone;
}

void UMV_UserWidget::CreateSpectatorWindow(UTexture2D* DynamicTex)
{
	// Set Dynamic Texture
	DynamicTexture = DynamicTex;

	// Create and add window
	SpectatorWindow = SNew(SWindow)
		.ScreenPosition(FVector2D(100, 100))
		.ClientSize(FVector2D(WindowSizeX, WindowSizeY))
		.SizingRule(ESizingRule::UserSized)
		.Title(FText::FromString(TEXT("SpectatorWindow")))
		.AutoCenter(EAutoCenter::PrimaryWorkArea)
		.FocusWhenFirstShown(false)
		.UseOSWindowBorder(true)
		.UserResizeBorder(true)
		.CreateTitleBar(true)
		.SupportsTransparency(EWindowTransparency::PerWindow)
		.InitialOpacity(1.f)
		.SupportsMaximize(true);
	FSlateApplication & SlateApp = FSlateApplication::Get();
	SpectatorWindow->SetViewportSizeDrivenByWindow(false);

	SlateApp.AddWindow(SpectatorWindow.ToSharedRef(), true);

	TSharedPtr<SWidget> SlateWidget = this->TakeWidget();
	TSharedRef<SConstraintCanvas> ViewportWidget = SNew(SConstraintCanvas);

	// Add slate widget to window
	ViewportWidget->AddSlot()
		.Offset(BIND_UOBJECT_ATTRIBUTE(FMargin, GetFullScreenOffset))
		.Anchors(BIND_UOBJECT_ATTRIBUTE(FAnchors, GetAnchorsInViewport))
		.Alignment(BIND_UOBJECT_ATTRIBUTE(FVector2D, GetAlignmentInViewport))
		[
			SlateWidget.ToSharedRef()
		];


	ViewportWidget->SetVisibility(EVisibility::Visible);

	SpectatorWindow->SetContent(ViewportWidget);
	SpectatorWindow->ShowWindow();
	SpectatorWindow->SetWindowMode(WindowMode);

	// Set game Type
	SetIsStandaloneGame(GetWorld()->WorldType == EWorldType::Game);
}

void UMV_UserWidget::DestroySpectatorWindow()
{
	// Destroy window
	if (SpectatorWindow.Get() != nullptr)
	{
		if (bIsStandaloneGame == false)
		{
			SpectatorWindow->RequestDestroyWindow();
		}
		else
		{
			SpectatorWindow->DestroyWindowImmediately();
		}
	}
}

void UMV_UserWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMV_UserWidget::NativeTick(const FGeometry & MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}
