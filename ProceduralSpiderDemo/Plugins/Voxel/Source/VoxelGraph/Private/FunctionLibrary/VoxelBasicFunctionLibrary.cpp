// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "FunctionLibrary/VoxelBasicFunctionLibrary.h"
#include "VoxelGraphMigration.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	REGISTER_VOXEL_FUNCTION_MIGRATION("GetQueryToWorld", UVoxelBasicFunctionLibrary, GetLocalToWorldTransform);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSeedBuffer UVoxelBasicFunctionLibrary::HashPosition(
	const FVoxelVectorBuffer& Position,
	const FVoxelSeed& Seed,
	const int32 RoundingDecimals) const
{
	const uint64 SeedHash = FVoxelUtilities::MurmurHash64(Seed);
	const int32 RoundValue = FVoxelUtilities::IntPow(10, FMath::Clamp(RoundingDecimals, 0, 8));

	FVoxelSeedBuffer Seeds;
	Seeds.Allocate(Position.Num());

	for (int32 Index = 0; Index < Position.Num(); Index++)
	{
		const FVector3f RoundedPosition = FVoxelUtilities::RoundToFloat(Position[Index] * RoundValue) / RoundValue;
		Seeds.Set(Index, FVoxelSeed(FVoxelUtilities::MurmurHash64(SeedHash ^ FVoxelUtilities::MurmurHash(RoundedPosition))));
	}

	return Seeds;
}

FVoxelVectorBuffer UVoxelBasicFunctionLibrary::RandUnitVector(const FVoxelSeedBuffer& Seed) const
{
	FVoxelVectorBuffer Result;
	Result.Allocate(Seed.Num());

	for (int32 Index = 0; Index < Seed.Num(); Index++)
	{
		const FRandomStream RandomStream(Seed[Index]);

		Result.Set(Index, FVector3f(RandomStream.GetUnitVector()));
	}

	return Result;
}

int32 UVoxelBasicFunctionLibrary::GetLOD() const
{
	const FVoxelGraphParameters::FLOD* Parameter = Query->FindParameter<FVoxelGraphParameters::FLOD>();
	if (!Parameter)
	{
		// Default to 0
		return 0;
	}

	return Parameter->Value;
}

int32 UVoxelBasicFunctionLibrary::GetLODStep() const
{
	return 1 << GetLOD();
}

bool UVoxelBasicFunctionLibrary::IsPreviewScene() const
{
	return Query.IsPreview();
}

ECollisionEnabled::Type UVoxelBasicFunctionLibrary::GetCollisionEnabled(const FBodyInstance& BodyInstance) const
{
	return BodyInstance.GetCollisionEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FTransform UVoxelBasicFunctionLibrary::GetLocalToWorldTransform() const
{
	return Query->Context.Environment.LocalToWorld;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelRuntimePinValue UVoxelBasicFunctionLibrary::ToBuffer(const FVoxelRuntimePinValue& Value) const
{
	return FVoxelRuntimePinValue::Make(FVoxelBuffer::MakeConstant(Value));
}