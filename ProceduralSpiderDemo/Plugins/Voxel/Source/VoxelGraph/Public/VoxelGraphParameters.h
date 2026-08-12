// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class FVoxelBufferSplitter;
class FVoxelGraphParameterManager;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace FVoxelGraphParameters
{
	struct VOXELGRAPH_API FUniformParameter
	{
		VOXEL_COUNT_INSTANCES();
	};
	struct VOXELGRAPH_API FBufferParameter
	{
		VOXEL_COUNT_INSTANCES();
	};

	template<typename T>
	constexpr bool IsUniform = std::derived_from<T, FUniformParameter>;

	template<typename T>
	concept HasSplit = requires(
		const T& Parameter,
		const FVoxelBufferSplitter& Splitter,
		TConstVoxelArrayView<T*> OutResult)
	{
		Parameter.Split(Splitter, OutResult);
	};

	template<typename T>
	constexpr bool IsBuffer =
		std::derived_from<T, FBufferParameter> &&
		std::is_constructible_v<T> &&
		HasSplit<T>;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern VOXELGRAPH_API FVoxelGraphParameterManager* GVoxelGraphParameterManager;

class VOXELGRAPH_API FVoxelGraphParameterManager : public FVoxelSingleton
{
public:
	template<typename T>
	requires FVoxelGraphParameters::IsUniform<T>
	FORCEINLINE static int32 GetUniformIndex()
	{
		VOXEL_STATIC_HELPER(int32)
		{
			StaticValue = 1 + GVoxelGraphParameterManager->GetUniformIndexImpl(FVoxelUtilities::GetCppFName<T>());
		}

		return StaticValue - 1;
	}
	template<typename T>
	requires FVoxelGraphParameters::IsBuffer<T>
	FORCEINLINE static int32 GetBufferIndex()
	{
		VOXEL_STATIC_HELPER(int32)
		{
			StaticValue = 1 + GVoxelGraphParameterManager->GetBufferIndexImpl<T>();
		}

		return StaticValue - 1;
	}

public:
	FORCEINLINE static int32 NumUniform()
	{
		return GVoxelGraphParameterManager->Uniforms_RequiresLock.Num();
	}
	FORCEINLINE static int32 NumBuffer()
	{
		return GVoxelGraphParameterManager->Buffers_RequiresLock.Num();
	}

public:
	using FConstructor = void (*)(void*);
	using FDestructor = void (*)(void*);

	using FSplit = void (*)(
		const FVoxelGraphParameters::FBufferParameter& Parameter,
		const FVoxelBufferSplitter& Splitter,
		TConstVoxelArrayView<FVoxelGraphParameters::FBufferParameter*> OutResult);

	struct FBufferInfo
	{
		int32 TypeSize = 0;
		FConstructor Constructor;
		FDestructor Destructor;
		FSplit Split;
	};
	FBufferInfo GetBufferInfo(int32 Index) const;

private:
	template<typename T>
	requires FVoxelGraphParameters::IsBuffer<T>
	int32 GetBufferIndexImpl()
	{
		return this->GetBufferIndexImpl(
			FVoxelUtilities::GetCppFName<T>(),
			FBufferInfo
			{
				sizeof(T),
				[](void* Data)
				{
					new (Data) T();
				},
				[](void* Data)
				{
					static_cast<T*>(Data)->~T();
				},
				[](
					const FVoxelGraphParameters::FBufferParameter& Parameter,
					const FVoxelBufferSplitter& Splitter,
					const TConstVoxelArrayView<FVoxelGraphParameters::FBufferParameter*> OutResult)
				{
					static_cast<const T&>(Parameter).Split(Splitter, OutResult.ReinterpretAs<T*>());
				}
			});
	}

private:
	FVoxelCriticalSection CriticalSection;
	TVoxelSet<FName> Uniforms_RequiresLock;
	TVoxelSet<FName> Buffers_RequiresLock;
	TVoxelArray<FBufferInfo> BufferInfos_RequiresLock;

	int32 GetUniformIndexImpl(FName Name);

	int32 GetBufferIndexImpl(
		FName Name,
		FBufferInfo Info);
};