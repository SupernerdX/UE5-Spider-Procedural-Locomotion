// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphVisuals.h"
#include "VoxelEdGraph.h"
#include "VoxelPointSet.h"
#include "VoxelExposedSeed.h"
#include "VoxelGraphSchema.h"
#include "VoxelGraphToolkit.h"
#include "SVoxelGraphPinSeed.h"
#include "VoxelGraphEditorSettings.h"
#include "Nodes/VoxelGraphNode_Knot.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Widgets/SVoxelGraphNode.h"
#include "Widgets/SVoxelGraphPins.h"
#include "Widgets/SVoxelGraphPinEnum.h"
#include "Widgets/SVoxelGraphPinRange.h"
#include "Widgets/SVoxelGraphPinObject.h"
#include "Widgets/SVoxelGraphNodeKnot.h"
#include "Widgets/SVoxelGraphNodeVariable.h"
#include "Widgets/SVoxelGraphPinIntContainer.h"
#include "Widgets/SVoxelGraphPinBodyInstance.h"

#include "Styling/SlateIconFinder.h"
#include "ConnectionDrawingPolicy.h"

TVoxelArray<FVoxelGraphVisuals::FPinWidgetFactory> FVoxelGraphVisuals::PinWidgetFactories;

FSlateIcon FVoxelGraphVisuals::GetNodeIcon(const FString& IconName)
{
	if (IconName == "Loop")      { return FSlateIcon("EditorStyle", "GraphEditor.Macro.Loop_16x"    ); }
	if (IconName == "Gate")      { return FSlateIcon("EditorStyle", "GraphEditor.Macro.Gate_16x"    ); }
	if (IconName == "Do N")      { return FSlateIcon("EditorStyle", "GraphEditor.Macro.DoN_16x"     ); }
	if (IconName == "Do Once")   { return FSlateIcon("EditorStyle", "GraphEditor.Macro.DoOnce_16x"  ); }
	if (IconName == "IsValid")   { return FSlateIcon("EditorStyle", "GraphEditor.Macro.IsValid_16x" ); }
	if (IconName == "FlipFlop")  { return FSlateIcon("EditorStyle", "GraphEditor.Macro.FlipFlop_16x"); }
	if (IconName == "ForEach")   { return FSlateIcon("EditorStyle", "GraphEditor.Macro.ForEach_16x" ); }
	if (IconName == "Event")     { return FSlateIcon("EditorStyle", "GraphEditor.Event_16x"         ); }
	if (IconName == "Sequence")  { return FSlateIcon("EditorStyle", "GraphEditor.Sequence_16x"      ); }
	if (IconName == "Cast")      { return FSlateIcon("EditorStyle", "GraphEditor.Cast_16x"          ); }
	if (IconName == "Select")    { return FSlateIcon("EditorStyle", "GraphEditor.Select_16x"        ); }
	if (IconName == "Switch")    { return FSlateIcon("EditorStyle", "GraphEditor.Switch_16x"        ); }
	if (IconName == "MakeArray") { return FSlateIcon("EditorStyle", "GraphEditor.MakeArray_16x"     ); }

	return FSlateIcon("EditorStyle", "Kismet.AllClasses.FunctionIcon");
}

FLinearColor FVoxelGraphVisuals::GetNodeColor(const FString& ColorName)
{
#define CASE(Name, Color) \
	if (ColorName == TEXT(Name)) \
	{ \
		return Color; \
	}

	CASE("Blue", FLinearColor(0.f, 0.68359375f, 1.f));
	CASE("Green", FLinearColor(0.039216f, 0.666667f, 0.f));
	CASE("Red", FLinearColor(1.f, 0.f, 0.f));
	CASE("LightRed", FLinearColor(0.8f, 0.3f, 0.3f));
	CASE("Orange", FLinearColor(1.f, 0.546875f, 0.f));
	CASE("White", FLinearColor::White);

#undef CASE

	ensure(false);
	return FLinearColor::Black;
}

FSlateIcon FVoxelGraphVisuals::GetPinIcon(const FVoxelPinType& Type)
{
	static const FSlateIcon VariableIcon(FAppStyle::GetAppStyleSetName(), "Kismet.VariableList.TypeIcon");
	const FVoxelPinType ExposedType = Type.GetExposedType();

	if (Type.IsBuffer())
	{
		if (Type.IsBufferArray())
		{
			static const FSlateIcon ArrayIcon(FAppStyle::GetAppStyleSetName(), "Kismet.VariableList.ArrayTypeIcon");
			return ArrayIcon;
		}
		else
		{
			static const FSlateIcon BufferIcon(FVoxelEditorStyle::GetStyleSetName(), "Pill.Buffer");
			return BufferIcon;
		}
	}

	if (ExposedType.IsClass())
	{
		if (const UClass* Class = ExposedType.GetBaseClass())
		{
			return FSlateIconFinder::FindIconForClass(Class);
		}
		else
		{
			return VariableIcon;
		}
	}

	if (ExposedType.IsObject())
	{
		if (const UClass* Class = ExposedType.GetObjectClass())
		{
			return FSlateIconFinder::FindIconForClass(Class);
		}
		else
		{
			return VariableIcon;
		}
	}

	return VariableIcon;
}

FLinearColor FVoxelGraphVisuals::GetPinColor(const FVoxelPinType& InType)
{
	// Buffer has same color as inner
	const FVoxelPinType Type = InType.GetInnerExposedType();

	const UGraphEditorSettings* Settings = GetDefault<UGraphEditorSettings>();
	const UVoxelGraphEditorSettings* VoxelSettings = GetDefault<UVoxelGraphEditorSettings>();

	if (Type.IsWildcard())
	{
		return Settings->WildcardPinTypeColor;
	}
	else if (Type.Is<bool>())
	{
		return Settings->BooleanPinTypeColor;
	}
	else if (Type.Is<float>())
	{
		return Settings->FloatPinTypeColor;
	}
	else if (Type.Is<double>())
	{
		return Settings->DoublePinTypeColor;
	}
	else if (Type.Is<int32>())
	{
		return Settings->IntPinTypeColor;
	}
	else if (Type.Is<int64>())
	{
		return Settings->Int64PinTypeColor;
	}
	else if (Type.Is<FName>())
	{
		return Settings->StringPinTypeColor;
	}
	else if (Type.Is<uint8>())
	{
		return Settings->BytePinTypeColor;
	}
	else if (Type.Is<FVoxelPointSet>())
	{
		return VoxelSettings->PointSetPinColor;
	}
	else if (Type.Is<FVector>())
	{
		return Settings->VectorPinTypeColor;
	}
	else if (Type.Is<FVoxelDoubleVector>())
	{
		return VoxelSettings->DoubleVectorPinColor;
	}
	else if (Type.Is<FVector2D>())
	{
		return VoxelSettings->Vector2DPinColor;
	}
	else if (Type.Is<FVoxelDoubleVector2D>())
	{
		return VoxelSettings->DoubleVector2DPinColor;
	}
	else if (
		Type.Is<FRotator>() ||
		Type.Is<FQuat>())
	{
		return Settings->RotatorPinTypeColor;
	}
	else if (Type.Is<FTransform>())
	{
		return Settings->TransformPinTypeColor;
	}
	else if (Type.Is<FVoxelExposedSeed>())
	{
		return VoxelSettings->SeedPinColor;
	}
	else if (Type.IsClass())
	{
		return Settings->ClassPinTypeColor;
	}
	else if (Type.IsObject())
	{
		return Settings->ObjectPinTypeColor;
	}
	else if (Type.IsStruct())
	{
		return Settings->StructPinTypeColor;
	}
	else
	{
		ensureVoxelSlow(false);
		return Settings->DefaultPinTypeColor;
	}
}

TSharedPtr<SGraphPin> FVoxelGraphVisuals::GetPinWidget(UEdGraphPin* Pin)
{
	const FVoxelPinType Type = FVoxelPinType(Pin->PinType);
	if (!Type.IsValid())
	{
		if (Pin->DefaultObject)
		{
			return SNew(SVoxelGraphPinObject, Pin);
		}
		else
		{
			return SNew(SVoxelGraphPin, Pin);
		}
	}

#define SNewPerf(Name, Pin) [&] { VOXEL_SCOPE_COUNTER(#Name); return SNew(Name, Pin); }()

	if (Type.IsBufferArray())
	{
		return SNewPerf(SVoxelGraphPin, Pin);
	}

	const FVoxelPinType InnerType = Type.GetInnerExposedType();

	for (const FPinWidgetFactory& PinWidgetFactory : PinWidgetFactories)
	{
		if (const TSharedPtr<SGraphPin> Widget = PinWidgetFactory(InnerType, Pin))
		{
			return Widget;
		}
	}

	if (InnerType.IsWildcard())
	{
		return SNewPerf(SVoxelGraphPin, Pin);
	}
	else if (InnerType.Is<bool>())
	{
		return SNewPerf(SVoxelGraphPinBool, Pin);
	}
	else if (InnerType.Is<float>())
	{
		return SNewPerf(SVoxelGraphPinNum<float>, Pin);
	}
	else if (InnerType.Is<double>())
	{
		return SNewPerf(SVoxelGraphPinNum<double>, Pin);
	}
	else if (InnerType.Is<int32>())
	{
		return SNewPerf(SVoxelGraphPinInteger, Pin);
	}
	else if (InnerType.Is<int64>())
	{
		return SNewPerf(SVoxelGraphPinNum<int64>, Pin);
	}
	else if (InnerType.Is<FName>())
	{
		return SNewPerf(SVoxelGraphPinString, Pin);
	}
	else if (InnerType.Is<uint8>())
	{
		if (InnerType.GetEnum())
		{
			return SNewPerf(SVoxelGraphPinEnum, Pin);
		}
		else
		{
			return SNewPerf(SVoxelGraphPinInteger, Pin);
		}
	}
	else if (InnerType.Is<FVector2D>())
	{
		return SNewPerf(SVoxelGraphPinVector2D<float>, Pin);
	}
	else if (InnerType.Is<FVoxelDoubleVector2D>())
	{
		return SNewPerf(SVoxelGraphPinVector2D<double>, Pin);
	}
	else if (
		InnerType.Is<FVector>() ||
		InnerType.Is<FRotator>() ||
		InnerType.Is<FQuat>())
	{
		return SNewPerf(SVoxelGraphPinVector<float>, Pin);
	}
	else if (InnerType.Is<FVoxelDoubleVector>())
	{
		return SNewPerf(SVoxelGraphPinVector<double>, Pin);
	}
	else if (
		InnerType.Is<FLinearColor>() ||
		InnerType.Is<FVoxelDoubleLinearColor>())
	{
		return SNewPerf(SVoxelGraphPinColor, Pin);
	}
	else if (InnerType.Is<FIntPoint>())
	{
		return SNewPerf(SVoxelGraphPinIntContainer<FIntPoint>, Pin);
	}
	else if (InnerType.Is<FIntVector>())
	{
		return SNewPerf(SVoxelGraphPinIntContainer<FIntVector>, Pin);
	}
	else if (InnerType.Is<FIntVector4>())
	{
		return SNewPerf(SVoxelGraphPinIntContainer<FIntVector4>, Pin);
	}
	else if (InnerType.Is<FInt64Point>())
	{
		return SNewPerf(SVoxelGraphPinIntContainer<FInt64Point>, Pin);
	}
	else if (InnerType.Is<FInt64Vector>())
	{
		return SNewPerf(SVoxelGraphPinIntContainer<FInt64Vector>, Pin);
	}
	else if (InnerType.Is<FInt64Vector4>())
	{
		return SNewPerf(SVoxelGraphPinIntContainer<FInt64Vector4>, Pin);
	}
	else if (InnerType.Is<FVoxelExposedSeed>())
	{
		return SNewPerf(SVoxelGraphPinSeed, Pin);
	}
	else if (InnerType.Is<FVoxelFloatRange>())
	{
		return SNewPerf(SVoxelGraphPinFloatRange, Pin);
	}
	else if (InnerType.Is<FVoxelInt32Range>())
	{
		return SNewPerf(SVoxelGraphPinInt32Range, Pin);
	}
	else if (InnerType.Is<FBodyInstance>())
	{
		return SNewPerf(SVoxelGraphPinBodyInstance, Pin);
	}
	else if (InnerType.IsClass())
	{
		return SNewPerf(SVoxelGraphPinClass, Pin);
	}
	else if (InnerType.IsObject())
	{
		return SNewPerf(SVoxelGraphPinObject, Pin);
	}
	else if (InnerType.IsStruct())
	{
		return SNewPerf(SVoxelGraphPin, Pin);
	}
	else
	{
		ensure(false);
		return nullptr;
	}

#undef SNewPerf
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelGraphConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
public:
	const TSharedPtr<FVoxelGraphToolkit> Toolkit;

	FVoxelGraphConnectionDrawingPolicy(
		const int32 InBackLayerID,
		const int32 InFrontLayerID,
		const float ZoomFactor,
		const FSlateRect& InClippingRect,
		FSlateWindowElementList& InDrawElements,
		const UVoxelEdGraph& Graph)
		: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
		, Toolkit(StaticCastSharedPtr<FVoxelGraphToolkit>(Graph.GetGraphToolkit()))
	{
		ArrowImage = nullptr;
		ArrowRadius = FVector2D::ZeroVector;
	}

	virtual void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params) override
	{
		FConnectionDrawingPolicy::DetermineWiringStyle(OutputPin, InputPin, Params);

		if (HoveredPins.Contains(InputPin) &&
			HoveredPins.Contains(OutputPin))
		{
			Params.WireThickness *= 5;
		}

		if ((InputPin && InputPin->bOrphanedPin) ||
			(OutputPin && OutputPin->bOrphanedPin))
		{
			Params.WireColor = FLinearColor::Red;
			return;
		}

		if (!ensure(OutputPin))
		{
			return;
		}

		Params.WireColor = FVoxelGraphVisuals::GetPinColor(OutputPin->PinType);

		if (!ensure(Toolkit))
		{
			return;
		}

		const UVoxelGraphNode* OutputPinNode = Cast<UVoxelGraphNode>(OutputPin->GetOwningNodeUnchecked());
		if (!OutputPinNode)
		{
			return;
		}

		const UVoxelEdGraph* EdGraph = Cast<UVoxelEdGraph>(OutputPinNode->GetGraph());
		if (!EdGraph)
		{
			return;
		}

		if (EdGraph->WeakDebugNode == OutputPinNode &&
			OutputPinNode->PreviewedPin == OutputPin->PinName)
		{
			Params.bDrawBubbles = true;
			Params.WireThickness = FMath::Max(Params.WireThickness, 4.f);
		}
	}
};

struct FVoxelGraphConnectionDrawingPolicyFactory : public FGraphPanelPinConnectionFactory
{
public:
	virtual FConnectionDrawingPolicy* CreateConnectionPolicy(const UEdGraphSchema* Schema, const int32 InBackLayerID, const int32 InFrontLayerID, const float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const override
	{
		if (!Schema->IsA<UVoxelGraphSchema>())
		{
			return nullptr;
		}

		return new FVoxelGraphConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements, *CastChecked<UVoxelEdGraph>(InGraphObj));
	}
};

class FVoxelGraphPanelPinFactory : public FGraphPanelPinFactory
{
	virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* Pin) const override
	{
		VOXEL_FUNCTION_COUNTER();

		const UVoxelGraphSchema* Schema = Cast<UVoxelGraphSchema>(Pin->GetSchema());
		if (!Schema)
		{
			return nullptr;
		}

		return FVoxelGraphVisuals::GetPinWidget(Pin);
	}
};

class FVoxelGraphNodeFactory : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* InNode) const override
	{
		VOXEL_FUNCTION_COUNTER();

		const UVoxelGraphSchema* Schema = Cast<UVoxelGraphSchema>(InNode->GetSchema());
		if (!Schema)
		{
			return nullptr;
		}

		UVoxelGraphNode* Node = Cast<UVoxelGraphNode>(InNode);
		if (!Node)
		{
			return nullptr;
		}

		if (UVoxelGraphNode_Knot* Knot = Cast<UVoxelGraphNode_Knot>(Node))
		{
			return SNew(SVoxelGraphNodeKnot, Knot);
		}

		if (Node->IsVariable())
		{
			return SNew(SVoxelGraphNodeVariable, Node);
		}

		return SNew(SVoxelGraphNode, Node);
	}
};

VOXEL_RUN_ON_STARTUP_EDITOR_COMMANDLET()
{
	FEdGraphUtilities::RegisterVisualNodeFactory(MakeShared<FVoxelGraphNodeFactory>());
	FEdGraphUtilities::RegisterVisualPinFactory(MakeShared<FVoxelGraphPanelPinFactory>());
	FEdGraphUtilities::RegisterVisualPinConnectionFactory(MakeShared<FVoxelGraphConnectionDrawingPolicyFactory>());
}