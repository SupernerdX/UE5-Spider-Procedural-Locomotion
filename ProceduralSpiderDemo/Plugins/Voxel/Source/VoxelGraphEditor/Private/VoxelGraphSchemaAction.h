// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraph.h"

class UVoxelGraphNode;
class UVoxelFunctionLibraryAsset;
struct FVoxelNode;
struct FVoxelGraphToolkit;

struct FVoxelGraphSchemaAction : public FEdGraphSchemaAction
{
	using FEdGraphSchemaAction::FEdGraphSchemaAction;

	virtual FName GetTypeId() const final override
	{
		return StaticGetTypeId();
	}
	static FName StaticGetTypeId()
	{
		return STATIC_FNAME("FVoxelGraphSchemaAction");
	}
	virtual FName GetVoxelTypeId() const
	{
		return StaticGetTypeId();
	}

	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color)
	{
		static const FSlateIcon DefaultIcon("EditorStyle", "NoBrush");
		Icon = DefaultIcon;
		Color = FLinearColor::White;
	}

	UVoxelGraphNode* Apply(UEdGraph& ParentGraph, const UE_506_SWITCH(FVector2D, FVector2f)& Location, bool bSelectNewNode = false);
};

struct FVoxelGraphSchemaAction_NewComment : public FVoxelGraphSchemaAction
{
	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;

	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;
};

struct FVoxelGraphSchemaAction_Paste : public FVoxelGraphSchemaAction
{
	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;

	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
};

struct FVoxelGraphSchemaAction_NewCallFunctionNode : public FVoxelGraphSchemaAction
{
	FGuid Guid;
	bool bCallParent = false;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;

	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;
};

struct FVoxelGraphSchemaAction_NewCallExternalFunctionNode : public FVoxelGraphSchemaAction
{
	TVoxelObjectPtr<UVoxelFunctionLibraryAsset> FunctionLibrary;
	FGuid Guid;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;

	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;

	static FName StaticGetVoxelTypeId()
	{
		return STATIC_FNAME("FVoxelGraphSchemaAction_NewCallExternalFunctionNode");
	}
	virtual FName GetVoxelTypeId() const override
	{
		return StaticGetVoxelTypeId();
	}
};

struct FVoxelGraphSchemaAction_NewCallParentMainGraphNode : public FVoxelGraphSchemaAction
{
	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;

	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelGraphSchemaAction_NewParameterUsage : public FVoxelGraphSchemaAction
{
	FGuid Guid;
	FVoxelPinType Type;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;
};

struct FVoxelGraphSchemaAction_NewFunctionInputUsage : public FVoxelGraphSchemaAction
{
	FGuid Guid;
	FVoxelPinType Type;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;
};

struct FVoxelGraphSchemaAction_NewFunctionInputDefaultUsage : public FVoxelGraphSchemaAction
{
	FGuid Guid;
	FVoxelPinType Type;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;
};

struct FVoxelGraphSchemaAction_NewFunctionInputPreviewUsage : public FVoxelGraphSchemaAction
{
	FGuid Guid;
	FVoxelPinType Type;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;
};

struct FVoxelGraphSchemaAction_NewOutputUsage : public FVoxelGraphSchemaAction
{
	FGuid Guid;
	FVoxelPinType Type;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;
};

struct FVoxelGraphSchemaAction_NewLocalVariableUsage : public FVoxelGraphSchemaAction
{
	FGuid Guid;
	FVoxelPinType Type;
	bool bIsDeclaration = false;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelGraphSchemaAction_NewParameter : public FVoxelGraphSchemaAction
{
	TOptional<FVoxelPinType> TypeOverride;
	FName NameOverride;
	FString Category;
	TWeakPtr<FVoxelGraphToolkit> WeakToolkit;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
};

struct FVoxelGraphSchemaAction_NewFunctionInput : public FVoxelGraphSchemaAction
{
	TOptional<FVoxelPinType> TypeOverride;
	FName NameOverride;
	FString Category;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
};

struct FVoxelGraphSchemaAction_NewFunctionOutput : public FVoxelGraphSchemaAction
{
	TOptional<FVoxelPinType> TypeOverride;
	FName NameOverride;
	FString Category;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
};

struct FVoxelGraphSchemaAction_NewLocalVariable : public FVoxelGraphSchemaAction
{
	TOptional<FVoxelPinType> TypeOverride;
	FName NameOverride;
	FString Category;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelGraphSchemaAction_NewFunction : public FVoxelGraphSchemaAction
{
	FString Category;
	bool bOpenNewGraph = true;

	TVoxelObjectPtr<UVoxelTerminalGraph> OutNewFunction;
	TWeakPtr<FVoxelGraphToolkit> WeakToolkit;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
};

struct FVoxelGraphSchemaAction_NewStructNode : public FVoxelGraphSchemaAction
{
	TSharedPtr<const FVoxelNode> Node;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;

	static FName StaticGetVoxelTypeId()
	{
		return STATIC_FNAME("FVoxelGraphSchemaAction_NewStructNode");
	}
	virtual FName GetVoxelTypeId() const override
	{
		return StaticGetVoxelTypeId();
	}
};

struct FVoxelGraphSchemaAction_NewPromotableStructNode : public FVoxelGraphSchemaAction_NewStructNode
{
	TArray<FVoxelPinType> PinTypes;

	using FVoxelGraphSchemaAction_NewStructNode::FVoxelGraphSchemaAction_NewStructNode;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
};

struct FVoxelGraphSchemaAction_NewKnotNode : public FVoxelGraphSchemaAction
{
	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;

	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelGraphSchemaAction_NewCustomizeParameter : public FVoxelGraphSchemaAction
{
	FGuid Guid;
	FVoxelPinType Type;

	using FVoxelGraphSchemaAction::FVoxelGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, UE_506_SWITCH(FVector2D, const FVector2f&) Location, bool bSelectNewNode = true) override;
	virtual void GetIcon(FSlateIcon& Icon, FLinearColor& Color) override;
};