// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelBoolBuffer;
struct FVoxelByteBuffer;
struct FVoxelInt32Buffer;

class VOXELGRAPH_API FVoxelBufferSplitter
{
public:
	explicit FVoxelBufferSplitter(const FVoxelBoolBuffer& Indices);

	FVoxelBufferSplitter(
		const FVoxelByteBuffer& Indices,
		int32 NumOutputs);

	FVoxelBufferSplitter(
		const FVoxelInt32Buffer& Indices,
		int32 NumOutputs);

public:
	FORCEINLINE int32 Num() const
	{
		return PrivateNum;
	}
	FORCEINLINE int32 NumOutputs() const
	{
		return PrivateNumOutputs;
	}
	FORCEINLINE int32 GetOutputNum(const int32 Index) const
	{
		checkVoxelSlow(PrivateOutputToNum[Index] > 0);
		return PrivateOutputToNum[Index];
	}
	FORCEINLINE TVoxelOptional<int32> GetUniqueOutput() const
	{
		if (PrivateValidOutputs.Num() != 1)
		{
			return {};
		}

		return PrivateValidOutputs[0];
	}
	FORCEINLINE TConstVoxelArrayView<int32> GetValidOutputs() const
	{
		return PrivateValidOutputs;
	}

	template<typename LambdaType>
	requires LambdaHasSignature_V<LambdaType, void(int32 ReadIndex, int32 WriteIndex, int32 OutputIndex)>
	FORCEINLINE void Iterate(LambdaType Lambda) const
	{
		ensureVoxelSlow(!GetUniqueOutput());

		switch (PrivateType)
		{
		default: VOXEL_ASSUME(false);
		case EType::Bool: return this->Iterate_Bool(Lambda);
		case EType::Byte: return this->Iterate_Byte(Lambda);
		case EType::Int32: return this->Iterate_Int32(Lambda);
		}
	}

private:
	int32 PrivateNum = 0;
	int32 PrivateNumOutputs = 0;
	TVoxelInlineArray<int32, 8> PrivateOutputToNum;
	TVoxelInlineArray<int32, 8> PrivateValidOutputs;

	enum class EType : uint8
	{
		Bool,
		Byte,
		Int32
	};
	EType PrivateType = {};

	TConstVoxelArrayView<bool> Indices_Bool;
	TConstVoxelArrayView<uint8> Indices_Byte;
	TConstVoxelArrayView<int32> Indices_Int32;

	template<typename LambdaType>
	FORCENOINLINE void Iterate_Bool(LambdaType Lambda) const
	{
		VOXEL_FUNCTION_COUNTER_NUM(Num());
		checkVoxelSlow(PrivateType == EType::Bool);

		int32 FalseIndex = 0;
		int32 TrueIndex = 0;
		for (int32 Index = 0; Index < Num(); Index++)
		{
			if (Indices_Bool[Index] == false)
			{
				Lambda(Index, FalseIndex++, 0);
			}
			else
			{
				Lambda(Index, TrueIndex++, 1);
			}
		}
		checkVoxelSlow(FalseIndex == PrivateOutputToNum[0]);
		checkVoxelSlow(TrueIndex == PrivateOutputToNum[1]);
	}
	template<typename LambdaType>
	FORCENOINLINE void Iterate_Byte(LambdaType Lambda) const
	{
		VOXEL_FUNCTION_COUNTER_NUM(Num());
		checkVoxelSlow(PrivateType == EType::Byte);

		TVoxelInlineArray<int32, 16> OutputIndices;
		FVoxelUtilities::SetNumZeroed(OutputIndices, NumOutputs());

		for (int32 Index = 0; Index < Num(); Index++)
		{
			const int32 OutputIndex = Indices_Byte[Index];
			if (OutputIndex >= NumOutputs())
			{
				continue;
			}

			Lambda(Index, OutputIndices[OutputIndex]++, OutputIndex);
		}

		for (int32 OutputIndex = 0; OutputIndex < NumOutputs(); OutputIndex++)
		{
			checkVoxelSlow(OutputIndices[OutputIndex] == PrivateOutputToNum[OutputIndex]);
		}
	}
	template<typename LambdaType>
	FORCENOINLINE void Iterate_Int32(LambdaType Lambda) const
	{
		VOXEL_FUNCTION_COUNTER_NUM(Num());
		checkVoxelSlow(PrivateType == EType::Int32);

		TVoxelInlineArray<int32, 16> OutputIndices;
		FVoxelUtilities::SetNumZeroed(OutputIndices, NumOutputs());

		for (int32 Index = 0; Index < Num(); Index++)
		{
			const int32 OutputIndex = Indices_Int32[Index];
			if (OutputIndex < 0 ||
				OutputIndex >= NumOutputs())
			{
				continue;
			}

			Lambda(Index, OutputIndices[OutputIndex]++, OutputIndex);
		}

		for (int32 OutputIndex = 0; OutputIndex < NumOutputs(); OutputIndex++)
		{
			checkVoxelSlow(OutputIndices[OutputIndex] == PrivateOutputToNum[OutputIndex]);
		}
	}
};