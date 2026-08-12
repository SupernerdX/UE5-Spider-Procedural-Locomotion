// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelGraphStaticMeshBuffer.h"

struct FVoxelPointSet;
struct FVoxelGraphMergedNodeRef;
struct FVoxelPointIdBuffer;

struct VOXEL_API FVoxelPointUtilities
{
public:
	static TSharedPtr<const FVoxelBuffer> FindExtractedAttribute(
		const FVoxelPointSet& PointSet,
		FName AttributeName,
		FName& OutName,
		TVoxelArray<FString>& OutExtractors);

	static TSharedPtr<const FVoxelBuffer> ParseAttribute(
		const FVoxelPointSet& PointSet,
		const FVoxelPinType& Type,
		FName AttributeName,
		const FVoxelGraphMergedNodeRef& NodeRef);

public:
	static void SetExtractedAttribute(
		FVoxelBuffer& Attribute,
		const TSharedRef<const FVoxelBuffer>& NewValue,
		const TArray<FString>& Extractors,
		int32 ExtractorIndex,
		FName AttributeName,
		const FVoxelGraphMergedNodeRef& NodeRef);

public:
	static TSharedPtr<const FVoxelBuffer> ExtractAttribute(
		const FVoxelPinType& Type,
		const TSharedRef<const FVoxelBuffer>& Buffer,
		TConstVoxelArrayView<FString> Extractors,
		FName AttributeName,
		const FVoxelGraphMergedNodeRef& NodeRef);

	static TSharedPtr<const FVoxelBuffer> ExtractVectorComponent(
		const FVoxelBuffer& Buffer,
		const FString& Extractor,
		const FVoxelPinType& ReturnPinType,
		FName AttributeName,
		const FVoxelGraphMergedNodeRef& NodeRef);

	static TSharedPtr<const FVoxelBuffer> ExtractVector(
		const TSharedRef<const FVoxelBuffer>& Buffer,
		const FString& Extractor,
		const FVoxelPinType& ReturnPinType,
		FName AttributeName,
		const FVoxelGraphMergedNodeRef& NodeRef);

	static TSharedPtr<const FVoxelBuffer> ExtractQuat(
		const TSharedRef<const FVoxelQuaternionBuffer>& Buffer,
		const FString& Extractor,
		const FVoxelPinType& ReturnPinType,
		FName AttributeName,
		const FVoxelGraphMergedNodeRef& NodeRef);

	static TSharedPtr<const FVoxelBuffer> ExtractTransform(
		const TSharedRef<const FVoxelTransformBuffer>& Buffer,
		const FString& Extractor,
		const FVoxelPinType& ReturnPinType,
		FName AttributeName,
		const FVoxelGraphMergedNodeRef& NodeRef);

public:
	static FVoxelSeedBuffer PointIdToSeed(const FVoxelPointIdBuffer& Buffer, const FVoxelSeed& Seed);
};