// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPointAttributeNodes.h"
#include "VoxelPointId.h"
#include "VoxelBufferAccessor.h"
#include "VoxelCompilationGraph.h"
#include "Nodes/VoxelVectorNodes.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelIntegerBuffers.h"
#include "VoxelGraphPositionParameter.h"
#include "Scatter/VoxelPointUtilities.h"
#include "Scatter/VoxelScatterFunctionLibrary.h"
#include "Utilities/VoxelBufferMathUtilities.h"
#include "Metadata/Accessors/PCGAttributeExtractor.h"

void FVoxelNode_SetPointAttribute::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelPointSet> Points = InPin.Get(Query);

	VOXEL_GRAPH_WAIT(Points)
	{
		const TValue<FName> BaseName = NamePin.Get(Query);
		const TValue<FVoxelBuffer> Value = ValuePin.Get<FVoxelBuffer>(Points->MakeQuery(Query));

		VOXEL_GRAPH_WAIT(Points, BaseName, Value)
		{
			if (!Points->CheckNum(this, Value->Num_Slow()))
			{
				OutPin.Set(Query, Points);
				return;
			}

			const FName Name = AddParentPrefix(BaseName);

			FName AttributeName;
			TVoxelArray<FString> Extractors;
			const TSharedPtr<const FVoxelBuffer> ExistingAttribute = FVoxelPointUtilities::FindExtractedAttribute(
				*Points,
				Name,
				AttributeName,
				Extractors);

			if (Extractors.Num() == 0)
			{
				const TSharedRef<FVoxelPointSet> NewPoints = Points->MakeSharedCopy();
				NewPoints->Add(Name, Value);
				OutPin.Set(Query, NewPoints);
				return;
			}

			if (!ExistingAttribute)
			{
				VOXEL_MESSAGE(Error, "{0}: Attribute {1} does not exist",
					this,
					AttributeName);

				OutPin.Set(Query, Points);
				return;
			}

			if (!ensure(ExistingAttribute->IsA<FVoxelBufferStruct>()))
			{
				VOXEL_MESSAGE(Error, "{0}: Invalid attribute {1}, should be a ",
					this,
					AttributeName);

				OutPin.Set(Query, Points);
				return;
			}

			const TSharedRef<FVoxelPointSet> NewPoints = Points->MakeSharedCopy();
			const TSharedRef<FVoxelBufferStruct> NewAttribute = MakeSharedCopy(ExistingAttribute->AsChecked<FVoxelBufferStruct>());

			FVoxelPointUtilities::SetExtractedAttribute(
				*NewAttribute,
				Value,
				Extractors,
				0,
				AttributeName,
				GetNodeRef());

			NewPoints->Add(AttributeName, NewAttribute);

			OutPin.Set(Query, NewPoints);
		};
	};
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_SetPointAttribute::GetPromotionTypes(const FVoxelPin& Pin) const
{
	return FVoxelPinTypeSet::AllBuffers();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_GetPointAttribute::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FName> BaseName = NamePin.Get(Query);

	VOXEL_GRAPH_WAIT(BaseName)
	{
		if (Query.IsPreview() &&
			BaseName == FVoxelPointAttributes::Position)
		{
			const FVoxelGraphParameters::FPosition3D* Parameter = Query->FindParameter<FVoxelGraphParameters::FPosition3D>();
			if (!Parameter)
			{
				VOXEL_MESSAGE(Error, "{0}: No valid position", this);
				return;
			}

			ValuePin.Set(Query, FVoxelRuntimePinValue::Make(Parameter->GetLocalPosition_Double(Query)));
			return;
		}

		const FVoxelGraphParameters::FPointSet* Parameter = Query->FindParameter<FVoxelGraphParameters::FPointSet>();
		if (!Parameter)
		{
			VOXEL_MESSAGE(Error, "{0}: No points passed in input", this);
			return;
		}

		const TSharedRef<const FVoxelPointSet> PointSet = Parameter->Value.ToSharedRef();
		if (PointSet->Num() == 0)
		{
			return;
		}

		const FName Name = AddParentPrefix(BaseName);
		const FVoxelPinType ReturnType = ValuePin.GetType_RuntimeOnly();

		const TSharedPtr<const FVoxelBuffer> Attribute =  FVoxelPointUtilities::ParseAttribute(
			*PointSet,
			ReturnType,
			Name,
			GetNodeRef());
		if (!Attribute)
		{
			return;
		}

		ValuePin.Set(Query, FVoxelRuntimePinValue::Make(Attribute.ToSharedRef()));
	};
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_GetPointAttribute::GetPromotionTypes(const FVoxelPin& Pin) const
{
	return FVoxelPinTypeSet::AllBuffers();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNode_SetPointAttributeBase::ExpandPins(
	FNode& Node,
	TArray<FPin*> Pins,
	const TArray<FPin*>& AllPins) const
{
	check(Pins.Num() == 2);
	check(AllPins.Num() == 3);

	Pins.Insert(MakeConstant(Node, FVoxelPinValue::Make(GetAttributeName())), 1);

	return Call_Single<FVoxelNode_SetPointAttribute>(Pins);
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNode_GetPointAttributeBase::ExpandPins(
	FNode& Node,
	TArray<FPin*> Pins,
	const TArray<FPin*>& AllPins) const
{
	check(Pins.Num() == 0);
	check(AllPins.Num() == 1);

	Pins.Add(MakeConstant(Node, FVoxelPinValue::Make(GetAttributeName())));

	return Call_Single<FVoxelNode_GetPointAttribute>(Pins, AllPins[0]->Type);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_SetPointPosition::Compute(const FVoxelGraphQuery Query) const
{
	// Read the cache-bounds extension first so we can extend the chunk-bounds parameter
	// before fetching upstream points. Same pattern as FVoxelNode_PruneByDistance.
	const TValue<FVoxelVectorBuffer> CacheBoundsExtension = CacheBoundsExtensionPin.Get(Query);

	VOXEL_GRAPH_WAIT(CacheBoundsExtension)
	{
		const TValue<FVoxelPointSet> Points = INLINE_LAMBDA
		{
			const FVoxelGraphParameters::FPointSetChunkBounds* BoundsParameter = Query->FindParameter<FVoxelGraphParameters::FPointSetChunkBounds>();
			if (!BoundsParameter)
			{
				return InPin.Get(Query);
			}

			const FVector3d Extension = FVector3d(FVector(((*CacheBoundsExtension)[0])));
			if (Extension.IsNearlyZero())
			{
				return InPin.Get(Query);
			}

			FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
			NewQuery.RemoveParameter<FVoxelGraphParameters::FPointSetChunkBounds>();

			FVoxelGraphParameters::FPointSetChunkBounds& Parameter = NewQuery.AddParameter<FVoxelGraphParameters::FPointSetChunkBounds>(*BoundsParameter);
			Parameter.ExtendBounds(BoundsParameter->GetInitialBounds().Extend(Extension));

			return InPin.Get(FVoxelGraphQuery(NewQuery, Query.GetCallstack()));
		};

		VOXEL_GRAPH_WAIT(Points)
		{
			const TValue<FVoxelDoubleVectorBuffer> Position = PositionPin.Get(Points->MakeQuery(Query));

			VOXEL_GRAPH_WAIT(Points, Position)
			{
				if (!Points->CheckNum(this, Position->Num_Slow()))
				{
					OutPin.Set(Query, Points);
					return;
				}

				const TSharedRef<FVoxelPointSet> NewPoints = Points->MakeSharedCopy();
				NewPoints->Add(FVoxelPointAttributes::Position, Position);
				OutPin.Set(Query, NewPoints);
			};
		};
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_GetPointSeed::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelSeed> Seed = SeedPin.Get(Query);

	VOXEL_GRAPH_WAIT(Seed)
	{
		const FVoxelGraphParameters::FPointSet* Parameter = Query->FindParameter<FVoxelGraphParameters::FPointSet>();
		if (!Parameter)
		{
			VOXEL_MESSAGE(Error, "{0}: No points passed in input", this);
			return;
		}

		if (Parameter->Value->Num() == 0)
		{
			return;
		}

		const FVoxelPointIdBuffer* IdBuffer = Parameter->Value->Find<FVoxelPointIdBuffer>(FVoxelPointAttributes::Id);
		if (!IdBuffer)
		{
			VOXEL_MESSAGE(Error, "{0}: Missing attribute Id", this);
			return;
		}

		ValuePin.Set(Query, FVoxelPointUtilities::PointIdToSeed(*IdBuffer, Seed));
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_ApplyTranslation::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelPointSet> Points = InPin.Get(Query);

	VOXEL_GRAPH_WAIT(Points)
	{
		if (Points->Num() == 0)
		{
			return;
		}

		const TValue<FVoxelDoubleVectorBuffer> Translation = TranslationPin.Get(Points->MakeQuery(Query));

		VOXEL_GRAPH_WAIT(Points, Translation)
		{
			CheckVoxelBuffersNum(*Points, Translation);

			const FVoxelDoubleVectorBuffer* PositionBuffer = Points->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
			if (!PositionBuffer)
			{
				VOXEL_MESSAGE(Error, "{0}: Missing attribute Position", this);
				return;
			}

			const TSharedRef<FVoxelPointSet> NewPoints = Points->MakeSharedCopy();
			NewPoints->Add(FVoxelPointAttributes::Position, FVoxelBufferMathUtilities::Add(*PositionBuffer, *Translation));
			OutPin.Set(Query, NewPoints);
		};
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_ApplyRotation::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelPointSet> Points = InPin.Get(Query);

	VOXEL_GRAPH_WAIT(Points)
	{
		if (Points->Num() == 0)
		{
			return;
		}

		const TValue<FVoxelQuaternionBuffer> Rotation = RotationPin.Get(Points->MakeQuery(Query));

		VOXEL_GRAPH_WAIT(Points, Rotation)
		{
			CheckVoxelBuffersNum(*Points, Rotation);

			const FVoxelQuaternionBuffer* RotationBuffer = Points->Find<FVoxelQuaternionBuffer>(FVoxelPointAttributes::Rotation);
			if (!RotationBuffer)
			{
				VOXEL_MESSAGE(Error, "{0}: Missing attribute Rotation", this);
				return;
			}

			const TSharedRef<FVoxelPointSet> NewPoints = Points->MakeSharedCopy();
			NewPoints->Add(FVoxelPointAttributes::Rotation, FVoxelBufferMathUtilities::Combine(*RotationBuffer, *Rotation));
			OutPin.Set(Query, NewPoints);
		};
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_ApplyScale::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelPointSet> Points = InPin.Get(Query);

	VOXEL_GRAPH_WAIT(Points)
	{
		if (Points->Num() == 0)
		{
			return;
		}

		const TValue<FVoxelVectorBuffer> Scale = ScalePin.Get(Points->MakeQuery(Query));

		VOXEL_GRAPH_WAIT(Points, Scale)
		{
			CheckVoxelBuffersNum(*Points, Scale);

			const TSharedRef<FVoxelPointSet> NewPoints = Points->MakeSharedCopy();

			const FVoxelVectorBuffer* ScaleBuffer = Points->Find<FVoxelVectorBuffer>(FVoxelPointAttributes::Scale);
			if (ScaleBuffer)
			{
				NewPoints->Add(FVoxelPointAttributes::Scale, FVoxelBufferMathUtilities::Multiply(*ScaleBuffer, *Scale));
			}
			else
			{
				NewPoints->Add(FVoxelPointAttributes::Scale, Scale);
			}

			OutPin.Set(Query, NewPoints);
		};
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_TransformPoints::Compute(const FVoxelGraphQuery Query) const
{
	// Read the cache-bounds extension first so we can extend the chunk-bounds parameter
	// before fetching upstream points. Same pattern as FVoxelNode_PruneByDistance.
	const TValue<FVoxelVectorBuffer> CacheBoundsExtension = CacheBoundsExtensionPin.Get(Query);

	VOXEL_GRAPH_WAIT(CacheBoundsExtension)
	{
	const TValue<FVoxelPointSet> Points = INLINE_LAMBDA
	{
		const FVoxelGraphParameters::FPointSetChunkBounds* BoundsParameter = Query->FindParameter<FVoxelGraphParameters::FPointSetChunkBounds>();
		if (!BoundsParameter)
		{
			return InPin.Get(Query);
		}

		const FVector3d Extension = FVector3d(FVector(((*CacheBoundsExtension)[0])));
		if (Extension.IsNearlyZero())
		{
			return InPin.Get(Query);
		}

		FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
		NewQuery.RemoveParameter<FVoxelGraphParameters::FPointSetChunkBounds>();

		FVoxelGraphParameters::FPointSetChunkBounds& Parameter = NewQuery.AddParameter<FVoxelGraphParameters::FPointSetChunkBounds>(*BoundsParameter);
		Parameter.ExtendBounds(BoundsParameter->GetInitialBounds().Extend(Extension));

		return InPin.Get(FVoxelGraphQuery(NewQuery, Query.GetCallstack()));
	};

	VOXEL_GRAPH_WAIT(Points)
	{
		if (Points->Num() == 0)
		{
			return;
		}

		const FVoxelGraphQuery PointQuery = Points->MakeQuery(Query);

		const TValue<FVoxelSeed> Seed = SeedPin.Get(Query);
		const TValue<FVoxelVectorBuffer> OffsetMin = OffsetMinPin.Get(PointQuery);
		const TValue<FVoxelVectorBuffer> OffsetMax = OffsetMaxPin.Get(PointQuery);
		const TValue<FVoxelBoolBuffer> OffsetInLocalSpace = bOffsetInLocalSpacePin.Get(PointQuery);
		const TValue<FVoxelFloatRangeBuffer> Roll = RollPin.Get(PointQuery);
		const TValue<FVoxelFloatRangeBuffer> Pitch = PitchPin.Get(PointQuery);
		const TValue<FVoxelFloatRangeBuffer> Yaw = YawPin.Get(PointQuery);
		const TValue<FVoxelBoolBuffer> AbsoluteRotation = bAbsoluteRotationPin.Get(PointQuery);
		const TValue<FVoxelVectorBuffer> ScaleMin = ScaleMinPin.Get(PointQuery);
		const TValue<FVoxelVectorBuffer> ScaleMax = ScaleMaxPin.Get(PointQuery);
		const TValue<FVoxelBoolBuffer> AbsoluteScale = bAbsoluteScalePin.Get(PointQuery);
		const TValue<FVoxelBoolBuffer> UniformScale = bUniformScalePin.Get(PointQuery);

		VOXEL_GRAPH_WAIT(Points, Seed, OffsetMin, OffsetMax, OffsetInLocalSpace, Roll, Pitch, Yaw, AbsoluteRotation, ScaleMin, ScaleMax, AbsoluteScale, UniformScale)
		{
			FVoxelNodeStatScope StatScope(*this, Points->Num());

			const FVoxelPointIdBuffer* IdBuffer = Points->Find<FVoxelPointIdBuffer>(FVoxelPointAttributes::Id);
			if (!IdBuffer)
			{
				VOXEL_MESSAGE(Error, "{0}: Missing attribute Id", this);
				return;
			}

			const TSharedRef<FVoxelPointSet> NewPoints = Points->MakeSharedCopy();
			const int32 Num = Points->Num();

			// Position transform
			{
				const FVoxelDoubleVectorBuffer* PositionBuffer = Points->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
				if (!PositionBuffer)
				{
					VOXEL_MESSAGE(Error, "{0}: Missing attribute Position", this);
					return;
				}

				const FVoxelQuaternionBuffer* RotationBuffer = Points->Find<FVoxelQuaternionBuffer>(FVoxelPointAttributes::Rotation);

				const FVoxelPointRandom RandomX(Seed, STATIC_HASH("TransformPoints_OffsetX"));
				const FVoxelPointRandom RandomY(Seed, STATIC_HASH("TransformPoints_OffsetY"));
				const FVoxelPointRandom RandomZ(Seed, STATIC_HASH("TransformPoints_OffsetZ"));

				FVoxelDoubleVectorBuffer NewPositionBuffer;
				NewPositionBuffer.Allocate(Num);

				for (int32 Index = 0; Index < Num; Index++)
				{
					const FVoxelPointId PointId = (*IdBuffer)[Index];
					const FVector3f OffsetMinValue = (*OffsetMin)[Index];
					const FVector3f OffsetMaxValue = (*OffsetMax)[Index];

					FVector3d Offset;
					Offset.X = FMath::Lerp(static_cast<double>(OffsetMinValue.X), static_cast<double>(OffsetMaxValue.X), static_cast<double>(RandomX.GetFraction(PointId)));
					Offset.Y = FMath::Lerp(static_cast<double>(OffsetMinValue.Y), static_cast<double>(OffsetMaxValue.Y), static_cast<double>(RandomY.GetFraction(PointId)));
					Offset.Z = FMath::Lerp(static_cast<double>(OffsetMinValue.Z), static_cast<double>(OffsetMaxValue.Z), static_cast<double>(RandomZ.GetFraction(PointId)));

					if (RotationBuffer &&
						(*OffsetInLocalSpace)[Index])
					{
						const FQuat4f PointRotation = (*RotationBuffer)[Index];
						Offset = FQuat(PointRotation).RotateVector(Offset);
					}

					NewPositionBuffer.Set(Index, (*PositionBuffer)[Index] + Offset);
				}

				NewPoints->Add(FVoxelPointAttributes::Position, MakeSharedCopy(MoveTemp(NewPositionBuffer)));
			}

			// Rotation transform
			{
				const FVoxelQuaternionBuffer* RotationBuffer = Points->Find<FVoxelQuaternionBuffer>(FVoxelPointAttributes::Rotation);

				const FVoxelPointRandom RandomPitch(Seed, STATIC_HASH("TransformPoints_Pitch"));
				const FVoxelPointRandom RandomYaw(Seed, STATIC_HASH("TransformPoints_Yaw"));
				const FVoxelPointRandom RandomRoll(Seed, STATIC_HASH("TransformPoints_Roll"));

				FVoxelQuaternionBuffer NewRotationBuffer;
				NewRotationBuffer.Allocate(Num);

				for (int32 Index = 0; Index < Num; Index++)
				{
					const FVoxelPointId PointId = (*IdBuffer)[Index];
					const FVoxelFloatRange PitchRange = (*Pitch)[Index];
					const FVoxelFloatRange YawRange = (*Yaw)[Index];
					const FVoxelFloatRange RollRange = (*Roll)[Index];

					const float RandomPitchValue = FMath::Lerp(PitchRange.Min, PitchRange.Max, RandomPitch.GetFraction(PointId));
					const float RandomYawValue = FMath::Lerp(YawRange.Min, YawRange.Max, RandomYaw.GetFraction(PointId));
					const float RandomRollValue = FMath::Lerp(RollRange.Min, RollRange.Max, RandomRoll.GetFraction(PointId));

					const FQuat4f RandomQuat = FQuat4f(FRotator(RandomPitchValue, RandomYawValue, RandomRollValue).Quaternion());

					if ((*AbsoluteRotation)[Index] ||
						!RotationBuffer)
					{
						NewRotationBuffer.Set(Index, RandomQuat);
					}
					else
					{
						NewRotationBuffer.Set(Index, (*RotationBuffer)[Index] * RandomQuat);
					}
				}

				NewPoints->Add(FVoxelPointAttributes::Rotation, MakeSharedCopy(MoveTemp(NewRotationBuffer)));
			}

			// Scale transform
			{
				const FVoxelVectorBuffer* ScaleBuffer = Points->Find<FVoxelVectorBuffer>(FVoxelPointAttributes::Scale);

				const FVoxelPointRandom RandomScaleX(Seed, STATIC_HASH("TransformPoints_ScaleX"));
				const FVoxelPointRandom RandomScaleY(Seed, STATIC_HASH("TransformPoints_ScaleY"));
				const FVoxelPointRandom RandomScaleZ(Seed, STATIC_HASH("TransformPoints_ScaleZ"));

				FVoxelVectorBuffer NewScaleBuffer;
				NewScaleBuffer.Allocate(Num);

				for (int32 Index = 0; Index < Num; Index++)
				{
					const FVoxelPointId PointId = (*IdBuffer)[Index];
					const FVector3f ScaleMinValue = (*ScaleMin)[Index];
					const FVector3f ScaleMaxValue = (*ScaleMax)[Index];

					FVector3f RandomScale;
					if ((*UniformScale)[Index])
					{
						const float UniformValue = FMath::Lerp(ScaleMinValue.X, ScaleMaxValue.X, RandomScaleX.GetFraction(PointId));
						RandomScale = FVector3f(UniformValue);
					}
					else
					{
						RandomScale.X = FMath::Lerp(ScaleMinValue.X, ScaleMaxValue.X, RandomScaleX.GetFraction(PointId));
						RandomScale.Y = FMath::Lerp(ScaleMinValue.Y, ScaleMaxValue.Y, RandomScaleY.GetFraction(PointId));
						RandomScale.Z = FMath::Lerp(ScaleMinValue.Z, ScaleMaxValue.Z, RandomScaleZ.GetFraction(PointId));
					}

					if ((*AbsoluteScale)[Index] ||
						!ScaleBuffer)
					{
						NewScaleBuffer.Set(Index, RandomScale);
					}
					else
					{
						NewScaleBuffer.Set(Index, (*ScaleBuffer)[Index] * RandomScale);
					}
				}

				NewPoints->Add(FVoxelPointAttributes::Scale, MakeSharedCopy(MoveTemp(NewScaleBuffer)));
			}

			OutPin.Set(Query, NewPoints);
		};
	};
	};
}