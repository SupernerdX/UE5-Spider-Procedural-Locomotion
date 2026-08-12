// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBuffer.h"

struct VOXELGRAPH_API FVoxelBufferAccessor
{
	FVoxelBufferAccessor() = default;

	template<typename... ArgTypes>
	FORCEINLINE explicit FVoxelBufferAccessor(const ArgTypes&... Args)
	{
		checkStatic(sizeof...(Args) > 1);

		PrivateNum = 1;
		VOXEL_FOLD_EXPRESSION(this->Add(FVoxelBufferAccessor::GetNum(Args)));
	}

	template<typename T>
	FORCEINLINE static bool MergeNum(int32& Num, const T& Buffer)
	{
		const int32 BufferNum = FVoxelBufferAccessor::GetNum(Buffer);

		if (Num == 1)
		{
			Num = BufferNum;
			return true;
		}

		if (BufferNum == Num ||
			BufferNum == 1)
		{
			return true;
		}

		return false;
	}

	FORCEINLINE bool IsValid() const
	{
		return PrivateNum != -1;
	}
	FORCEINLINE int32 Num() const
	{
		return PrivateNum;
	}

private:
	int32 PrivateNum = -1;

	template<typename T>
	FORCEINLINE static int32 GetNum(const T& Buffer)
	{
		if constexpr (TIsTSharedRef_V<T>)
		{
			return FVoxelBufferAccessor::GetNum(*Buffer);
		}
		else if constexpr (std::is_same_v<T, int32>)
		{
			return Buffer;
		}
		else if constexpr (std::is_same_v<T, FVoxelBuffer>)
		{
			return Buffer.Num_Slow();
		}
		else
		{
			return Buffer.Num();
		}
	}
	FORCEINLINE void Add(const int32 Num)
	{
		if (PrivateNum == 1)
		{
			PrivateNum = Num;
			return;
		}

		if (Num == 1 ||
			Num == PrivateNum)
		{
			return;
		}

		PrivateNum = -1;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define CheckVoxelBuffersNum(...) \
	if (!FVoxelBufferAccessor(__VA_ARGS__).IsValid()) \
	{ \
		RaiseBufferError(); \
		return; \
	}

#define ComputeVoxelBuffersNum(...) FVoxelBufferAccessor(__VA_ARGS__).Num(); CheckVoxelBuffersNum(__VA_ARGS__)

#define CheckVoxelBuffersNum_Return(...) \
	if (!FVoxelBufferAccessor(__VA_ARGS__).IsValid()) \
	{ \
		RaiseBufferError(); \
		return FVoxelBuffer::DefaultBuffer; \
	}

#define ComputeVoxelBuffersNum_Return(...) FVoxelBufferAccessor(__VA_ARGS__).Num(); CheckVoxelBuffersNum_Return(__VA_ARGS__)