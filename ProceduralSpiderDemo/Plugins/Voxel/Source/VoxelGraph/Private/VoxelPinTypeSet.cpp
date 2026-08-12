// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPinTypeSet.h"
#include "VoxelPinTypeSetRegistry.h"

#if WITH_EDITOR
TMulticastDelegate<void(const FVoxelPinType&)> FVoxelPinTypeSet::OnUserTypePreChange;
TMulticastDelegate<void(const FVoxelPinType&)> FVoxelPinTypeSet::OnUserTypeChanged;
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelPinTypeSet::All()
{
	FVoxelPinTypeSet Set;
	Set.bInfinite = true;
	return Set;
}

FVoxelPinTypeSet FVoxelPinTypeSet::AllUniforms()
{
	FVoxelPinTypeSet Set;
	Set.Selectors.Add([](const FVoxelPinType& Type)
	{
		return !Type.IsBuffer();
	});
	return Set;
}

FVoxelPinTypeSet FVoxelPinTypeSet::AllBuffers()
{
	FVoxelPinTypeSet Set;
	Set.Selectors.Add([](const FVoxelPinType& Type)
	{
		return
			Type.IsBuffer() &&
			!Type.IsBufferArray();
	});
	return Set;
}

FVoxelPinTypeSet FVoxelPinTypeSet::AllBufferArrays()
{
	FVoxelPinTypeSet Set;
	Set.Selectors.Add([](const FVoxelPinType& Type)
	{
		return Type.IsBufferArray();
	});
	return Set;
}

FVoxelPinTypeSet FVoxelPinTypeSet::AllEnums()
{
	FVoxelPinTypeSet Set;
	Set.Selectors.Add([](const FVoxelPinType& Type)
	{
		return Type.Is<uint8>() && Type.GetEnum();
	});
	return Set;
}

FVoxelPinTypeSet FVoxelPinTypeSet::AllObjects()
{
	FVoxelPinTypeSet Set;
	Set.Selectors.Add([](const FVoxelPinType& Type)
	{
		return Type.GetExposedType().IsObject();
	});
	return Set;
}

FVoxelPinTypeSet FVoxelPinTypeSet::AllParameters()
{
	// Parameters are either uniforms or arrays
	FVoxelPinTypeSet Result;
	Result.Add(AllUniforms());
	Result.Add(AllBufferArrays());
	return Result;
}

FVoxelPinTypeSet FVoxelPinTypeSet::AllUserStructs()
{
	FVoxelPinTypeSet Set;
	Set.Selectors.Add([](const FVoxelPinType& Type)
	{
		return Type.IsUserDefinedStruct();
	});
	return Set;
}

const TVoxelSet<FVoxelPinType>& FVoxelPinTypeSet::GetAllTypes()
{
	return GVoxelPinTypeSetRegistry->GetTypes();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void FVoxelPinTypeSet::Add(const FVoxelPinType& Type)
{
	ExplicitTypes.Add(Type);
	InlinedCachedTypes.Reset();
}

void FVoxelPinTypeSet::Add(const FVoxelPinTypeSet& TypeSet)
{
	bInfinite |= TypeSet.bInfinite;
	ExplicitTypes.Append(TypeSet.ExplicitTypes);
	Selectors.Append(TypeSet.Selectors);
	InlinedCachedTypes.Reset();
}

void FVoxelPinTypeSet::Add(const TConstVoxelArrayView<FVoxelPinType> InTypes)
{
	ExplicitTypes.Append(InTypes);
	InlinedCachedTypes.Reset();
}

void FVoxelPinTypeSet::Add(TFunction<bool(const FVoxelPinType& Type)> Selector)
{
	Selectors.Add(MoveTemp(Selector));
	InlinedCachedTypes.Reset();
}

FVoxelPinTypeSet FVoxelPinTypeSet::GetInnerTypes() const
{
	FVoxelPinTypeSet Result;
	Result.Selectors.Add([This = *this](const FVoxelPinType& Type)
	{
		if (Type.IsBuffer())
		{
			return false;
		}

		return
			This.Contains(Type) ||
			This.Contains(Type.GetBufferType());
	});
	return Result;
}

FVoxelPinTypeSet FVoxelPinTypeSet::GetBufferTypes() const
{
	FVoxelPinTypeSet Result;
	Result.Selectors.Add([This = *this](const FVoxelPinType& Type)
	{
		if (!Type.IsBuffer())
		{
			return false;
		}

		return
			This.Contains(Type) ||
			This.Contains(Type.GetInnerType());
	});
	return Result;
}

bool FVoxelPinTypeSet::Contains(const FVoxelPinType& Type) const
{
	if (bInfinite)
	{
		return true;
	}

	if (ExplicitTypes.Contains(Type))
	{
		return true;
	}

	for (const TFunction<bool(const FVoxelPinType& Type)>& Selector : Selectors)
	{
		if (Selector(Type))
		{
			return true;
		}
	}

	return false;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
EVoxelIterate FVoxelPinTypeSet::IterateImpl(const TFunctionRef<EVoxelIterate(const FVoxelPinType& Type)> Lambda) const
{
	VOXEL_FUNCTION_COUNTER();

	if (bInfinite)
	{
		for (const FVoxelPinType& Type : GVoxelPinTypeSetRegistry->GetTypes())
		{
			if (Lambda(Type) == EVoxelIterate::Stop)
			{
				return EVoxelIterate::Stop;
			}
		}

		return EVoxelIterate::Continue;
	}

	if (IsExplicit())
	{
		for (const FVoxelPinType& Type : GetExplicitTypes())
		{
			if (Lambda(Type) == EVoxelIterate::Stop)
			{
				return EVoxelIterate::Stop;
			}
		}

		return EVoxelIterate::Continue;
	}

	if (!InlinedCachedTypes.IsSet())
	{
		TVoxelArray<FVoxelPinType> Types;
		for (const FVoxelPinType& Type : GVoxelPinTypeSetRegistry->GetTypes())
		{
			if (Contains(Type))
			{
				Types.Add(Type);
			}
		}

		InlinedCachedTypes = Types;
	}

	for (const FVoxelPinType& Type : InlinedCachedTypes.GetValue())
	{
		if (Lambda(Type) == EVoxelIterate::Stop)
		{
			return EVoxelIterate::Stop;
		}
	}

	return EVoxelIterate::Continue;
}
#endif