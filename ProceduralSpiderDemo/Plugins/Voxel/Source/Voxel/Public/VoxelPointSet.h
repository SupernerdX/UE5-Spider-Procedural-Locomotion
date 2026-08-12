// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBuffer.h"
#include "VoxelGraphQuery.h"
#include "VoxelPointSet.generated.h"

struct VOXEL_API FVoxelPointAttributes
{
	static const FName Id;
	static const FName Mesh;
	static const FName Position;
	static const FName Rotation;
	static const FName Scale;
	static const FName Density;
	static const FName BoundsMin;
	static const FName BoundsMax;
	static const FName Color;
	static const FName Steepness;
	static const FName SurfaceTypes;
	static const FName IsNeighbor;

	static FName MakeParent(const FName Name)
	{
		return TEXTVIEW("Parent.") + Name;
	}
};

USTRUCT()
struct VOXEL_API FVoxelPointSet
	: public FVoxelVirtualStruct
#if CPP
	, public TSharedFromThis<FVoxelPointSet>
#endif
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	FORCEINLINE int32 Num() const
	{
		return PrivateNum;
	}
	FORCEINLINE const TVoxelMap<FName, TSharedPtr<const FVoxelBuffer>>& GetAttributes() const
	{
		return NameToAttribute;
	}

public:
	const FVoxelBuffer* Find(const FName Name) const
	{
		return NameToAttribute.FindSmartPtr(Name);
	}
	TSharedPtr<const FVoxelBuffer> FindShared(const FName Name) const
	{
		return NameToAttribute.FindRef(Name);
	}

	template<typename T>
	const T* Find(const FName Name) const
	{
		const FVoxelBuffer* Buffer = NameToAttribute.FindSmartPtr(Name);
		if (!Buffer ||
			!ensureVoxelSlow(Buffer->IsA<T>()))
		{
			return nullptr;
		}

		return &Buffer->AsChecked<T>();
	}

public:
	void SetNum(int32 NewNum);
	void Add(FName Name, const TSharedRef<const FVoxelBuffer>& Buffer);
	FVoxelGraphQuery MakeQuery(FVoxelGraphQuery Query) const;
	bool CheckNum(const FVoxelNode* Node, int32 BufferNum) const;
	TSharedRef<FVoxelPointSet> Gather(TConstVoxelArrayView<int32> Indices) const;
	TVoxelArray<TSharedPtr<FVoxelPointSet>> Split(const FVoxelBufferSplitter& Splitter) const;
	void KeepAttributes(const TVoxelSet<FName>& AttributesToKeep);
	bool Equals(const FVoxelPointSet& Other) const;
	int64 GetAllocatedSize() const;

public:
	static TSharedRef<const FVoxelPointSet> Merge(TVoxelArray<TSharedRef<const FVoxelPointSet>> PointSets);

public:
	template<typename T>
	requires std::derived_from<T, FVoxelBuffer>
	void Add(const FName Name, T&& Buffer)
	{
		this->Add(Name, MakeSharedCopy(MoveTemp(Buffer)));
	}

private:
	int32 PrivateNum = 0;
	TVoxelMap<FName, TSharedPtr<const FVoxelBuffer>> NameToAttribute;
};

namespace FVoxelGraphParameters
{
	struct VOXEL_API FPointSet : FBufferParameter
	{
		TSharedPtr<const FVoxelPointSet> Value;

		void Split(
			const FVoxelBufferSplitter& Splitter,
			TConstVoxelArrayView<FPointSet*> OutResult) const;
	};
}