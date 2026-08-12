// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPinValueOps.h"

class VOXELGRAPH_API FVoxelPinValueOpsManager : public FVoxelSingleton
{
public:
	TVoxelMap<FVoxelPinType, TSharedPtr<FVoxelPinValueOps>> RuntimeTypeToOps;

	//~ Begin FVoxelSingleton Interface
	virtual void Initialize() override
	{
		VOXEL_FUNCTION_COUNTER();

		const TVoxelArray<UScriptStruct*> Structs = GetDerivedStructs<FVoxelPinValueOps>();

		RuntimeTypeToOps.Reserve(Structs.Num());

		for (const UScriptStruct* Struct : Structs)
		{
			FString Name = Struct->GetStructCPPName();
			verify(Name.RemoveFromStart("FVoxelPinValueOps_"));
			verify(Name.RemoveFromStart("F"));

			FString Path = Struct->GetPackage()->GetName() + "." + Name;

			UScriptStruct* TargetStruct = FindObject<UScriptStruct>(nullptr, *Path);
			if (!TargetStruct)
			{
				// Hack for FVoxelOctahedron
				Path.ReplaceInline(TEXT("VoxelGraph"), TEXT("VoxelCore"));

				TargetStruct = FindObject<UScriptStruct>(nullptr, *Path);
			}
			check(TargetStruct);

			const TSharedRef<FVoxelPinValueOps> Ops = MakeSharedStruct<FVoxelPinValueOps>(Struct);
			const EVoxelPinValueOpsUsage Usage = Ops->GetUsage();

			if (EnumHasAllFlags(Usage, EVoxelPinValueOpsUsage::Fixup))
			{
				// HasFixup should be declared on the exposed type itself, not on the runtime type,
				// because FVoxelPinValueBase::Fixup only knows the exposed type
				ensure(!EnumHasAnyFlags(Usage, EVoxelPinValueOpsUsage::GetExposedType));
				ensure(!EnumHasAnyFlags(Usage, EVoxelPinValueOpsUsage::MakeRuntimeValue));
				ensure(!EnumHasAnyFlags(Usage, EVoxelPinValueOpsUsage::HasPinDefaultValue));
			}

			RuntimeTypeToOps.Add_EnsureNew(
				FVoxelPinType::MakeStruct(TargetStruct),
				Ops);
		}
	}
	//~ End FVoxelSingleton Interface
};
FVoxelPinValueOpsManager* GVoxelPinValueOpsManager = new FVoxelPinValueOpsManager();

TSharedPtr<FVoxelPinValueOps> FVoxelPinValueOps::Find(
	const FVoxelPinType& RuntimeType,
	const EVoxelPinValueOpsUsage Usage)
{
	const TSharedPtr<FVoxelPinValueOps> Ops = GVoxelPinValueOpsManager->RuntimeTypeToOps.FindRef(RuntimeType);
	if (!Ops ||
		!EnumHasAllFlags(Ops->GetUsage(), Usage))
	{
		return {};
	}

	return Ops;
}