// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelRuntimePinValue.h"
#include "VoxelPreviewHandler.generated.h"

class UVoxelTerminalGraph;

USTRUCT()
struct VOXELGRAPH_API FVoxelPreviewHandler
	: public FVoxelVirtualStruct
#if CPP
	, public TSharedFromThis<FVoxelPreviewHandler>
#endif
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	static TConstVoxelArrayView<TSharedRef<const FVoxelPreviewHandler>> GetHandlers();

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bShowFullDistance = false;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bShowFullValue = false;

public:
	int32 PreviewSize = 0;
	FTransform LocalToWorld;
	TVoxelAtomic<bool> bIsInvalidated;
	TFunction<void()> OnInvalidated;

	TVoxelFuture<TVoxelArray<uint8>> Compute(UVoxelTerminalGraph& TerminalGraph);
	FVoxelFuture ComputeAtMousePosition(UVoxelTerminalGraph& TerminalGraph);

public:
	using FAddStat = TFunctionRef<void(
		const FString& Name,
		bool bGlobalSpacing,
		const TFunction<FString()>& Tooltip,
		const TFunction<TArray<FString>()>& GetValues)>;

	void SetMousePosition(const FVector2D& NewMousePosition);

public:
	virtual bool SupportsType(const FVoxelPinType& Type) const
	{
		return false;
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) VOXEL_PURE_VIRTUAL();
	virtual void BuildStats(const FAddStat& AddStat);

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const VOXEL_PURE_VIRTUAL();

protected:
	FVector2D MousePosition = FVector2D::ZeroVector;
	bool bIsMousePositionLocked = false;

private:
	FVoxelCriticalSection DependencyTrackers_CriticalSection;
	TVoxelChunkedArray<TSharedPtr<FVoxelDependencyTracker>> DependencyTrackers_RequiresLock;
};