// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Preview/VoxelScalarPreviewHandler.h"
#include "Buffer/VoxelIntegerBuffers.h"
#include "VoxelIntegerPreviewHandlers.generated.h"

USTRUCT(DisplayName = "Byte")
struct VOXELGRAPH_API FVoxelPreviewHandler_Byte : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		if (!Type.Is<uint8>() &&
			!Type.Is<FVoxelByteBuffer>())
		{
			return false;
		}

		return Type.GetInnerType().GetEnum() == nullptr;
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelByteBuffer> Buffer = FVoxelByteBuffer::MakeSharedDefault();
	uint8 MinValue = 0.f;
	uint8 MaxValue = 0.f;
};

USTRUCT(DisplayName = "Integer")
struct VOXELGRAPH_API FVoxelPreviewHandler_Integer : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<int32>() ||
			Type.Is<int64>() ||
			Type.Is<FVoxelInt32Buffer>() ||
			Type.Is<FVoxelInt64Buffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

public:
	static FString ValueToString(const FString& Prefix, int64 Value);

private:
	TSharedRef<const FVoxelInt64Buffer> Buffer = FVoxelInt64Buffer::MakeSharedDefault();
	int64 MinValue = 0;
	int64 MaxValue = 0;
};

USTRUCT(DisplayName = "Int Point")
struct VOXELGRAPH_API FVoxelPreviewHandler_IntPoint : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<FIntPoint>() ||
			Type.Is<FInt64Point>() ||
			Type.Is<FVoxelIntPointBuffer>() ||
			Type.Is<FVoxelInt64PointBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelInt64PointBuffer> Buffer = FVoxelInt64PointBuffer::MakeSharedDefault();
	FInt64Point MinValue = FInt64Point(ForceInit);
	FInt64Point MaxValue = FInt64Point(ForceInit);
};

USTRUCT(DisplayName = "Int Vector")
struct VOXELGRAPH_API FVoxelPreviewHandler_IntVector : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<FIntVector>() ||
			Type.Is<FInt64Vector>() ||
			Type.Is<FVoxelIntVectorBuffer>() ||
			Type.Is<FVoxelInt64VectorBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelInt64VectorBuffer> Buffer = FVoxelInt64VectorBuffer::MakeSharedDefault();
	FInt64Vector MinValue = FInt64Vector(ForceInit);
	FInt64Vector MaxValue = FInt64Vector(ForceInit);
};

USTRUCT(DisplayName = "Int Vector 4")
struct VOXELGRAPH_API FVoxelPreviewHandler_IntVector4 : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<FIntVector4>() ||
			Type.Is<FInt64Vector4>() ||
			Type.Is<FVoxelIntVector4Buffer>() ||
			Type.Is<FVoxelInt64Vector4Buffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelInt64Vector4Buffer> Buffer = FVoxelInt64Vector4Buffer::MakeSharedDefault();
	FInt64Vector4 MinValue = FInt64Vector4(ForceInit);
	FInt64Vector4 MaxValue = FInt64Vector4(ForceInit);
};