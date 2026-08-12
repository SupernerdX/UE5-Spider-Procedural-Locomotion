// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Preview/VoxelScalarPreviewHandler.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelFloatPreviewHandlers.generated.h"

USTRUCT(DisplayName = "Grayscale")
struct VOXELGRAPH_API FVoxelPreviewHandler_Grayscale_Float : public FVoxelGrayscalePreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<float>() ||
			Type.Is<FVoxelFloatBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

public:
	static FString ValueToString(bool bFullValue, const FString& Prefix, float Value);

private:
	TSharedRef<const FVoxelFloatBuffer> Buffer = FVoxelFloatBuffer::MakeSharedDefault();
	float MinValue = 0.f;
	float MaxValue = 0.f;
};

USTRUCT(DisplayName = "Vector2D")
struct VOXELGRAPH_API FVoxelPreviewHandler_Vector2D : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<FVector2D>() ||
			Type.Is<FVoxelVector2DBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelVector2DBuffer> Buffer = FVoxelVector2DBuffer::MakeSharedDefault();
	FVector2f MinValue = FVector2f(ForceInit);
	FVector2f MaxValue = FVector2f(ForceInit);
};

USTRUCT(DisplayName = "Vector")
struct VOXELGRAPH_API FVoxelPreviewHandler_Vector : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<FVector>() ||
			Type.Is<FVoxelVectorBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelVectorBuffer> Buffer = FVoxelVectorBuffer::MakeSharedDefault();
	FVector3f MinValue = FVector3f(ForceInit);
	FVector3f MaxValue = FVector3f(ForceInit);
};

USTRUCT(DisplayName = "Color")
struct VOXELGRAPH_API FVoxelPreviewHandler_Color : public FVoxelScalarPreviewHandler
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	//~ Begin FVoxelPreviewHandler Interface
	virtual bool SupportsType(const FVoxelPinType& Type) const override
	{
		return
			Type.Is<FLinearColor>() ||
			Type.Is<FVoxelLinearColorBuffer>();
	}
	virtual void Initialize(const FVoxelRuntimePinValue& Value) override;

	virtual void GetColors(TVoxelArrayView<FLinearColor> Colors) const override;
	virtual TArray<FString> GetValueAt(int32 Index, bool bFullValue) const override;
	virtual TArray<FString> GetMinValue(bool bFullValue) const override;
	virtual TArray<FString> GetMaxValue(bool bFullValue) const override;
	//~ End FVoxelPreviewHandler Interface

private:
	TSharedRef<const FVoxelLinearColorBuffer> Buffer = FVoxelLinearColorBuffer::MakeSharedDefault();
	FLinearColor MinValue = FLinearColor(ForceInit);
	FLinearColor MaxValue = FLinearColor(ForceInit);
};