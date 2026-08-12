// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelObjectPinType.h"

struct FVoxelObjectPinTypeStatics
{
	bool bInitialized = false;
	TVoxelMap<const UScriptStruct*, TSharedPtr<const FVoxelObjectPinType>> StructToObjectPinType;
	TVoxelMap<const UClass*, TVoxelArray<TSharedPtr<const FVoxelObjectPinType>>> ClassToObjectPinTypes;

	void InitializeIfNeeded()
	{
		if (bInitialized)
		{
			return;
		}

		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());

		StructToObjectPinType.Reserve(128);
		ClassToObjectPinTypes.Reserve(128);

		for (const UScriptStruct* Struct : GetDerivedStructs<FVoxelObjectPinType>())
		{
			const TSharedRef<FVoxelObjectPinType> Instance = MakeSharedStruct<FVoxelObjectPinType>(Struct);
			StructToObjectPinType.Add_EnsureNew(Instance->GetStruct(), Instance);
			ClassToObjectPinTypes.FindOrAdd(Instance->GetClass()).Add(Instance);
		}

		StructToObjectPinType.Shrink();
		ClassToObjectPinTypes.Shrink();

		check(!bInitialized);
		bInitialized = true;
	}
};
FVoxelObjectPinTypeStatics GVoxelObjectPinTypeStatics;

TVoxelArray<UClass*> FVoxelObjectPinType::GetAllowedClasses() const
{
	return { GetClass() };
}

UObject* FVoxelObjectPinType::GetObject(const FConstVoxelStructView Struct) const
{
	const TVoxelObjectPtr<UObject> WeakObject = GetWeakObject(Struct);
	ensureVoxelSlowNoSideEffects(WeakObject.IsValid_Slow() || WeakObject.IsExplicitlyNull());
	return WeakObject.Resolve();
}

void FVoxelObjectPinType::RegisterPinType(const TSharedRef<const FVoxelObjectPinType>& PinType)
{
	check(IsInGameThread());

	GVoxelObjectPinTypeStatics.StructToObjectPinType.Add_EnsureNew(PinType->GetStruct(), PinType);
	GVoxelObjectPinTypeStatics.ClassToObjectPinTypes.FindOrAdd(PinType->GetClass()).Add(PinType);
}

const TVoxelMap<const UScriptStruct*, TSharedPtr<const FVoxelObjectPinType>>& FVoxelObjectPinType::StructToPinType()
{
	GVoxelObjectPinTypeStatics.InitializeIfNeeded();
	return GVoxelObjectPinTypeStatics.StructToObjectPinType;
}

const TVoxelMap<const UClass*, TVoxelArray<TSharedPtr<const FVoxelObjectPinType>>>& FVoxelObjectPinType::ClassToPinTypes()
{
	GVoxelObjectPinTypeStatics.InitializeIfNeeded();
	return GVoxelObjectPinTypeStatics.ClassToObjectPinTypes;
}