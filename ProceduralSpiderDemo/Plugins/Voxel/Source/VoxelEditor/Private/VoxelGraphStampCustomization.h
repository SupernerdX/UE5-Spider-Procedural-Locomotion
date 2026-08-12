// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelStampRef.h"
#include "VoxelStampRuntime.h"
#include "VoxelStampCustomization.h"
#include "VoxelParameterOverridesDetails.h"

template<typename T>
class TVoxelGraphStampCustomization : public FVoxelStampCustomization
{
public:
	virtual void CustomizeChildren(
		const TSharedRef<IPropertyHandle> PropertyHandle,
		IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		FVoxelStampCustomization::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);

		const auto GatherOwners = [&](TVoxelArray<FVoxelParameterOverridesDetails::FWeakOwner>& OutOwners)
		{
			TMap<const FProperty*, TSharedPtr<IPropertyHandle>> PropertyToHandle;
			FVoxelEditorUtilities::ForeachData<T>(PropertyHandle, [&](T& Stamp)
			{
				FString PropertyChain;
				if (const FProperty* ParameterOverridesProperty = Stamp.GetParameterOverridesProperty())
				{
					PropertyChain = FVoxelEditorUtilities::GeneratePropertyChainPart(ParameterOverridesProperty, -1);
					PropertyChain += FVoxelEditorUtilities::GeneratePropertyChain(PropertyHandle);
				}

				OutOwners.Add(FVoxelParameterOverridesDetails::FWeakOwner
					{
						&Stamp,
						[StampRef = Stamp.GetStampRef()]() -> uint64
						{
							const TSharedPtr<const FVoxelStampRuntime> Runtime = StampRef.ResolveStampRuntime();
							if (!Runtime)
							{
								return 0;
							}
							return Runtime->GetPropertyHash();
						},
						[StampRef = Stamp.GetStampRef()](FVoxelDependencyCollector& DependencyCollector) -> TSharedPtr<FVoxelGraphEnvironment>
						{
							const TSharedPtr<const FVoxelStampRuntime> Runtime = StampRef.ResolveStampRuntime();
							if (!Runtime)
							{
								return nullptr;
							}
							return StampRef.template AsChecked<T>().CreateEnvironment(*Runtime, DependencyCollector);
						},
						PropertyHandle,
						PropertyChain,
					});
			});
		};

		KeepAlive(FVoxelParameterOverridesDetails::Create(
			ChildBuilder,
			GatherOwners,
			FVoxelEditorUtilities::MakeRefreshDelegate(this, CustomizationUtils),
			"Parameters"));
	}
};