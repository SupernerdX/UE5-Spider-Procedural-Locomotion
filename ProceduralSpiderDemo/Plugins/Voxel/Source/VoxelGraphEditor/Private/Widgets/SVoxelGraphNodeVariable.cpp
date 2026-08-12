// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphNodeVariable.h"
#include "SCommentBubble.h"
#include "Nodes/VoxelGraphNode_FunctionInput.h"
#include "Nodes/VoxelGraphNode_FunctionOutput.h"
#include "Nodes/VoxelGraphNode_LocalVariable.h"

VOXEL_INITIALIZE_STYLE(GraphVariableStyle)
{
	Set("Graph.Function.FunctionOutputParameterIcon", new IMAGE_BRUSH_SVG("Graphs/FunctionOutputParameter", CoreStyleConstants::Icon20x20));
}

void SVoxelGraphNodeVariable::Construct(const FArguments& InArgs, UVoxelGraphNode* InNode)
{
	GraphNode = InNode;
	NodeDefinition = GetVoxelNode().GetNodeDefinition();

	SetCursor( EMouseCursor::CardinalCross );

	UpdateGraphNode();
}

void SVoxelGraphNodeVariable::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	LeftNodeBox.Reset();
	RightNodeBox.Reset();

	SetupErrorReporting();

	ContentScale.Bind(this, &SGraphNode::GetContentScale);

	const FText NodeTitle = GraphNode->GetNodeTitle(ENodeTitleType::EditableTitle);

	FMargin ContentPadding(2.f, 0.f);
	if (NodeTitle.IsEmpty())
	{
		ContentPadding.Top = 2.f;
		ContentPadding.Bottom = 2.f;
	}
	else
	{
		ContentPadding.Top = 20.f;
		if (UVoxelGraphNode* Node = Cast<UVoxelGraphNode>(GraphNode))
		{
			if (Node->HasExecutionFlow())
			{
				ContentPadding.Top = 7.f;
			}
		}
	}

	GetOrAddSlot(ENodeZone::Center)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Graph.VarNode.Body"))
			]
			+ SOverlay::Slot()
			.VAlign(VAlign_Top)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Graph.VarNode.ColorSpill"))
				.ColorAndOpacity(this, &SVoxelGraphNodeVariable::GetVariableColor)
			]
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Graph.VarNode.Gloss"))
			]
			+ SOverlay::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Center)
			.Padding(0.f, 8.f)
			[
				UpdateTitleWidget(NodeTitle)
			]
			+ SOverlay::Slot()
			.Padding(1.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.FillWidth(1.0f)
				.Padding(ContentPadding)
				[
					SAssignNew(LeftNodeBox, SVerticalBox)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.Padding(ContentPadding)
				[
					SAssignNew(RightNodeBox, SVerticalBox)
				]
			]
		]
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Top)
		.AutoHeight()
		.Padding(FMargin(5.0f, 1.0f))
		[
			ErrorReporting->AsWidget()
		]
	];

	{
		TSharedPtr<SCommentBubble> CommentBubble;

		SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

		GetOrAddSlot(ENodeZone::TopCenter)
#if VOXEL_ENGINE_VERSION < 506
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
#else
		.SlotOffset2f(TAttribute<FVector2f>(CommentBubble.Get(), &SCommentBubble::GetOffset2f))
		.SlotSize2f(TAttribute<FVector2f>(CommentBubble.Get(), &SCommentBubble::GetSize2f))
#endif
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];
	}

	CreatePinWidgets();

	// Align single pin widgets vertically on both sides
	if (LeftNodeBox->NumSlots() == RightNodeBox->NumSlots() &&
		LeftNodeBox->NumSlots() == 1)
	{
		RightNodeBox->GetSlot(RightNodeBox->NumSlots() - 1).SetFillHeight(1.f);
	}

	CreateInputSideAddButton(LeftNodeBox);
	CreateOutputSideAddButton(RightNodeBox);
}

const FSlateBrush* SVoxelGraphNodeVariable::GetShadowBrush(const bool bSelected) const
{
	return bSelected ? FAppStyle::GetBrush(TEXT("Graph.VarNode.ShadowSelected")) : FAppStyle::GetBrush(TEXT("Graph.VarNode.Shadow"));
}

void SVoxelGraphNodeVariable::GetOverlayBrushes(const bool bSelected, UE_506_SWITCH(const FVector2D, const FVector2f&) WidgetSize, TArray<FOverlayBrushInfo>& Brushes) const
{
	SVoxelGraphNode::GetOverlayBrushes(bSelected, WidgetSize, Brushes);

	const FSlateBrush* CornerIcon = nullptr;
	if (GraphNode->IsA<UVoxelGraphNode_FunctionInputBase>())
	{
		CornerIcon = FAppStyle::GetBrush("Graph.Function.FunctionParameterIcon");
	}
	else if (GraphNode->IsA<UVoxelGraphNode_LocalVariable>())
	{
		CornerIcon = FAppStyle::GetBrush("Graph.Function.FunctionLocalVariableIcon");
	}
	else if (GraphNode->IsA<UVoxelGraphNode_FunctionOutput>())
	{
		CornerIcon = FVoxelEditorStyle::GetBrush("Graph.Function.FunctionOutputParameterIcon");
	}

	if (!CornerIcon)
	{
		return;
	}

	FOverlayBrushInfo BrushInfo;
	BrushInfo.Brush = CornerIcon;
	BrushInfo.OverlayOffset.X = WidgetSize.X - CornerIcon->GetImageSize().X / 2.f - 3.f;
	BrushInfo.OverlayOffset.Y = -CornerIcon->GetImageSize().Y / 2.f + 2.f;
	Brushes.Add(BrushInfo);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FSlateColor SVoxelGraphNodeVariable::GetVariableColor() const
{
	return GraphNode->GetNodeTitleColor();
}

TSharedRef<SWidget> SVoxelGraphNodeVariable::UpdateTitleWidget(const FText& InTitleText)
{
	if (InTitleText.IsEmpty())
	{
		return SNullWidget::NullWidget;
	}

	return
		SNew(STextBlock)
		.TextStyle(FAppStyle::Get(), "Graph.Node.NodeTitle")
		.Text(InTitleText);
}