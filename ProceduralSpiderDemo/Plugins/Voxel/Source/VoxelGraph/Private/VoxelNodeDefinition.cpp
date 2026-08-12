// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNodeDefinition.h"

#if WITH_EDITOR
TSharedRef<IVoxelNodeDefinition::FCategoryNode> IVoxelNodeDefinition::FNode::MakeRoot(const bool bIsInput)
{
	const TSharedRef<FCategoryNode> Node = MakeShareable(new FCategoryNode({}, nullptr));
	Node->bPrivateIsInput = bIsInput;
	return Node;
}

IVoxelNodeDefinition::FNode::FNode(
	const EType Type,
	const FName Name,
	const TSharedPtr<FNode>& Parent)
	: Type(Type)
	, Name(Name)
	, WeakParent(Parent)
	, bPrivateIsInput(Parent ? Parent->bPrivateIsInput : false)
{
	TArray<FString> StringPath;
	StringPath.Add(Name.ToString());
	for (TSharedPtr<FNode> It = WeakParent.Pin(); It; It = It->WeakParent.Pin())
	{
		StringPath.Add(It->Name.ToString());
	}
	// Remove root
	ensure(FName(StringPath.Pop(EAllowShrinking::No)).IsNone());

	PrivatePath = TArray<FName>(StringPath);
	PrivateConcatenatedPath = FName(FString::Join(StringPath, TEXT("|")));
}

#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
TSharedRef<IVoxelNodeDefinition::FNode> IVoxelNodeDefinition::FVariadicPinNode::AddPin(const FName PinName)
{
	ensure(!AddedPins.Contains(PinName));
	AddedPins.Add(PinName);

	const TSharedRef<FNode> Node = MakeShareable(new FNode(EType::Pin, PinName, AsShared()));
	Children.Add(Node);
	return Node;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
TSharedRef<IVoxelNodeDefinition::FCategoryNode> IVoxelNodeDefinition::FCategoryNode::FindOrAddCategory(const FString& Category)
{
	if (Category.IsEmpty())
	{
		return SharedThis(this);
	}

	return FindOrAddCategory(TArray<FName>(FVoxelUtilities::ParseCategory(Category)));
}

TSharedRef<IVoxelNodeDefinition::FCategoryNode> IVoxelNodeDefinition::FCategoryNode::FindOrAddCategory(const TArray<FName>& Path)
{
	if (Path.Num() == 0)
	{
		return SharedThis(this);
	}

	const TSharedRef<FCategoryNode> FirstNode = INLINE_LAMBDA
	{
		const FName FixedPath = FixupCollapsedCategory(Path[0]);
		if (const TSharedPtr<FCategoryNode> Node = CategoryNameToNode.FindRef(FixedPath))
		{
			return Node.ToSharedRef();
		}

		const TSharedRef<FCategoryNode> Node = MakeShareable(new FCategoryNode(FixedPath, AsShared()));
		Children.Add(Node);
		CategoryNameToNode.Add(Node->Name, Node);
		return Node;
	};

	TArray<FName> ChildPath = Path;
	ChildPath.RemoveAt(0);
	return FirstNode->FindOrAddCategory(ChildPath);
}

TSharedRef<IVoxelNodeDefinition::FVariadicPinNode> IVoxelNodeDefinition::FCategoryNode::AddVariadicPin(const FName VariadicPinName)
{
	ensure(!AddedVariadicPins.Contains(VariadicPinName));
	AddedVariadicPins.Add(VariadicPinName);

	const TSharedRef<FVariadicPinNode> Node = MakeShareable(new FVariadicPinNode(VariadicPinName, AsShared()));
	Children.Add(Node);
	return Node;
}

TSharedRef<IVoxelNodeDefinition::FNode> IVoxelNodeDefinition::FCategoryNode::AddPin(const FName PinName)
{
	ensure(!AddedPins.Contains(PinName));
	AddedPins.Add(PinName);

	const TSharedRef<FNode> Node = MakeShareable(new FNode(EType::Pin, PinName, AsShared()));
	Children.Add(Node);
	return Node;
}

FName IVoxelNodeDefinition::FCategoryNode::FixupCollapsedCategory(const FName Path)
{
	FString PathTrunc = Path.ToString();
	PathTrunc.RemoveSpacesInline();
	if (!PathTrunc.RemoveFromEnd("::collapsed"))
	{
		return Path;
	}

	FString PathString = Path.ToString();
	PathString.RemoveFromEnd("collapsed");
	PathString.TrimEndInline();
	PathString.RemoveFromEnd("::");
	PathString.TrimEndInline();

	PathString = "__collapsed__" + PathString;

	return FName(PathString);
}
#endif