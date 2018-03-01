// Fill out your copyright notice in the Description page of Project Settings.

#include "MV_SpectatorWindowActor.h"
#include "MV_UserWidget.h"

#include "Runtime/Engine/Classes/Components/SphereComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "Runtime/RenderCore/Public/RendererInterface.h"

#include "Engine/Engine.h"

#include "SlateBasics.h"
#include "ScreenRendering.h"
#include "RenderCore.h"
#include "RHIStaticStates.h"
#include "RendererInterface.h"
#include "SlateApplication.h"
#include "PipelineStateCache.h"

// Sets default values
AMV_SpectatorWindowActor::AMV_SpectatorWindowActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create just for visualisation
	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(40.0f);
}

// Called when the game starts or when spawned
void AMV_SpectatorWindowActor::BeginPlay()
{
	Super::BeginPlay();

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
		MV_UserWidget->CreateSpectatorWindow(DynamicTexture);
	}

	// Register slate rendered delegate
	FSlateRenderer* SlateRenderer = FSlateApplication::Get().GetRenderer();//.Get();
	SlateRenderer->OnSlateWindowRendered().RemoveAll(this);
	SlateRenderer->OnSlateWindowRendered().AddUObject(this, &AMV_SpectatorWindowActor::OnSlateRendered);
}

void AMV_SpectatorWindowActor::BeginDestroy()
{
	Super::BeginDestroy();
}

void AMV_SpectatorWindowActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (FSlateApplication::IsInitialized())	// During shutdown, Slate may have already been destroyed by the time our viewport gets cleaned up
	{
		UE_LOG(LogTemp, Warning, TEXT("EndPlay SlateRenderer"));
		FSlateRenderer* SlateRenderer = FSlateApplication::Get().GetRenderer();
		SlateRenderer->OnSlateWindowRendered().RemoveAll(this);

		FlushRenderingCommands();
	}

	// Stop copy to texture
	bIsBufferReady = false;

	// Destroy Window, only if widget exists
	if (MV_UserWidget != nullptr)
	{
		MV_UserWidget->DestroySpectatorWindow();
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AMV_SpectatorWindowActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update Viewport geometry
	if (PlayerController && PlayerController->GetLocalPlayer())
	{
		UGameViewportClient* GameViewportClient = PlayerController->GetLocalPlayer()->ViewportClient;
		FGeometry ViewportGeometry;
		const bool bResult = FindViewportGeometry(GameViewportClient->GetWindow(), ViewportGeometry);
		if (bResult)
		{
			ViewportPositionGeometry = ViewportGeometry.LocalToAbsolute(FVector2D::ZeroVector);
			ViewportSizeGeometry = ViewportGeometry.GetLocalSize();
		}
	}

	// Update texture
	if (bIsBufferReady && DynamicTexture != nullptr)
	{
		UpdateTexture();
	}
}

void AMV_SpectatorWindowActor::UpdateTextureRegions(UTexture2D * Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D * Regions, uint32 SrcPitch, uint32 SrcBpp, uint8 * SrcData, bool bFreeData)
{
	if (Texture && Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}

		if (bFreeData)
		{
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			});
	}
}

void AMV_SpectatorWindowActor::CreateTexture(bool argForceMake)
{
	if (DynamicTexture == nullptr || argForceMake == true)
	{
		// create buffers to collate pixel data into
		DynamicTextureBufferSize = DynamicTextureWidth * DynamicTextureHeight * DynamicTextureBytesPerPixel;
		DynamicTextureBufferSizeSqrt = DynamicTextureWidth * DynamicTextureBytesPerPixel;
		DynamicTextureBuffer = new uint8[DynamicTextureBufferSize];

		// create dynamic texture
		DynamicTexture = UTexture2D::CreateTransient(DynamicTextureWidth, DynamicTextureHeight, EPixelFormat::PF_R8G8B8A8);
		DynamicTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		DynamicTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
		DynamicTexture->AddToRoot();		// Guarantee no garbage collection by adding it as a root reference
		DynamicTexture->UpdateResource();	// Update the texture with new variable values.

		// Create a new texture region with the width and height of our dynamic texture
		DynamicTextureUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, DynamicTextureWidth, DynamicTextureHeight);
	}
}

void AMV_SpectatorWindowActor::UpdateTexture()
{
	UpdateTextureRegions(DynamicTexture, 0, 1, DynamicTextureUpdateTextureRegion, DynamicTextureBufferSizeSqrt, (uint32)4, DynamicTextureBuffer, false);
}

bool AMV_SpectatorWindowActor::FindViewportGeometry(TSharedPtr<SWindow> WindowWidget, FGeometry & OutGeometry) const
{
	if (WindowWidget.IsValid())
	{
		return FindViewportGeometryInternal(WindowWidget->GetWindowGeometryInWindow(), WindowWidget, OutGeometry);
	}

	return false;
}

bool AMV_SpectatorWindowActor::FindViewportGeometryInternal(const FGeometry & Geometry, TSharedPtr<SWidget> Widget, FGeometry & OutGeometry) const
{
	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	Widget->ArrangeChildren(Geometry, ArrangedChildren);
	for (int32 Index = 0; Index < ArrangedChildren.Num(); ++Index)
	{
		TSharedPtr<SWidget> ChildWidget = ArrangedChildren[Index].Widget;
		FGeometry ChildGeometry = ArrangedChildren[Index].Geometry;

		//@todo: Don't understand why casting not working??? It's always return true .IsValid()
		//TSharedPtr<SViewport> Viewport = StaticCastSharedPtr<SViewport>(ChildWidget);
		// !!! OK !!! I know now why it is not working. We need dynamic cast. My Bad :{
		static FName NAME_Viewport(TEXT("SGameLayerManager"));
		if (ChildWidget->GetType() == NAME_Viewport)
		{
			OutGeometry = ArrangedChildren[Index].Geometry;
			return true;
		}
		else
		{
			const bool bResult = FindViewportGeometryInternal(ChildGeometry, ChildWidget, OutGeometry);
			if (bResult)
			{
				return true;
			}
		}
	}

	return false;
}

void AMV_SpectatorWindowActor::OnSlateRendered(SWindow & SlateWindow, void * ViewportRHIPtr)
{
	if (GEngine == nullptr || GEngine->GameViewport == nullptr || GWorld == nullptr)
	{
		return;
	}

	if (!SlateWindow.IsFocusedInitially())
	{
		return;
	}

	const FViewportRHIRef* ViewportRHI = (const FViewportRHIRef*)ViewportRHIPtr;
	static const FName RendererModuleName("Renderer");
	IRendererModule& RendererModule = FModuleManager::GetModuleChecked<IRendererModule>(RendererModuleName);

	UGameViewportClient* GameViewportClient = GEngine->GameViewport;
	FVector2D WindowSize = GameViewportClient->GetWindow()->GetSizeInScreen();
	FVector2D UV = ViewportPositionGeometry / WindowSize;
	FVector2D UVSize = ViewportSizeGeometry / WindowSize;

	struct FCopyVideoFrame
	{
		FViewportRHIRef ViewportRHI;
		IRendererModule* RendererModule;
		FIntPoint Resolution;
		FVector2D UV;
		FVector2D UVSize;
	};

	FCopyVideoFrame CopyVideoFrame =
	{
		*ViewportRHI,
		&RendererModule,
		FIntPoint(DynamicTextureWidth, DynamicTextureHeight),
		UV,
		UVSize,
	};

	FCopyVideoFrame Context = CopyVideoFrame;

	ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
		[&, Context](FRHICommandListImmediate& RHICmdList)
	{
		FPooledRenderTargetDesc OutputDesc(FPooledRenderTargetDesc::Create2DDesc(Context.Resolution, PF_R8G8B8A8, FClearValueBinding::None, TexCreate_None, TexCreate_RenderTargetable, false));

		const auto FeatureLevel = GMaxRHIFeatureLevel;

		TRefCountPtr<IPooledRenderTarget> ResampleTexturePooledRenderTarget;
		Context.RendererModule->RenderTargetPoolFindFreeElement(RHICmdList, OutputDesc, ResampleTexturePooledRenderTarget, TEXT("RemoteControlTexture"));
		check(ResampleTexturePooledRenderTarget);

		const FSceneRenderTargetItem& DestRenderTarget = ResampleTexturePooledRenderTarget->GetRenderTargetItem();

		SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());
		RHICmdList.SetViewport(0, 0, 0.0f, Context.Resolution.X, Context.Resolution.Y, 1.0f);

		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

		// @todo livestream: Ideally this "desktop background color" should be configurable in the editor's preferences
		//RHICmdList.Clear(true, FLinearColor(0.02f, 0.02f, 0.2f), false, 0.f, false, 0x00, FIntRect());

		FTexture2DRHIRef ViewportBackBuffer = RHICmdList.GetViewportBackBuffer(Context.ViewportRHI);

		auto ShaderMap = GetGlobalShaderMap(FeatureLevel);
		TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
		TShaderMapRef<FScreenPS> PixelShader(ShaderMap);

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = Context.RendererModule->GetFilterVertexDeclaration().VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
		//RHICmdList.SetGraphicsPipelineState(GetAndOrCreateGraphicsPipelineState(RHICmdList, GraphicsPSOInit, EApplyRendertargetOption::CheckApply));

		PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), ViewportBackBuffer);

		Context.RendererModule->DrawRectangle(
			RHICmdList,
			0, 0,		// Dest X, Y
			Context.Resolution.X, Context.Resolution.Y,	// Dest Width, Height
			Context.UV.X, Context.UV.Y,		// Source U, V
			Context.UVSize.X, Context.UVSize.Y,		// Source USize, VSize
			Context.Resolution,		// Target buffer size
			FIntPoint(1, 1),		// Source texture size
			*VertexShader,
			EDRF_Default);

		FIntRect Rect = FIntRect(0, 0, Context.Resolution.X, Context.Resolution.Y);

		TArray<FColor> OutData;
		RHICmdList.ReadSurfaceData(
			DestRenderTarget.TargetableTexture,
			Rect,
			OutData,
			FReadSurfaceDataFlags()
		);

		// BGRA to RGBA
		for (int32 Index = 0; Index < OutData.Num(); Index++)
		{
			auto Tmp = OutData[Index].B;
			OutData[Index].B = OutData[Index].R;
			OutData[Index].R = Tmp;
		}

		FMemory::Memcpy(DynamicTextureBuffer, OutData.GetData(), DynamicTextureBufferSize);
		bIsBufferReady = true;
	});
}

void AMV_SpectatorWindowActor::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	CreateTexture(true); // do force new texture
	UpdateTexture();
}

void AMV_SpectatorWindowActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CreateTexture(true); // do force new texture
	UpdateTexture();
}

