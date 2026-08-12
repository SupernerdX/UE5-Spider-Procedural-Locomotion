// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Preview/VoxelScalarPreviewHandler.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "VoxelDoublePreviewHandlers.generated.h"

USTRUCT(DisplayName = "Grayscale")
struct VOXELGRAPH_API FVoxelPreviewHandler_Grayscale_Double : public FVoxelGrayscalePreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<double>() ||
			Type.Is<FVoxelDoubleBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

public:
	static FString ValueToString(bool bFullValue, const FString& Prefix, double Value);

private:
	TSharedRef<const FVoxelDoubleBuffer> Buffer = FVoxelDoubleBuffer::MakeSharedDefault();
	double MinValue = 0.f;
	double MaxValue = 0.f;
};

USTRUCT(DisplayName = "Double Vector2D")
struct VOXELGRAPH_API FVoxelPreviewHandler_DoubleVector2D : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<FVoxelDoubleVector2D>() ||
			Type.Is<FVoxelDoubleVector2DBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelDoubleVector2DBuffer> Buffer = FVoxelDoubleVector2DBuffer::MakeSharedDefault();
	FVoxelDoubleVector2D MinValue = FVoxelDoubleVector2D();
	FVoxelDoubleVector2D MaxValue = FVoxelDoubleVector2D();
};

USTRUCT(DisplayName = "Double Vector")
struct VOXELGRAPH_API FVoxelPreviewHandler_DoubleVector : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<FVoxelDoubleVector>() ||
			Type.Is<FVoxelDoubleVectorBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelDoubleVectorBuffer> Buffer = FVoxelDoubleVectorBuffer::MakeSharedDefault();
	FVoxelDoubleVector MinValue = FVoxelDoubleVector();
	FVoxelDoubleVector MaxValue = FVoxelDoubleVector();
};

USTRUCT(DisplayName = "Double Color")
struct VOXELGRAPH_API FVoxelPreviewHandler_DoubleColor : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<FVoxelDoubleLinearColor>() ||
			Type.Is<FVoxelDoubleLinearColorBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelDoubleLinearColorBuffer> Buffer = FVoxelDoubleLinearColorBuffer::MakeSharedDefault();
	FVoxelDoubleLinearColor MinValue = FVoxelDoubleLinearColor();
	FVoxelDoubleLinearColor MaxValue = FVoxelDoubleLinearColor();
};