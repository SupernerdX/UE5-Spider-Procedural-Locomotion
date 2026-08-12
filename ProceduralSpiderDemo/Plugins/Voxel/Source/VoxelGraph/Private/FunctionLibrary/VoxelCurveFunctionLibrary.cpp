// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "FunctionLibrary/VoxelCurveFunctionLibrary.h"
#include "VoxelDependency.h"
#include "VoxelInvalidationCallstack.h"
#include "VoxelCurveFunctionLibraryImpl.ispc.generated.h"

struct FVoxelCurveData
{
	TSharedPtr<const FRichCurve> Curve;

	FString Error;
	float DefaultValue = 0.f;
	TVoxelArray<ispc::FRichCurveKey> Keys;
};

class FVoxelCurveManager : public FVoxelSingleton
{
public:
	//~ Begin FVoxelSingleton Interface
	virtual void Initialize() override
	{
#if WITH_EDITOR
		FCoreUObjectDelegates::OnObjectPropertyChanged.AddLambda([this](UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent)
		{
			if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
			{
				return;
			}

			const UCurveFloat* Curve = Cast<UCurveFloat>(Object);
			if (!Curve)
			{
				return;
			}

			UpdateCurve_GameThread(*Curve);
		});
#endif
	}
	//~ End FVoxelSingleton Interface

	TSharedRef<FVoxelRuntimeCurve> GetRuntimeCurve_GameThread(const UCurveFloat& Curve)
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());

		TSharedPtr<FVoxelRuntimeCurve>& RuntimeCurve = CurveToRuntimeCurve.FindOrAdd(&Curve);
		if (RuntimeCurve)
		{
			return RuntimeCurve.ToSharedRef();
		}

		RuntimeCurve = MakeShared<FVoxelRuntimeCurve>();
		RuntimeCurve->Dependency = FVoxelDependency::Create("Curve "+ Curve.GetName());

		UpdateCurve_GameThread(Curve);

		return RuntimeCurve.ToSharedRef();
	}
	void UpdateCurve_GameThread(const UCurveFloat& Curve) const
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());

		const TSharedPtr<FVoxelRuntimeCurve> RuntimeCurve = CurveToRuntimeCurve.FindRef(&Curve);
		if (!RuntimeCurve)
		{
			return;
		}

		FVoxelInvalidationScope Scope(Curve);

		RuntimeCurve->Update(
			Curve.FloatCurve,
			true);
	}

private:
	TVoxelMap<TVoxelObjectPtr<const UCurveFloat>, TSharedPtr<FVoxelRuntimeCurve>> CurveToRuntimeCurve;
};
FVoxelCurveManager* GVoxelCurveManager = new FVoxelCurveManager();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelRuntimeCurve::Update(
	const FRichCurve& Curve,
	const bool bInvalidateDependency)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelCurveData> Data = MakeShared<FVoxelCurveData>();
	Data->Curve = MakeSharedCopy(Curve);
	Data->DefaultValue = Curve.DefaultValue;

	for (const FRichCurveKey& Key : Curve.Keys)
	{
		if (Key.TangentWeightMode != RCTWM_WeightedNone)
		{
			Data->Error += "Weighted tangents are not supported - please disable Fast Curve\n";
		}

		ispc::FRichCurveKey ISPCKey;
		ISPCKey.Time = Key.Time;
		ISPCKey.Value = Key.Value;
		ISPCKey.ArriveTangent = Key.ArriveTangent;
		ISPCKey.LeaveTangent = Key.LeaveTangent;

		switch (Key.InterpMode)
		{
		default: ensure(false);
		case RCIM_Linear: ISPCKey.InterpMode = ispc::RCIM_Linear; break;
		case RCIM_Constant: ISPCKey.InterpMode = ispc::RCIM_Constant; break;
		case RCIM_Cubic: ISPCKey.InterpMode = ispc::RCIM_Cubic; break;
		}

		Data->Keys.Add(ISPCKey);
	}

	// Add dummy keys for infinity
	if (Curve.Keys.Num() >= 2)
	{
		const double MaxTime = FMath::Max3(
			1.e3f,
			FMath::Abs(Curve.Keys[0].Time),
			FMath::Abs(Curve.Keys.Last().Time));

		if (Curve.PreInfinityExtrap == RCCE_Linear)
		{
			ispc::FRichCurveKey ISPCKey;
			ISPCKey.Time = -MaxTime;

			const double DeltaTime = Curve.Keys[1].Time - Curve.Keys[0].Time;
			if (FMath::IsNearlyZero(DeltaTime))
			{
				ISPCKey.Value = Curve.Keys[0].Value;
			}
			else
			{
				const double DeltaValue = Curve.Keys[1].Value - Curve.Keys[0].Value;
				const double Slope = DeltaValue / DeltaTime;

				ISPCKey.Value = Slope * (-MaxTime - double(Curve.Keys[0].Time)) + Curve.Keys[0].Value;
			}

			ISPCKey.ArriveTangent = 0;
			ISPCKey.LeaveTangent = 0;
			ISPCKey.InterpMode = ispc::RCIM_Linear;
			Data->Keys.Insert(ISPCKey, 0);
		}
		else if (Curve.PreInfinityExtrap == RCCE_Constant)
		{
			ispc::FRichCurveKey ISPCKey;
			ISPCKey.Time = -MaxTime;
			ISPCKey.Value = Curve.Keys[0].Value;
			ISPCKey.ArriveTangent = 0;
			ISPCKey.LeaveTangent = 0;
			ISPCKey.InterpMode = ispc::RCIM_Constant;
			Data->Keys.Insert(ISPCKey, 0);
		}
		else
		{
			Data->Error += "Unsupported PreInfinityExtrap: " + UEnum::GetValueAsString(Curve.PreInfinityExtrap) + " - please disable Fast Curve\n";
		}

		if (Curve.PostInfinityExtrap == RCCE_Linear)
		{
			ispc::FRichCurveKey ISPCKey;
			ISPCKey.Time = MaxTime;

			const double DeltaTime = Curve.Keys.Last(1).Time - Curve.Keys.Last().Time;

			if (FMath::IsNearlyZero(DeltaTime))
			{
				ISPCKey.Value = Curve.Keys.Last().Value;
			}
			else
			{
				const double DeltaValue = Curve.Keys.Last(1).Value - Curve.Keys.Last().Value;
				const double Slope = DeltaValue / DeltaTime;

				ISPCKey.Value = Slope * (MaxTime - double(Curve.Keys.Last().Time)) + Curve.Keys.Last().Value;
			}

			ISPCKey.ArriveTangent = 0;
			ISPCKey.LeaveTangent = 0;
			ISPCKey.InterpMode = ispc::RCIM_Linear;
			Data->Keys.Add(ISPCKey);
		}
		else if (Curve.PostInfinityExtrap == RCCE_Constant)
		{
			ispc::FRichCurveKey ISPCKey;
			ISPCKey.Time = MaxTime;
			ISPCKey.Value = Curve.Keys.Last().Value;
			ISPCKey.ArriveTangent = 0;
			ISPCKey.LeaveTangent = 0;
			ISPCKey.InterpMode = ispc::RCIM_Constant;
			Data->Keys.Add(ISPCKey);
		}
		else
		{
			Data->Error += "Unsupported PostInfinityExtrap: " + UEnum::GetValueAsString(Curve.PostInfinityExtrap) + " - please disable Fast Curve\n";
		}
	}

	Data->Error.RemoveFromEnd("\n");

	{
		VOXEL_SCOPE_LOCK(CriticalSection);
		Data_RequiresLock = Data;
	}

	if (bInvalidateDependency)
	{
		Dependency->Invalidate();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EVoxelPinValueOpsUsage  FVoxelPinValueOps_FVoxelRuntimeCurveRef::GetUsage() const
{
	return
		EVoxelPinValueOpsUsage::GetExposedType |
		EVoxelPinValueOpsUsage::MakeRuntimeValue;
}

FVoxelPinType FVoxelPinValueOps_FVoxelRuntimeCurveRef::GetExposedType() const
{
	return FVoxelPinType::Make<FVoxelCurve>();
}

FVoxelRuntimePinValue FVoxelPinValueOps_FVoxelRuntimeCurveRef::MakeRuntimeValue(
	const FVoxelPinValue& Value,
	const FVoxelPinType::FRuntimeValueContext& Context) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(Value.Is<FVoxelCurve>()))
	{
		return {};
	}

	const TSharedRef<FVoxelRuntimeCurve> RuntimeCurve = MakeShared<FVoxelRuntimeCurve>();
	RuntimeCurve->Dependency = FVoxelDependency::Create("Inline Curve");

	RuntimeCurve->Update(
		Value.Get<FVoxelCurve>().Curve,
		false);

	const TSharedRef<FVoxelRuntimeCurveRef> Result = MakeShared<FVoxelRuntimeCurveRef>();
	Result->RuntimeCurve = RuntimeCurve;
	return FVoxelRuntimePinValue::Make(Result);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelCurveRefPinType::Convert(
	const bool bSetObject,
	TVoxelObjectPtr<UCurveFloat>& OutObject,
	UCurveFloat& InObject,
	FVoxelCurveRef& Struct)
{
	if (bSetObject)
	{
		OutObject = Struct.Object;
	}
	else
	{
		Struct.Object = InObject;
		Struct.RuntimeCurveRef.RuntimeCurve = GVoxelCurveManager->GetRuntimeCurve_GameThread(InObject);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelRuntimeCurveRef UVoxelCurveFunctionLibrary::MakeCurveFromAsset(const FVoxelCurveRef& Asset) const
{
	if (!Asset.RuntimeCurveRef.RuntimeCurve)
	{
		VOXEL_MESSAGE(Error, "{0}: Curve is null", this);
		return {};
	}

	return Asset.RuntimeCurveRef;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer UVoxelCurveFunctionLibrary::SampleCurve(
	const FVoxelFloatBuffer& Value,
	const FVoxelRuntimeCurveRef& Curve,
	const bool bFastCurve) const
{
	if (!Curve.RuntimeCurve)
	{
		VOXEL_MESSAGE(Error, "{0}: Curve is null", this);
		return 0.f;
	}
	const FVoxelRuntimeCurve& RuntimeCurve = *Curve.RuntimeCurve;

	TSharedPtr<const FVoxelCurveData> Data;
	{
		VOXEL_SCOPE_LOCK(RuntimeCurve.CriticalSection);
		Data = RuntimeCurve.Data_RequiresLock;
	}

	Query->Context.DependencyCollector.AddDependency(*RuntimeCurve.Dependency);

	if (!bFastCurve)
	{
		FVoxelFloatBuffer ReturnValue;
		ReturnValue.Allocate(Value.Num());

		for (int32 Index = 0; Index < Value.Num(); Index++)
		{
			ReturnValue.Set(Index, Data->Curve->Eval(Value[Index]));
		}

		return ReturnValue;
	}

	if (!Data->Error.IsEmpty())
	{
		VOXEL_MESSAGE(Error, "{0}: {1}", this, Data->Error);
		return 0.f;
	}
	if (Data->Keys.Num() == 0)
	{
		return Data->DefaultValue;
	}
	if (Data->Keys.Num() == 1)
	{
		return Data->Keys[0].Value;
	}

	FVoxelFloatBuffer ReturnValue;
	ReturnValue.Allocate(Value.Num());

	ispc::VoxelCurveFunctionLibrary_SampleCurve(
		Data->Keys.GetData(),
		Data->Keys.Num(),
		Value.GetData(),
		Value.Num(),
		ReturnValue.GetData());

	return ReturnValue;
}