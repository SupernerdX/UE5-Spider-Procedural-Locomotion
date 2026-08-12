// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinType.h"

#if WITH_EDITOR
struct VOXELGRAPH_API FVoxelPinTypeSet
{
public:
	static TMulticastDelegate<void(const FVoxelPinType&)> OnUserTypePreChange;
	static TMulticastDelegate<void(const FVoxelPinType&)> OnUserTypeChanged;

	static FVoxelPinTypeSet All();
	static FVoxelPinTypeSet AllUniforms();
	static FVoxelPinTypeSet AllBuffers();
	static FVoxelPinTypeSet AllBufferArrays();
	static FVoxelPinTypeSet AllEnums();
	static FVoxelPinTypeSet AllObjects();
	static FVoxelPinTypeSet AllParameters();
	static FVoxelPinTypeSet AllUserStructs();
	static const TVoxelSet<FVoxelPinType>& GetAllTypes();

public:
	FVoxelPinTypeSet() = default;

	bool IsInfinite() const
	{
		return bInfinite;
	}
	bool IsExplicit() const
	{
		return
			!bInfinite &&
			Selectors.Num() == 0;
	}
	const TVoxelSet<FVoxelPinType>& GetExplicitTypes() const
	{
		ensure(IsExplicit());
		return ExplicitTypes;
	}

public:
	void Add(const FVoxelPinType& Type);
	void Add(const FVoxelPinTypeSet& TypeSet);
	void Add(TConstVoxelArrayView<FVoxelPinType> InTypes);
	void Add(TFunction<bool(const FVoxelPinType& Type)> Selector);

	template<typename T>
	void Add()
	{
		this->Add(FVoxelPinType::Make<T>());
	}

	FVoxelPinTypeSet GetInnerTypes() const;
	FVoxelPinTypeSet GetBufferTypes() const;

	bool Contains(const FVoxelPinType& Type) const;

public:
	template<typename LambdaType>
	requires
	(
		LambdaHasSignature_V<LambdaType, void(const FVoxelPinType&)> ||
		LambdaHasSignature_V<LambdaType, EVoxelIterate(const FVoxelPinType&)>
	)
	EVoxelIterate Iterate(LambdaType Lambda) const
	{
		return this->IterateImpl([&](const FVoxelPinType& Type)
		{
			if constexpr (LambdaHasSignature_V<LambdaType, void(const FVoxelPinType&)>)
			{
				Lambda(Type);
				return EVoxelIterate::Continue;
			}
			else
			{
				return Lambda(Type);
			}
		});
	}

private:
	bool bInfinite = false;
	TVoxelSet<FVoxelPinType> ExplicitTypes;
	TVoxelArray<TFunction<bool(const FVoxelPinType& Type)>> Selectors;
	mutable TVoxelOptional<TVoxelArray<FVoxelPinType>> InlinedCachedTypes;

	EVoxelIterate IterateImpl(TFunctionRef<EVoxelIterate(const FVoxelPinType& Type)> Lambda) const;
};
#endif