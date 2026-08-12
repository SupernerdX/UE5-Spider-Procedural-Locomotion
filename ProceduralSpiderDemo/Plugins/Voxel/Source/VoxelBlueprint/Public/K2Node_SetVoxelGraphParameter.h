// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "K2Node_VoxelGraphParameterBase.h"
#include "K2Node_SetVoxelGraphParameter.generated.h"

UCLASS(Abstract)
class VOXELBLUEPRINT_API UK2Node_SetVoxelGraphParameterBase : public UK2Node_VoxelGraphParameterBase
{
	GENERATED_BODY()

public:
	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	//~ End UEdGraphNode Interface

	//~ Begin UK2Node Interface
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	//~ End UK2Node Interface

	//~ Begin UK2Node_VoxelGraphParameterBase Interface
	virtual bool IsPinWildcard(const UEdGraphPin& Pin) const override;
	virtual UEdGraphPin* GetParameterNamePin() const override;
	//~ End UK2Node_VoxelGraphParameterBase Interface

	virtual FName GetOwnerPinName() const VOXEL_PURE_VIRTUAL({});
	virtual UK2Node_VoxelGraphParameterBase* SpawnGetterNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) VOXEL_PURE_VIRTUAL({});
};

UCLASS()
class VOXELBLUEPRINT_API UK2Node_SetVoxelHeightGraphParameter : public UK2Node_SetVoxelGraphParameterBase
{
	GENERATED_BODY()

public:
	UK2Node_SetVoxelHeightGraphParameter();

	//~ Begin UK2Node_VoxelGraphParameterBase Interface
	virtual UClass* GetGraphClassType() const override;
	virtual FName GetGraphPinName() const override;
	virtual FName GetOwnerPinName() const override;
	virtual UK2Node_VoxelGraphParameterBase* SpawnGetterNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	//~ End UK2Node_VoxelGraphParameterBase Interface
};

UCLASS()
class VOXELBLUEPRINT_API UK2Node_SetVoxelVolumeGraphParameter : public UK2Node_SetVoxelGraphParameterBase
{
	GENERATED_BODY()

public:
	UK2Node_SetVoxelVolumeGraphParameter();

	//~ Begin UK2Node_VoxelGraphParameterBase Interface
	virtual UClass* GetGraphClassType() const override;
	virtual FName GetGraphPinName() const override;
	virtual FName GetOwnerPinName() const override;
	virtual UK2Node_VoxelGraphParameterBase* SpawnGetterNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	//~ End UK2Node_VoxelGraphParameterBase Interface
};

UCLASS()
class VOXELBLUEPRINT_API UK2Node_SetVoxelHeightSculptGraphParameter : public UK2Node_SetVoxelGraphParameterBase
{
	GENERATED_BODY()

public:
	UK2Node_SetVoxelHeightSculptGraphParameter();

	//~ Begin UK2Node_VoxelGraphParameterBase Interface
	virtual UClass* GetGraphClassType() const override;
	virtual FName GetGraphPinName() const override;
	virtual FName GetOwnerPinName() const override;
	virtual UK2Node_VoxelGraphParameterBase* SpawnGetterNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	//~ End UK2Node_VoxelGraphParameterBase Interface
};

UCLASS()
class VOXELBLUEPRINT_API UK2Node_SetVoxelVolumeSculptGraphParameter : public UK2Node_SetVoxelGraphParameterBase
{
	GENERATED_BODY()

public:
	UK2Node_SetVoxelVolumeSculptGraphParameter();

	//~ Begin UK2Node_VoxelGraphParameterBase Interface
	virtual UClass* GetGraphClassType() const override;
	virtual FName GetGraphPinName() const override;
	virtual FName GetOwnerPinName() const override;
	virtual UK2Node_VoxelGraphParameterBase* SpawnGetterNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	//~ End UK2Node_VoxelGraphParameterBase Interface
};