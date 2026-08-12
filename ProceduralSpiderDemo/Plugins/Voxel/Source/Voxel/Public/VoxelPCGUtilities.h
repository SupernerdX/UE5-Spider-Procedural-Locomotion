// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Metadata/PCGMetadata.h"

struct VOXEL_API FVoxelPCGUtilities
{
public:
	template<typename Allocator>
	static void AddPointsToMetadata(
		UPCGMetadata& Metadata,
		TVoxelArray<FPCGPoint, Allocator>& Points)
	{
		VOXEL_FUNCTION_COUNTER();

		TArray<TTuple<int64, int64>> Entries;
		FVoxelUtilities::SetNumFast(Entries, Points.Num());

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			FPCGPoint& Point = Points[Index];
			uint64 OldEntry = Point.MetadataEntry;
			Point.MetadataEntry = Metadata.AddEntryPlaceholder();
			Entries[Index] = { Point.MetadataEntry, OldEntry };
		}

		Metadata.AddDelayedEntries(Entries);
	}

public:
	template<typename T>
	static void AddAttribute(
		UPCGMetadata& Metadata,
		const TConstVoxelArrayView<FPCGPoint> Points,
		const FName Name,
		const TConstVoxelArrayView<T> Values,
		const T DefaultValue = FVoxelUtilities::MakeSafe<T>(),
		const bool bAllowInterpolation = true)
	{
		VOXEL_FUNCTION_COUNTER();

		if (!ensure(Metadata.GetLocalItemCount() == Values.Num()) ||
			!ensure(Points.Num() == Values.Num()))
		{
			return;
		}

		TVoxelArray<PCGMetadataEntryKey> EntryKeys;
		FVoxelUtilities::SetNumFast(EntryKeys, Points.Num());

		for (int64 Index = 0; Index < Points.Num(); Index++)
		{
			EntryKeys[Index] = Points[Index].MetadataEntry;
		}

		FPCGMetadataAttribute<T>* Attribute = Metadata.FindOrCreateAttribute<T>(Name, DefaultValue, bAllowInterpolation, true);
		if (!ensure(Attribute))
		{
			return;
		}

		const TArray<PCGMetadataValueKey> ValueKeys = Attribute->AddValues(Values);
		Attribute->SetValuesFromValueKeys(EntryKeys, ValueKeys);
	}

	template<typename T>
	static void AddDefaultAttributeIfNeeded(
		UPCGMetadata& Metadata,
		const FName Name,
		const T DefaultValue = FVoxelUtilities::MakeSafe<T>(),
		const bool bAllowInterpolation = true)
	{
		VOXEL_FUNCTION_COUNTER();

		if (Metadata.HasAttribute(Name))
		{
			return;
		}

		Metadata.FindOrCreateAttribute<T>(Name, DefaultValue, bAllowInterpolation, true);
	}

public:
	template<typename LambdaType>
	static void SwitchOnAttribute(
		const EPCGMetadataTypes AttributeType,
		const FPCGMetadataAttributeBase& Attribute,
		LambdaType&& Lambda)
	{
		switch (AttributeType)
		{
		default:
		{
			ensure(false);
			return;
		}
		case EPCGMetadataTypes::Float:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<float>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::Double:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<double>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::Integer32:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<int32>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::Integer64:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<int64>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::Vector2:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<FVector2D>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::Vector:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<FVector>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::Vector4:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<FVector4>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::Quaternion:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<FQuat>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::Transform:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<FTransform>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::String:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<FString>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::Boolean:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<bool>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::Rotator:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<FRotator>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::Name:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<FName>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::SoftObjectPath:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<FSoftObjectPath>&>(Attribute));
			return;
		}
		case EPCGMetadataTypes::SoftClassPath:
		{
			Lambda(static_cast<const FPCGMetadataAttribute<FSoftClassPath>&>(Attribute));
			return;
		}
		}
	}
};