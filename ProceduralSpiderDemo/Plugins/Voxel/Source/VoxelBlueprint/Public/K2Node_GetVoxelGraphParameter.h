// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "K2Node_VoxelGraphParameterBase.h"
#include "K2Node_GetVoxelGraphParameter.generated.h"

UCLASS()
class VOXELBLUEPRINT_API UK2Node_GetVoxelHeightGraphParameter : public UK2Node_VoxelGraphParameterBase
{
	GENERATED_BODY()

public:
	UK2Node_GetVoxelHeightGraphParameter();

	//~ Begin UK2Node_VoxelGraphParameterBase Interface
	virtual bool IsPinWildcard(const UEdGraphPin& Pin) const override;
	virtual UEdGraphPin* GetParameterNamePin() const override;
	virtual UClass* GetGraphClassType() const override;
	virtual FName GetGraphPinName() const override;
	//~ End UK2Node_VoxelGraphParameterBase Interface
};

UCLASS()
class VOXELBLUEPRINT_API UK2Node_GetVoxelVolumeGraphParameter : public UK2Node_VoxelGraphParameterBase
{
	GENERATED_BODY()

public:
	UK2Node_GetVoxelVolumeGraphParameter();

	//~ Begin UK2Node_VoxelGraphParameterBase Interface
	virtual bool IsPinWildcard(const UEdGraphPin& Pin) const override;
	virtual UEdGraphPin* GetParameterNamePin() const override;
	virtual UClass* GetGraphClassType() const override;
	virtual FName GetGraphPinName() const override;
	//~ End UK2Node_VoxelGraphParameterBase Interface
};

UCLASS()
class VOXELBLUEPRINT_API UK2Node_GetVoxelHeightSculptGraphParameter : public UK2Node_VoxelGraphParameterBase
{
	GENERATED_BODY()

public:
	UK2Node_GetVoxelHeightSculptGraphParameter();

	//~ Begin UK2Node_VoxelGraphParameterBase Interface
	virtual bool IsPinWildcard(const UEdGraphPin& Pin) const override;
	virtual UEdGraphPin* GetParameterNamePin() const override;
	virtual UClass* GetGraphClassType() const override;
	virtual FName GetGraphPinName() const override;
	//~ End UK2Node_VoxelGraphParameterBase Interface
};

UCLASS()
class VOXELBLUEPRINT_API UK2Node_GetVoxelVolumeSculptGraphParameter : public UK2Node_VoxelGraphParameterBase
{
	GENERATED_BODY()

public:
	UK2Node_GetVoxelVolumeSculptGraphParameter();

	//~ Begin UK2Node_VoxelGraphParameterBase Interface
	virtual bool IsPinWildcard(const UEdGraphPin& Pin) const override;
	virtual UEdGraphPin* GetParameterNamePin() const override;
	virtual UClass* GetGraphClassType() const override;
	virtual FName GetGraphPinName() const override;
	//~ End UK2Node_VoxelGraphParameterBase Interface
};