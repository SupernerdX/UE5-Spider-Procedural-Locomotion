// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinType.h"
#include "VoxelPinValueOps.h"
#include "VoxelExternalParameter.h"
#include "VoxelSplineParameters.generated.h"

USTRUCT()
struct VOXEL_API FVoxelSplineParameter : public FVoxelExternalParameter
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	virtual FVoxelPinType GetType() const VOXEL_PURE_VIRTUAL({});
	virtual FVoxelPinValue GetDefaultValue() const VOXEL_PURE_VIRTUAL({});
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(DisplayName = "Float Spline Parameter")
struct VOXEL_API FVoxelFloatSplineParameter : public FVoxelSplineParameter
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	float Default = 0.f;

	//~ Begin FVoxelSplineParameter Interface
	virtual FVoxelPinType GetType() const override
	{
		return FVoxelPinType::Make<float>();
	}
	virtual FVoxelPinValue GetDefaultValue() const override
	{
		return FVoxelPinValue::Make(Default);
	}
	//~ End FVoxelSplineParameter Interface
};

USTRUCT()
struct FVoxelPinValueOps_FVoxelFloatSplineParameter : public FVoxelPinValueOps
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	//~ Begin FVoxelPinValueOps Interface
	virtual EVoxelPinValueOpsUsage GetUsage() const override
	{
		return EVoxelPinValueOpsUsage::IsNumericType;
	}
	virtual bool IsNumericType() const override
	{
		return true;
	}
	//~ End FVoxelPinValueOps Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(DisplayName = "Vector 2D Spline Parameter")
struct VOXEL_API FVoxelVector2DSplineParameter : public FVoxelSplineParameter
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	FVector2D Default = FVector2D(ForceInit);

	//~ Begin FVoxelSplineParameter Interface
	virtual FVoxelPinType GetType() const override
	{
		return FVoxelPinType::Make<FVector2D>();
	}
	virtual FVoxelPinValue GetDefaultValue() const override
	{
		return FVoxelPinValue::Make(Default);
	}
	//~ End FVoxelSplineParameter Interface
};

USTRUCT()
struct FVoxelPinValueOps_FVoxelVector2DSplineParameter : public FVoxelPinValueOps
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	//~ Begin FVoxelPinValueOps Interface
	virtual EVoxelPinValueOpsUsage GetUsage() const override
	{
		return EVoxelPinValueOpsUsage::IsNumericType;
	}
	virtual bool IsNumericType() const override
	{
		return true;
	}
	//~ End FVoxelPinValueOps Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(DisplayName = "Vector Spline Parameter")
struct VOXEL_API FVoxelVectorSplineParameter : public FVoxelSplineParameter
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	FVector Default = FVector(ForceInit);

	//~ Begin FVoxelSplineParameter Interface
	virtual FVoxelPinType GetType() const override
	{
		return FVoxelPinType::Make<FVector>();
	}
	virtual FVoxelPinValue GetDefaultValue() const override
	{
		return FVoxelPinValue::Make(Default);
	}
	//~ End FVoxelSplineParameter Interface
};

USTRUCT()
struct FVoxelPinValueOps_FVoxelVectorSplineParameter : public FVoxelPinValueOps
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	//~ Begin FVoxelPinValueOps Interface
	virtual EVoxelPinValueOpsUsage GetUsage() const override
	{
		return EVoxelPinValueOpsUsage::IsNumericType;
	}
	virtual bool IsNumericType() const override
	{
		return true;
	}
	//~ End FVoxelPinValueOps Interface
};