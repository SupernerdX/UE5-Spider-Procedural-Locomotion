// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelSamplePreviousStampsNodes.h"
#include "VoxelQuery.h"
#include "VoxelMetadata.h"
#include "VoxelGraphPositionParameter.h"
#include "Graphs/VoxelStampGraphParameters.h"
#include "Graphs/VoxelPreviewStampDataNodes.h"
#include "Surface/VoxelSmartSurfaceFunctionLibrary.h"
#include "VoxelSamplePreviousStampsNodesImpl.ispc.generated.h"
#include "Surface/VoxelPreviewDataNode_SmartSurfaceTypePreviewData.h"

FVoxelNode_SamplePreviousStampsBase::FVoxelNode_SamplePreviousStampsBase()
{
	FixupMetadataPins();
}

void FVoxelNode_SamplePreviousStampsBase::Initialize(FInitializer& Initializer)
{
	VOXEL_FUNCTION_COUNTER();

	for (FMetadataPin& MetadataPin : MetadataPins)
	{
		Initializer.InitializePinRef(MetadataPin.PinRef);
	}
}

void FVoxelNode_SamplePreviousStampsBase::PostSerialize()
{
	Super::PostSerialize();

	FixupMetadataPins();
}

#if WITH_EDITOR
FVoxelNode::EPostEditChange FVoxelNode_SamplePreviousStampsBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_OWN_MEMBER_NAME(MetadatasToQuery))
	{
		FixupMetadataPins();
		return EPostEditChange::Reconstruct;
	}

	return EPostEditChange::None;
}
#endif

void FVoxelNode_SamplePreviousStampsBase::FixupMetadataPins()
{
	VOXEL_FUNCTION_COUNTER();

	for (const FMetadataPin& MetadataPin : MetadataPins)
	{
		RemovePin(MetadataPin.PinRef.GetName());
	}
	MetadataPins.Reset();

	const TArray<TObjectPtr<UVoxelMetadata>> TempMetadatasToQuery = TVoxelSet<TObjectPtr<UVoxelMetadata>>(MetadatasToQuery).Array();
	MetadatasToQuery = TempMetadatasToQuery;

	for (UVoxelMetadata* Metadata : MetadatasToQuery)
	{
		if (!Metadata)
		{
			continue;
		}

		MetadataPins.Add(FMetadataPin
		{
			FVoxelMetadataRef(Metadata),
			CreateOutputPin(
				Metadata->GetInnerType().GetBufferType(),
				Metadata->GetFName(),
				VOXEL_PIN_METADATA(
					void,
					nullptr,
					DisplayName(Metadata->GetName()),
					Category("Metadata"))),
		});
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_SamplePreviousHeightStamps::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelDoubleVector2DBuffer> InPosition = PositionPin.Get(Query);

	VOXEL_GRAPH_WAIT(InPosition)
	{
		FVoxelDoubleVector2DBuffer Position = *InPosition;
		Position.ExpandConstants(Position.Num());

		VOXEL_FUNCTION_COUNTER_NUM(Position.Num());
		FVoxelNodeStatScope StatScope(*this, Position.Num());

		const FVoxelGraphParameters::FHeightStamp* HeightStamp = Query->FindParameter<FVoxelGraphParameters::FHeightStamp>();
		if (!HeightStamp)
		{
			VOXEL_MESSAGE(Error, "{0}: not in a height stamp", this);
			return;
		}
		if (HeightStamp->BlendMode != EVoxelHeightBlendMode::Override)
		{
			VOXEL_MESSAGE(Error, "{0}: blend mode should be set to Override", this);
			return;
		}

		if (Query.IsPreview())
		{
			const FVoxelGraphParameters::FHeightStampPreviewData* HeightPreviewData = Query->FindParameter<FVoxelGraphParameters::FHeightStampPreviewData>();
			if (!HeightPreviewData)
			{
				VOXEL_MESSAGE(Error, "{0}: No Height Preview Data node in Editor Graph found", this);
				return;
			}

			FVoxelGraphQueryImpl& NewGraphQuery = HeightPreviewData->Query.CloneParameters();
			NewGraphQuery.AddParameter<FVoxelGraphParameters::FPosition2D>().SetLocalPosition(*InPosition);

			FVoxelGraphQuery NewQuery = FVoxelGraphQuery(NewGraphQuery, Query.GetCallstack());

			TOptional<TValue<FVoxelFloatBuffer>> Heights;
			if (HeightPin.ShouldCompute())
			{
				Heights = HeightPreviewData->ValuePin.Get(NewQuery);
			}
			TOptional<TValue<FVoxelSurfaceTypeBlendBuffer>> SurfaceTypes;
			if (SurfaceTypePin.ShouldCompute())
			{
				SurfaceTypes = HeightPreviewData->SurfaceTypePin.Get(NewQuery);
			}

			TVoxelArray<TValue<FVoxelBuffer>> MetadataToBuffer;
			MetadataToBuffer.Reserve(MetadataPins.Num());

			TSharedPtr<TVoxelMap<FVoxelMetadataRef, int32>> MetadataToBufferIndex = MakeShared<TVoxelMap<FVoxelMetadataRef, int32>>();
			MetadataToBufferIndex->Reserve(MetadataPins.Num());

			for (const FMetadataPin& MetadataPin : MetadataPins)
			{
				if (!MetadataPin.PinRef.ShouldCompute())
				{
					continue;
				}

				if (const TPinRef_Input<FVoxelBuffer>* Pin = HeightPreviewData->MetadataPins.Find(MetadataPin.MetadataRef))
				{
					int32 Index = MetadataToBuffer.Add(Pin->Get(NewQuery));
					MetadataToBufferIndex->Add_EnsureNew(MetadataPin.MetadataRef, Index);
				}
			}

			const int32 NumPositions = Position.Num();
			VOXEL_GRAPH_WAIT(NumPositions, Heights, SurfaceTypes, MetadataToBuffer, MetadataToBufferIndex)
			{
				if (Heights.IsSet())
				{
					HeightPin.Set(Query, *Heights);
				}
				if (SurfaceTypes.IsSet())
				{
					SurfaceTypePin.Set(Query, *SurfaceTypes);
				}

				for (const FMetadataPin& MetadataPin : MetadataPins)
				{
					if (!MetadataPin.PinRef.ShouldCompute())
					{
						continue;
					}

					TSharedPtr<const FVoxelBuffer> Result;
					if (const int32* Index = MetadataToBufferIndex->Find(MetadataPin.MetadataRef))
					{
						Result = MetadataToBuffer[*Index];
					}
					else
					{
						Result = MetadataPin.MetadataRef.MakeDefaultBuffer(NumPositions);
					}

					MetadataPin.PinRef.Set(Query, FVoxelRuntimePinValue::Make(Result.ToSharedRef()));
				}
			};
			return;
		}

		const FVoxelGraphParameters::FHeightQuery* HeightQuery = Query->FindParameter<FVoxelGraphParameters::FHeightQuery>();
		if (!HeightQuery ||
			!HeightQuery->QueryPrevious)
		{
			VOXEL_MESSAGE(Error, "{0}: no previous stamp data", this);
			return;
		}

		const FVoxelGraphParameters::FHeightQueryPrevious* HeightQueryPrevious = Query->FindParameter<FVoxelGraphParameters::FHeightQueryPrevious>();

		// Will be null in bulk queries
		if (HeightQueryPrevious)
		{
			if (HeightQueryPrevious->bQueried ||
				HeightQueryPrevious->GlobalPositions.X.GetData() != Position.X.GetData() ||
				HeightQueryPrevious->GlobalPositions.Y.GetData() != Position.Y.GetData())
			{
				HeightQueryPrevious = nullptr;
			}
		}

		VOXEL_SCOPE_COUNTER_COND(HeightQueryPrevious, "Query global previous");

		FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
		if (SurfaceTypePin.ShouldCompute() ||
			(HeightQueryPrevious && HeightQueryPrevious->bQueryPreviousSurfaceTypes))
		{
			SurfaceTypes.AllocateZeroed(Position.Num());
		}

		TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
		MetadataToBuffer.Reserve(MetadataPins.Num());

		for (const FMetadataPin& MetadataPin : MetadataPins)
		{
			if (MetadataPin.PinRef.ShouldCompute())
			{
				MetadataToBuffer.Add_EnsureNew(
					MetadataPin.MetadataRef,
					MetadataPin.MetadataRef.MakeDefaultBuffer(Position.Num()));
			}
		}

		if (HeightQueryPrevious)
		{
			for (const FVoxelMetadataRef& Metadata : HeightQueryPrevious->PreviousMetadatasToQuery)
			{
				if (MetadataToBuffer.Contains(Metadata))
				{
					continue;
				}

				MetadataToBuffer.Add_EnsureNew(
					Metadata,
					Metadata.MakeDefaultBuffer(Position.Num()));
			}
		}

		FVoxelFloatBuffer Height;
		Height.Allocate(Position.Num());
		Height.SetAll(FVoxelUtilities::NaNf());

		FVoxelDoubleVector2DBuffer QueryPosition;
		{
			VOXEL_SCOPE_COUNTER_NUM("VoxelSamplePreviousStampsNodes_TransformHeightPositions", Position.Num());

			QueryPosition.Allocate(Position.Num());

			ispc::VoxelSamplePreviousStampsNodes_TransformHeightPositions(
				HeightQuery->StampToQuery.ISPC(),
				Position.X.GetData(),
				Position.Y.GetData(),
				QueryPosition.X.GetData(),
				QueryPosition.Y.GetData(),
				Position.Num());
		}

		HeightQuery->QueryPrevious->Query(FVoxelHeightSparseQuery::Create(
			HeightQuery->Query,
			Height.View(),
			SurfaceTypes.View(),
			MetadataToBuffer,
			QueryPosition,
			SurfaceTypes.Num() > 0,
			MetadataToBuffer.KeyArray()));

		if (HeightQueryPrevious)
		{
			ensure(!HeightQueryPrevious->bQueried);
			HeightQueryPrevious->bQueried = true;

			if (HeightQueryPrevious->bQueryPreviousValues)
			{
				HeightQueryPrevious->OutValues = HeightPin.ShouldCompute() ? Height.MakeDeepCopy() : Height;
			}

			if (HeightQueryPrevious->bQueryPreviousSurfaceTypes)
			{
				HeightQueryPrevious->OutSurfaceTypes = SurfaceTypes;
			}

			HeightQueryPrevious->OutMetadataToBuffer.Reserve(HeightQueryPrevious->PreviousMetadatasToQuery.Num());

			for (const FVoxelMetadataRef& Metadata : HeightQueryPrevious->PreviousMetadatasToQuery)
			{
				HeightQueryPrevious->OutMetadataToBuffer.Add_EnsureNew(Metadata, MetadataToBuffer[Metadata]);
			}
		}

		if (HeightPin.ShouldCompute())
		{
			VOXEL_SCOPE_COUNTER_NUM("VoxelSamplePreviousStampsNodes_TransformHeights", Height.Num());

			ispc::VoxelSamplePreviousStampsNodes_TransformHeights(
				HeightQuery->StampToQuery.ISPC(),
				Position.X.GetData(),
				Position.Y.GetData(),
				Height.GetData(),
				Height.Num());

			HeightPin.Set(Query, Height);
		}

		if (SurfaceTypePin.ShouldCompute())
		{
			SurfaceTypePin.Set(Query, SurfaceTypes);
		}

		for (const FMetadataPin& MetadataPin : MetadataPins)
		{
			if (MetadataPin.PinRef.ShouldCompute())
			{
				MetadataPin.PinRef.Set(Query, FVoxelRuntimePinValue::Make(MetadataToBuffer[MetadataPin.MetadataRef]));
			}
		}
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_SamplePreviousVolumeStamps::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelDoubleVectorBuffer> InPosition = PositionPin.Get(Query);

	VOXEL_GRAPH_WAIT(InPosition)
	{
		FVoxelDoubleVectorBuffer Position = *InPosition;
		Position.ExpandConstants(Position.Num());

		VOXEL_FUNCTION_COUNTER_NUM(Position.Num());
		FVoxelNodeStatScope StatScope(*this, Position.Num());

		const FVoxelGraphParameters::FVolumeStamp* VolumeStamp = Query->FindParameter<FVoxelGraphParameters::FVolumeStamp>();
		if (!VolumeStamp)
		{
			VOXEL_MESSAGE(Error, "{0}: not in a volume stamp", this);
			return;
		}
		if (VolumeStamp->BlendMode != EVoxelVolumeBlendMode::Override)
		{
			VOXEL_MESSAGE(Error, "{0}: blend mode should be set to Override", this);
			return;
		}

		if (Query.IsPreview())
		{
			const FVoxelGraphParameters::FVolumeStampPreviewData* VolumePreviewData = Query->FindParameter<FVoxelGraphParameters::FVolumeStampPreviewData>();
			if (!VolumePreviewData)
			{
				VOXEL_MESSAGE(Error, "{0}: No Volume Preview Data node in Editor Graph found", this);
				return;
			}

			FVoxelGraphQueryImpl& NewGraphQuery = VolumePreviewData->Query.CloneParameters();
			NewGraphQuery.AddParameter<FVoxelGraphParameters::FPosition3D>().SetLocalPosition(*InPosition);

			FVoxelGraphQuery NewQuery = FVoxelGraphQuery(NewGraphQuery, Query.GetCallstack());

			TOptional<TValue<FVoxelFloatBuffer>> Distances;
			if (DistancePin.ShouldCompute())
			{
				Distances = VolumePreviewData->ValuePin.Get(NewQuery);
			}
			TOptional<TValue<FVoxelSurfaceTypeBlendBuffer>> SurfaceTypes;
			if (SurfaceTypePin.ShouldCompute())
			{
				SurfaceTypes = VolumePreviewData->SurfaceTypePin.Get(NewQuery);
			}

			TVoxelArray<TValue<FVoxelBuffer>> MetadataToBuffer;
			MetadataToBuffer.Reserve(MetadataPins.Num());

			TSharedPtr<TVoxelMap<FVoxelMetadataRef, int32>> MetadataToBufferIndex = MakeShared<TVoxelMap<FVoxelMetadataRef, int32>>();
			MetadataToBufferIndex->Reserve(MetadataPins.Num());

			for (const FMetadataPin& MetadataPin : MetadataPins)
			{
				if (!MetadataPin.PinRef.ShouldCompute())
				{
					continue;
				}

				if (const TPinRef_Input<FVoxelBuffer>* Pin = VolumePreviewData->MetadataPins.Find(MetadataPin.MetadataRef))
				{
					int32 Index = MetadataToBuffer.Add(Pin->Get(NewQuery));
					MetadataToBufferIndex->Add_EnsureNew(MetadataPin.MetadataRef, Index);
				}
			}

			const int32 NumPositions = Position.Num();
			VOXEL_GRAPH_WAIT(NumPositions, Distances, SurfaceTypes, MetadataToBuffer, MetadataToBufferIndex)
			{
				if (Distances.IsSet())
				{
					DistancePin.Set(Query, *Distances);
				}
				if (SurfaceTypes.IsSet())
				{
					SurfaceTypePin.Set(Query, *SurfaceTypes);
				}

				for (const FMetadataPin& MetadataPin : MetadataPins)
				{
					if (!MetadataPin.PinRef.ShouldCompute())
					{
						continue;
					}

					TSharedPtr<const FVoxelBuffer> Result;
					if (const int32* Index = MetadataToBufferIndex->Find(MetadataPin.MetadataRef))
					{
						Result = MetadataToBuffer[*Index];
					}
					else
					{
						Result = MetadataPin.MetadataRef.MakeDefaultBuffer(NumPositions);
					}

					MetadataPin.PinRef.Set(Query, FVoxelRuntimePinValue::Make(Result.ToSharedRef()));
				}
			};
			return;
		}

		const FVoxelGraphParameters::FVolumeQuery* VolumeQuery = Query->FindParameter<FVoxelGraphParameters::FVolumeQuery>();
		if (!VolumeQuery ||
			!VolumeQuery->QueryPrevious)
		{
			VOXEL_MESSAGE(Error, "{0}: no previous stamp data", this);
			return;
		}

		const FVoxelGraphParameters::FVolumeQueryPrevious* VolumeQueryPrevious = Query->FindParameter<FVoxelGraphParameters::FVolumeQueryPrevious>();

		// Will be null in bulk queries
		if (VolumeQueryPrevious)
		{
			if (VolumeQueryPrevious->bQueried ||
				VolumeQueryPrevious->GlobalPositions.X.GetData() != Position.X.GetData() ||
				VolumeQueryPrevious->GlobalPositions.Y.GetData() != Position.Y.GetData() ||
				VolumeQueryPrevious->GlobalPositions.Z.GetData() != Position.Z.GetData())
			{
				VolumeQueryPrevious = nullptr;
			}
		}

		VOXEL_SCOPE_COUNTER_COND(VolumeQueryPrevious, "Query global previous");

		FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
		if (SurfaceTypePin.ShouldCompute() ||
			(VolumeQueryPrevious && VolumeQueryPrevious->bQueryPreviousSurfaceTypes))
		{
			SurfaceTypes.AllocateZeroed(Position.Num());
		}

		TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
		MetadataToBuffer.Reserve(MetadataPins.Num());

		for (const FMetadataPin& MetadataPin : MetadataPins)
		{
			if (MetadataPin.PinRef.ShouldCompute())
			{
				MetadataToBuffer.Add_EnsureNew(
					MetadataPin.MetadataRef,
					MetadataPin.MetadataRef.MakeDefaultBuffer(Position.Num()));
			}
		}

		if (VolumeQueryPrevious)
		{
			for (const FVoxelMetadataRef& Metadata : VolumeQueryPrevious->PreviousMetadatasToQuery)
			{
				if (MetadataToBuffer.Contains(Metadata))
				{
					continue;
				}

				MetadataToBuffer.Add_EnsureNew(
					Metadata,
					Metadata.MakeDefaultBuffer(Position.Num()));
			}
		}

		FVoxelFloatBuffer Distance;
		Distance.Allocate(Position.Num());
		Distance.SetAll(FVoxelUtilities::NaNf());

		FVoxelDoubleVectorBuffer QueryPosition;
		{
			VOXEL_SCOPE_COUNTER_NUM("VoxelSamplePreviousStampsNodes_TransformVolumePositions", Position.Num());

			QueryPosition.Allocate(Position.Num());

			ispc::VoxelSamplePreviousStampsNodes_TransformVolumePositions(
				VolumeQuery->StampToQuery.ISPC(),
				Position.X.GetData(),
				Position.Y.GetData(),
				Position.Z.GetData(),
				QueryPosition.X.GetData(),
				QueryPosition.Y.GetData(),
				QueryPosition.Z.GetData(),
				Position.Num());
		}

		VolumeQuery->QueryPrevious->Query(FVoxelVolumeSparseQuery::Create(
			VolumeQuery->Query,
			Distance.View(),
			SurfaceTypes.View(),
			MetadataToBuffer,
			QueryPosition,
			SurfaceTypes.Num() > 0,
			MetadataToBuffer.KeyArray()));

		if (VolumeQueryPrevious)
		{
			ensure(!VolumeQueryPrevious->bQueried);
			VolumeQueryPrevious->bQueried = true;

			if (VolumeQueryPrevious->bQueryPreviousValues)
			{
				VolumeQueryPrevious->OutValues = DistancePin.ShouldCompute() ? Distance.MakeDeepCopy() : Distance;
			}

			if (VolumeQueryPrevious->bQueryPreviousSurfaceTypes)
			{
				VolumeQueryPrevious->OutSurfaceTypes = SurfaceTypes;
			}

			VolumeQueryPrevious->OutMetadataToBuffer.Reserve(VolumeQueryPrevious->PreviousMetadatasToQuery.Num());

			for (const FVoxelMetadataRef& Metadata : VolumeQueryPrevious->PreviousMetadatasToQuery)
			{
				VolumeQueryPrevious->OutMetadataToBuffer.Add_EnsureNew(Metadata, MetadataToBuffer[Metadata]);
			}
		}

		if (DistancePin.ShouldCompute())
		{
			VOXEL_SCOPE_COUNTER_NUM("VoxelSamplePreviousStampsNodes_TransformDistances", Distance.Num());

			ispc::VoxelSamplePreviousStampsNodes_TransformDistances(
				VolumeQuery->StampToQuery.ISPC(),
				Distance.GetData(),
				Distance.Num());

			DistancePin.Set(Query, Distance);
		}

		if (SurfaceTypePin.ShouldCompute())
		{
			SurfaceTypePin.Set(Query, SurfaceTypes);
		}

		for (const FMetadataPin& MetadataPin : MetadataPins)
		{
			if (MetadataPin.PinRef.ShouldCompute())
			{
				MetadataPin.PinRef.Set(Query, FVoxelRuntimePinValue::Make(MetadataToBuffer[MetadataPin.MetadataRef]));
			}
		}
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_SampleTerrain::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelDoubleVectorBuffer> InPosition = PositionPin.Get(Query);

	VOXEL_GRAPH_WAIT(InPosition)
	{
		FVoxelDoubleVectorBuffer Position = *InPosition;
		Position.ExpandConstants(Position.Num());

		VOXEL_FUNCTION_COUNTER_NUM(Position.Num());
		FVoxelNodeStatScope StatScope(*this, Position.Num());

		if (Query.IsPreview())
		{
			const FVoxelGraphParameters::FSmartSurfaceTypePreview* Parameter = Query->FindParameter<FVoxelGraphParameters::FSmartSurfaceTypePreview>();
			if (!Parameter)
			{
				VOXEL_MESSAGE(Error, "{0}: No Smart Surface Type Preview Data node in Editor Graph found", this);
				return;
			}

			FVoxelGraphQueryImpl& NewGraphQuery = Parameter->Query.CloneParameters();
			NewGraphQuery.AddParameter<FVoxelGraphParameters::FPosition3D>().SetLocalPosition(*InPosition);

			FVoxelGraphQuery NewQuery = FVoxelGraphQuery(NewGraphQuery, Query.GetCallstack());

			TOptional<TValue<FVoxelFloatBuffer>> Distances;
			if (DistancePin.ShouldCompute())
			{
				Distances = Parameter->ValuePin.Get(NewQuery);
			}
			TOptional<TValue<FVoxelSurfaceTypeBlendBuffer>> SurfaceTypes;
			if (SurfaceTypePin.ShouldCompute())
			{
				SurfaceTypes = Parameter->SurfaceTypePin.Get(NewQuery);
			}

			TVoxelArray<TValue<FVoxelBuffer>> MetadataToBuffer;
			MetadataToBuffer.Reserve(MetadataPins.Num());

			TSharedPtr<TVoxelMap<FVoxelMetadataRef, int32>> MetadataToBufferIndex = MakeShared<TVoxelMap<FVoxelMetadataRef, int32>>();
			MetadataToBufferIndex->Reserve(MetadataPins.Num());

			for (const FMetadataPin& MetadataPin : MetadataPins)
			{
				if (!MetadataPin.PinRef.ShouldCompute())
				{
					continue;
				}

				if (const TPinRef_Input<FVoxelBuffer>* Pin = Parameter->MetadataPins.Find(MetadataPin.MetadataRef))
				{
					int32 Index = MetadataToBuffer.Add(Pin->Get(NewQuery));
					MetadataToBufferIndex->Add_EnsureNew(MetadataPin.MetadataRef, Index);
				}
			}

			const int32 NumPositions = Position.Num();
			VOXEL_GRAPH_WAIT(NumPositions, Distances, SurfaceTypes, MetadataToBuffer, MetadataToBufferIndex)
			{
				if (Distances.IsSet())
				{
					DistancePin.Set(Query, *Distances);
				}
				if (SurfaceTypes.IsSet())
				{
					SurfaceTypePin.Set(Query, *SurfaceTypes);
				}

				for (const FMetadataPin& MetadataPin : MetadataPins)
				{
					if (!MetadataPin.PinRef.ShouldCompute())
					{
						continue;
					}

					TSharedPtr<const FVoxelBuffer> Result;
					if (const int32* Index = MetadataToBufferIndex->Find(MetadataPin.MetadataRef))
					{
						Result = MetadataToBuffer[*Index];
					}
					else
					{
						Result = MetadataPin.MetadataRef.MakeDefaultBuffer(NumPositions);
					}

					MetadataPin.PinRef.Set(Query, FVoxelRuntimePinValue::Make(Result.ToSharedRef()));
				}
			};
			return;
		}

		const FVoxelGraphParameters::FSmartSurfaceUniform* SmartSurface = Query->FindParameter<FVoxelGraphParameters::FSmartSurfaceUniform>();
		if (!SmartSurface)
		{
			VOXEL_MESSAGE(Error, "{0}: no previous stamp data, can only be used in surface graphs", this);
			return;
		}

		const FVoxelGraphParameters::FQuery* QueryParameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
		if (!ensureVoxelSlow(QueryParameter))
		{
			VOXEL_MESSAGE(Error, "{0}: no previous stamp data", this);
			return;
		}

		FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
		if (SurfaceTypePin.ShouldCompute())
		{
			SurfaceTypes.AllocateZeroed(Position.Num());
		}

		TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
		MetadataToBuffer.Reserve(MetadataPins.Num());

		for (const FMetadataPin& MetadataPin : MetadataPins)
		{
			if (MetadataPin.PinRef.ShouldCompute())
			{
				MetadataToBuffer.Add_EnsureNew(
					MetadataPin.MetadataRef,
					MetadataPin.MetadataRef.MakeDefaultBuffer(Position.Num()));
			}
		}

		const FVoxelFloatBuffer Distance = QueryParameter->Query.SampleVolumeLayer(
			SmartSurface->WeakLayer,
			Position,
			SurfaceTypes.View(),
			MetadataToBuffer);

		if (DistancePin.ShouldCompute())
		{
			DistancePin.Set(Query, Distance);
		}

		if (SurfaceTypePin.ShouldCompute())
		{
			SurfaceTypePin.Set(Query, SurfaceTypes);
		}

		for (const FMetadataPin& MetadataPin : MetadataPins)
		{
			if (MetadataPin.PinRef.ShouldCompute())
			{
				MetadataPin.PinRef.Set(Query, FVoxelRuntimePinValue::Make(MetadataToBuffer[MetadataPin.MetadataRef]));
			}
		}
	};
}