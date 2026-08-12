// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelTemplateNode.h"
#include "VoxelPointSet.h"
#include "Buffer/VoxelClassBuffer.h"
#include "Buffer/VoxelGraphStaticMeshBuffer.h"
#include "Buffer/VoxelRangeBuffers.h"
#include "VoxelPointAttributeNodes.generated.h"

USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_SetPointAttribute : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);
	VOXEL_INPUT_PIN(FName, Name, "MyAttribute");
	VOXEL_INPUT_PIN(FVoxelWildcardBuffer, Value, nullptr);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

public:
	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
#endif
	//~ End FVoxelNode Interface

protected:
	virtual FName AddParentPrefix(const FName Name) const
	{
		return Name;
	}
};

USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_GetPointAttribute : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FName, Name, "MyAttribute");
	VOXEL_OUTPUT_PIN(FVoxelWildcardBuffer, Value);

public:
	//~ Begin FVoxelNode Interface
	virtual bool IsPureNode() const override
	{
		return true;
	}

	virtual void Compute(FVoxelGraphQuery Query) const override;
#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
#endif
	//~ End FVoxelNode Interface

protected:
	virtual FName AddParentPrefix(const FName Name) const
	{
		return Name;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(Category = "Point")
struct VOXELPCG_API FVoxelNode_SetParentPointAttribute : public FVoxelNode_SetPointAttribute
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual FName AddParentPrefix(const FName Name) const override
	{
		return FVoxelPointAttributes::MakeParent(Name);
	}
};

USTRUCT(Category = "Point")
struct VOXELPCG_API FVoxelNode_GetParentPointAttribute : public FVoxelNode_GetPointAttribute
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual FName AddParentPrefix(const FName Name) const override
	{
		return FVoxelPointAttributes::MakeParent(Name);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(Category = "Point", meta = (Abstract, AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelTemplateNode_SetPointAttributeBase : public FVoxelTemplateNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

	virtual bool IsPureNode() const override
	{
		return false;
	}

	virtual FPin* ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const override;
	virtual FName GetAttributeName() const VOXEL_PURE_VIRTUAL({});
};

USTRUCT(Category = "Point", meta = (Abstract, AllowList = "PCG, Scatter", PinTypeAliases = "/Script/VoxelGraph.VoxelPointSet"))
struct VOXELPCG_API FVoxelTemplateNode_GetPointAttributeBase : public FVoxelTemplateNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual FPin* ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const override;
	virtual FName GetAttributeName() const VOXEL_PURE_VIRTUAL({});
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Keywords = "static"))
struct VOXELPCG_API FVoxelTemplateNode_SetPointMesh : public FVoxelTemplateNode_SetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelGraphStaticMeshBuffer, Mesh, nullptr);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Mesh;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetPointMesh : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelGraphStaticMeshBuffer, Mesh);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Mesh;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetParentPointMesh : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelGraphStaticMeshBuffer, Mesh);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::MakeParent(FVoxelPointAttributes::Mesh);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_SetPointPosition : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);
	VOXEL_INPUT_PIN(FVoxelDoubleVectorBuffer, Position, nullptr);
	// Maximum per-axis displacement this node applies relative to the input point's
	// original chunk-local position. Tells the cache layer to query upstream points
	// from a region extended by this much on each side, so that points which displace
	// into this chunk from neighbors are actually generated and reach the cache.
	// Leave at zero if no displacement crosses chunk boundaries, or if this node is
	// downstream of any Cache node.
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, CacheBoundsExtension, FVector::ZeroVector);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetPointPosition : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelDoubleVectorBuffer, Position);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Position;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetParentPointPosition : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelDoubleVectorBuffer, Position);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::MakeParent(FVoxelPointAttributes::Position);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_SetPointRotation : public FVoxelTemplateNode_SetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelQuaternionBuffer, Rotation, nullptr);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Rotation;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetPointRotation : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelQuaternionBuffer, Rotation);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Rotation;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetParentPointRotation : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelQuaternionBuffer, Rotation);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::MakeParent(FVoxelPointAttributes::Rotation);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_SetPointScale : public FVoxelTemplateNode_SetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelVectorBuffer, Scale, FVector::OneVector);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Scale;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetPointScale : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelVectorBuffer, Scale);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Scale;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetParentPointScale : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelVectorBuffer, Scale);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::MakeParent(FVoxelPointAttributes::Scale);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_GetPointSeed : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelSeed, Seed, nullptr);
	VOXEL_OUTPUT_PIN(FVoxelSeedBuffer, Value);

	//~ Begin FVoxelNode Interface
	virtual bool IsPureNode() const override
	{
		return true;
	}

	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_SetPointDensity : public FVoxelTemplateNode_SetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Density, 1.f);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Density;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetPointDensity : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelFloatBuffer, Density);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Density;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_SetPointBoundsMin : public FVoxelTemplateNode_SetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelVectorBuffer, BoundsMin, FVector::ZeroVector);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::BoundsMin;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetPointBoundsMin : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelVectorBuffer, BoundsMin);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::BoundsMin;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_SetPointBoundsMax : public FVoxelTemplateNode_SetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelVectorBuffer, BoundsMax, FVector::ZeroVector);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::BoundsMax;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetPointBoundsMax : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelVectorBuffer, BoundsMax);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::BoundsMax;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_SetPointColor : public FVoxelTemplateNode_SetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelLinearColorBuffer, Color, FLinearColor::White);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Color;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetPointColor : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelLinearColorBuffer, Color);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Color;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_SetPointSteepness : public FVoxelTemplateNode_SetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Steepness, 0.f);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Steepness;
	}
};

USTRUCT()
struct VOXELPCG_API FVoxelTemplateNode_GetPointSteepness : public FVoxelTemplateNode_GetPointAttributeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_OUTPUT_PIN(FVoxelFloatBuffer, Steepness);

	virtual FName GetAttributeName() const override
	{
		return FVoxelPointAttributes::Steepness;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_ApplyTranslation : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);
	VOXEL_INPUT_PIN(FVoxelDoubleVectorBuffer, Translation, nullptr);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};

USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_ApplyRotation : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);
	VOXEL_INPUT_PIN(FVoxelQuaternionBuffer, Rotation, nullptr);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};

USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_ApplyScale : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, Scale, FVector::OneVector);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Randomizes point transforms using the point's seed.
// Applies random position offsets, rotations, and scale variations.
USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_TransformPoints : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);

	// Position Settings
	// Minimum position offset
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, OffsetMin, FVector::ZeroVector, Category("Position"));
	// Maximum position offset
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, OffsetMax, FVector::ZeroVector, Category("Position"));
	// If true, offset is applied in local space (relative to point rotation)
	VOXEL_INPUT_PIN(FVoxelBoolBuffer, bOffsetInLocalSpace, false, Category("Position"));

	// Rotation Settings
	// Roll range in degrees
	VOXEL_INPUT_PIN(FVoxelFloatRangeBuffer, Roll, nullptr, Category("Rotation"));
	// Pitch range in degrees
	VOXEL_INPUT_PIN(FVoxelFloatRangeBuffer, Pitch, nullptr, Category("Rotation"));
	// Yaw range in degrees
	VOXEL_INPUT_PIN(FVoxelFloatRangeBuffer, Yaw, nullptr, Category("Rotation"));
	// If true, replaces existing rotation. If false, combines with existing.
	VOXEL_INPUT_PIN(FVoxelBoolBuffer, bAbsoluteRotation, false, Category("Rotation"));

	// Scale Settings
	// Minimum scale
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, ScaleMin, FVector::OneVector, Category("Scale"));
	// Maximum scale
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, ScaleMax, FVector::OneVector, Category("Scale"));
	// If true, replaces existing scale. If false, multiplies with existing.
	VOXEL_INPUT_PIN(FVoxelBoolBuffer, bAbsoluteScale, false, Category("Scale"));
	// If true, uses uniform scaling (X value for all axes)
	VOXEL_INPUT_PIN(FVoxelBoolBuffer, bUniformScale, true, Category("Scale"));

	// Seed for randomization - combines with point IDs for deterministic results
	VOXEL_INPUT_PIN(FVoxelSeed, Seed, nullptr);

	// Maximum per-axis displacement this node may apply (worst-case bound on
	// |OffsetMin|/|OffsetMax|). Tells the cache layer to query upstream points from a
	// region extended by this much on each side, so points that displace into this chunk
	// from neighbors are actually generated and reach the cache. Leave at zero if no
	// displacement crosses chunk boundaries, or if downstream of any Cache node.
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, CacheBoundsExtension, FVector::ZeroVector);

	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};