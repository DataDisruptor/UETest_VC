// Copyright 2021 LIV Inc. - MIT License
#include "LivRenderingModule.h"

#include "ShaderCore.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FLivRenderingModule"

void FLivRenderingModule::StartupModule()
{
	// Maps virtual shader source directory /Plugin/Liv to the plugin's actual Shaders directory.
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("Liv"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/Liv"), PluginShaderDir);
}

void FLivRenderingModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLivRenderingModule, LIVRendering)