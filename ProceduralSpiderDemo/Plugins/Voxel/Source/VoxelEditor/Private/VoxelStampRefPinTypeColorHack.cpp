// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelStampRef.h"
#include "UObject/UObjectThreadContext.h"

class UVoxelEdGraphSchema_K2_Hack : public UEdGraphSchema_K2
{
public:
	using UEdGraphSchema_K2::UEdGraphSchema_K2;

	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override
	{
		const FName TypeName = PinType.PinCategory;
		const UGraphEditorSettings* Settings = GetDefault<UGraphEditorSettings>();

		if (TypeName == PC_Struct)
		{
			if (const UStruct* Struct = Cast<UStruct>(PinType.PinSubCategoryObject))
			{
				if (Struct->IsChildOf(StaticStructFast<FVoxelStampRef>()))
				{
					return Settings->ObjectPinTypeColor;
				}
			}
		}

		return UEdGraphSchema_K2::GetPinTypeColor(PinType);
	}
};

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	UEdGraphSchema_K2* Object = GetMutableDefault<UEdGraphSchema_K2>();
	if (!ensure(Object))
	{
		return;
	}

	void*& VTable = *reinterpret_cast<void**>(Object);

	// Allocate object on zeroed memory to avoid issues in UObjectBase::~UObjectBase
	TVoxelStaticArray<uint8, sizeof(UVoxelEdGraphSchema_K2_Hack)> Memory{ ForceInit };

	TGuardValue<bool> Guard(GIsRetrievingVTablePtr, true);
	FVTableHelper Helper;
	UEdGraphSchema_K2* NewObject = ::new (Memory.GetData()) UVoxelEdGraphSchema_K2_Hack(Helper);

	ON_SCOPE_EXIT
	{
		NewObject->~UEdGraphSchema_K2();
	};

	VTable = *reinterpret_cast<void**>(NewObject);
}