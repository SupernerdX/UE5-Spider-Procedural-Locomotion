// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelGraphQuery.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"

namespace FVoxelGraphParameters
{
	struct VOXELGRAPH_API FPosition2D : FBufferParameter
	{
	public:
		FPosition2D() = default;

		static const FPosition2D* Find(const FVoxelGraphQuery& Query);

		int32 Num() const;

		void Split(
			const FVoxelBufferSplitter& Splitter,
			TConstVoxelArrayView<FPosition2D*> OutResult) const;

		void SetLocalPosition(const FVoxelVector2DBuffer& LocalPosition);
		void SetLocalPosition(const FVoxelDoubleVector2DBuffer& LocalPosition);
		void SetWorldPosition(const FVoxelVector2DBuffer& WorldPosition);
		void SetWorldPosition(const FVoxelDoubleVector2DBuffer& WorldPosition);

		FVoxelVector2DBuffer GetLocalPosition_Float(const FVoxelGraphQuery& Query) const;
		FVoxelVector2DBuffer GetWorldPosition_Float(const FVoxelGraphQuery& Query) const;
		FVoxelDoubleVector2DBuffer GetLocalPosition_Double(const FVoxelGraphQuery& Query) const;
		FVoxelDoubleVector2DBuffer GetWorldPosition_Double(const FVoxelGraphQuery& Query) const;

	private:
		mutable TVoxelOptional<FVoxelVector2DBuffer> LocalPosition_Float;
		mutable TVoxelOptional<FVoxelVector2DBuffer> WorldPosition_Float;
		mutable TVoxelOptional<FVoxelDoubleVector2DBuffer> LocalPosition_Double;
		mutable TVoxelOptional<FVoxelDoubleVector2DBuffer> WorldPosition_Double;
	};

	struct VOXELGRAPH_API FPosition3D : FBufferParameter
	{
	public:
		FPosition3D() = default;

		int32 Num() const;

		void Split(
			const FVoxelBufferSplitter& Splitter,
			TConstVoxelArrayView<FPosition3D*> OutResult) const;

		void SetLocalPosition(const FVoxelVectorBuffer& LocalPosition);
		void SetLocalPosition(const FVoxelDoubleVectorBuffer& LocalPosition);
		void SetWorldPosition(const FVoxelVectorBuffer& WorldPosition);
		void SetWorldPosition(const FVoxelDoubleVectorBuffer& WorldPosition);

		const FPosition2D& GetPosition2D() const;

		FVoxelVectorBuffer GetLocalPosition_Float(const FVoxelGraphQuery& Query) const;
		FVoxelVectorBuffer GetWorldPosition_Float(const FVoxelGraphQuery& Query) const;
		FVoxelDoubleVectorBuffer GetLocalPosition_Double(const FVoxelGraphQuery& Query) const;
		FVoxelDoubleVectorBuffer GetWorldPosition_Double(const FVoxelGraphQuery& Query) const;

	private:
		mutable TVoxelOptional<FPosition2D> Position2D;
		mutable TVoxelOptional<FVoxelVectorBuffer> LocalPosition_Float;
		mutable TVoxelOptional<FVoxelVectorBuffer> WorldPosition_Float;
		mutable TVoxelOptional<FVoxelDoubleVectorBuffer> LocalPosition_Double;
		mutable TVoxelOptional<FVoxelDoubleVectorBuffer> WorldPosition_Double;
	};
}