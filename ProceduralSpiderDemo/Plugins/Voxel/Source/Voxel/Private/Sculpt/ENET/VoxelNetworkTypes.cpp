// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNetworkTypes.h"

namespace Voxel::Network
{

#if VOXEL_DEBUG
DEFINE_VOXEL_INSTANCE_COUNTER(INode);
#endif

TVoxelOptional<FTime> GTimeOverride;

void FTime::SetNow_TestOnly(const TVoxelOptional<FTime> NewTime)
{
	GTimeOverride = NewTime;
}

FTime FTime::Now()
{
	if (GTimeOverride)
	{
		return GTimeOverride.GetValue();
	}

	// Makes errors obvious & ensure comparing to Null is always much newer
	constexpr int32 Offset = 100000000;
	static const double BaseTime = FPlatformTime::Seconds();

	const double Time = FPlatformTime::Seconds() - BaseTime;
	const int64 Milliseconds = Offset + FMath::RoundToInt64(Time * 1000);

	ensure(Milliseconds <= MAX_int32);

	FTime Result;
	Result.Milliseconds = Milliseconds;
	return Result;
}

}