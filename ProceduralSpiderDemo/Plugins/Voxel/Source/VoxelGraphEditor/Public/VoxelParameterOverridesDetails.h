// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelParameterOverridesOwner.h"

class FVoxelParameterView;
class FVoxelCategoryBuilder;
class FVoxelGraphEnvironment;
class FVoxelParameterDetails;
class FVoxelGraphParametersView;
class FVoxelParameterDetailsParameterProvider;
struct FVoxelParameterOverrides;

struct VOXELGRAPHEDITOR_API FVoxelParameterDetailsBuilder
{
	FVoxelParameterDetailsBuilder(IDetailLayoutBuilder& LayoutBuilder)
		: LayoutBuilder(&LayoutBuilder)
	{}
	FVoxelParameterDetailsBuilder(IDetailChildrenBuilder& ChildrenBuilder)
		: ChildrenBuilder(&ChildrenBuilder)
	{}

	bool IsLayoutBuilder() const
	{
		if (LayoutBuilder)
		{
			check(!ChildrenBuilder);
			return true;
		}
		else
		{
			check(ChildrenBuilder);
			return false;
		}
	}
	IDetailsView* GetDetailsView() const
	{
		if (IsLayoutBuilder())
		{
			return LayoutBuilder->UE_506_SWITCH(GetDetailsView, GetDetailsViewSharedPtr().Get)();
		}
		else
		{
			return ChildrenBuilder->GetParentCategory().GetParentLayout().UE_506_SWITCH(GetDetailsView, GetDetailsViewSharedPtr().Get)();
		}
	}
	IDetailLayoutBuilder& GetLayoutBuilder() const
	{
		return *LayoutBuilder;
	}
	IDetailChildrenBuilder& GetChildrenBuilder() const
	{
		return *ChildrenBuilder;
	}

private:
	IDetailLayoutBuilder* LayoutBuilder = nullptr;
	IDetailChildrenBuilder* ChildrenBuilder = nullptr;
};

class VOXELGRAPHEDITOR_API FVoxelParameterOverridesDetails
	: public FVoxelTicker
	, public TSharedFromThis<FVoxelParameterOverridesDetails>
{
public:
	using FGetHash = TFunction<uint64()>;
	using FMakeEnvironment = TFunction<TSharedPtr<FVoxelGraphEnvironment>(FVoxelDependencyCollector&)>;

	struct FWeakOwner
	{
		FVoxelParameterOverridesOwnerPtr OwnerPtr;
		FGetHash GetHash;
		FMakeEnvironment MakeEnvironment;
		TFunction<void()> PreEditChange;
		TFunction<void()> PostEditChange;
		FString PropertyChain;

		FWeakOwner() = default;
		FWeakOwner(
			const FVoxelParameterOverridesOwnerPtr& OwnerPtr,
			const FGetHash& GetHash,
			const FMakeEnvironment& MakeEnvironment,
			const TFunction<void()>& PreEditChange,
			const TFunction<void()>& PostEditChange)
			: OwnerPtr(OwnerPtr)
			, GetHash(GetHash)
			, MakeEnvironment(MakeEnvironment)
			, PreEditChange(PreEditChange)
			, PostEditChange(PostEditChange)
		{
		}
		FWeakOwner(
			const FVoxelParameterOverridesOwnerPtr& OwnerPtr,
			const FGetHash& GetHash,
			const FMakeEnvironment& MakeEnvironment,
			const TSharedRef<IPropertyHandle>& PropertyHandle,
			const FString& PropertyChain)
			: OwnerPtr(OwnerPtr)
			, GetHash(GetHash)
			, MakeEnvironment(MakeEnvironment)
			, PreEditChange([=]
			{
				PropertyHandle->NotifyPreChange();
			})
			, PostEditChange([=]
			{
				PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
			})
			, PropertyChain(PropertyChain)
		{
		}
	};
	struct FOwner
	{
		IVoxelParameterOverridesOwner& Parameters;
		TFunction<void()> PreEditChange;
		TFunction<void()> PostEditChange;
		const FString& PropertyChain;
	};

public:
	static TSharedPtr<FVoxelParameterOverridesDetails> Create(
		const FVoxelParameterDetailsBuilder& DetailLayout,
		TFunctionRef<void(TVoxelArray<FWeakOwner>&)> GatherOwners,
		const FSimpleDelegate& RefreshDelegate,
		const FString& BaseCategory = {})
	{
		return CreateImpl(
			DetailLayout,
			GatherOwners,
			RefreshDelegate,
			BaseCategory,
			{});
	}
	static TSharedPtr<FVoxelParameterOverridesDetails> CreateForParameter(
		const FVoxelParameterDetailsBuilder& DetailLayout,
		TFunctionRef<void(TVoxelArray<FWeakOwner>&)> GatherOwners,
		const FSimpleDelegate& RefreshDelegate,
		const FGuid& ParameterDefaultValueGuid)
	{
		return CreateImpl(
			DetailLayout,
			GatherOwners,
			RefreshDelegate,
			{},
			ParameterDefaultValueGuid);
	}

private:
	static TSharedPtr<FVoxelParameterOverridesDetails> CreateImpl(
		const FVoxelParameterDetailsBuilder& DetailLayout,
		TFunctionRef<void(TVoxelArray<FWeakOwner>&)> GatherOwners,
		const FSimpleDelegate& RefreshDelegate,
		const FString& BaseCategory,
		TOptional<FGuid> ParameterDefaultValueGuid);

public:
	VOXEL_COUNT_INSTANCES();

	//~ Begin FVoxelTicker Interface
	virtual void Tick() override;
	//~ End FVoxelTicker Interface

public:
	TVoxelArray<FOwner> GetOwners() const;

	void ForceRefresh() const
	{
		ensure(RefreshDelegate.ExecuteIfBound());
	}

	void GenerateView(
		const TVoxelArray<FVoxelParameterView*>& ParameterViews,
		const FVoxelDetailInterface& DetailInterface);

	void AddOrphans(
		FVoxelCategoryBuilder& CategoryBuilder,
		const FString& BaseCategory);

private:
	const TVoxelArray<FWeakOwner> Owners;
	const FSimpleDelegate RefreshDelegate;
	const TOptional<FGuid> ParameterDefaultValueGuid;

	TWeakPtr<SWidget> WeakDetailsView;
	bool bWindowValid = false;

	TVoxelArray<TVoxelObjectPtr<const UVoxelGraph>> Graphs;
	TVoxelArray<TSharedPtr<FVoxelGraphParametersView>> GraphParametersViews;
	TVoxelMap<FGuid, TSharedPtr<FVoxelParameterDetails>> GuidToParameterDetails;

	struct FCachedEnvironment
	{
		TSharedPtr<FVoxelGraphEnvironment> Environment;
		uint64 Hash = 0;
		FGetHash GetHash;
		FMakeEnvironment MakeEnvironment;
	};
	TVoxelArray<FCachedEnvironment> CachedEnvironments;
	TSharedPtr<FVoxelDependencyTracker> DependencyTracker;

private:
	FVoxelParameterOverridesDetails(
		const TVoxelArray<FWeakOwner>& Owners,
		const FSimpleDelegate& RefreshDelegate,
		const TOptional<FGuid> ParameterDefaultValueGuid)
		: Owners(Owners)
		, RefreshDelegate(RefreshDelegate)
		, ParameterDefaultValueGuid(ParameterDefaultValueGuid)
	{
	}

	void Initialize(
		const FVoxelParameterDetailsBuilder& DetailLayout,
		const FString& BaseCategory);

	bool IsWindowValid() const;
	void UpdateEnvironments();
};