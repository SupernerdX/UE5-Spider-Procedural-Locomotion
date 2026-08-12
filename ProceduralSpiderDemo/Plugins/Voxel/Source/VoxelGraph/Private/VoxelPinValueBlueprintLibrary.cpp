// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPinValueBlueprintLibrary.h"

DEFINE_FUNCTION(UVoxelPinValueBlueprintLibrary::execK2_MakeVoxelPinValue)
{
	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	Stack.StepCompiledIn<FProperty>(nullptr);

	const FProperty* Property = Stack.MostRecentProperty;
	const void* PropertyAddress = Stack.MostRecentPropertyAddress;

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	Stack.StepCompiledIn<FProperty>(nullptr);

	P_FINISH;

	if (!ensure(Property))
	{
		VOXEL_MESSAGE(Error, "Invalid value");
		return;
	}

	const FVoxelPinValue Value = FVoxelPinValue::MakeFromProperty(*Property, PropertyAddress);
	if (!Value.IsValid())
	{
		VOXEL_MESSAGE(Error, "Invalid value");
		return;
	}

	P_NATIVE_BEGIN;
	*(FVoxelPinValue*)RESULT_PARAM = Value;
	P_NATIVE_END;
}

DEFINE_FUNCTION(UVoxelPinValueBlueprintLibrary::execK2_BreakVoxelPinValue)
{
	P_GET_STRUCT(FVoxelPinValue, Value);
	P_GET_PROPERTY_REF(FBoolProperty, bIsValid);

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	Stack.StepCompiledIn<FProperty>(nullptr);

	// Is null on startup, when Python is enabled
	if (!Stack.PropertyChainForCompiledIn)
	{
		P_NATIVE_BEGIN;
		Value = {};
		bIsValid = false;
		P_NATIVE_END;

		return;
	}

	const FProperty* Property = Stack.MostRecentProperty;
	void* PropertyAddress = Stack.MostRecentPropertyAddress;

	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;

	Stack.StepCompiledIn<FProperty>(nullptr);

	P_FINISH;

	if (!ensure(Property))
	{
		P_NATIVE_BEGIN;
		Value = {};
		bIsValid = false;
		P_NATIVE_END;

		VOXEL_MESSAGE(Error, "Invalid value");
		return;
	}

	if (!Value.IsValid())
	{
		P_NATIVE_BEGIN;
		Value = {};
		bIsValid = false;
		P_NATIVE_END;
		return;
	}

	FVoxelPinType Type(*Property);

	if (!Value.GetType().CanBeCastedTo_K2(Type))
	{
		VOXEL_MESSAGE(Warning, "Requested {0} type with {1} type value", Type.ToString(), Value.GetType().ToString());

		P_NATIVE_BEGIN;
		Value = {};
		bIsValid = false;
		P_NATIVE_END;
		return;
	}

	P_NATIVE_BEGIN;
	Value.ExportToProperty(*Property, PropertyAddress);
	bIsValid = true;
	P_NATIVE_END;
}