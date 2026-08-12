// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinType.h"
#if WITH_EDITOR
#include "Kismet2/EnumEditorUtils.h"
#include "Kismet2/StructureEditorUtils.h"
#endif

#if WITH_EDITOR
struct FVoxelPinTypeSetRegistry
	: public FVoxelSingleton
	, public FEnumEditorUtils::INotifyOnEnumChanged
	, public FStructureEditorUtils::INotifyOnStructChanged
{
public:
	const TVoxelSet<FVoxelPinType>& GetTypes();

public:
	//~ Begin INotifyOnEnumChanged Interface
	virtual void PreChange(
		const UUserDefinedEnum* Changed,
		FEnumEditorUtils::EEnumEditorChangeInfo ChangedType) override;

	virtual void PostChange(
		const UUserDefinedEnum* Changed,
		FEnumEditorUtils::EEnumEditorChangeInfo ChangedType) override;
	//~ End INotifyOnEnumChanged Interface

public:
	//~ Begin INotifyOnStructChanged Interface
	virtual void PreChange(
		const UUserDefinedStruct* Changed,
		FStructureEditorUtils::EStructureEditorChangeInfo ChangedType) override;

	virtual void PostChange(
		const UUserDefinedStruct* Changed,
		FStructureEditorUtils::EStructureEditorChangeInfo ChangedType) override;
	//~ End INotifyOnStructChanged Interface

private:
	TVoxelSet<FVoxelPinType> Types;
	TVoxelSet<FVoxelPinType> UserTypes;

	void InitializeTypes();
	void InitializeUserTypes();

private:
	void OnUserTypeChanged(const FVoxelPinType& ChangedType);
	void RunTests() const;
};
#endif

#if WITH_EDITOR
extern FVoxelPinTypeSetRegistry* GVoxelPinTypeSetRegistry;
#endif