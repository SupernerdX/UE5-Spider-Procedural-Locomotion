// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelRuntimeStruct.h"
#include "VoxelObjectPinType.h"
#include "StructUtils/UserDefinedStruct.h"

FVoxelRuntimeStruct::FVoxelRuntimeStruct(
	const FConstVoxelStructView Struct,
	const FVoxelPinType::FRuntimeValueContext& Context)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());

	UScriptStruct& ScriptStruct = *Struct.GetScriptStruct();

	UserStruct = CastEnsured<UUserDefinedStruct>(&ScriptStruct);

	PropertyNameToValue.Reserve(16);

	for (const FProperty& Property : GetStructProperties(ScriptStruct))
	{
		if (!FVoxelPinType::IsSupported(Property))
		{
			continue;
		}

		const FVoxelPinValue Value = FVoxelPinValue::MakeFromProperty(
			Property,
			Property.ContainerPtrToValuePtr<void>(Struct.GetStructMemory()));

		PropertyNameToValue.Add_EnsureNew(
			Property.GetFName(),
			FVoxelPinType::MakeRuntimeValue(GetRuntimeType(Value.GetType()), Value, Context));
	}
}

FVoxelPinType FVoxelRuntimeStruct::GetRuntimeType(const FVoxelPinType& Type)
{
	if (!Type.IsObject())
	{
		return Type;
	}

	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TVoxelMap<const UClass*, TVoxelArray<TSharedPtr<const FVoxelObjectPinType>>>& ClassToPinTypes = FVoxelObjectPinType::ClassToPinTypes();

	const UClass* Class = Type.GetObjectClass();

	TSharedPtr<const FVoxelObjectPinType> PinType;
	while (!PinType)
	{
		check(Class);

		if (const TVoxelArray<TSharedPtr<const FVoxelObjectPinType>>* PinTypes = ClassToPinTypes.Find(Class))
		{
			ensure(PinTypes->Num() == 1);
			PinType = (*PinTypes)[0];
		}

		Class = Class->GetSuperClass();
	}
	check(PinType);

	return FVoxelPinType::MakeStruct(PinType->GetStruct());
}