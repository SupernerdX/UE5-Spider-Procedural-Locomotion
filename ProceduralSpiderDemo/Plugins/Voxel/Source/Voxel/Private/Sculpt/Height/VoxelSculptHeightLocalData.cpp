// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelSculptHeightLocalData.h"
#include "Sculpt/Height/VoxelSculptHeight.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"
#include "Sculpt/Height/VoxelSculptHeightContext.h"
#include "Sculpt/Height/VoxelHeightModifier.h"
#include "Sculpt/Height/VoxelSculptHeightAsset.h"
#include "Bulk/VoxelDummyBulkLoader.h"
#include "Bulk/VoxelBulkPtr.h"

FVoxelSculptHeightLocalData::FVoxelSculptHeightLocalData(AVoxelSculptHeight& SculptActor)
	: IVoxelSculptHeightDataSource(SculptActor)
	, BulkLoader(MakeShared<FVoxelDummyBulkLoader>())
{
}

FVoxelSculptHeightLocalData::~FVoxelSculptHeightLocalData()
{
	VOXEL_FUNCTION_COUNTER();

	// Ensure all promises are fired
	ClearQueue();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<IVoxelBulkLoader> FVoxelSculptHeightLocalData::GetBulkLoader() const
{
	return BulkLoader;
}

void FVoxelSculptHeightLocalData::SetSculptData(
	const TVoxelBulkRef<FVoxelSculptHeightData>& NewData,
	const TSharedRef<IVoxelBulkLoader>& NewBulkLoader)
{
	if (GetData().GetHash() == NewData.GetHash() &&
		BulkLoader == NewBulkLoader)
	{
		return;
	}

	VOXEL_FUNCTION_COUNTER();

	ClearQueue();

	BulkLoader = NewBulkLoader;

	if (!WeakAsset.IsExplicitlyNull() &&
		IsEditor())
	{
		// Don't edit assets in PIE

		if (UVoxelSculptHeightAsset* Asset = WeakAsset.Resolve_Ensured())
		{
			Asset->SetData(NewData);
		}
	}

	SetData(NewData);
}

FVoxelFuture FVoxelSculptHeightLocalData::ApplyModifier(const TSharedRef<FVoxelHeightModifier>& Modifier)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TSharedPtr<IVoxelHeightModifierRuntime> ModifierRuntime = Modifier->GetRuntime();
	if (!ensureVoxelSlow(ModifierRuntime))
	{
		return {};
	}

	FVoxelPromise Promise;

	QueuedModifiers.Add(FQueuedModifier
	{
		Promise,
		ModifierRuntime
	});

	ProcessQueue();

	return Promise;
}

TSharedPtr<FVoxelSculptHeightLocalData> FVoxelSculptHeightLocalData::AsLocalData()
{
	return AsShared();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptHeightLocalData::SetAsset(UVoxelSculptHeightAsset* Asset)
{
	if (Asset == WeakAsset)
	{
		return;
	}

	VOXEL_FUNCTION_COUNTER();

	ClearQueue();

	OnAssetDataChangedPtr = MakeSharedVoid();
	WeakAsset = Asset;

	if (Asset)
	{
		if (Asset->GetData()->IsEmpty())
		{
			// Replace the asset data with ours
			const TVoxelBulkRef<FVoxelSculptHeightData> Data = GetData();
			Data.FullyLoadSync(*BulkLoader);
			Asset->SetData(Data);
		}

		BulkLoader = Asset->GetBulkLoader();
		SetData(Asset->GetData());

		Asset->OnDataChanged.Add(MakeWeakPtrDelegate(OnAssetDataChangedPtr, MakeWeakPtrLambda(this, [this](const TVoxelBulkRef<FVoxelSculptHeightData>& NewData)
		{
			SetData(NewData);
		})));
	}
	else
	{
		const TVoxelBulkRef<FVoxelSculptHeightData> Data = GetData();
		Data.FullyLoadSync(*BulkLoader);

		BulkLoader = MakeShared<FVoxelDummyBulkLoader>();
		SetData(Data);
	}
}

TSharedRef<IVoxelSculptHeightDataSource> FVoxelSculptHeightLocalData::Duplicate(AVoxelSculptHeight& SculptActor) const
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelSculptHeightLocalData> Result = MakeShared<FVoxelSculptHeightLocalData>(SculptActor);
	Result->BulkLoader = BulkLoader;
	Result->SetData(GetData());
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptHeightLocalData::ClearQueue()
{
	VOXEL_FUNCTION_COUNTER();

	if (PendingModifier)
	{
		PendingModifier->Promise.Set();
	}
	PendingModifier.Reset();

	for (const FQueuedModifier& QueuedModifier : QueuedModifiers)
	{
		QueuedModifier.Promise.Set();
	}
	QueuedModifiers.Empty();
}

void FVoxelSculptHeightLocalData::ProcessQueue()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	while (true)
	{
		if (!ProcessQueue_ShouldContinue())
		{
			break;
		}
	}
}

bool FVoxelSculptHeightLocalData::ProcessQueue_ShouldContinue()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (PendingModifier)
	{
		if (!PendingModifier->Future.IsComplete())
		{
			return false;
		}

		const TVoxelBulkRef<FVoxelSculptHeightData> NewData = MakeVoxelBulkRef(PendingModifier->Future.GetSharedValueChecked());

		if (IsEditor())
		{
			// Don't edit assets in PIE

			if (UVoxelSculptHeightAsset* Asset = WeakAsset.Resolve_Ensured())
			{
				Asset->SetData(NewData);
			}
		}

		SetData(NewData);

		PendingModifier->Promise.Set();
		PendingModifier.Reset();
		return true;
	}

	if (QueuedModifiers.Num() > 0)
	{
		const TVoxelOptional<FVoxelSculptHeightContext> Context = GetContext();
		if (!ensure(Context))
		{
			return false;
		}

		const FQueuedModifier QueuedModifier = MoveTemp(QueuedModifiers[0]);
		QueuedModifiers.RemoveAt(0);

		const TVoxelFuture<const FVoxelSculptHeightData> Future = GetData()->ApplyModifier(
			BulkLoader,
			*Context,
			GetData().GetHash(),
			QueuedModifier.ModifierRuntime.ToSharedRef());

		ensure(!PendingModifier);
		PendingModifier.Emplace(FPendingModifier
		{
			QueuedModifier.Promise,
			Future
		});

		FVoxelFuture(Future).Then_GameThread(MakeWeakPtrLambda(this, [this]
		{
			ProcessQueue();
		}));

		return true;
	}

	return false;
}