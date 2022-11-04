// Copyright 2021 LIV Inc. - MIT License
#include "LivClipPlane.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/CollisionProfile.h"

ULivClipPlane::ULivClipPlane(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Find_ClipPlaneMaterial(TEXT("/LIV/M_ClipPlane"));
	if (Find_ClipPlaneMaterial.Succeeded())
	{
		ClipPlaneMaterial = Find_ClipPlaneMaterial.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Find_ClipPlaneDebugMaterial(TEXT("/Engine/EngineMaterials/WorldGridMaterial"));
	if (Find_ClipPlaneDebugMaterial.Succeeded())
	{
		ClipPlaneDebugMaterial = Find_ClipPlaneDebugMaterial.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Find_ClipPlaneStaticMesh(TEXT("/LIV/SM_ClipPlane"));
	if (Find_ClipPlaneStaticMesh.Succeeded())
	{
		SetStaticMesh(Find_ClipPlaneStaticMesh.Object);
	}

	SetMaterial(0, ClipPlaneMaterial);
	SetCastShadow(false);
	SetWorldScale3D(FVector(1, 50, 50));
	SetHiddenInGame(true);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

void ULivClipPlane::SetDebugEnabled(bool bDebugEnabled)
{
	if (bDebugEnabled)
	{
		SetMaterial(0, ClipPlaneDebugMaterial);
	}
	else
	{
		SetMaterial(0, ClipPlaneMaterial);
	}
}

bool ULivClipPlane::GetDebugEnabled() const
{
	return GetMaterial(0) == ClipPlaneDebugMaterial;
}


