// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelSampleStampNodes.h"
#include "Graphs/VoxelStampGraphParameters.h"
#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "VoxelMetadata.h"
#include "VoxelGraphMigration.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	REGISTER_VOXEL_NODE_MIGRATION("SampleHeightStamp", FVoxelNode_SampleHeightStamp);
	REGISTER_VOXEL_NODE_MIGRATION("SampleVolumeStamp", FVoxelNode_SampleVolumeStamp);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_SampleHeightStampBase::ComputeImpl(
	const FVoxelGraphQuery& Query,
	const FVoxelDoubleVector2DBuffer& InPosition,
	const FVoxelFloatBuffer& PreviousHeight,
	const TSharedPtr<const FVoxelHeightStampRuntime>& StampRuntime) const
{
	const int32 Num = ComputeVoxelBuffersNum(InPosition, PreviousHeight);

	VOXEL_FUNCTION_COUNTER_NUM(Num);
	FVoxelNodeStatScope StatScope(*this, Num);

	FVoxelDoubleVector2DBuffer LocalPosition = InPosition;
	LocalPosition.ExpandConstants(Num);

	if (!StampRuntime)
	{
		VOXEL_MESSAGE(Error, "{0}: Invalid stamp", this);
		return;
	}

	const FVoxelGraphParameters::FQuery* QueryParameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!QueryParameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot sample stamp here, no query", this);
		return;
	}

	FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
	if (SurfaceTypePin.ShouldCompute())
	{
		SurfaceTypes.AllocateZeroed(Num);
	}

	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
	MetadataToBuffer.Reserve(MetadataPins.Num());

	for (const FMetadataPin& MetadataPin : MetadataPins)
	{
		if (MetadataPin.PinRef.ShouldCompute())
		{
			MetadataToBuffer.Add_EnsureNew(
				MetadataPin.MetadataRef,
				MetadataPin.MetadataRef.MakeDefaultBuffer(Num));
		}
	}

	FVoxelFloatBuffer Height;
	Height.Allocate(Num);
	Height.CopyFrom(PreviousHeight, 0, 0, Num);

	const FVoxelQuery VoxelQuery(
		0,
		FVoxelLayers::Empty(),
		QueryParameter->Query.SurfaceTypeTable,
		Query->Context.DependencyCollector);

	const FVoxelHeightSparseQuery SparseQuery = FVoxelHeightSparseQuery::Create(
		VoxelQuery,
		Height.View(),
		SurfaceTypes.View(),
		MetadataToBuffer,
		LocalPosition,
		SurfaceTypes.Num() > 0,
		MetadataToBuffer.KeyArray());

	StampRuntime->Apply(
		SparseQuery,
		{});

	HeightPin.Set(Query, Height);

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
}

void FVoxelNode_SampleVolumeStampBase::ComputeImpl(
	const FVoxelGraphQuery& Query,
	const FVoxelDoubleVectorBuffer& InPosition,
	const FVoxelFloatBuffer& PreviousDistance,
	const TSharedPtr<const FVoxelVolumeStampRuntime>& StampRuntime) const
{
	if (!StampRuntime)
	{
		VOXEL_MESSAGE(Error, "{0}: Invalid stamp", this);
		return;
	}

	const FVoxelGraphParameters::FQuery* QueryParameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!QueryParameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot sample stamp here, no query", this);
		return;
	}

	const int32 Num = ComputeVoxelBuffersNum(InPosition, PreviousDistance);

	VOXEL_FUNCTION_COUNTER_NUM(Num);
	FVoxelNodeStatScope StatScope(*this, Num);

	FVoxelDoubleVectorBuffer LocalPosition = InPosition;
	LocalPosition.ExpandConstants(Num);

	FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
	if (SurfaceTypePin.ShouldCompute())
	{
		SurfaceTypes.AllocateZeroed(Num);
	}

	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
	MetadataToBuffer.Reserve(MetadataPins.Num());

	for (const FMetadataPin& MetadataPin : MetadataPins)
	{
		if (MetadataPin.PinRef.ShouldCompute())
		{
			MetadataToBuffer.Add_EnsureNew(
				MetadataPin.MetadataRef,
				MetadataPin.MetadataRef.MakeDefaultBuffer(Num));
		}
	}

	FVoxelFloatBuffer Distance;
	Distance.Allocate(Num);
	Distance.CopyFrom(PreviousDistance, 0, 0, Num);

	const FVoxelQuery VoxelQuery(
		0,
		FVoxelLayers::Empty(),
		QueryParameter->Query.SurfaceTypeTable,
		Query->Context.DependencyCollector);

	const FVoxelVolumeSparseQuery SparseQuery = FVoxelVolumeSparseQuery::Create(
		VoxelQuery,
		Distance.View(),
		SurfaceTypes.View(),
		MetadataToBuffer,
		LocalPosition,
		SurfaceTypes.Num() > 0,
		MetadataToBuffer.KeyArray());

	StampRuntime->Apply(
		SparseQuery,
		{});

	DistancePin.Set(Query, Distance);

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
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FString FVoxelNode_SampleHeightStamp::GetDisplayName() const
{
	if (!Stamp)
	{
		return "Sample Height Stamp";
	}

	const FString Identifier = Stamp->GetIdentifier();

	if (!Identifier.IsEmpty())
	{
		return "Sample Height Stamp: " + Identifier;
	}

	FString Name = Stamp.GetStruct()->GetDisplayNameText().ToString();
	ensureVoxelSlow(Name.RemoveFromStart("Voxel "));
	ensureVoxelSlow(Name.RemoveFromEnd(" Stamp"));
	return "Sample Height Stamp: " + Name;
}
#endif

void FVoxelNode_SampleHeightStamp::Initialize(FInitializer& Initializer)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Initialize(Initializer);

	const TSharedPtr<FVoxelStampRuntime> Runtime = FVoxelStampRuntime::Create(
		{},
		Stamp,
		{});

	StampRuntime = CastStructEnsured<FVoxelHeightStampRuntime>(Runtime);
}

void FVoxelNode_SampleHeightStamp::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelDoubleVector2DBuffer> Position = PositionPin.Get(Query);
	const TValue<FVoxelFloatBuffer> PreviousHeight = PreviousHeightPin.Get(Query);

	VOXEL_GRAPH_WAIT(Position, PreviousHeight)
	{
		ComputeImpl(
			Query,
			*Position,
			PreviousHeightPin.IsDefaultValue()
			? FVoxelFloatBuffer(FVoxelUtilities::NaNf())
			: *PreviousHeight,
			StampRuntime);
	};
}

void FVoxelNode_SampleHeightStamp::ComputeNoCachePin(
	const FVoxelGraphQuery Query,
	const int32 PinIndex) const
{
	checkVoxelSlow(BoundsPin.GetPinIndex() == PinIndex);

	if (StampRuntime)
	{
		BoundsPin.Set(Query, StampRuntime->GetLocalBounds());
	}
	else
	{
		VOXEL_MESSAGE(Error, "{0}: Invalid stamp", this);
		BoundsPin.Set(Query, FVoxelBox2D());
	}
}

#if WITH_EDITOR
FString FVoxelNode_SampleVolumeStamp::GetDisplayName() const
{
	if (!Stamp)
	{
		return "Sample Volume Stamp";
	}

	const FString Identifier = Stamp->GetIdentifier();

	if (!Identifier.IsEmpty())
	{
		return "Sample Volume Stamp: " + Identifier;
	}

	FString Name = Stamp.GetStruct()->GetDisplayNameText().ToString();
	ensureVoxelSlow(Name.RemoveFromStart("Voxel "));
	ensureVoxelSlow(Name.RemoveFromEnd(" Stamp"));
	return "Sample Volume Stamp: " + Name;
}
#endif

void FVoxelNode_SampleVolumeStamp::Initialize(FInitializer& Initializer)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Initialize(Initializer);

	const TSharedPtr<FVoxelStampRuntime> Runtime = FVoxelStampRuntime::Create(
		{},
		Stamp,
		{});

	StampRuntime = CastStructEnsured<FVoxelVolumeStampRuntime>(Runtime);
}

void FVoxelNode_SampleVolumeStamp::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelDoubleVectorBuffer> Position = PositionPin.Get(Query);
	const TValue<FVoxelFloatBuffer> PreviousDistance = PreviousDistancePin.Get(Query);

	VOXEL_GRAPH_WAIT(Position, PreviousDistance)
	{
		ComputeImpl(
			Query,
			*Position,
			PreviousDistancePin.IsDefaultValue()
			? FVoxelFloatBuffer(FVoxelUtilities::NaNf())
			: *PreviousDistance,
			StampRuntime);
	};
}

void FVoxelNode_SampleVolumeStamp::ComputeNoCachePin(
	const FVoxelGraphQuery Query,
	const int32 PinIndex) const
{
	checkVoxelSlow(BoundsPin.GetPinIndex() == PinIndex);

	if (StampRuntime)
	{
		BoundsPin.Set(Query, StampRuntime->GetLocalBounds());
	}
	else
	{
		VOXEL_MESSAGE(Error, "{0}: Invalid stamp", this);
		BoundsPin.Set(Query, FVoxelBox());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_SampleHeightStampParameter::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelHeightGraphStampWrapper> Stamp = StampPin.Get(Query);
	const TValue<FVoxelDoubleVector2DBuffer> Position = PositionPin.Get(Query);
	const TValue<FVoxelFloatBuffer> PreviousHeight = PreviousHeightPin.Get(Query);

	VOXEL_GRAPH_WAIT(Stamp, Position, PreviousHeight)
	{
		ComputeImpl(
			Query,
			*Position,
			PreviousHeightPin.IsDefaultValue()
			? FVoxelFloatBuffer(FVoxelUtilities::NaNf())
			: *PreviousHeight,
			Stamp->Stamp);
	};
}

void FVoxelNode_SampleHeightStampParameter::ComputeNoCachePin(
	const FVoxelGraphQuery Query,
	const int32 PinIndex) const
{
	checkVoxelSlow(BoundsPin.GetPinIndex() == PinIndex);

	const TValue<FVoxelHeightGraphStampWrapper> Stamp = StampPin.Get(Query);

	VOXEL_GRAPH_WAIT(Stamp)
	{
		if (Stamp->Stamp)
		{
			BoundsPin.Set(Query, Stamp->Stamp->GetLocalBounds());
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: Invalid stamp", this);
			BoundsPin.Set(Query, FVoxelBox2D());
		}
	};
}

void FVoxelNode_SampleVolumeStampParameter::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelVolumeGraphStampWrapper> Stamp = StampPin.Get(Query);
	const TValue<FVoxelDoubleVectorBuffer> Position = PositionPin.Get(Query);
	const TValue<FVoxelFloatBuffer> PreviousDistance = PreviousDistancePin.Get(Query);

	VOXEL_GRAPH_WAIT(Stamp, Position, PreviousDistance)
	{
		ComputeImpl(
			Query,
			*Position,
			PreviousDistancePin.IsDefaultValue()
			? FVoxelFloatBuffer(FVoxelUtilities::NaNf())
			: *PreviousDistance,
			Stamp->Stamp);
	};
}

void FVoxelNode_SampleVolumeStampParameter::ComputeNoCachePin(
	const FVoxelGraphQuery Query,
	const int32 PinIndex) const
{
	checkVoxelSlow(BoundsPin.GetPinIndex() == PinIndex);

	const TValue<FVoxelVolumeGraphStampWrapper> Stamp = StampPin.Get(Query);

	VOXEL_GRAPH_WAIT(Stamp)
	{
		if (Stamp->Stamp)
		{
			BoundsPin.Set(Query, Stamp->Stamp->GetLocalBounds());
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: Invalid stamp", this);
			BoundsPin.Set(Query, FVoxelBox());
		}
	};
}