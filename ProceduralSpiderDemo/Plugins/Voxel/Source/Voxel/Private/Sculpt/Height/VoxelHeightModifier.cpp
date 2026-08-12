// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelHeightModifier.h"

void FVoxelHeightSerializedModifier::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	if (Ar.IsLoading())
	{
		FString StructName;
		Ar << StructName;

		if (StructName.IsEmpty())
		{
			Modifier.Reset();
		}
		else
		{
			static const TVoxelMap<FName, const UScriptStruct*> NameToStruct = INLINE_LAMBDA
			{
				TVoxelMap<FName, const UScriptStruct*> Result;
				Result.Reserve(32);

				for (const UScriptStruct* Struct : GetDerivedStructs<FVoxelHeightModifier>())
				{
					Result.Add_EnsureNew(Struct->GetFName(), Struct);
				}

				return Result;
			};

			const UScriptStruct* Struct = NameToStruct.FindRef(FName(StructName));
			if (!ensure(Struct))
			{
				LOG_VOXEL(Error, "Failed to find modifier struct %s", *StructName);
				Ar.SetError();
				return;
			}

			Modifier = MakeSharedStruct<FVoxelHeightModifier>(Struct);
		}
	}
	else
	{
		FString StructName;
		if (Modifier)
		{
			StructName = Modifier->GetStruct()->GetName();
		}
		Ar << StructName;
	}

	if (!Modifier)
	{
		return;
	}

	Modifier->GetStruct()->SerializeItem(
		Ar,
		Modifier.Get(),
		nullptr);
}