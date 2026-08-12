// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphParametersView.h"
#include "VoxelGraph.h"
#include "VoxelParameterView.h"
#include "VoxelGraphParametersViewContext.h"

TVoxelArray<TVoxelArray<FVoxelParameterView*>> FVoxelGraphParametersView::GetCommonChildren(const TConstVoxelArrayView<TSharedPtr<FVoxelGraphParametersView>> ParametersViews)
{
	VOXEL_FUNCTION_COUNTER();

	if (ParametersViews.Num() == 0)
	{
		return {};
	}

	bool bGuidsSet = false;
	TVoxelSet<FGuid> Guids;
	for (const TSharedPtr<FVoxelGraphParametersView>& ParametersView : ParametersViews)
	{
		const TConstVoxelArrayView<FVoxelParameterView*> Children = ParametersView->GetChildren();

		TVoxelSet<FGuid> NewGuids;
		NewGuids.Reserve(Children.Num());

		for (const FVoxelParameterView* Child : Children)
		{
			ensure(!NewGuids.Contains(Child->Guid));
			NewGuids.Add(Child->Guid);
		}

		if (bGuidsSet)
		{
			Guids = Guids.Intersect(NewGuids);
		}
		else
		{
			bGuidsSet = true;
			Guids = MoveTemp(NewGuids);
		}
	}

	const TVoxelArray<FGuid> GuidArray = Guids.Array();

	TVoxelArray<TVoxelArray<FVoxelParameterView*>> Result;
	Result.SetNum(GuidArray.Num());

	for (const TSharedPtr<FVoxelGraphParametersView>& ParametersView : ParametersViews)
	{
		for (int32 Index = 0; Index < GuidArray.Num(); Index++)
		{
			const FGuid Guid = GuidArray[Index];

			FVoxelParameterView* ChildParameterView = ParametersView->FindByGuid(Guid);
			if (!ensure(ChildParameterView))
			{
				return {};
			}

			Result[Index].Add(ChildParameterView);
		}
	}

	return Result;
}

FVoxelGraphParametersView::FVoxelGraphParametersView()
	: Context(MakeShared<FVoxelGraphParametersViewContext>())
{
}