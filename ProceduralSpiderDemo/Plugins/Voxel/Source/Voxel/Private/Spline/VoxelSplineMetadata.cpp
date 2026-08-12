// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Spline/VoxelSplineMetadata.h"
#include "Spline/VoxelSplineComponent.h"
#include "Spline/VoxelSplineParameters.h"
#include "VoxelGraph.h"
#include "VoxelGraphParametersView.h"
#include "VoxelParameterView.h"

void UVoxelSplineMetadata::Fixup(const UVoxelGraph& Graph)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelGraphParametersView> ParametersView = Graph.GetParametersView_ValidGraph();

	TVoxelMap<FGuid, FVoxelParameter> GuidToParameter;
	GuidToParameter.Reserve(Graph.NumParameters());

	Graph.ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
	{
		if (!Parameter.Type.CanBeCastedTo<FVoxelSplineParameter>())
		{
			return;
		}

		GuidToParameter.Add_EnsureNew(Guid, Parameter);
	});

	for (auto It = GuidToValues.CreateIterator(); It; ++It)
	{
		if (!GuidToParameter.Contains(It.Key()))
		{
			It.RemoveCurrent();
		}
	}

	for (const auto& It : GuidToParameter)
	{
		if (!GuidToValues.Contains(It.Key))
		{
			GuidToValues.Add(It.Key);
		}
		FVoxelSplineMetadataValues& Values = GuidToValues[It.Key];

		FVoxelParameter Parameter = It.Value;
		Parameter.Type = FVoxelRuntimePinValue(Parameter.Type).Get<FVoxelSplineParameter>().GetType();
		Values.Parameter = Parameter;

		FVoxelParameterView* ParameterView = ParametersView->FindByGuid(It.Key);
		if (!ensure(ParameterView))
		{
			continue;
		}

		const FVoxelPinValue SplineParameterValue = ParameterView->GetValue();
		if (!ensure(SplineParameterValue.CanBeCastedTo<FVoxelSplineParameter>()))
		{
			continue;
		}

		const FVoxelPinValue NewDefaultValue = SplineParameterValue.Get<FVoxelSplineParameter>().GetDefaultValue();

		if (Values.DefaultValue == NewDefaultValue)
		{
			continue;
		}

		for (TVoxelInstancedStruct<FVoxelPinValue>& Value : Values.Values)
		{
			if (Value == Values.DefaultValue)
			{
				Value = NewDefaultValue;
			}
		}

		Values.DefaultValue = NewDefaultValue;
	}

	UVoxelSplineComponent& Component = *GetOuterUVoxelSplineComponent();

	Fixup(Component.GetNumberOfSplinePoints(), &Component);

	FVoxelUtilities::ReorderMapKeys(GuidToValues, GuidToParameter.KeyArray());
}

TSharedRef<FVoxelSplineMetadataRuntime> UVoxelSplineMetadata::GetRuntime() const
{
	VOXEL_FUNCTION_COUNTER();

	const UVoxelSplineComponent& Component = *GetOuterUVoxelSplineComponent();

	const TSharedRef<FVoxelSplineMetadataRuntime> RuntimeValues = MakeShared<FVoxelSplineMetadataRuntime>();
	RuntimeValues->GuidToFloatValues.Reserve(GuidToValues.Num());
	RuntimeValues->GuidToVector2DValues.Reserve(GuidToValues.Num());
	RuntimeValues->GuidToVectorValues.Reserve(GuidToValues.Num());

	for (const auto& It : GuidToValues)
	{
		if (It.Value.Parameter.Type.Is<float>())
		{
			TVoxelArray<float>& Values = RuntimeValues->GuidToFloatValues.Add_EnsureNew(It.Key);
			Values.Reserve(It.Value.Values.Num() + 1);

			for (const TVoxelInstancedStruct<FVoxelPinValue>& Value : It.Value.Values)
			{
				Values.Add(Value->Get<float>());
			}

			if (Component.IsClosedLoop())
			{
				Values.Add(Values[0]);
			}
		}
		else if (It.Value.Parameter.Type.Is<FVector2D>())
		{
			TVoxelArray<FVector2f>& Values = RuntimeValues->GuidToVector2DValues.Add_EnsureNew(It.Key);
			Values.Reserve(It.Value.Values.Num() + 1);

			for (const TVoxelInstancedStruct<FVoxelPinValue>& Value : It.Value.Values)
			{
				Values.Add(FVector2f(Value->Get<FVector2D>()));
			}

			if (Component.IsClosedLoop())
			{
				Values.Add(Values[0]);
			}
		}
		else if (It.Value.Parameter.Type.Is<FVector>())
		{
			TVoxelArray<FVector3f>& Values = RuntimeValues->GuidToVectorValues.Add_EnsureNew(It.Key);
			Values.Reserve(It.Value.Values.Num() + 1);

			for (const TVoxelInstancedStruct<FVoxelPinValue>& Value : It.Value.Values)
			{
				Values.Add(FVector3f(Value->Get<FVector>()));
			}

			if (Component.IsClosedLoop())
			{
				Values.Add(Values[0]);
			}
		}
		else
		{
			ensure(false);
		}
	}

	return RuntimeValues;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSplineMetadata::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSplineMetadata::InsertPoint(const int32 Index, const float t, const bool bClosedLoop)
{
	VOXEL_FUNCTION_COUNTER();

	Modify();

	for (auto& It : GuidToValues)
	{
		if (Index == It.Value.Values.Num())
		{
			It.Value.Values.Emplace(FVoxelPinValue(It.Value.Parameter.Type));
		}
		else if (ensure(Index < It.Value.Values.Num()))
		{
			It.Value.Values.EmplaceAt(Index, FVoxelPinValue(It.Value.Parameter.Type));
		}
	}
}

void UVoxelSplineMetadata::UpdatePoint(const int32 Index, const float t, const bool bClosedLoop)
{
	VOXEL_FUNCTION_COUNTER();

	Modify();
}

void UVoxelSplineMetadata::AddPoint(const float InputKey)
{
	VOXEL_FUNCTION_COUNTER();

	Modify();

	for (auto& It : GuidToValues)
	{
		It.Value.Values.Emplace(FVoxelPinValue(It.Value.Parameter.Type));
	}
}

void UVoxelSplineMetadata::RemovePoint(const int32 Index)
{
	VOXEL_FUNCTION_COUNTER();

	Modify();

	for (auto& It : GuidToValues)
	{
		if (ensure(It.Value.Values.IsValidIndex(Index)))
		{
			It.Value.Values.RemoveAt(Index);
		}
	}
}

void UVoxelSplineMetadata::DuplicatePoint(const int32 Index)
{
	VOXEL_FUNCTION_COUNTER();

	Modify();

	for (auto& It : GuidToValues)
	{
		if (ensure(It.Value.Values.IsValidIndex(Index)))
		{
			It.Value.Values.Insert(MakeCopy(It.Value.Values[Index]), Index);
		}
	}
}

void UVoxelSplineMetadata::CopyPoint(const USplineMetadata* FromSplineMetadata, const int32 FromIndex, const int32 ToIndex)
{
	VOXEL_FUNCTION_COUNTER();

	const UVoxelSplineMetadata& FromMetadata = *CastChecked<UVoxelSplineMetadata>(FromSplineMetadata);

	Modify();

	for (auto& It : GuidToValues)
	{
		const FVoxelSplineMetadataValues* FromValues = FromMetadata.GuidToValues.Find(It.Key);
		if (!ensureVoxelSlow(FromValues) ||
			!ensure(FromValues->Values.IsValidIndex(FromIndex) ||
			!ensure(It.Value.Values.IsValidIndex(ToIndex))))
		{
			continue;
		}

		It.Value.Values[ToIndex] = FromValues->Values[FromIndex];
	}
}

void UVoxelSplineMetadata::Reset(const int32 NumPoints)
{
	VOXEL_FUNCTION_COUNTER();

	Modify();

	for (auto& It : GuidToValues)
	{
		It.Value.Values.Reset(NumPoints);
	}
}

void UVoxelSplineMetadata::Fixup(const int32 NumPoints, USplineComponent*)
{
	VOXEL_FUNCTION_COUNTER();

	for (auto& It : GuidToValues)
	{
		FVoxelSplineMetadataValues& Values = It.Value;

		Values.DefaultValue.Fixup(Values.Parameter.Type);

		if (Values.Values.Num() != NumPoints)
		{
			Values.Values.SetNum(NumPoints);
		}

		for (TVoxelInstancedStruct<FVoxelPinValue>& Value : Values.Values)
		{
			if (!Value.IsValid())
			{
				Value = Values.DefaultValue;
			}

			Value->Fixup(Values.Parameter.Type);
		}
	}
}