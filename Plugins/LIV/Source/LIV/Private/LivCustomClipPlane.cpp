// Copyright 2021 LIV Inc. - MIT License

#include "LivCustomClipPlane.h"

#include "EngineUtils.h"
#include "Components/BrushComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/CollisionProfile.h"

/**
 * Proxy
 */

class FClipPlaneMeshSceneProxy : public FStaticMeshSceneProxy
{
public:

	virtual SIZE_T GetTypeHash() const override;

	/** Initialization constructor. */
	FClipPlaneMeshSceneProxy(ULivCustomClipPlane* Component);

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
};

SIZE_T FClipPlaneMeshSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

FClipPlaneMeshSceneProxy::FClipPlaneMeshSceneProxy(ULivCustomClipPlane* Component)
	: FStaticMeshSceneProxy(Component, false)
{
}

FPrimitiveViewRelevance FClipPlaneMeshSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance ViewRelevance = FStaticMeshSceneProxy::GetViewRelevance(View);
	//ViewRelevance.bDrawRelevance = View->bIsSceneCapture;
	ViewRelevance.bDrawRelevance = false;
	return ViewRelevance;
}

/// 

ULivCustomClipPlane::ULivCustomClipPlane(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Find_ClipPlaneMaterial(TEXT("/LIV/M_ClipPlane"));
	if (Find_ClipPlaneMaterial.Succeeded())
	{
		ClipPlaneMaterial = Find_ClipPlaneMaterial.Object;
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

FPrimitiveSceneProxy* ULivCustomClipPlane::CreateSceneProxy()
{
	if (GetStaticMesh() == nullptr || GetStaticMesh()->GetRenderData() == nullptr)
	{
		return nullptr;
	}

	const FStaticMeshLODResourcesArray& LODResources = GetStaticMesh()->GetRenderData()->LODResources;
	if (LODResources.Num() == 0 || LODResources[FMath::Clamp<int32>(GetStaticMesh()->GetMinLOD().Default, 0, LODResources.Num() - 1)].VertexBuffers.StaticMeshVertexBuffer.GetNumVertices() == 0)
	{
		return nullptr;
	}
	
	FPrimitiveSceneProxy* Proxy = ::new FClipPlaneMeshSceneProxy(this);

#if STATICMESH_ENABLE_DEBUG_RENDERING
	SendRenderDebugPhysics(Proxy);
#endif

	return Proxy;
}
