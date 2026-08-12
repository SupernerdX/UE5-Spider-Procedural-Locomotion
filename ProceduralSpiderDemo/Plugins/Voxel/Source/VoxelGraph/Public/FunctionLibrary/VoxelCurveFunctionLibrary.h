// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelPinValueOps.h"
#include "Curves/CurveFloat.h"
#include "VoxelObjectPinType.h"
#include "VoxelFunctionLibrary.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelCurveFunctionLibrary.generated.h"

struct FVoxelCurveData;

struct VOXELGRAPH_API FVoxelRuntimeCurve
{
	TSharedPtr<FVoxelDependency> Dependency;
	FVoxelCriticalSection CriticalSection;
	TSharedPtr<const FVoxelCurveData> Data_RequiresLock;

	void Update(
		const FRichCurve& Curve,
		bool bInvalidateDependency);
};

USTRUCT(DisplayName = "Curve")
struct VOXELGRAPH_API FVoxelRuntimeCurveRef
{
	GENERATED_BODY()

	TSharedPtr<FVoxelRuntimeCurve> RuntimeCurve;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXELGRAPH_API FVoxelCurve
{
	GENERATED_BODY()

	FVoxelCurve()
	{
		Curve.AddKey(0.f, 0.f);
	}

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FRichCurve Curve;

	// Property->Identical will check transient FIndexedCurve::KeyHandlesToIndices which will not match
	bool operator==(const FVoxelCurve& Other) const
	{
		return Curve == Other.Curve;
	}
};

template<>
struct TStructOpsTypeTraits<FVoxelCurve> : public TStructOpsTypeTraitsBase2<FVoxelCurve>
{
	enum
	{
		WithIdenticalViaEquality = true,
	};
};

USTRUCT()
struct VOXELGRAPH_API FVoxelCurveRef
{
	GENERATED_BODY()

	TVoxelObjectPtr<UCurveFloat> Object;
	FVoxelRuntimeCurveRef RuntimeCurveRef;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXELGRAPH_API FVoxelPinValueOps_FVoxelRuntimeCurveRef : public FVoxelPinValueOps
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	//~ Begin FVoxelPinValueOps Interface
	virtual EVoxelPinValueOpsUsage GetUsage() const override;
	virtual FVoxelPinType GetExposedType() const override;

	virtual FVoxelRuntimePinValue MakeRuntimeValue(
		const FVoxelPinValue& Value,
		const FVoxelPinType::FRuntimeValueContext& Context) const override;
	//~ End FVoxelPinValueOps Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_OBJECT_PIN_TYPE(FVoxelCurveRef);

USTRUCT()
struct VOXELGRAPH_API FVoxelCurveRefPinType : public FVoxelObjectPinType
{
	GENERATED_BODY()

	DEFINE_VOXEL_OBJECT_PIN_TYPE(FVoxelCurveRef, UCurveFloat);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class VOXELGRAPH_API UVoxelCurveFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Math|Curve")
	FVoxelRuntimeCurveRef MakeCurveFromAsset(const FVoxelCurveRef& Asset) const;

	UFUNCTION(Category = "Math|Curve")
	FVoxelFloatBuffer SampleCurve(
		const FVoxelFloatBuffer& Value,
		const FVoxelRuntimeCurveRef& Curve,
		UPARAM(meta = (AdvancedDisplay)) bool bFastCurve = true) const;
};