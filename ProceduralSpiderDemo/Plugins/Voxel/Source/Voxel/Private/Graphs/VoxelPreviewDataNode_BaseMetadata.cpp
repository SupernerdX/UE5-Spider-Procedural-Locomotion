// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelPreviewDataNode_BaseMetadata.h"
#include "VoxelMetadata.h"

FVoxelPreviewDataNode_BaseMetadata::FVoxelPreviewDataNode_BaseMetadata()
{
	FixupMetadataPins();
}

void FVoxelPreviewDataNode_BaseMetadata::Initialize(FInitializer& Initializer)
{
	VOXEL_FUNCTION_COUNTER();

	for (FVoxelQueryParameterNode_MetadataPin& MetadataPin : MetadataPins)
	{
		Initializer.InitializePinRef(MetadataPin.PinRef);
	}
}

void FVoxelPreviewDataNode_BaseMetadata::PostSerialize()
{
	Super::PostSerialize();

	FixupMetadataPins();
}

#if WITH_EDITOR
FVoxelNode::EPostEditChange FVoxelPreviewDataNode_BaseMetadata::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_OWN_MEMBER_NAME(MetadatasToQuery))
	{
		FixupMetadataPins();
		return EPostEditChange::Reconstruct;
	}

	return EPostEditChange::None;
}
#endif

void FVoxelPreviewDataNode_BaseMetadata::FixupMetadataPins()
{
	VOXEL_FUNCTION_COUNTER();

	for (const FVoxelQueryParameterNode_MetadataPin& MetadataPin : MetadataPins)
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

		MetadataPins.Add(FVoxelQueryParameterNode_MetadataPin
		{
			FVoxelMetadataRef(Metadata),
			TPinRef_Input<FVoxelBuffer>(CreateInputPin(
				Metadata->GetInnerType().GetBufferType(),
				Metadata->GetFName(),
				VOXEL_PIN_METADATA(
					void,
					nullptr,
					DisplayName(Metadata->GetName()),
					Category("Metadata")))),
		});
	}
}