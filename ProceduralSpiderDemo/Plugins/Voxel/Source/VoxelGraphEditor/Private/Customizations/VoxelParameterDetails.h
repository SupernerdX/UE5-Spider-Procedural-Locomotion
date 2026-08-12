// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelPinType.h"

class FVoxelParameterView;
class FVoxelGraphEnvironment;
class FVoxelParameterOverridesDetails;

class FVoxelParameterDetails : public TSharedFromThis<FVoxelParameterDetails>
{
public:
	FVoxelParameterOverridesDetails& OverridesDetails;
	const FGuid Guid;
	const TVoxelArray<FVoxelParameterView*> ParameterViews;
	FName OrphanName;

	FVoxelPinType RowExposedType;
	FVoxelPinType RowType;

	FVoxelParameterDetails(
		FVoxelParameterOverridesDetails& ContainerDetail,
		const FGuid& Guid,
		const TVoxelArray<FVoxelParameterView*>& ParameterViews);

	void InitializeOrphan(const TArray<FVoxelPinValue>& Values);

	void ComputeEditorGraphs(
		FVoxelDependencyCollector& DependencyCollector,
		const TVoxelArray<TSharedRef<FVoxelGraphEnvironment>>& Environments);

public:
	bool IsValid() const
	{
		return
			ensure(PropertyHandle) &&
			PropertyHandle->IsValidHandle();
	}
	bool IsOrphan() const
	{
		return ParameterViews.Num() == 0;
	}

public:
	void Tick();

public:
	void MakeRow(const FVoxelDetailInterface& DetailInterface);

	void BuildRow(
		const FVoxelDetailInterface& DetailInterface,
		FDetailWidgetRow& Row,
		const TSharedRef<SWidget>& ValueWidget);

public:
	ECheckBoxState IsEnabled() const;
	void SetEnabled(bool bNewEnabled) const;

public:
	bool CanResetToDefault() const;
	void ResetToDefault();

public:
	void PreEditChange() const;
	void PostEditChange() const;

private:
	FString GetRowName() const;
	TSharedPtr<TStructOnScope<FVoxelPinValue>> GetEmptyValue(int32 Index) const;

private:
	mutable TArray<TSharedPtr<TStructOnScope<FVoxelPinValue>>> EmptyStructOnScopes;
	TArray<TSharedPtr<TStructOnScope<FVoxelPinValue>>> OrphanStructOnScopes;
	TSharedPtr<IStructureDataProvider> ParameterDetailsProvider;

	double LastSyncTime = 0.;
	bool bForceEnableOverride = false;
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<FVoxelInstancedStructDetailsWrapper> StructWrapper;

	bool bIsVisible = true;
	bool bIsReadOnly = false;
	FString DisplayName;

	friend struct FVoxelParameterStructProvider;
};