// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMeshComponentSettings.h"
#include "Scatter/VoxelNode_ScatterBase.h"
#include "Scatter/VoxelScatterNodeRuntime.h"
#include "VoxelScatterMeshNode.generated.h"

struct FVoxelClassBuffer;

USTRUCT()
struct VOXEL_API FVoxelNode_ScatterMesh : public FVoxelNode_ScatterBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	FVoxelNode_ScatterMesh();

	//~ Begin FVoxelNode_ScatterBase Interface
	virtual void Initialize(FInitializer& Initializer) override;
	virtual TSharedRef<FVoxelScatterNodeRuntime> MakeRuntime() const override;
	virtual void PostSerialize() override;
	//~ End FVoxelNode_ScatterBase Interface

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	FName ComponentTypeAttributeOverride;

	UPROPERTY(EditAnywhere, Category = "Config")
	TMap<FName, FName> SpawnedComponentPropertyOverrides;

	UPROPERTY(EditAnywhere, Category = "Config")
	FName CollisionProfileAttributeOverride;

	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FName> PerInstanceDataAttributes;

public:
	struct FLayerPin
	{
		TPinRef_Input<FVoxelPointSet> Points;
		TPinRef_Input<TSubclassOf<UInstancedStaticMeshComponent>> ComponentClass;
		TPinRef_Input<FVoxelMeshComponentSettings> RenderSettings;
		TPinRef_Input<FBodyInstance> CollisionProfile;
		TPinRef_Input<int32> NumChunks;
	};
	TVoxelArray<FLayerPin> LayerPins;

	UPROPERTY()
	int32 NumLayerPins = 1;

	void FixupLayerPins();

public:
#if WITH_EDITOR
	class FDefinition : public Super::FDefinition
	{
	public:
		GENERATED_VOXEL_NODE_DEFINITION_BODY(FVoxelNode_ScatterMesh);

		virtual FString GetAddPinLabel() const override;
		virtual FString GetAddPinTooltip() const override;

		virtual bool CanAddInputPin() const override { return true; }
		virtual void AddInputPin() override;

		virtual bool CanRemoveInputPin() const override;
		virtual void RemoveInputPin() override;
	};
#endif
};

class VOXEL_API FVoxelScatterMeshNodeRuntime : public TVoxelScatterNodeRuntime<FVoxelNode_ScatterMesh>
{
public:
	//~ Begin FVoxelScatterNodeRuntime Interface
	virtual void Initialize(FVoxelGraphQueryImpl& Query) override;
	virtual void Compute(
		const FVoxelSubsystem& Subsystem,
		const TVoxelArray<FVector>& Invokers) override;
	virtual void Render(FVoxelRuntime& Runtime) override;
	virtual void Destroy(FVoxelRuntime& Runtime) override;
	//~ End FVoxelScatterNodeRuntime Interface

private:
	TArray<int32> RenderDistances;
	int32 TotalChunksRenderDistance = 0;

private:
	struct FAttributeOverrides
	{
	private:
		struct FData
		{
			FMinimalName Name;
			FVoxelRuntimePinValue Value;
		};
		TVoxelArray<FData> Attributes;
	
	public:
		void AddAttribute(
			FName Name,
			const FVoxelRuntimePinValue& Value);
		void ApplyAttributes(
			UObject& Object,
			const TVoxelMap<FName, FProperty*>& NameToProperty);
	};
	struct FRenderData
	{
		TVoxelArray<FTransform> Transforms;
		TVoxelArray<float> PerInstanceCustomData;

		TVoxelObjectPtr<UStaticMesh> Mesh;
		UClass* Class = nullptr;
		FName ProfileName;

		FAttributeOverrides AttributeOverrides;
	};
	struct FChunk
	{
		const FIntVector ChunkKey;
		int32 LayerIndex;
		TSharedPtr<FVoxelDependencyTracker> DependencyTracker;
		TVoxelArray<TVoxelObjectPtr<UInstancedStaticMeshComponent>> Components;

		FVoxelMeshComponentSettings RenderSettings;
		FBodyInstance CollisionProfile;

		TVoxelSet<UClass*> ComponentClassesToCreate;
		TVoxelMap<uint64, TSharedPtr<FRenderData>> KeyToRenderData;
		FAttributeOverrides ConstantAttributeOverrides;

		int32 NumPerInstanceCustomData = 0;

		explicit FChunk(
			const FIntVector& ChunkKey,
			const int32 LayerIndex)
			: ChunkKey(ChunkKey)
			, LayerIndex(LayerIndex)
		{
		}
	};
	TVoxelMap<FIntVector, TSharedPtr<FChunk>> ChunkKeyToChunk;

	TVoxelChunkedArray<TSharedPtr<FChunk>> ChunksToRender;
	TVoxelChunkedArray<TSharedPtr<FChunk>> ChunksToDestroy;

	void ComputeChunk(
		const FVoxelSubsystem& Subsystem,
		FChunk& Chunk) const;

	void RenderChunk(
		FVoxelRuntime& Runtime,
		FChunk& Chunk) const;

	static void DestroyChunk(
		FVoxelRuntime& Runtime,
		FChunk& Chunk);
};