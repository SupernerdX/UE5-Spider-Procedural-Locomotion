// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "SplineMetadataDetailsFactory.h"
#include "VoxelSplineMetadataDetails.generated.h"

class UVoxelSplineMetadata;

UCLASS()
class UVoxelSplineMetadataDetailsFactory : public USplineMetadataDetailsFactoryBase
{
	GENERATED_BODY()

public:
	//~ Begin USplineMetadataDetailsFactoryBase Interface
	virtual TSharedPtr<ISplineMetadataDetails> Create() override;
	virtual UClass* GetMetadataClass() const override;
	//~ End USplineMetadataDetailsFactoryBase Interface
};

class FVoxelSplineMetadataStructureDataProvider : public IStructureDataProvider
{
public:
	const TVoxelObjectPtr<UVoxelSplineMetadata> WeakMetadata;
	const FGuid Guid;
	TSet<int32> SelectedKeys;

	FVoxelSplineMetadataStructureDataProvider(
		const TVoxelObjectPtr<UVoxelSplineMetadata> WeakMetadata,
		const FGuid Guid)
		: WeakMetadata(WeakMetadata)
		, Guid(Guid)
	{
	}

	//~ Begin IStructureDataProvider Interface
	virtual bool IsValid() const override;
	virtual const UStruct* GetBaseStructure() const override;
	virtual void GetInstances(TArray<TSharedPtr<FStructOnScope>>& OutInstances, const UStruct* ExpectedBaseStructure) const override;
	//~ End IStructureDataProvider Interface
};

class FVoxelSplineMetadataDetails
	: public ISplineMetadataDetails
	, public TSharedFromThis<FVoxelSplineMetadataDetails>
{
public:
	virtual ~FVoxelSplineMetadataDetails() = default;

	//~ Begin ISplineMetadataDetails Interface
	virtual FName GetName() const override;
	virtual FText GetDisplayName() const override;
	virtual void Update(USplineComponent* SplineComponent, const TSet<int32>& SelectedKeys) override;
	virtual void GenerateChildContent(IDetailGroup& DetailGroup) override;
	//~ End ISplineMetadataDetails Interface

private:
	TVoxelArray<TSharedPtr<FVoxelSplineMetadataStructureDataProvider>> StructProviders;
};