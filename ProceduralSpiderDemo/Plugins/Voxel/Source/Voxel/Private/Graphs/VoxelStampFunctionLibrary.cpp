// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelStampFunctionLibrary.h"
#include "Graphs/VoxelStampGraphParameters.h"
#include "Surface/VoxelSurfaceTypeBlendUtilities.h"
#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "VoxelStampTree.h"
#include "VoxelHeightLayer.h"
#include "VoxelVolumeLayer.h"
#include "VoxelGraphMigration.h"
#include "VoxelGraphPositionParameter.h"
#include "Graphs/VoxelPreviewStampDataNodes.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	REGISTER_VOXEL_FUNCTION_MIGRATION("BlendMaterials", UVoxelStampFunctionLibrary, BlendSurfaceTypes);
	REGISTER_VOXEL_FUNCTION_MIGRATION("GetHeightSmoothness", UVoxelStampFunctionLibrary, GetHeightSmoothness);
	REGISTER_VOXEL_FUNCTION_MIGRATION("GetHeightBlendMode", UVoxelStampFunctionLibrary, GetHeightBlendMode);
	REGISTER_VOXEL_FUNCTION_MIGRATION("IsHeightOverrideBlendMode", UVoxelStampFunctionLibrary, IsHeightOverrideBlendMode);
	REGISTER_VOXEL_FUNCTION_MIGRATION("GetVolumeSmoothness", UVoxelStampFunctionLibrary, GetVolumeSmoothness);
	REGISTER_VOXEL_FUNCTION_MIGRATION("GetVolumeBlendMode", UVoxelStampFunctionLibrary, GetVolumeBlendMode);
	REGISTER_VOXEL_FUNCTION_MIGRATION("IsVolumeOverrideBlendMode", UVoxelStampFunctionLibrary, IsVolumeOverrideBlendMode);
	REGISTER_VOXEL_FUNCTION_MIGRATION("QueryHeight", UVoxelStampFunctionLibrary, QueryHeight);
	REGISTER_VOXEL_FUNCTION_MIGRATION("QueryDistance", UVoxelStampFunctionLibrary, QueryDistance);
	REGISTER_VOXEL_FUNCTION_MIGRATION("QueryHeightMaterial", UVoxelStampFunctionLibrary, QueryHeightSurfaceType);
	REGISTER_VOXEL_FUNCTION_MIGRATION("QueryVolumeMaterial", UVoxelStampFunctionLibrary, QueryVolumeSurfaceType);
	REGISTER_VOXEL_FUNCTION_MIGRATION("QueryHeightMetadata", UVoxelStampFunctionLibrary, QueryHeightMetadata);
	REGISTER_VOXEL_FUNCTION_MIGRATION("QueryVolumeMetadata", UVoxelStampFunctionLibrary, QueryVolumeMetadata);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSurfaceTypeBlendBuffer UVoxelStampFunctionLibrary::BlendSurfaceTypes(
	const FVoxelSurfaceTypeBlendBuffer& A,
	const FVoxelSurfaceTypeBlendBuffer& B,
	const FVoxelFloatBuffer& Alpha) const
{
	CheckVoxelBuffersNum_Return(A, B, Alpha);

	return FVoxelSurfaceTypeBlendUtilities::Lerp(A, B, Alpha);
}

FVoxelFloatBuffer UVoxelStampFunctionLibrary::GetSurfaceTypeWeight(
	const FVoxelSurfaceTypeBlendBuffer& Blend,
	const FVoxelSurfaceTypeBuffer& Type) const
{
	const int32 Num = ComputeVoxelBuffersNum_Return(Blend, Type);

	FVoxelFloatBuffer Result;
	Result.Allocate(Num);

	for (int32 Index = 0; Index < Num; Index++)
	{
		const FVoxelSurfaceType TypeToFind = Type[Index];

		const float Weight = INLINE_LAMBDA
		{
			for (const FVoxelSurfaceTypeBlendLayer& Layer : Blend[Index].GetLayers())
			{
				if (Layer.Type == TypeToFind)
				{
					return Layer.Weight.ToFloat();
				}
			}

			return 0.f;
		};

		Result.Set(Index, Weight);
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSeed UVoxelStampFunctionLibrary::GetStampSeed() const
{
	const FVoxelGraphParameters::FStampSeed* Parameter = Query->FindParameter<FVoxelGraphParameters::FStampSeed>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Volume or Height Preview Data node in Editor Graph found", this);
		}
		return {};
	}

	return Parameter->Seed;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float UVoxelStampFunctionLibrary::GetHeightSmoothness() const
{
	const FVoxelGraphParameters::FHeightStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FHeightStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Height Preview Data node in Editor Graph found", this);
		}
		return 0.f;
	}

	return Parameter->Smoothness;
}

EVoxelHeightBlendMode UVoxelStampFunctionLibrary::GetHeightBlendMode() const
{
	const FVoxelGraphParameters::FHeightStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FHeightStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Height Preview Data node in Editor Graph found", this);
		}
		return {};
	}

	return Parameter->BlendMode;
}

bool UVoxelStampFunctionLibrary::IsHeightOverrideBlendMode() const
{
	return GetHeightBlendMode() == EVoxelHeightBlendMode::Override;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float UVoxelStampFunctionLibrary::GetVolumeSmoothness() const
{
	const FVoxelGraphParameters::FVolumeStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FVolumeStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Volume Preview Data node in Editor Graph found", this);
		}
		return 0.f;
	}

	return Parameter->Smoothness;
}

EVoxelVolumeBlendMode UVoxelStampFunctionLibrary::GetVolumeBlendMode() const
{
	const FVoxelGraphParameters::FVolumeStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FVolumeStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Volume Preview Data node in Editor Graph found", this);
		}
		return {};
	}

	return Parameter->BlendMode;
}

bool UVoxelStampFunctionLibrary::IsVolumeOverrideBlendMode() const
{
	return GetVolumeBlendMode() == EVoxelVolumeBlendMode::Override;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelStampFunctionLibrary::QueryHeight(
	const FVoxelDoubleVector2DBuffer& Position,
	const FVoxelWeakStackHeightLayer& Layer,
	FVoxelFloatBuffer& Height,
	bool& bIsValid) const
{
	if (Query.IsPreview())
	{
		const FVoxelGraphParameters::FHeightStampPreviewData* Parameter = Query->FindParameter<FVoxelGraphParameters::FHeightStampPreviewData>();
		if (!Parameter)
		{
			VOXEL_MESSAGE(Error, "{0}: No Height Preview Data node in Editor Graph found", this);
			Height = 0.f;
			bIsValid = true;
			return;
		}

		FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
		NewQuery.AddParameter<FVoxelGraphParameters::FPosition2D>().SetWorldPosition(Position);
		Height = *Parameter->ValuePin.GetSynchronous(Query.GetImpl());
		bIsValid = true;
		return;
	}

	const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot call here", this);
		return;
	}
	const FVoxelQuery& VoxelQuery = Parameter->Query;

	if (!VoxelQuery.CheckNoRecursion(Layer) ||
		!VoxelQuery.Layers.HasLayer(Layer, Query->Context.DependencyCollector))
	{
		return;
	}

	Height = VoxelQuery.SampleHeightLayer(Layer, Position);

	bIsValid = true;
}

void UVoxelStampFunctionLibrary::QueryDistance(
	const FVoxelDoubleVectorBuffer& Position,
	const FVoxelWeakStackVolumeLayer& Layer,
	FVoxelFloatBuffer& Distance,
	bool& bIsValid) const
{
	if (Query.IsPreview())
	{
		const FVoxelGraphParameters::FVolumeStampPreviewData* Parameter = Query->FindParameter<FVoxelGraphParameters::FVolumeStampPreviewData>();
		if (!Parameter)
		{
			VOXEL_MESSAGE(Error, "{0}: No Volume Preview Data node in Editor Graph found", this);
			Distance = 1e9f;
			bIsValid = true;
			return;
		}

		FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
		NewQuery.AddParameter<FVoxelGraphParameters::FPosition3D>().SetWorldPosition(Position);
		Distance = *Parameter->ValuePin.GetSynchronous(Query.GetImpl());
		bIsValid = true;
		return;
	}

	const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot call here", this);
		return;
	}
	const FVoxelQuery& VoxelQuery = Parameter->Query;

	if (!VoxelQuery.CheckNoRecursion(Layer) ||
		!VoxelQuery.Layers.HasLayer(Layer, Query->Context.DependencyCollector))
	{
		return;
	}

	Distance = VoxelQuery.SampleVolumeLayer(Layer, Position);

	bIsValid = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelStampFunctionLibrary::QueryHeightSurfaceType(
	const FVoxelDoubleVector2DBuffer& Position,
	const FVoxelWeakStackHeightLayer& Layer,
	FVoxelFloatBuffer& Height,
	FVoxelSurfaceTypeBlendBuffer& SurfaceType,
	bool& bIsValid) const
{
	if (Query.IsPreview())
	{
		const FVoxelGraphParameters::FHeightStampPreviewData* Parameter = Query->FindParameter<FVoxelGraphParameters::FHeightStampPreviewData>();
		if (!Parameter)
		{
			VOXEL_MESSAGE(Error, "{0}: No Height Preview Data node in Editor Graph found", this);
			Height = 0.f;
			bIsValid = true;
			return;
		}

		FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
		NewQuery.AddParameter<FVoxelGraphParameters::FPosition2D>().SetWorldPosition(Position);
		Height = *Parameter->ValuePin.GetSynchronous(Query.GetImpl());
		SurfaceType = *Parameter->SurfaceTypePin.GetSynchronous(Query.GetImpl());
		bIsValid = true;
		return;
	}

	const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot call here", this);
		return;
	}
	const FVoxelQuery& VoxelQuery = Parameter->Query;

	if (!VoxelQuery.CheckNoRecursion(Layer) ||
		!VoxelQuery.Layers.HasLayer(Layer, Query->Context.DependencyCollector))
	{
		return;
	}

	FVoxelDoubleVector2DBuffer SafePositions = Position;
	SafePositions.ExpandConstants();

	SurfaceType.AllocateZeroed(SafePositions.Num());

	Height = VoxelQuery.SampleHeightLayer(
		Layer,
		SafePositions,
		SurfaceType.View(),
		{});

	bIsValid = true;
}

void UVoxelStampFunctionLibrary::QueryVolumeSurfaceType(
	const FVoxelDoubleVectorBuffer& Position,
	const FVoxelWeakStackVolumeLayer& Layer,
	FVoxelFloatBuffer& Distance,
	FVoxelSurfaceTypeBlendBuffer& SurfaceType,
	bool& bIsValid) const
{
	if (Query.IsPreview())
	{
		const FVoxelGraphParameters::FVolumeStampPreviewData* Parameter = Query->FindParameter<FVoxelGraphParameters::FVolumeStampPreviewData>();
		if (!Parameter)
		{
			VOXEL_MESSAGE(Error, "{0}: No Volume Preview Data node in Editor Graph found", this);
			Distance = 1e9f;
			bIsValid = true;
			return;
		}

		FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
		NewQuery.AddParameter<FVoxelGraphParameters::FPosition3D>().SetWorldPosition(Position);
		Distance = *Parameter->ValuePin.GetSynchronous(Query.GetImpl());
		SurfaceType = *Parameter->SurfaceTypePin.GetSynchronous(Query.GetImpl());
		bIsValid = true;
		return;
	}

	const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot call here", this);
		return;
	}
	const FVoxelQuery& VoxelQuery = Parameter->Query;

	if (!VoxelQuery.CheckNoRecursion(Layer) ||
		!VoxelQuery.Layers.HasLayer(Layer, Query->Context.DependencyCollector))
	{
		return;
	}

	FVoxelDoubleVectorBuffer SafePositions = Position;
	SafePositions.ExpandConstants();

	SurfaceType.AllocateZeroed(SafePositions.Num());

	Distance = VoxelQuery.SampleVolumeLayer(
		Layer,
		SafePositions,
		SurfaceType.View(),
		{});

	bIsValid = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelStampFunctionLibrary::QueryHeightMetadata(
	const FVoxelDoubleVector2DBuffer& Position,
	const FVoxelWeakStackHeightLayer& Layer,
	const FVoxelFloatMetadataRef& Metadata,
	FVoxelFloatBuffer& Height,
	FVoxelFloatBuffer& Value,
	bool& bIsValid) const
{
	if (Query.IsPreview())
	{
		const FVoxelGraphParameters::FHeightStampPreviewData* Parameter = Query->FindParameter<FVoxelGraphParameters::FHeightStampPreviewData>();
		if (!Parameter)
		{
			VOXEL_MESSAGE(Error, "{0}: No Height Preview Data node in Editor Graph found", this);
			Height = 1e9f;
			bIsValid = true;
			return;
		}

		FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
		NewQuery.AddParameter<FVoxelGraphParameters::FPosition2D>().SetWorldPosition(Position);
		Height = *Parameter->ValuePin.GetSynchronous(Query.GetImpl());

		bool bFound = false;
		for (const auto& It : Parameter->MetadataPins)
		{
			if (It.Key == Metadata)
			{
				Value = It.Value.GetSynchronous(Query.GetImpl())->AsChecked<FVoxelFloatBuffer>();
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			Value = Metadata.MakeDefaultBuffer(Position.Num())->AsChecked<FVoxelFloatBuffer>();
		}

		bIsValid = true;
		return;
	}

	if (!Metadata.IsValid())
	{
		VOXEL_MESSAGE(Error, "{0}: Metadata is null", this);
		return;
	}

	const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot call here", this);
		return;
	}
	const FVoxelQuery& VoxelQuery = Parameter->Query;

	if (!VoxelQuery.CheckNoRecursion(Layer) ||
		!VoxelQuery.Layers.HasLayer(Layer, Query->Context.DependencyCollector))
	{
		return;
	}

	FVoxelDoubleVector2DBuffer SafePositions = Position;
	SafePositions.ExpandConstants();

	Value.AllocateZeroed(Position.Num());

	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
	MetadataToBuffer.Add_EnsureNew(Metadata, MakeSharedCopy(Value));

	Height = VoxelQuery.SampleHeightLayer(
		Layer,
		Position,
		{},
		MetadataToBuffer);

	bIsValid = true;
}

void UVoxelStampFunctionLibrary::QueryVolumeMetadata(
	const FVoxelDoubleVectorBuffer& Position,
	const FVoxelWeakStackVolumeLayer& Layer,
	const FVoxelFloatMetadataRef& Metadata,
	FVoxelFloatBuffer& Distance,
	FVoxelFloatBuffer& Value,
	bool& bIsValid) const
{
	if (Query.IsPreview())
	{
		const FVoxelGraphParameters::FVolumeStampPreviewData* Parameter = Query->FindParameter<FVoxelGraphParameters::FVolumeStampPreviewData>();
		if (!Parameter)
		{
			VOXEL_MESSAGE(Error, "{0}: No Volume Preview Data node in Editor Graph found", this);
			Distance = 1e9f;
			bIsValid = true;
			return;
		}

		FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
		NewQuery.AddParameter<FVoxelGraphParameters::FPosition3D>().SetWorldPosition(Position);
		Distance = *Parameter->ValuePin.GetSynchronous(Query.GetImpl());

		bool bFound = false;
		for (const auto& It : Parameter->MetadataPins)
		{
			if (It.Key == Metadata)
			{
				Value = It.Value.GetSynchronous(Query.GetImpl())->AsChecked<FVoxelFloatBuffer>();
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			Value = Metadata.MakeDefaultBuffer(Position.Num())->AsChecked<FVoxelFloatBuffer>();
		}

		bIsValid = true;
		return;
	}

	if (!Metadata.IsValid())
	{
		VOXEL_MESSAGE(Error, "{0}: Metadata is null", this);
		return;
	}

	const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot call here", this);
		return;
	}
	const FVoxelQuery& VoxelQuery = Parameter->Query;

	if (!VoxelQuery.CheckNoRecursion(Layer) ||
		!VoxelQuery.Layers.HasLayer(Layer, Query->Context.DependencyCollector))
	{
		return;
	}

	FVoxelDoubleVectorBuffer SafePositions = Position;
	SafePositions.ExpandConstants();

	Value.AllocateZeroed(Position.Num());

	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
	MetadataToBuffer.Add_EnsureNew(Metadata, MakeSharedCopy(Value));

	Distance = VoxelQuery.SampleVolumeLayer(
		Layer,
		SafePositions,
		{},
		MetadataToBuffer);

	bIsValid = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelStampFunctionLibrary::HasStampsInBounds(
	const FVoxelDoubleVectorBuffer& Position,
	const FVoxelWeakStackLayer& Layer,
	const EVoxelStampBehavior BehaviorMask) const
{
	// In preview mode, assume stamps exist for safety
	if (Query.IsPreview())
	{
		return true;
	}

	const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot call here", this);
		return true;
	}
	const FVoxelQuery& VoxelQuery = Parameter->Query;

	if (!VoxelQuery.Layers.HasLayer(Layer, Query->Context.DependencyCollector))
	{
		return false;
	}

	// Compute bounds from positions
	const FVoxelBox Bounds = FVoxelBox::FromPositions(
		Position.X.View(),
		Position.Y.View(),
		Position.Z.View());

	if (!Bounds.IsValid())
	{
		return false;
	}

	// Query only this specific layer's tree, not previous layers
	bool bHasStamps = false;

	if (Layer.Type == EVoxelLayerType::Height)
	{
		const TSharedPtr<const FVoxelHeightLayer> HeightLayer = VoxelQuery.Layers.FindHeightLayer(Layer, Query->Context.DependencyCollector);
		if (!HeightLayer)
		{
			return false;
		}

		HeightLayer->GetTree(VoxelQuery.LOD).ForeachElement_Unsorted(
			Query->Context.DependencyCollector,
			Bounds,
			BehaviorMask,
			[&](const FVoxelStampTreeElement& Element)
			{
				bHasStamps = true;
				return EVoxelIterate::Stop;
			});
	}
	else
	{
		check(Layer.Type == EVoxelLayerType::Volume);

		const TSharedPtr<const FVoxelVolumeLayer> VolumeLayer = VoxelQuery.Layers.FindVolumeLayer(Layer, Query->Context.DependencyCollector);
		if (!VolumeLayer)
		{
			return false;
		}

		VolumeLayer->GetTree(VoxelQuery.LOD).ForeachElement_Unsorted(
			Query->Context.DependencyCollector,
			Bounds,
			BehaviorMask,
			[&](const FVoxelStampTreeElement& Element)
			{
				bHasStamps = true;
				return EVoxelIterate::Stop;
			});
	}

	return bHasStamps;
}