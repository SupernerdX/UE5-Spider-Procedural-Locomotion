// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelNode_MetadataPinsBase.h"
#include "VoxelMetadata.h"

FVoxelNode_MetadataPinsBase::FVoxelNode_MetadataPinsBase()
{
	FixupMetadataPins();
}

void FVoxelNode_MetadataPinsBase::Initialize(FInitializer& Initializer)
{
	VOXEL_FUNCTION_COUNTER();

	for (FMetadataPin& MetadataPin : MetadataPins)
	{
		Initializer.InitializePinRef(MetadataPin.PinRef);
	}
}

void FVoxelNode_MetadataPinsBase::PostSerialize()
{
	Super::PostSerialize();

	FixupMetadataPins();
}

#if WITH_EDITOR
FVoxelNode::EPostEditChange FVoxelNode_MetadataPinsBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_OWN_MEMBER_NAME(MetadatasToQuery))
	{
		FixupMetadataPins();
		return EPostEditChange::Reconstruct;
	}

	return EPostEditChange::None;
}
#endif

void FVoxelNode_MetadataPinsBase::FixupMetadataPins()
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
