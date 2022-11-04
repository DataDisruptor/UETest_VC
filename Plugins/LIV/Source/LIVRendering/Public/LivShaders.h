// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "CopyTextureShaders.h"
#include "GlobalShader.h"
#include "MeshMaterialShader.h"
#include "RenderTargetPool.h"
#include "RHIDefinitions.h"
#include "Shader.h"
#include "ShaderParameters.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterMacros.h"
#include "ScreenPass.h"
#include "SceneView.h"
#include "SceneRenderTargetParameters.h"
#include "PostProcessTonemap.h"
#include "TextureResource.h"

/**
 * Simple vertex shader for alpha inversion of render target.
 * Does not require a vertex buffer as the vertices are encoded in the shader itself.
 */
class FLivSingleTextureVertexShader : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FLivSingleTextureVertexShader, Global, LIVRENDERING_API);
public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FLivSingleTextureVertexShader() {}

	FLivSingleTextureVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}
};

/**
 * Pixel shader for alpha inversion of render target.
 * Simply reads from one render target and writes to another with the alpha inverted.
 */
class FLivInvertAlphaPixelShader : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FLivInvertAlphaPixelShader, Global, LIVRENDERING_API);

public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FLivInvertAlphaPixelShader() {}

	FLivInvertAlphaPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		BindParams(Initializer.ParameterMap);
	}

	LIVRENDERING_API void SetInputParameters(FRHICommandList& InRHICmdList, FTextureResource* InInputTexture);
	LIVRENDERING_API void SetOutputParameters(FRHICommandList& InRHICmdList, FTextureResource* InOutputTexture);

	void BindParams(const FShaderParameterMap& ParameterMap);

protected:

	LAYOUT_FIELD(FShaderResourceParameter, InputTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InputTextureSampler);

	LAYOUT_FIELD(FShaderResourceParameter, OutputTexture);
	LAYOUT_FIELD(FShaderResourceParameter, OutputTextureSampler);

};


/**
 * Pixel shader to copy from one render target to another.
 */
class FLivCopyPixelShader : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FLivCopyPixelShader, Global, LIVRENDERING_API);

public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FLivCopyPixelShader() {}

	FLivCopyPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		BindParams(Initializer.ParameterMap);
	}

	LIVRENDERING_API void SetInputParameters(FRHICommandList& InRHICmdList, FTextureResource* InInputTexture);
	LIVRENDERING_API void SetOutputParameters(FRHICommandList& InRHICmdList, FTextureResource* InOutputTexture);

	void BindParams(const FShaderParameterMap& ParameterMap);

protected:

	LAYOUT_FIELD(FShaderResourceParameter, InputTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InputTextureSampler);

	LAYOUT_FIELD(FShaderResourceParameter, OutputTexture);
	LAYOUT_FIELD(FShaderResourceParameter, OutputTextureSampler);

};


class FLivCombineAlphaShader : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FLivCombineAlphaShader, Global, LIVRENDERING_API);

public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FLivCombineAlphaShader() {}

	FLivCombineAlphaShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		BindParams(Initializer.ParameterMap);
	}

	LIVRENDERING_API void SetInputParameters(FRHICommandList& InRHICmdList, FTextureResource* InColorTexture, FTextureResource* InAlphaTexture);
	LIVRENDERING_API void SetOutputParameters(FRHICommandList& InRHICmdList, FTextureResource* InOutputTexture);

	void BindParams(const FShaderParameterMap& ParameterMap);

protected:

	LAYOUT_FIELD(FShaderResourceParameter, InputColorTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InputColorTextureSampler);

	LAYOUT_FIELD(FShaderResourceParameter, InputAlphaTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InputAlphaTextureSampler);

	LAYOUT_FIELD(FShaderResourceParameter, OutputTexture);
	LAYOUT_FIELD(FShaderResourceParameter, OutputTextureSampler);

};


/**
 * Pixel shader for foreground segmentation when not using the global clip plane.
 */
class FLivForegroundSegmentationPixelShader : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FLivForegroundSegmentationPixelShader, Global, LIVRENDERING_API);

public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FLivForegroundSegmentationPixelShader() {}

	FLivForegroundSegmentationPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		BindParams(Initializer.ParameterMap);
	}

	LIVRENDERING_API void SetParameters(FRHICommandList& InRHICmdList, 
		FTextureResource* InInputBackgroundTexture, 
		FTextureResource* InInputForegroundTexture,
		FTextureResource* InOutputTexture);

	void BindParams(const FShaderParameterMap& ParameterMap);

protected:

	LAYOUT_FIELD(FShaderResourceParameter, InputForegroundTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InputForegroundTextureSampler);

	LAYOUT_FIELD(FShaderResourceParameter, InputBackgroundTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InputBackgroundTextureSampler);

	LAYOUT_FIELD(FShaderResourceParameter, OutputTexture);
	LAYOUT_FIELD(FShaderResourceParameter, OutputTextureSampler);

};


/**
 * Pixel shader for foreground segmentation when not using the global clip plane and using post processing.
 */
class FLivForegroundSegmentationPostProcessedPixelShader : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FLivForegroundSegmentationPostProcessedPixelShader, Global, LIVRENDERING_API);

public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FLivForegroundSegmentationPostProcessedPixelShader() {}

	FLivForegroundSegmentationPostProcessedPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		BindParams(Initializer.ParameterMap);
	}

	LIVRENDERING_API void SetParameters(FRHICommandList& InRHICmdList,
		FTextureResource* InInputSceneColorTexture,
		FTextureResource* InInputBackgroundDepthTexture,
		FTextureResource* InInputForegroundDepthTexture,
		FTextureResource* InOutputTexture);

	void BindParams(const FShaderParameterMap& ParameterMap);

protected:

	LAYOUT_FIELD(FShaderResourceParameter, InputSceneColorTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InputSceneColorTextureSampler);

	LAYOUT_FIELD(FShaderResourceParameter, InputBackgroundDepthTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InputBackgroundDepthTextureSampler);

	LAYOUT_FIELD(FShaderResourceParameter, InputForegroundDepthTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InputForegroundDepthTextureSampler);

	LAYOUT_FIELD(FShaderResourceParameter, OutputTexture);
	LAYOUT_FIELD(FShaderResourceParameter, OutputTextureSampler);

};

/**
 * RDG Shaders
 */

BEGIN_SHADER_PARAMETER_STRUCT(FLivSubmitParameters, LIVRENDERING_API)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, BackgroundTexture)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, ForegroundTexture)
END_SHADER_PARAMETER_STRUCT()

class FLivRDGScreenPassVS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGScreenPassVS, Global, LIVRENDERING_API);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters&)
	{
		return true;
	}

	FLivRDGScreenPassVS() = default;
	FLivRDGScreenPassVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{}
};

/**
* Pixel shader for alpha inversion of render target.
* Simply reads from one render target and writes to another with the alpha inverted.
*/
class FLivRDGInvertAlphaPS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGInvertAlphaPS, Global, LIVRENDERING_API);

	SHADER_USE_PARAMETER_STRUCT(FLivRDGInvertAlphaPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_TEXTURE(Texture2D, InputTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};

/**
 * Combines RGB color of one input and alpha of another.
 */
class FLivRDGCombineAlphaPS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGCombineAlphaPS, Global, LIVRENDERING_API);

	SHADER_USE_PARAMETER_STRUCT(FLivRDGCombineAlphaPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_TEXTURE(Texture2D, InputColorTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputColorSampler)
		SHADER_PARAMETER_TEXTURE(Texture2D, InputAlphaTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputAlphaSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};

/**
 * Does foreground segmentation for first bound render target and
 * a copy for the second render target.
 */
class FLivRDGForegroundSegmentationAndCopyPS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGForegroundSegmentationAndCopyPS, Global, LIVRENDERING_API);

	SHADER_USE_PARAMETER_STRUCT(FLivRDGForegroundSegmentationAndCopyPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_TEXTURE(Texture2D, InputBackgroundTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputBackgroundSampler)
		SHADER_PARAMETER_TEXTURE(Texture2D, InputForegroundTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputForegroundSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};

/**
 * Does foreground segmentation (post processed) for first bound render target and
 * a copy for the second render target.
 */
class FLivRDGForegroundSegmentationPPAndCopyPS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGForegroundSegmentationPPAndCopyPS, Global, LIVRENDERING_API);

	SHADER_USE_PARAMETER_STRUCT(FLivRDGForegroundSegmentationPPAndCopyPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_TEXTURE(Texture2D, InputBackgroundTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputBackgroundSampler)
		SHADER_PARAMETER_TEXTURE(Texture2D, InputBackgroundDepthTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputBackgroundDepthSampler)
		SHADER_PARAMETER_TEXTURE(Texture2D, InputForegroundDepthTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputForegroundDepthSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};


/**
 * Copy input (mainly for converting from floating point formats)
 */
class FLivRDGCopySceneColorDepthPS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGCopySceneColorDepthPS, Global, LIVRENDERING_API);

	SHADER_USE_PARAMETER_STRUCT(FLivRDGCopySceneColorDepthPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_STRUCT_INCLUDE(FSceneTextureShaderParameters, SceneTextures)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};

class FLivRDGCopySceneColorAndDepthPS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGCopySceneColorAndDepthPS, Global, LIVRENDERING_API);

	SHADER_USE_PARAMETER_STRUCT(FLivRDGCopySceneColorAndDepthPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_STRUCT_INCLUDE(FSceneTextureShaderParameters, SceneTextures)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};

class FLivRDGCopyFullSceneColorPS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGCopyFullSceneColorPS, Global, LIVRENDERING_API);

	SHADER_USE_PARAMETER_STRUCT(FLivRDGCopyFullSceneColorPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_STRUCT_INCLUDE(FSceneTextureShaderParameters, SceneTextures)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};


/**
 * Copy input (mainly for converting from floating point formats)
 */
class FLivRDGCopyDepthPS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGCopyDepthPS, Global, LIVRENDERING_API);

	SHADER_USE_PARAMETER_STRUCT(FLivRDGCopyDepthPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_STRUCT_INCLUDE(FSceneTextureShaderParameters, SceneTextures)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};

BEGIN_SHADER_PARAMETER_STRUCT(FLivFilmGrainParameters, )
SHADER_PARAMETER(FVector, GrainRandomFull)
SHADER_PARAMETER(FVector, GrainScaleBiasJitter)
END_SHADER_PARAMETER_STRUCT()

BEGIN_SHADER_PARAMETER_STRUCT(FLivTonemapParameters, )
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
	SHADER_PARAMETER_STRUCT_INCLUDE(FLivFilmGrainParameters, FilmGrain)
	SHADER_PARAMETER_STRUCT_INCLUDE(FTonemapperOutputDeviceParameters, OutputDevice)
	SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, Color)
	SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, Bloom)
	SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, Output)
	SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportTransform, ColorToBloom)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, ColorTexture)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, BloomTexture)
	// SM5 and above use Texture2D for EyeAdaptationTexture
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, EyeAdaptationTexture)
	SHADER_PARAMETER_RDG_TEXTURE(, ColorGradingLUT)
	SHADER_PARAMETER_TEXTURE(Texture2D, BloomDirtMaskTexture)
	SHADER_PARAMETER_SAMPLER(SamplerState, ColorSampler)
	SHADER_PARAMETER_SAMPLER(SamplerState, BloomSampler)
	SHADER_PARAMETER_SAMPLER(SamplerState, ColorGradingLUTSampler)
	SHADER_PARAMETER_SAMPLER(SamplerState, BloomDirtMaskSampler)
	SHADER_PARAMETER(FVector4, ColorScale0)
	SHADER_PARAMETER(FVector4, ColorScale1)
	SHADER_PARAMETER(FVector4, BloomDirtMaskTint)
	SHADER_PARAMETER(FVector4, ChromaticAberrationParams)
	SHADER_PARAMETER(FVector4, TonemapperParams)
	SHADER_PARAMETER(FVector4, LensPrincipalPointOffsetScale)
	SHADER_PARAMETER(FVector4, LensPrincipalPointOffsetScaleInverse)
	SHADER_PARAMETER(float, SwitchVerticalAxis)
	SHADER_PARAMETER(float, DefaultEyeExposure)
	SHADER_PARAMETER(float, EditorNITLevel)
	SHADER_PARAMETER(uint32, bOutputInHDR)
	// ES3_1 uses EyeAdaptationBuffer
	SHADER_PARAMETER_SRV(Buffer<float4>, EyeAdaptationBuffer)
END_SHADER_PARAMETER_STRUCT()

class FLivRDGSegmentPS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGSegmentPS, Global, LIVRENDERING_API);

	SHADER_USE_PARAMETER_STRUCT(FLivRDGSegmentPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_STRUCT_INCLUDE(FSceneTextureShaderParameters, SceneTextures)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputBackgroundTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputBackgroundSampler)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputBackgroundDepthTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputBackgroundDepthSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};

template<bool bPostProcessing>
class TLivRDGSegmentPS : public FLivRDGSegmentPS
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(TLivRDGSegmentPS, Global, LIVRENDERING_API);

	TLivRDGSegmentPS() {}
	TLivRDGSegmentPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLivRDGSegmentPS(Initializer)
	{}

	static const TCHAR* GetSourceFilename() { return TEXT("/Plugin/Liv/LivRDGSegmentPS.usf"); }
	static const TCHAR* GetFunctionName() { return TEXT("MainPS"); }

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		if(bPostProcessing)
		{
			OutEnvironment.SetDefine(TEXT("POSTPROCESSING"), 1);
		}
		else
		{
			OutEnvironment.SetDefine(TEXT("POSTPROCESSING"), 0);
		}
	}
};

/**
 * Does foreground segmentation (post processed) for first bound render target and
 * a copy for the second render target.
 */
class FLivRDGSegmentByDepthPS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGSegmentByDepthPS, Global, LIVRENDERING_API);

	SHADER_USE_PARAMETER_STRUCT(FLivRDGSegmentByDepthPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, BackgroundSceneColorDepthTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, BackgroundSceneColorDepthSampler)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, ForegroundSceneColorDepthTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, ForegroundSceneColorDepthSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};


class FLivRDGCopy2DCS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivRDGCopy2DCS, Global, LIVRENDERING_API);
		
	SHADER_USE_PARAMETER_STRUCT(FLivRDGCopy2DCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, SrcResource)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, DstResource)
		SHADER_PARAMETER(FIntVector, Dimensions)
	END_SHADER_PARAMETER_STRUCT()
};

/**
 * 
 */
template<ECopyTextureValueType ValueType, uint32 NumChannels>
class TLivRDGCopy2DCS : public FLivRDGCopy2DCS
{
public:
	DECLARE_EXPORTED_SHADER_TYPE(TLivRDGCopy2DCS, Global, LIVRENDERING_API);

	TLivRDGCopy2DCS() {}
	TLivRDGCopy2DCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLivRDGCopy2DCS(Initializer)
	{}

	//using FPermutationDomain = TShaderPermutationDomain<>;

	static constexpr uint32 ThreadGroupSizeX = CopyTextureCS::TThreadGroupSize<ECopyTextureResourceType::Texture2D>::X;
	static constexpr uint32 ThreadGroupSizeY = CopyTextureCS::TThreadGroupSize<ECopyTextureResourceType::Texture2D>::Y;
	static constexpr uint32 ThreadGroupSizeZ = CopyTextureCS::TThreadGroupSize<ECopyTextureResourceType::Texture2D>::Z;

	static const TCHAR* GetSourceFilename() { return TEXT("/Plugin/Liv/LivRDGCopyCS.usf"); }
	static const TCHAR* GetFunctionName() { return TEXT("CopyTextureCS"); }

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), ThreadGroupSizeX);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), ThreadGroupSizeY);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), ThreadGroupSizeZ);

		switch (ValueType)
		{
		case ECopyTextureValueType::Float:
			switch (NumChannels)
			{
			case 1: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("float"));  break;
			case 2: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("float2")); break;
			case 3: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("float3")); break;
			case 4: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("float4")); break;
			}
			break;

		case ECopyTextureValueType::Int32:
			switch (NumChannels)
			{
			case 1: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("int"));  break;
			case 2: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("int2")); break;
			case 3: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("int3")); break;
			case 4: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("int4")); break;
			}
			break;

		case ECopyTextureValueType::Uint32:
			switch (NumChannels)
			{
			case 1: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("uint"));  break;
			case 2: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("uint2")); break;
			case 3: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("uint3")); break;
			case 4: OutEnvironment.SetDefine(TEXT("VALUE_TYPE"), TEXT("uint4")); break;
			}
			break;
		}
	}
};


///


class FLivClipPlaneVS : public FMeshMaterialShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FLivClipPlaneVS, MeshMaterial, LIVRENDERING_API);

protected:

	FLivClipPlaneVS()
	{}

	FLivClipPlaneVS(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer)
		: FMeshMaterialShader(Initializer)
	{
		PassUniformBuffer.Bind(Initializer.ParameterMap, FSceneTextureUniformParameters::StaticStructMetadata.GetShaderVariableName());
	}

public:

	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	void GetShaderBindings(
		const FScene* Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		const FMeshPassProcessorRenderState& DrawRenderState,
		const FMeshMaterialShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);
	}
};


class FLivClipPlanePS : public FMeshMaterialShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FLivClipPlanePS, MeshMaterial, LIVRENDERING_API);

protected:

	FLivClipPlanePS()
	{}

	FLivClipPlanePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMeshMaterialShader(Initializer)
	{
		PassUniformBuffer.Bind(Initializer.ParameterMap, FSceneTextureUniformParameters::StaticStructMetadata.GetShaderVariableName());
	}

public:

	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	void GetShaderBindings(
		const FScene* Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		const FMeshPassProcessorRenderState& DrawRenderState,
		const FMeshMaterialShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);
	}
};

class FLivApplyEyeAdaptationPS : public FGlobalShader
{
public:

	DECLARE_EXPORTED_SHADER_TYPE(FLivApplyEyeAdaptationPS, Global, LIVRENDERING_API);

	SHADER_USE_PARAMETER_STRUCT(FLivApplyEyeAdaptationPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, EyeAdaptationTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, BackgroundTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, BackgroundSampler)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, ForegroundTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, ForegroundSampler)
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};

//////////////////////////////

// @TODO: move to separate header

BEGIN_SHADER_PARAMETER_STRUCT(FLivEyeAdaptationParameters, LIVRENDERING_API)
SHADER_PARAMETER(float, ExposureLowPercent)
SHADER_PARAMETER(float, ExposureHighPercent)
SHADER_PARAMETER(float, MinAverageLuminance)
SHADER_PARAMETER(float, MaxAverageLuminance)
SHADER_PARAMETER(float, ExposureCompensationSettings)
SHADER_PARAMETER(float, ExposureCompensationCurve)
SHADER_PARAMETER(float, DeltaWorldTime)
SHADER_PARAMETER(float, ExposureSpeedUp)
SHADER_PARAMETER(float, ExposureSpeedDown)
SHADER_PARAMETER(float, HistogramScale)
SHADER_PARAMETER(float, HistogramBias)
SHADER_PARAMETER(float, LuminanceMin)
SHADER_PARAMETER(float, BlackHistogramBucketInfluence)
SHADER_PARAMETER(float, GreyMult)
SHADER_PARAMETER(float, ExponentialUpM)
SHADER_PARAMETER(float, ExponentialDownM)
SHADER_PARAMETER(float, StartDistance)
SHADER_PARAMETER(float, LuminanceMax)
SHADER_PARAMETER(float, ForceTarget)
SHADER_PARAMETER(int, VisualizeDebugType)
SHADER_PARAMETER_TEXTURE(Texture2D, MeterMaskTexture)
SHADER_PARAMETER_SAMPLER(SamplerState, MeterMaskSampler)
END_SHADER_PARAMETER_STRUCT()

LIVRENDERING_API FLivEyeAdaptationParameters GetLivEyeAdaptationParameters(const FViewInfo& View, ERHIFeatureLevel::Type MinFeatureLevel);

LIVRENDERING_API FRDGTextureRef AddLivHistogramPass(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	const FLivEyeAdaptationParameters& EyeAdaptationParameters,
	FScreenPassTexture SceneColor,
	FRDGTextureRef EyeAdaptationTexture);

LIVRENDERING_API void AddLivHistogramEyeAdaptationPass(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	const FLivEyeAdaptationParameters& EyeAdaptationParameters,
	FRDGTextureRef HistogramTexture,
	FRDGTextureRef EyeAdaptationTexture);