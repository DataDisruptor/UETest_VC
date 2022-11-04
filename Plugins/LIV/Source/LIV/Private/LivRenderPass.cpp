// Copyright 2021 LIV Inc. - MIT License
#include "LivRenderPass.h"

#if PLATFORM_WINDOWS

#include "ScreenPass.h"
#include "SceneFilterRendering.h"
#include "LivConversions.h"
#include "LivShaders.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "LIV.h"
#include "LivPluginSettings.h"
#include "Windows/HideWindowsPlatformTypes.h"

void FLivRenderPass::InitLivPassPipelineState(FRHICommandList& RHICmdList, 
	const FScreenPassTextureViewport& Viewport,
	const FScreenPassPipelineState& PipelineState)
{
	RHICmdList.SetViewport(
		Viewport.Rect.Min.X,
		Viewport.Rect.Min.Y,
		0.0f,
		Viewport.Rect.Max.X,
		Viewport.Rect.Max.Y,
		1.0f);

	SetScreenPassPipelineState(RHICmdList, PipelineState);
}


void FLivRenderPass::DrawLivPass(FRHICommandList& RHICmdList, 
	const FScreenPassTextureViewport& Viewport,
	const FScreenPassPipelineState& PipelineState)
{
	DrawPostProcessPass(
		RHICmdList,
		Viewport.Rect.Min.X, Viewport.Rect.Min.Y,
		Viewport.Rect.Width(), Viewport.Rect.Height(),
		Viewport.Rect.Min.X, Viewport.Rect.Min.Y,
		Viewport.Rect.Width(), Viewport.Rect.Height(),
		Viewport.Rect.Size(),
		Viewport.Extent,
		PipelineState.VertexShader,
		eSSP_FULL,
		false,
		EDRF_UseTriangleOptimization);
}


void FLivRenderPass::AddSubmitPass(FRDGBuilder& GraphBuilder, FLivSubmitParameters* Parameters)
{
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("RDG Liv Submit Pass"),
		Parameters,
		ERDGPassFlags::Copy | ERDGPassFlags::NeverCull,
		[Parameters](FRHICommandList& InRHICmdList)
		{
			const auto OutForegroundPRT = Parameters->ForegroundTexture->GetPooledRenderTarget();
			const auto OutForegroundRHI = OutForegroundPRT->GetRenderTargetItem().ShaderResourceTexture->GetTexture2D();
			const auto ForegroundNativeTexture = static_cast<ID3D11Texture2D*>(OutForegroundRHI->GetNativeResource());

			LIV_Texture LivForegroundTexture{};
			LivForegroundTexture.type = LIV_TEXTURE_TYPE_COLOR_BUFFER;
			LivForegroundTexture.id = LIV_TEXTURE_FOREGROUND_COLOR_BUFFER_ID;
			LivForegroundTexture.dxgi_pixelFormat = GetRenderTargetFormat(OutForegroundRHI->GetFormat());
			LivForegroundTexture.d3d11_texturePtr = ForegroundNativeTexture;
			LivForegroundTexture.width = static_cast<int>(OutForegroundRHI->GetSizeX());
			LivForegroundTexture.height = -static_cast<int>(OutForegroundRHI->GetSizeY());
			LivForegroundTexture.colorSpace = LIV_TEXTURE_COLOR_SPACE_SRGB;

			const auto OutBackgroundPRT = Parameters->BackgroundTexture->GetPooledRenderTarget();
			const auto OutBackgroundRHI = OutBackgroundPRT->GetRenderTargetItem().ShaderResourceTexture->GetTexture2D();
			const auto BackgroundNativeTexture = static_cast<ID3D11Texture2D*>(OutBackgroundRHI->GetNativeResource());

			LIV_Texture LivBackgroundTexture{};
			LivBackgroundTexture.type = LIV_TEXTURE_TYPE_COLOR_BUFFER;
			LivBackgroundTexture.id = LIV_TEXTURE_BACKGROUND_COLOR_BUFFER_ID;
			LivBackgroundTexture.dxgi_pixelFormat = GetRenderTargetFormat(OutBackgroundRHI->GetFormat());
			LivBackgroundTexture.d3d11_texturePtr = BackgroundNativeTexture;
			LivBackgroundTexture.width = static_cast<int>(OutBackgroundRHI->GetSizeX());
			LivBackgroundTexture.height = -static_cast<int>(OutBackgroundRHI->GetSizeY());
			LivBackgroundTexture.colorSpace = LIV_TEXTURE_COLOR_SPACE_SRGB;

			LIV_AddTexture(&LivForegroundTexture);
			Parameters->ForegroundTexture->MarkResourceAsUsed();

			LIV_AddTexture(&LivBackgroundTexture);
			Parameters->BackgroundTexture->MarkResourceAsUsed();

			LIV_Submit();
		}
	);
}


FRDGTextureRef FLivRenderPass::CreateRDGTextureFromRenderTarget(
	FRDGBuilder& GraphBuilder,
	const FRenderTarget* RenderTarget,
	const TCHAR* DebugName)
{
	const FTexture2DRHIRef& Texture = RenderTarget->GetRenderTargetTexture();
	ensure(Texture.IsValid());

	FSceneRenderTargetItem Item;
	Item.TargetableTexture = Texture;
	Item.ShaderResourceTexture = Texture;

	FPooledRenderTargetDesc Desc;

	if (Texture)
	{
		Desc.Extent = Texture->GetSizeXY();
		Desc.Format = Texture->GetFormat();
		Desc.NumMips = Texture->GetNumMips();
	}
	else
	{
		Desc.Extent = RenderTarget->GetSizeXY();
		Desc.NumMips = 1;
	}

	Desc.DebugName = DebugName ? DebugName : TEXT("RenderTarget");
	Desc.TargetableFlags |= TexCreate_RenderTargetable | TexCreate_ShaderResource;

	if (RenderTarget->GetRenderTargetUAV().IsValid())
	{
		Desc.TargetableFlags |= TexCreate_UAV;
	}


	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;
	GRenderTargetPool.CreateUntrackedElement(Desc, PooledRenderTarget, Item);

	return GraphBuilder.RegisterExternalTexture(PooledRenderTarget, DebugName ? DebugName : TEXT("RenderTarget"));
}

FRDGTextureRef FLivRenderPass::CreateRDGTextureFromRenderTarget(
	FRDGBuilder& GraphBuilder,
	const FTextureResource* TextureResource,
	const TCHAR* DebugName)
{
	const FTextureRHIRef Texture = TextureResource->TextureRHI;

	ensure(Texture.IsValid());

	const FRHITexture2D* Texture2D = Texture->GetTexture2D();
	check(Texture2D != nullptr);

	FSceneRenderTargetItem Item;
	Item.TargetableTexture = Texture;
	Item.ShaderResourceTexture = Texture;

	FPooledRenderTargetDesc Desc;

	Desc.Extent = Texture2D->GetSizeXY();
	Desc.Format = Texture2D->GetFormat();
	Desc.NumMips = Texture2D->GetNumMips();

	Desc.DebugName = DebugName ? DebugName : TEXT("RenderTarget");
	Desc.TargetableFlags |= TexCreate_RenderTargetable | TexCreate_ShaderResource;

	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;
	GRenderTargetPool.CreateUntrackedElement(Desc, PooledRenderTarget, Item);

	return GraphBuilder.RegisterExternalTexture(PooledRenderTarget, DebugName ? DebugName : TEXT("RenderTarget"));
}

#endif