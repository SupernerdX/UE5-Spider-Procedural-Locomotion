// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelSculptVolumeLocalData.h"
#include "Bulk/VoxelDummyBulkLoader.h"
#include "Sculpt/Volume/VoxelSculptVolumeAsset.h"
#include "Sculpt/Volume/VoxelVolumeModifier.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"

FVoxelSculptVolumeLocalData::FVoxelSculptVolumeLocalData(AVoxelSculptVolume& SculptActor)
	: IVoxelSculptVolumeDataSource(SculptActor)
	, BulkLoader(MakeShared<FVoxelDummyBulkLoader>())
{
}

FVoxelSculptVolumeLocalData::~FVoxelSculptVolumeLocalData()
{
	VOXEL_FUNCTION_COUNTER();

	// Ensure all promises are fired
	ClearQueue();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<IVoxelBulkLoader> FVoxelSculptVolumeLocalData::GetBulkLoader() const
{
	return BulkLoader;
}

void FVoxelSculptVolumeLocalData::SetSculptData(
	const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData,
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

		if (UVoxelSculptVolumeAsset* Asset = WeakAsset.Resolve_Ensured())
		{
			Asset->SetData(NewData);
		}
	}

	SetData(NewData);
}

FVoxelFuture FVoxelSculptVolumeLocalData::ApplyModifier(const TSharedRef<FVoxelVolumeModifier>& Modifier)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TSharedPtr<IVoxelVolumeModifierRuntime> ModifierRuntime = Modifier->GetRuntime();
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

TSharedPtr<FVoxelSculptVolumeLocalData> FVoxelSculptVolumeLocalData::AsLocalData()
{
	return AsShared();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptVolumeLocalData::SetAsset(UVoxelSculptVolumeAsset* Asset)
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
			const TVoxelBulkRef<FVoxelSculptVolumeData> Data = GetData();
			Data.FullyLoadSync(*BulkLoader);
			Asset->SetData(Data);
		}

		BulkLoader = Asset->GetBulkLoader();
		SetData(Asset->GetData());

		Asset->OnDataChanged.Add(MakeWeakPtrDelegate(OnAssetDataChangedPtr, MakeWeakPtrLambda(this, [this](const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData)
		{
			SetData(NewData);
		})));
	}
	else
	{
		const TVoxelBulkRef<FVoxelSculptVolumeData> Data = GetData();
		Data.FullyLoadSync(*BulkLoader);

		BulkLoader = MakeShared<FVoxelDummyBulkLoader>();
		SetData(Data);
	}
}

TSharedRef<IVoxelSculptVolumeDataSource> FVoxelSculptVolumeLocalData::Duplicate(AVoxelSculptVolume& SculptActor) const
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelSculptVolumeLocalData> Result = MakeShared<FVoxelSculptVolumeLocalData>(SculptActor);
	Result->BulkLoader = BulkLoader;
	Result->SetData(GetData());
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptVolumeLocalData::ClearQueue()
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

void FVoxelSculptVolumeLocalData::ProcessQueue()
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

bool FVoxelSculptVolumeLocalData::ProcessQueue_ShouldContinue()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (PendingModifier)
	{
		if (!PendingModifier->Future.IsComplete())
		{
			return false;
		}

		const TVoxelBulkRef<FVoxelSculptVolumeData> NewData = MakeVoxelBulkRef(PendingModifier->Future.GetSharedValueChecked());

		if (IsEditor())
		{
			// Don't edit assets in PIE

			if (UVoxelSculptVolumeAsset* Asset = WeakAsset.Resolve_Ensured())
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
		const TVoxelOptional<FVoxelSculptVolumeContext> Context = GetContext();
		if (!ensure(Context))
		{
			return false;
		}

		const FQueuedModifier QueuedModifier = MoveTemp(QueuedModifiers[0]);
		QueuedModifiers.RemoveAt(0);

		const TVoxelFuture<const FVoxelSculptVolumeData> Future = GetData()->ApplyModifier(
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