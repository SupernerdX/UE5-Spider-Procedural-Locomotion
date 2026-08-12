// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampRefDataProvider.h"
#include "VoxelStampRef.h"

bool FVoxelStampRefDataProvider::IsValid() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!StructProperty->IsValidHandle())
	{
		return false;
	}

	bool bHasValidData = false;
	FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(StructProperty, [&](FVoxelStampRef&)
	{
		bHasValidData = true;
	});
	return bHasValidData;
}

const UStruct* FVoxelStampRefDataProvider::GetBaseStructure() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!StructProperty->IsValidHandle())
	{
		return nullptr;
	}

	TVoxelSet<UScriptStruct*> Structs;
	FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(StructProperty, [&](const FVoxelStampRef& StampRef)
	{
		Structs.Add(StampRef.GetStruct());
	});

	if (Structs.Num() == 0)
	{
		return nullptr;
	}

	const UScriptStruct* BaseStruct = Structs.GetFirstValue();
	for (const UScriptStruct* Struct : Structs)
	{
		if (!Struct)
		{
			// No common struct
			return nullptr;
		}

		while (!Struct->IsChildOf(BaseStruct))
		{
			BaseStruct = Cast<UScriptStruct>(BaseStruct->GetSuperStruct());

			if (!BaseStruct)
			{
				return nullptr;
			}
		}
	}

	for (const UScriptStruct* Struct : Structs)
	{
		ensure(Struct->IsChildOf(BaseStruct));
	}

	return BaseStruct;
}

void FVoxelStampRefDataProvider::GetInstances(TArray<TSharedPtr<FStructOnScope>>& OutInstances, const UStruct* ExpectedBaseStructure) const
{
	VOXEL_FUNCTION_COUNTER();
	ensure(OutInstances.Num() == 0);

	if (!StructProperty->IsValidHandle())
	{
		return;
	}

	const UScriptStruct* BaseStructure = Cast<UScriptStruct>(ExpectedBaseStructure);

	// The returned instances need to be compatible with base structure.
	// This function returns empty instances in case they are not compatible, with the idea that we have as many instances as we have outer objects.
	FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(StructProperty, [&](const FVoxelStampRef* StampRef)
	{
		if (!BaseStructure ||
			!StampRef ||
			!StampRef->IsA(BaseStructure))
		{
			OutInstances.Add(nullptr);
			return;
		}

		OutInstances.Add(MakeShared<FStructOnScope>(
			BaseStructure,
			static_cast<uint8*>(StampRef->GetStampData())));
	});

	TArray<UPackage*> Packages;
	StructProperty->GetOuterPackages(Packages);

	if (!ensure(Packages.Num() == OutInstances.Num()))
	{
		return;
	}

	for (int32 Index = 0; Index < OutInstances.Num(); Index++)
	{
		const TSharedPtr<FStructOnScope> Instance = OutInstances[Index];
		if (!Instance)
		{
			continue;
		}

		Instance->SetPackage(Packages[Index]);
	}
}

bool FVoxelStampRefDataProvider::IsPropertyIndirection() const
{
	return bIsPropertyIndirection;
}

uint8* FVoxelStampRefDataProvider::GetValueBaseAddress(uint8* ParentValueAddress, const UStruct* ExpectedBaseStructure) const
{
	if (!ensureVoxelSlow(ParentValueAddress))
	{
		return nullptr;
	}

	FVoxelStampRef& StampRef = *reinterpret_cast<FVoxelStampRef*>(ParentValueAddress);
	if (ExpectedBaseStructure &&
		StampRef.GetStruct() &&
		StampRef.GetStruct()->IsChildOf(ExpectedBaseStructure))
	{
		return static_cast<uint8*>(StampRef.GetStampData());
	}

	return nullptr;
}