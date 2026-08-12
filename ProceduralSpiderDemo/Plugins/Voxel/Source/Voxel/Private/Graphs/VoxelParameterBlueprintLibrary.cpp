// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelParameterBlueprintLibrary.h"
#include "Graphs/VoxelHeightGraphStampRef.h"
#include "Graphs/VoxelVolumeGraphStampRef.h"
#include "VoxelParameterOverridesOwner.h"
#include "Sculpt/Height/VoxelHeightSculptGraph.h"
#include "Sculpt/Volume/VoxelVolumeSculptGraph.h"
#include "Sculpt/Height/VoxelHeightSculptGraphWrapper.h"
#include "Sculpt/Volume/VoxelVolumeSculptGraphWrapper.h"

DEFINE_FUNCTION(UVoxelParameterBlueprintLibrary::execK2_GetVoxelHeightGraphParameter)
{
	P_GET_STRUCT(FVoxelHeightGraphStampRef, StampRef);
	P_GET_STRUCT(FName, Name);

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FProperty>(nullptr);

	P_FINISH;

	if (!ensure(Stack.MostRecentProperty))
	{
		return;
	}

	if (!StampRef.IsValid())
	{
		VOXEL_MESSAGE(Error, "Stamp is not valid");
		return;
	}

	FString Error;
	const FVoxelPinValue Value = StampRef->GetParameter(Name, &Error);
	if (!Value.IsValid())
	{
		VOXEL_MESSAGE(Error, "Failed to get parameter: {0}", Error);
		return;
	}

	const FVoxelPinType OutType = FVoxelPinType(*Stack.MostRecentProperty);
	if (!Value.GetType().CanBeCastedTo_K2(OutType))
	{
		VOXEL_MESSAGE(Error, "Parameter {0} has type {1}, but type {2} was expected",
			Name,
			Value.GetType().ToString(),
			OutType.ToString());
		return;
	}

	Value.ExportToProperty(*Stack.MostRecentProperty, Stack.MostRecentPropertyAddress);
}

DEFINE_FUNCTION(UVoxelParameterBlueprintLibrary::execK2_GetVoxelVolumeGraphParameter)
{
	P_GET_STRUCT(FVoxelVolumeGraphStampRef, StampRef);
	P_GET_STRUCT(FName, Name);

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FProperty>(nullptr);

	P_FINISH;

	if (!ensure(Stack.MostRecentProperty))
	{
		return;
	}

	if (!StampRef.IsValid())
	{
		VOXEL_MESSAGE(Error, "Stamp is not valid");
		return;
	}

	FString Error;
	const FVoxelPinValue Value = StampRef->GetParameter(Name, &Error);
	if (!Value.IsValid())
	{
		VOXEL_MESSAGE(Error, "Failed to get parameter: {0}", Error);
		return;
	}

	const FVoxelPinType OutType = FVoxelPinType(*Stack.MostRecentProperty);
	if (!Value.GetType().CanBeCastedTo_K2(OutType))
	{
		VOXEL_MESSAGE(Error, "Parameter {0} has type {1}, but type {2} was expected",
			Name,
			Value.GetType().ToString(),
			OutType.ToString());
		return;
	}

	Value.ExportToProperty(*Stack.MostRecentProperty, Stack.MostRecentPropertyAddress);
}

DEFINE_FUNCTION(UVoxelParameterBlueprintLibrary::execK2_GetVoxelHeightSculptGraphParameter)
{
	P_GET_STRUCT(FVoxelHeightSculptGraphWrapper, SculptGraph);
	P_GET_STRUCT(FName, Name);

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FProperty>(nullptr);

	P_FINISH;

	if (!ensure(Stack.MostRecentProperty))
	{
		return;
	}

	if (!SculptGraph.IsValid())
	{
		VOXEL_MESSAGE(Error, "Sculpt Graph is not valid");
		return;
	}

	TOptional<FVoxelParameter> TargetParameter;
	FGuid ParameterGuid;
	SculptGraph.Graph->ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
	{
		if (Parameter.Name == Name)
		{
			ensure(!TargetParameter.IsSet());
			TargetParameter = Parameter;
			ParameterGuid = Guid;
		}
	});

	if (!TargetParameter.IsSet())
	{
		TVoxelArray<FString> ValidParameters;
		SculptGraph.Graph->ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
		{
			ValidParameters.Add(Parameter.Name.ToString() + " (" + Parameter.Type.GetExposedType().ToString() + ")");
		});

		const FString Error = "Failed to find " + Name.ToString() + ". Valid parameters: " + FString::Join(ValidParameters, TEXT(", "));
		VOXEL_MESSAGE(Error, "Failed to get parameter: {0}", Error);
		return;
	}

	const FVoxelPinValue Value = INLINE_LAMBDA
	{
		FVoxelParameterValueOverride* ValueOverride = SculptGraph.ParameterOverrides.GuidToValueOverride.Find(ParameterGuid);
		if (!ValueOverride)
		{
			return FVoxelPinValue(TargetParameter.GetValue().Type.GetExposedType());
		}

		return ValueOverride->Value;
	};

	const FVoxelPinType OutType = FVoxelPinType(*Stack.MostRecentProperty);
	if (!Value.GetType().CanBeCastedTo_K2(OutType))
	{
		VOXEL_MESSAGE(Error, "Parameter {0} has type {1}, but type {2} was expected",
			Name,
			Value.GetType().ToString(),
			OutType.ToString());
		return;
	}

	Value.ExportToProperty(*Stack.MostRecentProperty, Stack.MostRecentPropertyAddress);
}

DEFINE_FUNCTION(UVoxelParameterBlueprintLibrary::execK2_GetVoxelVolumeSculptGraphParameter)
{
	P_GET_STRUCT(FVoxelVolumeSculptGraphWrapper, SculptGraph);
	P_GET_STRUCT(FName, Name);

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FProperty>(nullptr);

	P_FINISH;

	if (!ensure(Stack.MostRecentProperty))
	{
		return;
	}

	if (!SculptGraph.IsValid())
	{
		VOXEL_MESSAGE(Error, "Sculpt Graph is not valid");
		return;
	}

	TOptional<FVoxelParameter> TargetParameter;
	FGuid ParameterGuid;
	SculptGraph.Graph->ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
	{
		if (Parameter.Name == Name)
		{
			ensure(!TargetParameter.IsSet());
			TargetParameter = Parameter;
			ParameterGuid = Guid;
		}
	});

	if (!TargetParameter.IsSet())
	{
		TVoxelArray<FString> ValidParameters;
		SculptGraph.Graph->ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
		{
			ValidParameters.Add(Parameter.Name.ToString() + " (" + Parameter.Type.GetExposedType().ToString() + ")");
		});

		const FString Error = "Failed to find " + Name.ToString() + ". Valid parameters: " + FString::Join(ValidParameters, TEXT(", "));
		VOXEL_MESSAGE(Error, "Failed to get parameter: {0}", Error);
		return;
	}

	const FVoxelPinValue Value = INLINE_LAMBDA
	{
		FVoxelParameterValueOverride* ValueOverride = SculptGraph.ParameterOverrides.GuidToValueOverride.Find(ParameterGuid);
		if (!ValueOverride)
		{
			return FVoxelPinValue(TargetParameter.GetValue().Type.GetExposedType());
		}

		return ValueOverride->Value;
	};

	const FVoxelPinType OutType = FVoxelPinType(*Stack.MostRecentProperty);
	if (!Value.GetType().CanBeCastedTo_K2(OutType))
	{
		VOXEL_MESSAGE(Error, "Parameter {0} has type {1}, but type {2} was expected",
			Name,
			Value.GetType().ToString(),
			OutType.ToString());
		return;
	}

	Value.ExportToProperty(*Stack.MostRecentProperty, Stack.MostRecentPropertyAddress);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DEFINE_FUNCTION(UVoxelParameterBlueprintLibrary::execK2_SetVoxelHeightGraphParameter)
{
	P_GET_STRUCT(FVoxelHeightGraphStampRef, StampRef);
	P_GET_STRUCT(FName, Name);

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	Stack.StepCompiledIn<FProperty>(nullptr);

	const FProperty* Property = Stack.MostRecentProperty;
	const void* PropertyAddress = Stack.MostRecentPropertyAddress;

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	// OutValue, manually wired in UK2Node_SetVoxelGraphParameter::ExpandNode
	Stack.StepCompiledIn<FProperty>(nullptr);

	P_FINISH;

	if (!ensure(Property))
	{
		return;
	}

	if (!StampRef.IsValid())
	{
		VOXEL_MESSAGE(Error, "Stamp is not valid");
		return;
	}

	const FVoxelPinValue Value = FVoxelPinValue::MakeFromProperty(*Property, PropertyAddress);

	FString Error;
	if (StampRef->SetParameter(Name, Value, &Error))
	{
		StampRef.Update();
	}
	else
	{
		VOXEL_MESSAGE(Error, "{0}", Error);
	}
}

DEFINE_FUNCTION(UVoxelParameterBlueprintLibrary::execK2_SetVoxelVolumeGraphParameter)
{
	P_GET_STRUCT(FVoxelVolumeGraphStampRef, StampRef);
	P_GET_STRUCT(FName, Name);

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	Stack.StepCompiledIn<FProperty>(nullptr);

	const FProperty* Property = Stack.MostRecentProperty;
	const void* PropertyAddress = Stack.MostRecentPropertyAddress;

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	// OutValue, manually wired in UK2Node_SetVoxelGraphParameter::ExpandNode
	Stack.StepCompiledIn<FProperty>(nullptr);

	P_FINISH;

	if (!ensure(Property))
	{
		return;
	}

	if (!StampRef.IsValid())
	{
		VOXEL_MESSAGE(Error, "Stamp is not valid");
		return;
	}

	const FVoxelPinValue Value = FVoxelPinValue::MakeFromProperty(*Property, PropertyAddress);

	FString Error;
	if (StampRef->SetParameter(Name, Value, &Error))
	{
		StampRef.Update();
	}
	else
	{
		VOXEL_MESSAGE(Error, "{0}", Error);
	}
}

DEFINE_FUNCTION(UVoxelParameterBlueprintLibrary::execK2_SetVoxelHeightSculptGraphParameter)
{
	P_GET_STRUCT_REF(FVoxelHeightSculptGraphWrapper, SculptGraph);
	P_GET_STRUCT(FName, Name);

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	Stack.StepCompiledIn<FProperty>(nullptr);

	const FProperty* Property = Stack.MostRecentProperty;
	const void* PropertyAddress = Stack.MostRecentPropertyAddress;

	P_GET_STRUCT_REF(FVoxelHeightSculptGraphWrapper, OutSculptGraph);

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	// OutValue, manually wired in UK2Node_SetVoxelGraphParameter::ExpandNode
	Stack.StepCompiledIn<FProperty>(nullptr);

	P_FINISH;

	ON_SCOPE_EXIT
	{
		OutSculptGraph = SculptGraph;
	};

	if (!ensure(Property))
	{
		return;
	}

	if (!SculptGraph.IsValid())
	{
		VOXEL_MESSAGE(Error, "Sculpt graph is not valid");
		return;
	}

	TOptional<FVoxelParameter> TargetParameter;
	FGuid ParameterGuid;
	SculptGraph.Graph->ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
	{
		if (Parameter.Name == Name)
		{
			ensure(!TargetParameter.IsSet());
			TargetParameter = Parameter;
			ParameterGuid = Guid;
		}
	});

	if (!TargetParameter.IsSet())
	{
		TVoxelArray<FString> ValidParameters;
		SculptGraph.Graph->ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
		{
			ValidParameters.Add(Parameter.Name.ToString() + " (" + Parameter.Type.GetExposedType().ToString() + ")");
		});

		const FString Error = "Failed to find " + Name.ToString() + ". Valid parameters: " + FString::Join(ValidParameters, TEXT(", "));
		VOXEL_MESSAGE(Error, "{0}", Error);
		return;
	}

	FVoxelPinValue Value = FVoxelPinValue::MakeFromProperty(*Property, PropertyAddress);
	{
		const FVoxelPinType ExposedType = TargetParameter.GetValue().Type.GetExposedType();
		Value.Fixup(ExposedType);

		if (!Value.CanBeCastedTo(ExposedType))
		{
			const FString Error =
				"Invalid parameter type for " + Name.ToString() + ". Parameter has type " + ExposedType.ToString() +
				", but value of type " + Value.GetType().ToString() + " was passed";
			VOXEL_MESSAGE(Error, "{0}", Error);
			return;
		}
	}

	FVoxelParameterValueOverride ValueOverride;
	ValueOverride.bEnable = true;
	ValueOverride.Value = Value;
	ValueOverride.Value.Fixup();

	P_NATIVE_BEGIN;
	SculptGraph.ParameterOverrides.GuidToValueOverride.Add(ParameterGuid, ValueOverride);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UVoxelParameterBlueprintLibrary::execK2_SetVoxelVolumeSculptGraphParameter)
{
	P_GET_STRUCT_REF(FVoxelVolumeSculptGraphWrapper, SculptGraph);
	P_GET_STRUCT(FName, Name);

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	Stack.StepCompiledIn<FProperty>(nullptr);

	const FProperty* Property = Stack.MostRecentProperty;
	const void* PropertyAddress = Stack.MostRecentPropertyAddress;

	P_GET_STRUCT_REF(FVoxelVolumeSculptGraphWrapper, OutSculptGraph);

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	// OutValue, manually wired in UK2Node_SetVoxelGraphParameter::ExpandNode
	Stack.StepCompiledIn<FProperty>(nullptr);

	P_FINISH;

	ON_SCOPE_EXIT
	{
		OutSculptGraph = SculptGraph;
	};

	if (!ensure(Property))
	{
		return;
	}

	if (!SculptGraph.IsValid())
	{
		VOXEL_MESSAGE(Error, "Sculpt graph is not valid");
		return;
	}

	TOptional<FVoxelParameter> TargetParameter;
	FGuid ParameterGuid;
	SculptGraph.Graph->ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
	{
		if (Parameter.Name == Name)
		{
			ensure(!TargetParameter.IsSet());
			TargetParameter = Parameter;
			ParameterGuid = Guid;
		}
	});

	if (!TargetParameter.IsSet())
	{
		TVoxelArray<FString> ValidParameters;
		SculptGraph.Graph->ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
		{
			ValidParameters.Add(Parameter.Name.ToString() + " (" + Parameter.Type.GetExposedType().ToString() + ")");
		});

		const FString Error = "Failed to find " + Name.ToString() + ". Valid parameters: " + FString::Join(ValidParameters, TEXT(", "));
		VOXEL_MESSAGE(Error, "{0}", Error);
		return;
	}

	FVoxelPinValue Value = FVoxelPinValue::MakeFromProperty(*Property, PropertyAddress);
	{
		const FVoxelPinType ExposedType = TargetParameter.GetValue().Type.GetExposedType();
		Value.Fixup(ExposedType);

		if (!Value.CanBeCastedTo(ExposedType))
		{
			const FString Error =
				"Invalid parameter type for " + Name.ToString() + ". Parameter has type " + ExposedType.ToString() +
				", but value of type " + Value.GetType().ToString() + " was passed";
			VOXEL_MESSAGE(Error, "{0}", Error);
			return;
		}
	}

	FVoxelParameterValueOverride ValueOverride;
	ValueOverride.bEnable = true;
	ValueOverride.Value = Value;
	ValueOverride.Value.Fixup();

	P_NATIVE_BEGIN;
	SculptGraph.ParameterOverrides.GuidToValueOverride.Add(ParameterGuid, ValueOverride);
	P_NATIVE_END;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelParameterBlueprintLibrary::HasVoxelHeightGraphParameter(const FVoxelHeightGraphStampRef& Stamp, const FName Name)
{
	if (!Stamp.IsValid())
	{
		VOXEL_MESSAGE(Error, "Stamp is not valid");
		return false;
	}

	return Stamp->HasParameter(Name);
}

bool UVoxelParameterBlueprintLibrary::HasVoxelVolumeGraphParameter(const FVoxelVolumeGraphStampRef& Stamp, const FName Name)
{
	if (!Stamp.IsValid())
	{
		VOXEL_MESSAGE(Error, "Stamp is not valid");
		return false;
	}

	return Stamp->HasParameter(Name);
}

bool UVoxelParameterBlueprintLibrary::HasVoxelHeightSculptGraphParameter(const FVoxelHeightSculptGraphWrapper& SculptGraph, const FName Name)
{
	if (!SculptGraph.IsValid())
	{
		VOXEL_MESSAGE(Error, "Sculpt graph is not valid");
		return false;
	}

	bool bHasParameter = false;
	SculptGraph.Graph->ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
	{
		if (Parameter.Name == Name)
		{
			bHasParameter = true;
		}
	});
	return bHasParameter;
}

bool UVoxelParameterBlueprintLibrary::HasVoxelVolumeSculptGraphParameter(const FVoxelVolumeSculptGraphWrapper& SculptGraph, const FName Name)
{
	if (!SculptGraph.IsValid())
	{
		VOXEL_MESSAGE(Error, "Sculpt graph is not valid");
		return false;
	}

	bool bHasParameter = false;
	SculptGraph.Graph->ForeachParameter([&](const FGuid& Guid, const FVoxelParameter& Parameter)
	{
		if (Parameter.Name == Name)
		{
			bHasParameter = true;
		}
	});
	return bHasParameter;
}