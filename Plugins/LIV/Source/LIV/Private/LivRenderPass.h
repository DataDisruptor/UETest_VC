// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "RenderGraphEvent.h"
#include "PixelShaderUtils.h"
#include "RenderGraphBuilder.h"

#if PLATFORM_WINDOWS

// Only use by name in this header, we use private includes
// to access their definitions, should not leak into public
class FScreenPassTextureViewport;
struct FScreenPassPipelineState;
class FRenderTarget;
class FTextureResource;

struct LIV_API FLivRenderPass
{
	static void InitLivPassPipelineState(FRHICommandList& RHICmdList, 
		const FScreenPassTextureViewport& Viewport, 
		const FScreenPassPipelineState& PipelineState);
	
	static void DrawLivPass(FRHICommandList& RHICmdList,
		const FScreenPassTextureViewport& Viewport,
		const FScreenPassPipelineState& PipelineState);
	
	template <typename TShaderClass>
	static void AddLivPass(
		FRDGBuilder& GraphBuilder,
		FRDGEventName&& PassName,
		const FScreenPassTextureViewport& Viewport,
		const FScreenPassPipelineState& PipelineState,
		const TShaderRef<TShaderClass>& PixelShader,
		typename TShaderClass::FParameters* Parameters)
	{
		ClearUnusedGraphResources(PixelShader, Parameters);
		
		GraphBuilder.AddPass(
			Forward<FRDGEventName>(PassName),
			Parameters,
			ERDGPassFlags::Raster,
			[Parameters, Viewport, PixelShader, PipelineState](FRHICommandList& RHICmdList)
			{
				InitLivPassPipelineState(RHICmdList, Viewport, PipelineState);
				
				SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *Parameters);

				DrawLivPass(RHICmdList, Viewport, PipelineState);
			}
		);
	}

	static void AddSubmitPass(class FRDGBuilder& GraphBuilder, class FLivSubmitParameters* Parameters);

	static FRDGTextureRef CreateRDGTextureFromRenderTarget(FRDGBuilder& GraphBuilder, const FRenderTarget* RenderTarget, const TCHAR* DebugName = nullptr);
	static FRDGTextureRef CreateRDGTextureFromRenderTarget(FRDGBuilder& GraphBuilder, const FTextureResource* TextureResource, const TCHAR* DebugName = nullptr);
};

#endif