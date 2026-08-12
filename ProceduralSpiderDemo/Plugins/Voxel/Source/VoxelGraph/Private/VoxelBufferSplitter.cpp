// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelBufferSplitter.h"
#include "Buffer/VoxelBaseBuffers.h"

FVoxelBufferSplitter::FVoxelBufferSplitter(const FVoxelBoolBuffer& Indices)
{
	VOXEL_FUNCTION_COUNTER_NUM(Indices.Num());

	PrivateNum = Indices.Num();
	PrivateNumOutputs = 2;
	PrivateType = EType::Bool;
	Indices_Bool = Indices.View();

	int32 NumTrue = 0;
	for (const bool bValue : Indices_Bool)
	{
		if (bValue)
		{
			NumTrue++;
		}
	}

	PrivateOutputToNum.SetNumZeroed(PrivateNumOutputs);
	PrivateOutputToNum[0] = Indices.Num() - NumTrue;
	PrivateOutputToNum[1] = NumTrue;

	for (int32 Index = 0; Index < NumOutputs(); Index++)
	{
		if (PrivateOutputToNum[Index] > 0)
		{
			PrivateValidOutputs.Add(Index);
		}
	}
}

FVoxelBufferSplitter::FVoxelBufferSplitter(
	const FVoxelByteBuffer& Indices,
	const int32 InNumOutputs)
{
	VOXEL_FUNCTION_COUNTER_NUM(Indices.Num());

	PrivateNum = Indices.Num();
	PrivateNumOutputs = InNumOutputs;
	PrivateType = EType::Byte;
	PrivateOutputToNum.SetNumZeroed(NumOutputs());

	Indices_Byte = Indices.View();

	for (const uint8 Byte : Indices_Byte)
	{
		if (Byte >= NumOutputs())
		{
			continue;
		}

		PrivateOutputToNum[Byte]++;
	}

	for (int32 Index = 0; Index < NumOutputs(); Index++)
	{
		if (PrivateOutputToNum[Index] > 0)
		{
			PrivateValidOutputs.Add(Index);
		}
	}
}

FVoxelBufferSplitter::FVoxelBufferSplitter(
	const FVoxelInt32Buffer& Indices,
	const int32 InNumOutputs)
{
	VOXEL_FUNCTION_COUNTER_NUM(Indices.Num());

	PrivateNum = Indices.Num();
	PrivateNumOutputs = InNumOutputs;
	PrivateType = EType::Int32;
	PrivateOutputToNum.SetNumZeroed(NumOutputs());

	Indices_Int32 = Indices.View();

	for (const int32 Index : Indices_Int32)
	{
		if (Index < 0 ||
			Index >= NumOutputs())
		{
			continue;
		}

		PrivateOutputToNum[Index]++;
	}

	for (int32 Index = 0; Index < NumOutputs(); Index++)
	{
		if (PrivateOutputToNum[Index] > 0)
		{
			PrivateValidOutputs.Add(Index);
		}
	}
}