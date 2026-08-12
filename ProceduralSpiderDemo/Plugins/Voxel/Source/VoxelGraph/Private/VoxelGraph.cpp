// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraph.h"
#include "VoxelGraphTracker.h"
#include "VoxelCompiledGraph.h"
#include "VoxelTerminalGraph.h"
#include "VoxelDependency.h"
#include "VoxelTerminalGraphRuntime.h"
#include "VoxelFunctionLibraryAsset.h"
#include "VoxelInvalidationCallstack.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#if WITH_EDITOR
#include "ObjectTools.h"
#include "EdGraph/EdGraph.h"
#endif

UVoxelGraph::UVoxelGraph()
	: CompiledGraphDependency(FVoxelDependency::Create(GetPathName()))
{
	{
		UVoxelTerminalGraph* TerminalGraph = CreateDefaultSubobject<UVoxelTerminalGraph>("MainGraph");
		TerminalGraph->SetGuid_Hack(GVoxelMainTerminalGraphGuid);
		GuidToTerminalGraph.Add(GVoxelMainTerminalGraphGuid, TerminalGraph);
	}

	{
		UVoxelTerminalGraph* TerminalGraph = CreateDefaultSubobject<UVoxelTerminalGraph>("EditorGraph");
		TerminalGraph->SetGuid_Hack(GVoxelEditorTerminalGraphGuid);
		GuidToTerminalGraph.Add(GVoxelEditorTerminalGraphGuid, TerminalGraph);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGraph::Fixup()
{
	VOXEL_FUNCTION_COUNTER();

	// Remove stale terminal graphs
	for (auto It = GuidToTerminalGraph.CreateIterator(); It; ++It)
	{
		if (!ensure(It.Value()))
		{
			It.RemoveCurrent();
			continue;
		}

		if (It.Value()->GetOuterUVoxelGraph() == this)
		{
			continue;
		}

		if (!ensure(It.Value()->IsEditorTerminalGraph()) ||
			!ensure(It.Value()->GetOuterUVoxelGraph()->HasAnyFlags(RF_ClassDefaultObject | RF_DefaultSubObject)))
		{
			It.RemoveCurrent();
			continue;
		}

		UVoxelTerminalGraph* EditorGraph = NewObject<UVoxelTerminalGraph>(this, "EditorGraph", RF_Transactional);
		EditorGraph->SetGuid_Hack(GVoxelEditorTerminalGraphGuid);
		It.Value() = EditorGraph;
	}

	// Fix null GUIDs
	{
		TVoxelArray<TPair<FGuid, TObjectPtr<UVoxelTerminalGraph>>> GuidToTerminalGraphArray;
		for (auto& It : GuidToTerminalGraph)
		{
			GuidToTerminalGraphArray.Add(It);
		}

		for (auto& It : GuidToTerminalGraphArray)
		{
			if (!ensureVoxelSlow(It.Key.IsValid()))
			{
				It.Key = FGuid::NewGuid();
			}
		}

		GuidToTerminalGraph.Reset();

		for (const auto& It : GuidToTerminalGraphArray)
		{
			GuidToTerminalGraph.Add(It);
		}
	}

	// Before anything else, ensure terminal graph GUIDs are up to date
	for (auto& It : GuidToTerminalGraph)
	{
		It.Value->SetGuid_Hack(It.Key);
	}

	// Fixup terminal graphs
	ForeachTerminalGraph_NoInheritance([&](UVoxelTerminalGraph& TerminalGraph)
	{
		TerminalGraph.ConditionalPostLoad();
		TerminalGraph.Fixup();
	});

	// Compact to avoid adding into a hole
	GuidToTerminalGraph.CompactStable();
	GuidToParameter.CompactStable();

	// Fixup buffer types
	for (auto& It : GuidToParameter)
	{
		FVoxelParameter& Parameter = It.Value;
		if (!Parameter.Type.IsBuffer())
		{
			continue;
		}
		if (Parameter.Type.IsBufferArray())
		{
			continue;
		}

		ensure(false);
		Parameter.Type = Parameter.Type.GetInnerType();
	}

	// Fixup parameters
	{
		TSet<FName> Names;

		for (auto& It : GuidToParameter)
		{
			FVoxelParameter& Parameter = It.Value;
			Parameter.Fixup();
#if WITH_EDITOR
			Parameter.Category = FVoxelUtilities::SanitizeCategory(Parameter.Category);
#endif

			if (Parameter.Name.IsNone())
			{
				Parameter.Name = "Parameter";
			}

			while (Names.Contains(Parameter.Name))
			{
				Parameter.Name.SetNumber(Parameter.Name.GetNumber() + 1);
			}

			Names.Add(Parameter.Name);
		}

#if WITH_EDITOR
		// Sort parameters by category
		TMap<FString, TMap<FGuid, FVoxelParameter>> CategoryToGuidToParameter;
		for (const auto& It : GuidToParameter)
		{
			CategoryToGuidToParameter.FindOrAdd(It.Value.Category).Add(It.Key, It.Value);
		}

		GuidToParameter.Empty();
		for (const auto& CategoryIt : CategoryToGuidToParameter)
		{
			for (const auto& It : CategoryIt.Value)
			{
				GuidToParameter.Add(It.Key, It.Value);
			}
		}
#endif
	}

	FixupParameterOverrides();
}

bool UVoxelGraph::IsFunctionLibrary() const
{
	const bool bIsFunctionLibrary = GetTypedOuter<UVoxelFunctionLibraryAsset>() != nullptr;
	if (bIsFunctionLibrary)
	{
		ensure(!PrivateBaseGraph);
	}
	return bIsFunctionLibrary;
}

FString UVoxelGraph::GetGraphTypeName() const
{
	FString TypeName = GetClass()->GetName();
	TypeName.RemoveFromStart("Voxel");
	TypeName.RemoveFromEnd("Graph");

	if (TypeName.IsEmpty())
	{
		return "legacy";
	}

	return TypeName.ToLower();
}

#if WITH_EDITOR
FVoxelGraphMetadata UVoxelGraph::GetMetadata() const
{
	FVoxelGraphMetadata Metadata;
	Metadata.DisplayName = DisplayNameOverride.IsEmpty() ? GetName() : DisplayNameOverride;
	Metadata.Category = Category;
	Metadata.Description = Description;
	return Metadata;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelGraph::HasMainTerminalGraph() const
{
	return GuidToTerminalGraph.Contains(GVoxelMainTerminalGraphGuid);
}

UVoxelTerminalGraph& UVoxelGraph::GetMainTerminalGraph()
{
	ensure(!IsFunctionLibrary());
	TObjectPtr<UVoxelTerminalGraph>& MainGraph = GuidToTerminalGraph.FindOrAdd(GVoxelMainTerminalGraphGuid);
	if (!ensure(MainGraph))
	{
		MainGraph = NewObject<UVoxelTerminalGraph>(this, "MainGraph", RF_Transactional);
		GuidToTerminalGraph.Add(GVoxelMainTerminalGraphGuid, MainGraph);
	}
	check(MainGraph);
	return *MainGraph;
}

const UVoxelTerminalGraph& UVoxelGraph::GetMainTerminalGraph() const
{
	return ConstCast(this)->GetMainTerminalGraph();
}

UVoxelTerminalGraph* UVoxelGraph::GetMainTerminalGraph_CheckBaseGraphs()
{
	for (UVoxelGraph* BaseGraph : GetBaseGraphs())
	{
		if (BaseGraph->HasMainTerminalGraph())
		{
			return &BaseGraph->GetMainTerminalGraph();
		}
	}

	return nullptr;
}

const UVoxelTerminalGraph* UVoxelGraph::GetMainTerminalGraph_CheckBaseGraphs() const
{
	return ConstCast(this)->GetMainTerminalGraph_CheckBaseGraphs();
}

UVoxelTerminalGraph& UVoxelGraph::GetEditorTerminalGraph()
{
	ensure(!IsFunctionLibrary());
	TObjectPtr<UVoxelTerminalGraph>& EditorGraph = GuidToTerminalGraph.FindOrAdd(GVoxelEditorTerminalGraphGuid);
	if (!ensure(EditorGraph))
	{
		EditorGraph = NewObject<UVoxelTerminalGraph>(this, "EditorGraph", RF_Transactional);
		GuidToTerminalGraph.Add(GVoxelEditorTerminalGraphGuid, EditorGraph);
	}
	check(EditorGraph);
	return *EditorGraph;
}

const UVoxelTerminalGraph& UVoxelGraph::GetEditorTerminalGraph() const
{
	return ConstCast(this)->GetEditorTerminalGraph();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<const FVoxelCompiledGraph> UVoxelGraph::GetCompiledGraph(FVoxelDependencyCollector& DependencyCollector) const
{
	VOXEL_FUNCTION_COUNTER();

	DependencyCollector.AddDependency(*CompiledGraphDependency);

	if (CachedCompiledGraph)
	{
		return CachedCompiledGraph.ToSharedRef();
	}

	OnCompiledGraphChangedPtr = MakeSharedVoid();

#if WITH_EDITOR
	const FOnVoxelGraphChanged OnChanged = FOnVoxelGraphChanged::Make(OnCompiledGraphChangedPtr, MakeWeakObjectPtrLambda(this, [this]
	{
		CachedCompiledGraph.Reset();

		FVoxelInvalidationScope Scope(this);
		CompiledGraphDependency->Invalidate();
	}));

	GVoxelGraphTracker->OnTerminalGraphChanged(*this).Add(OnChanged);
#endif

	const TVoxelSet<FGuid> TerminalGraphs = GetTerminalGraphs();

	TVoxelArray<TSharedPtr<FVoxelCompiledTerminalGraph>> TerminalGraphRefs;
	TerminalGraphRefs.Reserve(TerminalGraphs.Num());

	TVoxelMap<FGuid, const FVoxelCompiledTerminalGraph*> GuidToCompiledTerminalGraph;
	GuidToCompiledTerminalGraph.Reserve(TerminalGraphs.Num());

	for (const FGuid& Guid : TerminalGraphs)
	{
		const UVoxelTerminalGraph* TerminalGraph = FindTerminalGraph(Guid);
		if (!ensure(TerminalGraph))
		{
			continue;
		}

#if WITH_EDITOR
		GVoxelGraphTracker->OnTerminalGraphCompiled(*TerminalGraph).Add(OnChanged);
#endif

		UVoxelTerminalGraphRuntime& Runtime = TerminalGraph->GetRuntime();
		Runtime.EnsureIsCompiled();

		const TSharedPtr<FVoxelCompiledTerminalGraph> CompiledTerminalGraph = Runtime.CompiledGraph.GetValue();

		if (!CompiledTerminalGraph)
		{
#if WITH_EDITOR
			VOXEL_MESSAGE(Error, "{0}: failed to compile {1}", this, TerminalGraph->GetDisplayName());
#else
			VOXEL_MESSAGE(Error, "{0}: failed to compile", this);
#endif
		}

		TerminalGraphRefs.Add(CompiledTerminalGraph);
		GuidToCompiledTerminalGraph.Add_EnsureNew(Guid, CompiledTerminalGraph.Get());
	}

	CachedCompiledGraph = MakeShareable(new FVoxelCompiledGraph(
		this,
		MoveTemp(TerminalGraphRefs),
		MoveTemp(GuidToCompiledTerminalGraph)));

	return CachedCompiledGraph.ToSharedRef();
}

void UVoxelGraph::LoadAllGraphs()
{
	VOXEL_FUNCTION_COUNTER();

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	TArray<FAssetData> AssetDatas;

	FARFilter Filter;
	Filter.ClassPaths.Add(StaticClassFast<UVoxelFunctionLibraryAsset>()->GetClassPathName());

	for (const TSubclassOf<UVoxelGraph>& Class : GetDerivedClasses<UVoxelGraph>())
	{
		Filter.ClassPaths.Add(Class->GetClassPathName());
	}

	AssetRegistryModule.Get().GetAssets(Filter, AssetDatas);

	for (const FAssetData& AssetData : AssetDatas)
	{
		(void)AssetData.GetAsset();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelTerminalGraph* UVoxelGraph::FindTerminalGraph_NoInheritance(const FGuid& Guid)
{
	return GuidToTerminalGraph.FindRef(Guid);
}

const UVoxelTerminalGraph* UVoxelGraph::FindTerminalGraph_NoInheritance(const FGuid& Guid) const
{
	return ConstCast(this)->FindTerminalGraph_NoInheritance(Guid);
}

void UVoxelGraph::ForeachTerminalGraph_NoInheritance(TFunctionRef<void(UVoxelTerminalGraph&)> Lambda)
{
	for (const auto& It : GuidToTerminalGraph)
	{
		check(It.Value);
		Lambda(*It.Value);
	}
}

void UVoxelGraph::ForeachTerminalGraph_NoInheritance(TFunctionRef<void(const UVoxelTerminalGraph&)> Lambda) const
{
	ConstCast(this)->ForeachTerminalGraph_NoInheritance(ReinterpretCastRef<TFunctionRef<void(UVoxelTerminalGraph&)>>(Lambda));
}

FGuid UVoxelGraph::FindTerminalGraphGuid_NoInheritance(const UVoxelTerminalGraph* TerminalGraph) const
{
	VOXEL_FUNCTION_COUNTER();

	FGuid Result;
	for (const auto& It : GuidToTerminalGraph)
	{
		if (It.Value == TerminalGraph)
		{
			ensure(!Result.IsValid());
			Result = It.Key;
		}
	}
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelTerminalGraph* UVoxelGraph::FindTerminalGraph(const FGuid& Guid)
{
	for (const UVoxelGraph* BaseGraph : GetBaseGraphs())
	{
		if (UVoxelTerminalGraph* TerminalGraph = BaseGraph->GuidToTerminalGraph.FindRef(Guid))
		{
			return TerminalGraph;
		}
	}
	return nullptr;
}

const UVoxelTerminalGraph* UVoxelGraph::FindTerminalGraph(const FGuid& Guid) const
{
	return ConstCast(this)->FindTerminalGraph(Guid);
}

UVoxelTerminalGraph& UVoxelGraph::FindTerminalGraphChecked(const FGuid& Guid)
{
	UVoxelTerminalGraph* TerminalGraph = FindTerminalGraph(Guid);
	check(TerminalGraph);
	return *TerminalGraph;
}

const UVoxelTerminalGraph& UVoxelGraph::FindTerminalGraphChecked(const FGuid& Guid) const
{
	return ConstCast(this)->FindTerminalGraphChecked(Guid);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSet<FGuid> UVoxelGraph::GetTerminalGraphs() const
{
	TVoxelSet<FGuid> Result;
	Result.Reserve(GuidToTerminalGraph.Num());

	for (const UVoxelGraph* BaseGraph : GetBaseGraphs())
	{
		for (const auto& It : BaseGraph->GuidToTerminalGraph)
		{
			if (!ensure(It.Key.IsValid()) ||
				!ensure(It.Value))
			{
				continue;
			}

			Result.Add(It.Key);
		}
	}

	return Result;
}

const UVoxelTerminalGraph* UVoxelGraph::FindTopmostTerminalGraph(const FGuid& Guid) const
{
	if (!ensure(Guid != GVoxelMainTerminalGraphGuid) ||
		!ensure(Guid != GVoxelEditorTerminalGraphGuid))
	{
		return nullptr;
	}

	const UVoxelGraph* LastBaseGraph = this;
	for (const UVoxelGraph* BaseGraph : GetBaseGraphs())
	{
		if (!BaseGraph->FindTerminalGraph(Guid))
		{
			break;
		}

		LastBaseGraph = BaseGraph;
	}

	const UVoxelTerminalGraph* TerminalGraph = LastBaseGraph->FindTerminalGraph(Guid);
	ensure(TerminalGraph);
	return TerminalGraph;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
UVoxelTerminalGraph& UVoxelGraph::AddTerminalGraph(
	const FGuid& Guid,
	const UVoxelTerminalGraph* Template)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(!GuidToTerminalGraph.Contains(Guid)))
	{
		return *GuidToTerminalGraph[Guid];
	}
	ensure(!FindTerminalGraphGuid_NoInheritance(Template).IsValid());

	UVoxelTerminalGraph* NewTerminalGraph;
	if (Template)
	{
		ensure(!Template->HasAnyFlags(RF_ClassDefaultObject));
		NewTerminalGraph = DuplicateObject(Template, this, NAME_None);
	}
	else
	{
		NewTerminalGraph = NewObject<UVoxelTerminalGraph>(this, NAME_None, RF_Transactional);
	}
	NewTerminalGraph->SetGuid_Hack(Guid);

	GuidToTerminalGraph.Add(Guid, NewTerminalGraph);
	GVoxelGraphTracker->NotifyTerminalGraphChanged(*this);

	return *NewTerminalGraph;
}

void UVoxelGraph::RemoveTerminalGraph(const FGuid& Guid)
{
	VOXEL_FUNCTION_COUNTER();

	if (!GetBaseGraph_Unsafe() &&
		!ensure(Guid != GVoxelMainTerminalGraphGuid))
	{
		return;
	}

	if (!ensure(Guid != GVoxelEditorTerminalGraphGuid))
	{
		return;
	}

	TObjectPtr<UVoxelTerminalGraph> TerminalGraph;
	ensure(GuidToTerminalGraph.RemoveAndCopyValue(Guid, TerminalGraph));

	if (TerminalGraph)
	{
		// Make sure no delegates are fired after removal
		// This will be properly reverted by undo

		TerminalGraph->Modify();
		TerminalGraph->MarkAsGarbage();

		TerminalGraph->GetRuntime().Modify();
		TerminalGraph->GetRuntime().MarkAsGarbage();
	}

	GVoxelGraphTracker->NotifyTerminalGraphChanged(*this);
}

void UVoxelGraph::ReorderTerminalGraphs(const TConstVoxelArrayView<FGuid> NewGuids)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<FGuid> FinalGuids;
	if (HasMainTerminalGraph())
	{
		FinalGuids.Add(GVoxelMainTerminalGraphGuid);
	}
	FinalGuids.Add(GVoxelEditorTerminalGraphGuid);

	for (const FGuid& Guid : NewGuids)
	{
		ensure(FindTerminalGraph(Guid));

		if (GuidToTerminalGraph.Contains(Guid))
		{
			FinalGuids.AddUnique(Guid);
		}
	}

	FVoxelUtilities::ReorderMapKeys(GuidToTerminalGraph, FinalGuids);

	GVoxelGraphTracker->NotifyTerminalGraphChanged(*this);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const FVoxelParameter* UVoxelGraph::FindParameter(const FGuid& Guid) const
{
	for (const UVoxelGraph* BaseGraph : GetBaseGraphs())
	{
		if (const FVoxelParameter* Parameter = BaseGraph->GuidToParameter.Find(Guid))
		{
			return Parameter;
		}
	}
	return nullptr;
}

const FVoxelParameter& UVoxelGraph::FindParameterChecked(const FGuid& Guid) const
{
	const FVoxelParameter* Parameter = FindParameter(Guid);
	check(Parameter);
	return *Parameter;
}

int32 UVoxelGraph::NumParameters() const
{
	if (!PrivateBaseGraph)
	{
		return GuidToParameter.Num();
	}

	int32 Result = 0;
	for (const UVoxelGraph* BaseGraph : GetBaseGraphs())
	{
		Result += BaseGraph->GuidToParameter.Num();
	}
	return Result;
}

TVoxelSet<FGuid> UVoxelGraph::GetParameters() const
{
	TVoxelInlineArray<const UVoxelGraph*, 1> BaseGraphs = GetBaseGraphs();

	// Add parent parameters first
	Algo::Reverse(BaseGraphs);

	TVoxelSet<FGuid> Result;
	for (const UVoxelGraph* BaseGraph : BaseGraphs)
	{
		Result.ReserveGrow(BaseGraph->GuidToParameter.Num());

		for (const auto& It : BaseGraph->GuidToParameter)
		{
			Result.Add_EnsureNew_EnsureNoGrow(It.Key);
		}
	}
	return Result;
}

bool UVoxelGraph::IsInheritedParameter(const FGuid& Guid) const
{
	return
		ensure(FindParameter(Guid)) &&
		!GuidToParameter.Contains(Guid);
}

void UVoxelGraph::ForeachParameter(const TFunctionRef<void(const FGuid&, const FVoxelParameter&)> Lambda) const
{
	if (!PrivateBaseGraph)
	{
		for (const auto& It : GuidToParameter)
		{
			Lambda(It.Key, It.Value);
		}
		return;
	}

	TVoxelInlineArray<const UVoxelGraph*, 1> BaseGraphs = GetBaseGraphs();

	// Add parent parameters first
	Algo::Reverse(BaseGraphs);

	for (const UVoxelGraph* BaseGraph : BaseGraphs)
	{
		for (const auto& It : BaseGraph->GuidToParameter)
		{
			Lambda(It.Key, It.Value);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelGraph::AddParameter(const FGuid& Guid, const FVoxelParameter& Parameter)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(!FindParameter(Guid));

	GuidToParameter.Add(Guid, Parameter);

	GVoxelGraphTracker->NotifyParameterChanged(*this);
}

void UVoxelGraph::RemoveParameter(const FGuid& Guid)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(GuidToParameter.Remove(Guid));

	GVoxelGraphTracker->NotifyParameterChanged(*this);
}

void UVoxelGraph::UpdateParameter(const FGuid& Guid, TFunctionRef<void(FVoxelParameter&)> Update)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelParameter* Parameter = GuidToParameter.Find(Guid);
	if (!ensure(Parameter))
	{
		return;
	}
	const FVoxelParameter PreviousParameter = *Parameter;

	Update(*Parameter);

	// If category changed move the parameter to the end to avoid reordering an entire category as a side effect
	if (PreviousParameter.Category != Parameter->Category)
	{
		FVoxelParameter ParameterCopy;
		verify(GuidToParameter.RemoveAndCopyValue(Guid, ParameterCopy));
		GuidToParameter.CompactStable();
		GuidToParameter.Add(Guid, ParameterCopy);
	}

	GVoxelGraphTracker->NotifyParameterChanged(*this);
}

void UVoxelGraph::ReorderParameters(const TConstVoxelArrayView<FGuid> NewGuids)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<FGuid> FinalGuids;
	for (const FGuid& Guid : NewGuids)
	{
		ensure(FindParameter(Guid));

		if (GuidToParameter.Contains(Guid))
		{
			FinalGuids.Add(Guid);
		}
	}
	FVoxelUtilities::ReorderMapKeys(GuidToParameter, FinalGuids);

	GVoxelGraphTracker->NotifyParameterChanged(*this);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelGraph* UVoxelGraph::GetBaseGraph_Unsafe() const
{
	if (!ensure(PrivateBaseGraph != this))
	{
		return nullptr;
	}
	if (PrivateBaseGraph)
	{
		ensure(!IsFunctionLibrary());
	}
	return PrivateBaseGraph;
}

#if WITH_EDITOR
void UVoxelGraph::SetBaseGraph(UVoxelGraph* NewBaseGraph)
{
	ensure(!IsFunctionLibrary());

	if (PrivateBaseGraph == NewBaseGraph)
	{
		return;
	}
	PrivateBaseGraph = NewBaseGraph;

	GVoxelGraphTracker->NotifyBaseGraphChanged(*this);
}
#endif

TVoxelInlineArray<UVoxelGraph*, 1> UVoxelGraph::GetBaseGraphs()
{
	if (!PrivateBaseGraph)
	{
		return { this };
	}

	TVoxelInlineSet<UVoxelGraph*, 2> Result;
	for (UVoxelGraph* Graph = this; Graph; Graph = ResolveObjectPtrFast(Graph->PrivateBaseGraph))
	{
		if (!Result.TryAdd(Graph))
		{
			VOXEL_MESSAGE(Error, "Hierarchy loop detected: {0}", Result.Array());
			return { this };
		}
	}
	return Result.Array<TInlineAllocator<1>>();
}

TVoxelInlineArray<const UVoxelGraph*, 1> UVoxelGraph::GetBaseGraphs() const
{
	return ConstCast(this)->GetBaseGraphs();
}

TVoxelSet<UVoxelGraph*> UVoxelGraph::GetChildGraphs_LoadedOnly()
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelSet<UVoxelGraph*> Result;
	Result.Reserve(64);
	ForEachObjectOfClass<UVoxelGraph>([&](UVoxelGraph& Graph)
	{
		if (Graph.GetBaseGraphs().Contains(this))
		{
			Result.Add(&Graph);
		}
	});
	return Result;
}

TVoxelSet<const UVoxelGraph*> UVoxelGraph::GetChildGraphs_LoadedOnly() const
{
	return ConstCast(this)->GetChildGraphs_LoadedOnly();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelGraph::PostLoad()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostLoad();

	Fixup();
}

void UVoxelGraph::PostInitProperties()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostInitProperties();

	FixupParameterOverrides();
}

void UVoxelGraph::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
void UVoxelGraph::PostEditUndo()
{
	Super::PostEditUndo();

	FixupParameterOverrides();
}

void UVoxelGraph::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!AssetIcon.bCustomIcon)
	{
		ThumbnailTools::CacheEmptyThumbnail(GetFullName(), GetPackage());
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_OWN_MEMBER_NAME(Category) ||
		PropertyChangedEvent.GetMemberPropertyName() == GET_OWN_MEMBER_NAME(Description) ||
		PropertyChangedEvent.GetMemberPropertyName() == GET_OWN_MEMBER_NAME(DisplayNameOverride))
	{
		if (HasMainTerminalGraph())
		{
			GVoxelGraphTracker->NotifyTerminalGraphMetaDataChanged(GetMainTerminalGraph());
		}
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_OWN_MEMBER_NAME(ParameterOverrides))
	{
		GVoxelGraphTracker->NotifyParameterValueChanged(*this);
	}
}

void UVoxelGraph::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);

	if (HasMainTerminalGraph())
	{
		GVoxelGraphTracker->NotifyTerminalGraphMetaDataChanged(GetMainTerminalGraph());
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelGraph::ShouldForceEnableOverride(const FGuid& Guid) const
{
	// Force enable our own parameters
	return GuidToParameter.Contains(Guid);
}

FVoxelParameterOverrides& UVoxelGraph::GetParameterOverrides()
{
	return ParameterOverrides;
}

FProperty* UVoxelGraph::GetParameterOverridesProperty() const
{
	return &FindFPropertyChecked(UVoxelGraph, ParameterOverrides);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FString UVoxelGraph::FFactoryInfo::GetDisplayName(const UClass* Class) const
{
	if (!DisplayNameOverride.IsEmpty())
	{
		return DisplayNameOverride;
	}

	FString Result = Class->GetDisplayNameText().ToString();
	Result.RemoveFromStart("Voxel ");
	return Result;
}
#endif