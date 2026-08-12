// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterMeshNode.h"
#include "Scatter/VoxelPointUtilities.h"
#include "Scatter/VoxelScatterSubsystem.h"
#include "Scatter/VoxelScatterCacheManager.h"
#include "Scatter/VoxelScatterFunctionLibrary.h"
#include "VoxelQuery.h"
#include "VoxelRuntime.h"
#include "VoxelSubsystem.h"
#include "VoxelBufferSplitter.h"
#include "Buffer/VoxelClassBuffer.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelGraphStaticMeshBuffer.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Graphs/VoxelStampGraphParameters.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, int32, GVoxelScatterMeshNumChunks, 3,
	"voxel.scatter.mesh.NumChunks",
	"Number of chunks to target in each direction",
	Voxel::RefreshAll);

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelScatterDebugInvalidations, false,
	"voxel.scatter.DebugInvalidations",
	"When enabled, draws a box around each scatter chunk whose dependency tracker is found to be invalidated and is being torn down for re-render.");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelNode_ScatterMesh::FVoxelNode_ScatterMesh()
{
	FixupLayerPins();
}

void FVoxelNode_ScatterMesh::Initialize(FInitializer& Initializer)
{
	VOXEL_FUNCTION_COUNTER();

	for (FLayerPin& LayerPin : LayerPins)
	{
		Initializer.InitializePinRef(LayerPin.Points);
		Initializer.InitializePinRef(LayerPin.ComponentClass);
		Initializer.InitializePinRef(LayerPin.RenderSettings);
		Initializer.InitializePinRef(LayerPin.CollisionProfile);
		Initializer.InitializePinRef(LayerPin.NumChunks);
	}
}

TSharedRef<FVoxelScatterNodeRuntime> FVoxelNode_ScatterMesh::MakeRuntime() const
{
	return MakeShared<FVoxelScatterMeshNodeRuntime>();
}

void FVoxelNode_ScatterMesh::PostSerialize()
{
	Super::PostSerialize();

	FixupLayerPins();
}

void FVoxelNode_ScatterMesh::FixupLayerPins()
{
	VOXEL_FUNCTION_COUNTER();

	for (const FLayerPin& Layer : LayerPins)
	{
		RemovePin(Layer.Points.GetName());
		RemovePin(Layer.ComponentClass.GetName());
		RemovePin(Layer.RenderSettings.GetName());
		RemovePin(Layer.CollisionProfile.GetName());
		RemovePin(Layer.NumChunks.GetName());
	}
	LayerPins.Reset();

	for (int32 Index = 0; Index < NumLayerPins; Index++)
	{
		const FString Category = "Layer " + FString::FromInt(Index);

		LayerPins.Add(FLayerPin
		{
			CreateInputPin<FVoxelPointSet>(
				FName("Points", Index + 1),
				VOXEL_PIN_METADATA(
					FVoxelPointSet,
					nullptr,
					DisplayName("Points"),
					Tooltip("Points of this layer"),
					Category(Category))),

			CreateInputPin<TSubclassOf<UInstancedStaticMeshComponent>>(
				FName("ComponentClass", Index + 1),
				VOXEL_PIN_METADATA(
					TSubclassOf<UInstancedStaticMeshComponent>,
					UInstancedStaticMeshComponent::StaticClass(),
					DisplayName("Component Class"),
					Tooltip("This layer default component class"),
					Category(Category))),

			CreateInputPin<FVoxelMeshComponentSettings>(
				FName("RenderSettings", Index + 1),
				VOXEL_PIN_METADATA(
					FVoxelMeshComponentSettings,
					nullptr,
					DisplayName("Render Settings"),
					Tooltip("This layer default render settings"),
					Category(Category))),

			CreateInputPin<FBodyInstance>(
				FName("CollisionProfile", Index + 1),
				VOXEL_PIN_METADATA(
					FBodyInstance,
					nullptr,
					DisplayName("Collision Profile"),
					Tooltip("This layer default collision profile"),
					Category(Category))),

			CreateInputPin<int32>(
				FName("NumChunks", Index + 1),
				VOXEL_PIN_METADATA(
					int32,
					1,
					DisplayName("Num Chunks"),
					Tooltip("Distance in chunks for this layer"),
					Category(Category)))
		});
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FString FVoxelNode_ScatterMesh::FDefinition::GetAddPinLabel() const
{
	return "Add Layer";
}

FString FVoxelNode_ScatterMesh::FDefinition::GetAddPinTooltip() const
{
	return "Adds an additional layer pin";
}

void FVoxelNode_ScatterMesh::FDefinition::AddInputPin()
{
	Node.NumLayerPins++;
	Node.FixupLayerPins();
}

bool FVoxelNode_ScatterMesh::FDefinition::CanRemoveInputPin() const
{
	return Node.NumLayerPins > 1;
}

void FVoxelNode_ScatterMesh::FDefinition::RemoveInputPin()
{
	Node.NumLayerPins--;
	Node.FixupLayerPins();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelScatterMeshNodeRuntime::Initialize(FVoxelGraphQueryImpl& Query)
{
	VOXEL_FUNCTION_COUNTER();

	RenderDistances.SetNum(GetEvaluator()->LayerPins.Num());
	TotalChunksRenderDistance = 0;
	for (int32 Index = 0; Index < GetEvaluator()->LayerPins.Num(); Index++)
	{
		const int32 NumChunks = FMath::Max(GetEvaluator()->LayerPins[Index].NumChunks.GetSynchronous(Query), 1);
		TotalChunksRenderDistance += NumChunks;
		RenderDistances[Index] = TotalChunksRenderDistance * GetChunkSize();
	}
}

void FVoxelScatterMeshNodeRuntime::Compute(
	const FVoxelSubsystem& Subsystem,
	const TVoxelArray<FVector>& Invokers)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(ChunksToRender.Num() == 0);
	ensure(ChunksToDestroy.Num() == 0);

	if (Invokers.Num() == 0 ||
		TotalChunksRenderDistance == 0)
	{
		return;
	}

	TVoxelMap<FIntVector, int32> NewChunkKeyToLayer;
	for (const FVector& Position : Invokers)
	{
		const FIntVector InvokerChunkPosition = FVoxelUtilities::RoundToInt(Position / GetChunkSize());

		const FIntVector Min = InvokerChunkPosition - TotalChunksRenderDistance;
		const FIntVector Max = InvokerChunkPosition + TotalChunksRenderDistance;

		{
			VOXEL_SCOPE_COUNTER("Find NewChunkKeys");

			NewChunkKeyToLayer.Reserve(FVoxelIntBox(Min, Max + 1).Count_int32());

			const float MaxDistance = TotalChunksRenderDistance * GetChunkSize();
			for (int32 Z = Min.Z; Z <= Max.Z; Z++)
			{
				for (int32 Y = Min.Y; Y <= Max.Y; Y++)
				{
					for (int32 X = Min.X; X <= Max.X; X++)
					{
						const FIntVector ChunkKey = FIntVector(X, Y, Z);

						const float Distance = FVector::Distance(FVector(ChunkKey) * GetChunkSize(), Position) - GetChunkSize() * 0.5f;

						if (Distance > MaxDistance)
						{
							continue;
						}

						int32& LayerIndex = NewChunkKeyToLayer.FindOrAdd(ChunkKey, [&](int32& NewLayerIndex)
						{
							NewLayerIndex = RenderDistances.Num() - 1;
						});

						for (int32 Index = 0; Index < RenderDistances.Num(); Index++)
						{
							if (Distance > RenderDistances[Index])
							{
								continue;
							}

							LayerIndex = FMath::Min(LayerIndex, Index);
							break;
						}
					}
				}
			}
		}
	}

	{
		VOXEL_SCOPE_COUNTER("Build ChunkKeyToChunk");

		TVoxelMap<FIntVector, TSharedPtr<FChunk>> OldChunkKeyToChunk = MoveTemp(ChunkKeyToChunk);

		ChunkKeyToChunk.Reserve(NewChunkKeyToLayer.Num());

		for (const auto& It : NewChunkKeyToLayer)
		{
			TSharedPtr<FChunk> Chunk;
			OldChunkKeyToChunk.RemoveAndCopyValue(It.Key, Chunk);

			if (Chunk)
			{
				if (Chunk->DependencyTracker->IsInvalidated() ||
					Chunk->LayerIndex != It.Value)
				{
					if (GVoxelScatterDebugInvalidations &&
						Chunk->DependencyTracker->IsInvalidated())
					{
						const FVoxelBox ChunkBounds =
							FVoxelBox(GetChunkSize() * FVector(Chunk->ChunkKey))
							.Extend(GetChunkSize() * 0.5f);

						FVoxelDebugDrawer()
						.Color(FLinearColor::Yellow)
						.LifeTime(1.f)
						.DrawBox(ChunkBounds, FTransform::Identity);
					}

					ChunksToDestroy.Add(Chunk);
					Chunk.Reset();
				}
			}

			if (!Chunk)
			{
				Chunk = MakeShared<FChunk>(It.Key, It.Value);
				ChunksToRender.Add(Chunk);
			}

			ChunkKeyToChunk.Add_EnsureNew(It.Key, Chunk);
		}

		for (const auto& It : OldChunkKeyToChunk)
		{
			ChunksToDestroy.Add(It.Value);
		}
	}

	for (const TSharedPtr<FChunk>& Chunk : ChunksToRender)
	{
		Voxel::AsyncTask(MakeWeakPtrLambda(this, [this, &Subsystem, Chunk]
		{
			ComputeChunk(Subsystem, *Chunk);
		}));
	}
}

void FVoxelScatterMeshNodeRuntime::Render(FVoxelRuntime& Runtime)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	for (const TSharedPtr<FChunk>& Chunk : ChunksToRender)
	{
		RenderChunk(Runtime, *Chunk);
	}
	ChunksToRender.Empty();

	for (const TSharedPtr<FChunk>& Chunk : ChunksToDestroy)
	{
		DestroyChunk(Runtime, *Chunk);
	}
	ChunksToDestroy.Empty();
}

void FVoxelScatterMeshNodeRuntime::Destroy(FVoxelRuntime& Runtime)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	ensure(ChunksToRender.Num() == 0);
	ensure(ChunksToDestroy.Num() == 0);

	for (const auto& It : ChunkKeyToChunk)
	{
		DestroyChunk(Runtime, *It.Value);
	}
	ChunkKeyToChunk.Empty();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelScatterMeshNodeRuntime::FAttributeOverrides::AddAttribute(
	const FName Name,
	const FVoxelRuntimePinValue& Value)
{
	Attributes.Add(FData{ FMinimalName(Name), Value });
}

void FVoxelScatterMeshNodeRuntime::FAttributeOverrides::ApplyAttributes(
	UObject& Object,
	const TVoxelMap<FName, FProperty*>& NameToProperty)
{
	for (const FData& Attribute : Attributes)
	{
		const FProperty* Property = NameToProperty.FindRef(FName(Attribute.Name));
		if (!Property)
		{
			continue;
		}

		if (!Attribute.Value.GetType().GetExposedType().CanBeCastedTo_K2(FVoxelPinType(*Property, true)))
		{
			continue;
		}

		Attribute.Value.ExportToProperty(*Property, Property->ContainerPtrToValuePtr<void>(&Object));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelScatterMeshNodeRuntime::ComputeChunk(
	const FVoxelSubsystem& Subsystem,
	FChunk& Chunk) const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelScatterMeshNodeRuntime Chunk"));

	ON_SCOPE_EXIT
	{
		Chunk.DependencyTracker = Subsystem.Finalize(DependencyCollector);
	};

	if (!ensure(GetEvaluator()->LayerPins.IsValidIndex(Chunk.LayerIndex)))
	{
		return;
	}

	FVoxelGraphContext Context = GetEvaluator().MakeContext(DependencyCollector);
	FVoxelGraphQueryImpl& Query = Context.MakeQuery();

	FVoxelQuery VoxelQuery(
		0,
		Subsystem.GetLayers(),
		Subsystem.GetSurfaceTypeTable(),
		DependencyCollector);

	Query.AddParameter<FVoxelGraphParameters::FQuery>(VoxelQuery);

	Query.AddParameter<FVoxelGraphParameters::FPointSetChunkBounds>(
		FVoxelBox(GetChunkSize() * FVector(Chunk.ChunkKey))
		.Extend(GetChunkSize() * 0.5f),
		true);

	{
		FVoxelGraphParameters::FScatterCacheManager& Parameter = Query.AddParameter<FVoxelGraphParameters::FScatterCacheManager>();
		Parameter.CacheManager = Subsystem.AsChecked<FVoxelScatterSubsystem>().GetCacheManager();
		Parameter.WeakActor = GetActorRef();
	}

	TSharedPtr<const FVoxelPointSet> Points = GetEvaluator()->LayerPins[Chunk.LayerIndex].Points.GetSynchronous(Query);
	if (Points->Num() == 0)
	{
		return;
	}

	const FVoxelGraphStaticMeshBuffer* MeshBuffer = Points->Find<FVoxelGraphStaticMeshBuffer>(FVoxelPointAttributes::Mesh);
	const FVoxelDoubleVectorBuffer* PositionBuffer = Points->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
	if (!MeshBuffer ||
		!PositionBuffer)
	{
		return;
	}

	if (const FVoxelBoolBuffer* IsNeighborBuffer = Points->Find<FVoxelBoolBuffer>(FVoxelPointAttributes::IsNeighbor))
	{
		if (IsNeighborBuffer->IsConstant())
		{
			if (IsNeighborBuffer->GetConstant())
			{
				return;
			}
		}
		else
		{
			Points = Points->Split(FVoxelBufferSplitter(*IsNeighborBuffer))[0];
			if (!Points ||
				Points->Num() == 0)
			{
				return;
			}

			MeshBuffer = Points->Find<FVoxelGraphStaticMeshBuffer>(FVoxelPointAttributes::Mesh);
			PositionBuffer = Points->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
			if (!MeshBuffer ||
				!PositionBuffer)
			{
				return;
			}
		}
	}

	const FVoxelQuaternionBuffer* RotationBuffer = Points->Find<FVoxelQuaternionBuffer>(FVoxelPointAttributes::Rotation);
	const FVoxelVectorBuffer* ScaleBuffer = Points->Find<FVoxelVectorBuffer>(FVoxelPointAttributes::Scale);

	Chunk.RenderSettings = *GetEvaluator()->LayerPins[Chunk.LayerIndex].RenderSettings.GetSynchronous(Query);
	Chunk.CollisionProfile = *GetEvaluator()->LayerPins[Chunk.LayerIndex].CollisionProfile.GetSynchronous(Query);

	VOXEL_SCOPE_COUNTER("Build render data");

	TVoxelArray<uint64> PointHashes;
	FVoxelUtilities::SetNumZeroed(PointHashes, Points->Num());

	MeshBuffer->HashCombine(PointHashes);

	const TSharedPtr<const FVoxelBuffer> ComponentTypeBuffer = FVoxelPointUtilities::ParseAttribute(
		*Points,
		FVoxelPinType::MakeClass(UInstancedStaticMeshComponent::StaticClass()),
		GetEvaluator()->ComponentTypeAttributeOverride,
		GetEvaluator()->GetNodeRef());
	if (ComponentTypeBuffer)
	{
		ComponentTypeBuffer->HashCombine(PointHashes);
	}
	const TSharedPtr<const FVoxelBuffer> CollisionProfileBuffer = FVoxelPointUtilities::ParseAttribute(
		*Points,
		FVoxelPinType::Make<FName>(),
		GetEvaluator()->CollisionProfileAttributeOverride,
		GetEvaluator()->GetNodeRef());
	if (CollisionProfileBuffer)
	{
		CollisionProfileBuffer->HashCombine(PointHashes);
	}

	TVoxelMap<FName, TSharedPtr<const FVoxelBuffer>> AttributeToBuffer;
	AttributeToBuffer.Reserve(GetEvaluator()->SpawnedComponentPropertyOverrides.Num());
	for (const auto& It : GetEvaluator()->SpawnedComponentPropertyOverrides)
	{
		const TSharedPtr<const FVoxelBuffer> Buffer = FVoxelPointUtilities::ParseAttribute(
			*Points,
			{},
			It.Value,
			GetEvaluator()->GetNodeRef());

		if (!Buffer)
		{
			continue;
		}

		// Constant attributes do not need to appear in permutations
		if (Buffer->IsConstant_Slow())
		{
			Chunk.ConstantAttributeOverrides.AddAttribute(It.Key, Buffer->GetGenericConstant());
			continue;
		}

		Buffer->HashCombine(PointHashes);
		AttributeToBuffer.Add_EnsureNew(It.Value, Buffer);
	}

	TVoxelArray<const FVoxelFloatBuffer*> PerInstanceDataBuffers;
	PerInstanceDataBuffers.Reserve(GetEvaluator()->PerInstanceDataAttributes.Num());
	for (const FName AttributeName : GetEvaluator()->PerInstanceDataAttributes)
	{
		const TSharedPtr<const FVoxelBuffer> Buffer = FVoxelPointUtilities::ParseAttribute(
			*Points,
			FVoxelPinType::Make<float>(),
			AttributeName,
			GetEvaluator()->GetNodeRef());

		if (!Buffer)
		{
			VOXEL_MESSAGE(
				Warning,
				"{0}: Failed to find Per Instance Custom Data Attribute {1}",
				GetEvaluator()->GetNodeRef(),
				AttributeName.ToString());
			PerInstanceDataBuffers.Add(nullptr);
			continue;
		}

		PerInstanceDataBuffers.Add(Buffer->As<FVoxelFloatBuffer>());
	}

	Chunk.NumPerInstanceCustomData = PerInstanceDataBuffers.Num();

	const TSubclassOf<UInstancedStaticMeshComponent> DefaultComponentClass = UInstancedStaticMeshComponent::StaticClass();
	for (int32 Index = 0; Index < Points->Num(); Index++)
	{
		const FVector Position = (*PositionBuffer)[Index];
		const FQuat4f Rotation = RotationBuffer ? (*RotationBuffer)[Index] : FQuat4f::Identity;
		const FVector3f Scale = ScaleBuffer ? (*ScaleBuffer)[Index] : FVector3f::OneVector;

		if (Position.ContainsNaN() ||
			Rotation.ContainsNaN() ||
			Scale.ContainsNaN())
		{
			continue;
		}

		FRenderData* RenderData = Chunk.KeyToRenderData.FindSmartPtr(PointHashes[Index]);
		if (!RenderData)
		{
			const TSharedRef<FRenderData> NewRenderData = MakeShared<FRenderData>();
			NewRenderData->Transforms.Reserve(Points->Num());
			NewRenderData->PerInstanceCustomData.Reserve(PerInstanceDataBuffers.Num() * Points->Num());
			NewRenderData->Mesh = (*MeshBuffer)[Index].StaticMesh;
			NewRenderData->Class = 
				ComponentTypeBuffer
				? ComponentTypeBuffer->GetGeneric(Index).Get<TSubclassOf<UInstancedStaticMeshComponent>>()
				: DefaultComponentClass;
			NewRenderData->ProfileName = 
				CollisionProfileBuffer
				? CollisionProfileBuffer->GetGeneric(Index).Get<FName>()
				: NAME_None;

			for (const auto& It : AttributeToBuffer)
			{
				NewRenderData->AttributeOverrides.AddAttribute(It.Key, It.Value->GetGeneric(Index));
			}

			Chunk.KeyToRenderData.Add_EnsureNew(PointHashes[Index], NewRenderData);
			RenderData = &NewRenderData.Get();

			Chunk.ComponentClassesToCreate.Add(NewRenderData->Class);
		}

		RenderData->Transforms.Add_EnsureNoGrow(FTransform(
			FQuat(Rotation),
			Position,
			FVector(Scale)));

		for (const FVoxelFloatBuffer* Buffer : PerInstanceDataBuffers)
		{
			if (Buffer)
			{
				RenderData->PerInstanceCustomData.Add_EnsureNoGrow((*Buffer)[Index]);
			}
			else
			{
				RenderData->PerInstanceCustomData.Add_EnsureNoGrow(0);
			}
		}
	}
}

void FVoxelScatterMeshNodeRuntime::RenderChunk(
	FVoxelRuntime& Runtime,
	FChunk& Chunk) const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	ensure(Chunk.Components.Num() == 0);
	Chunk.Components.Reserve(Chunk.KeyToRenderData.Num());

	TVoxelMap<const UClass*, TVoxelMap<FName, FProperty*>> ClassToPropertyToName;
	for (const UClass* Class : Chunk.ComponentClassesToCreate)
	{
		for (const auto& It : GetEvaluator()->SpawnedComponentPropertyOverrides)
		{
			FProperty* Property = Class->FindPropertyByName(It.Key);
			if (!Property)
			{
				continue;
			}

			ClassToPropertyToName.FindOrAdd(Class).Add_EnsureNew(It.Key, Property);
		}
	}

	for (const auto& It : Chunk.KeyToRenderData)
	{
		const UClass* Class = It.Value->Class;
		if (!Class)
		{
			VOXEL_MESSAGE(Warning, "{0}: Passed component is not valid", GetEvaluator()->GetNodeRef());
			continue;
		}

		if (!Class->IsChildOf(UInstancedStaticMeshComponent::StaticClass()))
		{
			VOXEL_MESSAGE(Warning, "{0}: Component {1} is not an InstancedStaticMeshComponent type", GetEvaluator()->GetNodeRef(), Class->GetName());
			continue;
		}

		UInstancedStaticMeshComponent* Component = Cast<UInstancedStaticMeshComponent>(Runtime.NewComponent(Class));
		if (!ensureVoxelSlow(Component))
		{
			continue;
		}

		Chunk.RenderSettings.ApplyToComponent(*Component);
		Component->BodyInstance = Chunk.CollisionProfile;

		if (!It.Value->ProfileName.IsNone())
		{
			Component->SetCollisionProfileName(It.Value->ProfileName);
		}

		if (const TVoxelMap<FName, FProperty*>* NameToProperty = ClassToPropertyToName.Find(Class))
		{
			It.Value->AttributeOverrides.ApplyAttributes(
				*Component,
				*NameToProperty);
			Chunk.ConstantAttributeOverrides.ApplyAttributes(
				*Component,
				*NameToProperty);
		}

		Component->bHasPerInstanceHitProxies = true;
		Component->SetStaticMesh(It.Value->Mesh.Resolve_Ensured());

		Component->AddInstances(It.Value->Transforms, false);

		Component->NumCustomDataFloats = Chunk.NumPerInstanceCustomData;
		Component->PerInstanceSMCustomData = It.Value->PerInstanceCustomData;

		Chunk.Components.Add(Component);
	}
	Chunk.KeyToRenderData.Empty();
}

void FVoxelScatterMeshNodeRuntime::DestroyChunk(
	FVoxelRuntime& Runtime,
	FChunk& Chunk)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	for (const TVoxelObjectPtr<UInstancedStaticMeshComponent>& WeakComponent : Chunk.Components)
	{
		UInstancedStaticMeshComponent* Component = WeakComponent.Resolve();
		if (!ensureVoxelSlow(Component))
		{
			continue;
		}

		Component->SetStaticMesh(nullptr);
		Component->ClearInstances();

		Runtime.RemoveComponent(Component);
	}
}