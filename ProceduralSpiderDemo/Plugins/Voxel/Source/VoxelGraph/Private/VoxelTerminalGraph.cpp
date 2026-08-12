// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelTerminalGraph.h"
#include "VoxelGraph.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraphRuntime.h"
#include "FunctionLibrary/VoxelCurveFunctionLibrary.h"

void FVoxelGraphFunctionInput::Fixup()
{
	DefaultPinValue.Fixup(Type.GetExposedType());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelGraph& UVoxelTerminalGraph::GetGraph()
{
	return *GetOuterUVoxelGraph();
}

const UVoxelGraph& UVoxelTerminalGraph::GetGraph() const
{
	return *GetOuterUVoxelGraph();
}

FGuid UVoxelTerminalGraph::GetGuid() const
{
	if (GetGraph().FindTerminalGraph_NoInheritance(PrivateGuid) != this)
	{
		ensureMsgf(!PrivateGuid.IsValid(),
			TEXT("Terminal graph '%s' (Guid=%s) unexpectedly has a valid guid while not registered in graph '%s'"),
			*GetPathName(),
			*PrivateGuid.ToString(),
			*GetGraph().GetPathName());
		ensureMsgf(!GetGraph().FindTerminalGraph_NoInheritance(PrivateGuid),
			TEXT("Terminal graph '%s' (Guid=%s) is not the registered terminal graph for guid in graph '%s'"),
			*GetPathName(),
			*PrivateGuid.ToString(),
			*GetGraph().GetPathName());

		ConstCast(this)->PrivateGuid = GetGraph().FindTerminalGraphGuid_NoInheritance(this);
			ensureMsgf(PrivateGuid.IsValid(),
				TEXT("Terminal graph '%s' failed to resolve a valid guid in graph '%s'"),
				*GetPathName(),
				*GetGraph().GetPathName());
	}
	return PrivateGuid;
}

void UVoxelTerminalGraph::SetGuid_Hack(const FGuid& Guid)
{
	PrivateGuid = Guid;
}

#if WITH_EDITOR
FString UVoxelTerminalGraph::GetDisplayName() const
{
	return GetMetadata().DisplayName;
}

FVoxelGraphMetadata UVoxelTerminalGraph::GetMetadata() const
{
	if (IsMainTerminalGraph() ||
		IsEditorTerminalGraph())
	{
		return GetGraph().GetMetadata();
	}

	const UVoxelTerminalGraph* TopmostTerminalGraph = GetGraph().FindTopmostTerminalGraph(GetGuid());
	if (!ensure(TopmostTerminalGraph))
	{
		return {};
	}

	return TopmostTerminalGraph->PrivateMetadata;
}

void UVoxelTerminalGraph::UpdateMetadata(TFunctionRef<void(FVoxelGraphMetadata&)> Lambda)
{
	if (!ensure(!IsMainTerminalGraph()) ||
		!ensure(!IsEditorTerminalGraph()) ||
		!ensure(IsTopmostTerminalGraph()))
	{
		return;
	}

	Lambda(PrivateMetadata);

	if (PrivateMetadata.DisplayName.IsEmpty())
	{
		PrivateMetadata.DisplayName = "MyFunction";
	}

	GVoxelGraphTracker->NotifyTerminalGraphMetaDataChanged(*this);
}

bool UVoxelTerminalGraph::CanBePlaced(const UVoxelGraph& Graph) const
{
	if (WhitelistedTypes.Num() == 0 ||
		Graph.IsFunctionLibrary())
	{
		return true;
	}

	const FString GraphType = Graph.GetGraphTypeName();
	for (const FString& Type : WhitelistedTypes)
	{
		if (Type == GraphType)
		{
			return true;
		}
	}

	return false;
}

TArray<FString> UVoxelTerminalGraph::GetWhitelistGraphTypes() const
{
	TArray<FString> Result;
	for (const UClass* DerivedClass : GetDerivedClasses<UVoxelGraph>())
	{
		FString Name = DerivedClass->GetName();
		Name.RemoveFromStart("Voxel");
		Name.RemoveFromEnd("Graph");
		Result.Add(Name);
	}

	return Result;
}
#endif

#if WITH_EDITOR
void UVoxelTerminalGraph::SetEdGraph_Hack(UEdGraph* NewEdGraph)
{
	EdGraph = NewEdGraph;
}

void UVoxelTerminalGraph::SetDisplayName_Hack(const FString& Name)
{
	PrivateMetadata.DisplayName = Name;
}

UEdGraph& UVoxelTerminalGraph::GetEdGraph()
{
	if (!EdGraph)
	{
		check(GVoxelGraphEditorInterface);
		EdGraph = GVoxelGraphEditorInterface->CreateEdGraph(*this);
	}
	return *EdGraph;
}

const UEdGraph& UVoxelTerminalGraph::GetEdGraph() const
{
	return ConstCast(this)->GetEdGraph();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const FVoxelGraphFunctionInput* UVoxelTerminalGraph::FindInput(const FGuid& Guid) const
{
	for (const UVoxelTerminalGraph* TerminalGraph : GetBaseTerminalGraphs())
	{
		if (const FVoxelGraphFunctionInput* Input = TerminalGraph->GuidToFunctionInput.Find(Guid))
		{
			return Input;
		}
	}
	return nullptr;
}

const FVoxelGraphFunctionOutput* UVoxelTerminalGraph::FindOutput(const FGuid& Guid) const
{
	for (const UVoxelTerminalGraph* TerminalGraph : GetBaseTerminalGraphs())
	{
		if (const FVoxelGraphFunctionOutput* Output = TerminalGraph->GuidToFunctionOutput.Find(Guid))
		{
			return Output;
		}
	}
	return nullptr;
}

const FVoxelGraphFunctionOutput* UVoxelTerminalGraph::FindOutputByName(const FName& Name, FGuid& OutGuid) const
{
	for (const UVoxelTerminalGraph* TerminalGraph : GetBaseTerminalGraphs())
	{
		for (const auto& It : TerminalGraph->GuidToFunctionOutput)
		{
			if (It.Value.Name == Name)
			{
				OutGuid = It.Key;
				return &It.Value;
			}
		}
	}
	return nullptr;
}

const FVoxelGraphLocalVariable* UVoxelTerminalGraph::FindLocalVariable(const FGuid& Guid) const
{
	// No inheritance
	return GuidToLocalVariable.Find(Guid);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const FVoxelGraphFunctionInput& UVoxelTerminalGraph::FindInputChecked(const FGuid& Guid) const
{
	const FVoxelGraphFunctionInput* Input = FindInput(Guid);
	check(Input);
	return *Input;
}

const FVoxelGraphFunctionOutput& UVoxelTerminalGraph::FindOutputChecked(const FGuid& Guid) const
{
	const FVoxelGraphFunctionOutput* Output = FindOutput(Guid);
	check(Output);
	return *Output;
}

const FVoxelGraphLocalVariable& UVoxelTerminalGraph::FindLocalVariableChecked(const FGuid& Guid) const
{
	const FVoxelGraphLocalVariable* LocalVariable = FindLocalVariable(Guid);
	check(LocalVariable);
	return *LocalVariable;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSet<FGuid> UVoxelTerminalGraph::GetFunctionInputs() const
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsFunction());

	TVoxelSet<FGuid> Result;
	// Add parent inputs first
	for (const UVoxelTerminalGraph* TerminalGraph : GetBaseTerminalGraphs().Reverse())
	{
		for (const auto& It : TerminalGraph->GuidToFunctionInput)
		{
			ensure(!Result.Contains(It.Key));
			Result.Add(It.Key);
		}
	}
	return Result;
}

TVoxelSet<FGuid> UVoxelTerminalGraph::GetFunctionOutputs() const
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsFunction());

	TVoxelSet<FGuid> Result;
	// Add parent outputs first
	for (const UVoxelTerminalGraph* TerminalGraph : GetBaseTerminalGraphs().Reverse())
	{
		for (const auto& It : TerminalGraph->GuidToFunctionOutput)
		{
			ensure(!Result.Contains(It.Key));
			Result.Add(It.Key);
		}
	}
	return Result;
}

TVoxelSet<FGuid> UVoxelTerminalGraph::GetLocalVariables() const
{
	TVoxelSet<FGuid> Result;
	for (const auto& It : GuidToLocalVariable)
	{
		Result.Add(It.Key);
	}
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelTerminalGraph::IsInheritedInput(const FGuid& Guid) const
{
	return
		ensure(FindInput(Guid)) &&
		!GuidToFunctionInput.Contains(Guid);
}

bool UVoxelTerminalGraph::IsInheritedOutput(const FGuid& Guid) const
{
	return
		ensure(FindOutput(Guid)) &&
		!GuidToFunctionOutput.Contains(Guid);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelTerminalGraph::AddFunctionInput(const FGuid& Guid, const FVoxelGraphFunctionInput& Input)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsFunction());

	ensure(!FindInput(Guid));
	GuidToFunctionInput.Add(Guid, Input);

	GVoxelGraphTracker->NotifyInputChanged(*this);
}

void UVoxelTerminalGraph::AddFunctionOutput(const FGuid& Guid, const FVoxelGraphFunctionOutput& Output)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsFunction());

	ensure(!FindOutput(Guid));
	GuidToFunctionOutput.Add(Guid, Output);

	GVoxelGraphTracker->NotifyOutputChanged(*this);
}

void UVoxelTerminalGraph::AddLocalVariable(const FGuid& Guid, const FVoxelGraphLocalVariable& LocalVariable)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(!FindLocalVariable(Guid));
	GuidToLocalVariable.Add(Guid, LocalVariable);

	GVoxelGraphTracker->NotifyLocalVariableChanged(*this);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelTerminalGraph::RemoveFunctionInput(const FGuid& Guid)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsFunction());

	ensure(GuidToFunctionInput.Remove(Guid));

	GVoxelGraphTracker->NotifyInputChanged(*this);
}

void UVoxelTerminalGraph::RemoveFunctionOutput(const FGuid& Guid)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsFunction());

	ensure(GuidToFunctionOutput.Remove(Guid));

	GVoxelGraphTracker->NotifyOutputChanged(*this);
}

void UVoxelTerminalGraph::RemoveLocalVariable(const FGuid& Guid)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(GuidToLocalVariable.Remove(Guid));

	GVoxelGraphTracker->NotifyLocalVariableChanged(*this);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelTerminalGraph::UpdateFunctionInput(const FGuid& Guid, TFunctionRef<void(FVoxelGraphFunctionInput&)> Update)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsFunction());

	FVoxelGraphFunctionInput* Input = GuidToFunctionInput.Find(Guid);
	if (!ensure(Input))
	{
		return;
	}
	const FVoxelGraphFunctionInput PreviousInput = *Input;

	Update(*Input);

	// If category changed move the member to the end to avoid reordering an entire category as a side effect
	if (PreviousInput.Category != Input->Category)
	{
		FVoxelGraphFunctionInput InputCopy;
		verify(GuidToFunctionInput.RemoveAndCopyValue(Guid, InputCopy));
		GuidToFunctionInput.CompactStable();
		GuidToFunctionInput.Add(Guid, InputCopy);
	}

	GVoxelGraphTracker->NotifyInputChanged(*this);
}

void UVoxelTerminalGraph::UpdateFunctionOutput(const FGuid& Guid, TFunctionRef<void(FVoxelGraphFunctionOutput&)> Update)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsFunction());

	FVoxelGraphFunctionOutput* Output = GuidToFunctionOutput.Find(Guid);
	if (!ensure(Output))
	{
		return;
	}
	const FVoxelGraphFunctionOutput PreviousOutput = *Output;

	Update(*Output);

	// If category changed move the member to the end to avoid reordering an entire category as a side effect
	if (PreviousOutput.Category != Output->Category)
	{
		FVoxelGraphFunctionOutput OutputCopy;
		verify(GuidToFunctionOutput.RemoveAndCopyValue(Guid, OutputCopy));
		GuidToFunctionOutput.CompactStable();
		GuidToFunctionOutput.Add(Guid, OutputCopy);
	}

	GVoxelGraphTracker->NotifyOutputChanged(*this);
}

void UVoxelTerminalGraph::UpdateLocalVariable(const FGuid& Guid, TFunctionRef<void(FVoxelGraphLocalVariable&)> Update)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelGraphLocalVariable* LocalVariable = GuidToLocalVariable.Find(Guid);
	if (!ensure(LocalVariable))
	{
		return;
	}
	const FVoxelGraphLocalVariable PreviousLocalVariable = *LocalVariable;

	Update(*LocalVariable);

	// If category changed move the member to the end to avoid reordering an entire category as a side effect
	if (PreviousLocalVariable.Category != LocalVariable->Category)
	{
		FVoxelGraphLocalVariable LocalVariableCopy;
		verify(GuidToLocalVariable.RemoveAndCopyValue(Guid, LocalVariableCopy));
		GuidToLocalVariable.CompactStable();
		GuidToLocalVariable.Add(Guid, LocalVariableCopy);
	}

	GVoxelGraphTracker->NotifyLocalVariableChanged(*this);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelTerminalGraph::ReorderFunctionInputs(const TConstVoxelArrayView<FGuid> NewGuids)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsFunction());

	TVoxelArray<FGuid> FinalGuids;
	for (const FGuid& Guid : NewGuids)
	{
		ensure(FindInput(Guid));

		if (GuidToFunctionInput.Contains(Guid))
		{
			FinalGuids.Add(Guid);
		}
	}
	FVoxelUtilities::ReorderMapKeys(GuidToFunctionInput, FinalGuids);

	GVoxelGraphTracker->NotifyInputChanged(*this);
}

void UVoxelTerminalGraph::ReorderFunctionOutputs(const TConstVoxelArrayView<FGuid> NewGuids)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsFunction());

	TVoxelArray<FGuid> FinalGuids;
	for (const FGuid& Guid : NewGuids)
	{
		ensure(FindOutput(Guid));

		if (GuidToFunctionOutput.Contains(Guid))
		{
			FinalGuids.Add(Guid);
		}
	}
	FVoxelUtilities::ReorderMapKeys(GuidToFunctionOutput, FinalGuids);

	GVoxelGraphTracker->NotifyOutputChanged(*this);
}

auto UVoxelTerminalGraph::ReorderLocalVariables(const TConstVoxelArrayView<FGuid> NewGuids) -> void
{
	VOXEL_FUNCTION_COUNTER();

	// No fixup needed because no inheritance
	FVoxelUtilities::ReorderMapKeys(GuidToLocalVariable, NewGuids);

	GVoxelGraphTracker->NotifyLocalVariableChanged(*this);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelTerminalGraph::UVoxelTerminalGraph()
{
	Runtime = CreateDefaultSubobject<UVoxelTerminalGraphRuntime>("Runtime");

	SetFlags(RF_Public);
	Runtime->SetFlags(RF_Public);
}

void UVoxelTerminalGraph::Fixup()
{
	VOXEL_FUNCTION_COUNTER();
	ensure(!HasAnyFlags(RF_NeedPostLoad));

	// Fixup GUID
	{
		const FGuid Guid = GetGraph().FindTerminalGraphGuid_NoInheritance(this);
		ensure(!PrivateGuid.IsValid() || PrivateGuid == Guid);

		PrivateGuid = Guid;
	}

	// Fixup display name
#if WITH_EDITOR
	if (!IsMainTerminalGraph() &&
		!IsEditorTerminalGraph() &&
		IsTopmostTerminalGraph() &&
		PrivateMetadata.DisplayName.IsEmpty())
	{
		PrivateMetadata.DisplayName = "MyFunction";
	}
#endif

	// Fixup input default values
	for (auto& It : GuidToFunctionInput)
	{
		It.Value.Fixup();
	}

	const auto FixupMap = [&](auto& GuidToProperty)
	{
		TVoxelSet<FName> Names;

		// Ensure names are unique
		for (auto& It : GuidToProperty)
		{
			FVoxelGraphProperty& Property = It.Value;
#if WITH_EDITOR
			Property.Category = FVoxelUtilities::SanitizeCategory(Property.Category);
#endif

			if (Property.Name.IsNone())
			{
				Property.Name = "MyMember";
			}

			while (Names.Contains(Property.Name))
			{
				Property.Name.SetNumber(Property.Name.GetNumber() + 1);
			}

			// TODO REMOVE MIGRATION
			if (Property.Type.Is<FVoxelCurve>())
			{
				Property.Type = FVoxelPinType::Make<FVoxelRuntimeCurveRef>();
			}

			Names.Add(Property.Name);
		}

		using Type = typename VOXEL_GET_TYPE(GuidToProperty)::ValueType;

#if WITH_EDITOR
		// Sort elements by category
		TMap<FString, TMap<FGuid, Type>> CategoryToGuidToProperty;
		for (const auto& It : GuidToProperty)
		{
			CategoryToGuidToProperty.FindOrAdd(It.Value.Category).Add(It.Key, It.Value);
		}

		GuidToProperty.Empty();
		for (const auto& CategoryIt : CategoryToGuidToProperty)
		{
			for (const auto& It : CategoryIt.Value)
			{
				GuidToProperty.Add(It.Key, It.Value);
			}
		}
#endif
	};

	FixupMap(GuidToFunctionInput);
	FixupMap(GuidToFunctionOutput);
	FixupMap(GuidToLocalVariable);
}

bool UVoxelTerminalGraph::IsFunction() const
{
	if (IsEditorTerminalGraph())
	{
		return false;
	}

	if (GetGraph().IsFunctionLibrary())
	{
		// Main graph is a function in function libraries
		return true;
	}

	if (IsMainTerminalGraph())
	{
		return false;
	}

	return true;
}

bool UVoxelTerminalGraph::IsMainTerminalGraph() const
{
	return GetGuid() == GVoxelMainTerminalGraphGuid;
}

bool UVoxelTerminalGraph::IsEditorTerminalGraph() const
{
	return GetGuid() == GVoxelEditorTerminalGraphGuid;
}

bool UVoxelTerminalGraph::IsTopmostTerminalGraph() const
{
	ensure(!IsMainTerminalGraph());
	ensure(!IsEditorTerminalGraph());
	return GetGraph().FindTopmostTerminalGraph(GetGuid()) == this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelInlineSet<UVoxelTerminalGraph*, 8> UVoxelTerminalGraph::GetBaseTerminalGraphs()
{
	const FGuid Guid = GetGuid();

	TVoxelInlineSet<UVoxelTerminalGraph*, 8> Result;
	for (UVoxelGraph* BaseGraph : GetGraph().GetBaseGraphs())
	{
		UVoxelTerminalGraph* TerminalGraph = BaseGraph->FindTerminalGraph_NoInheritance(Guid);
		if (!TerminalGraph ||
			!ensure(!Result.Contains(TerminalGraph)))
		{
			continue;
		}

		Result.Add_CheckNew(TerminalGraph);
	}
	return Result;
}

TVoxelInlineSet<const UVoxelTerminalGraph*, 8> UVoxelTerminalGraph::GetBaseTerminalGraphs() const
{
	return ConstCast(this)->GetBaseTerminalGraphs();
}

TVoxelSet<UVoxelTerminalGraph*> UVoxelTerminalGraph::GetChildTerminalGraphs_LoadedOnly()
{
	VOXEL_FUNCTION_COUNTER();

	const FGuid Guid = GetGuid();

	TVoxelSet<UVoxelTerminalGraph*> Result;
	for (UVoxelGraph* ChildGraph : GetGraph().GetChildGraphs_LoadedOnly())
	{
		UVoxelTerminalGraph* TerminalGraph = ChildGraph->FindTerminalGraph_NoInheritance(Guid);
		if (!TerminalGraph ||
			!ensure(!Result.Contains(TerminalGraph)))
		{
			continue;
		}

		Result.Add_CheckNew(TerminalGraph);
	}
	return Result;
}

TVoxelSet<const UVoxelTerminalGraph*> UVoxelTerminalGraph::GetChildTerminalGraphs_LoadedOnly() const
{
	return ConstCast(this)->GetChildTerminalGraphs_LoadedOnly();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelTerminalGraph::PostLoad()
{
	Super::PostLoad();

	// Ensure Graph is fixed up first so we have a valid GUID
	GetGraph().ConditionalPostLoad();

	if (!GetGraph().FindTerminalGraphGuid_NoInheritance(this).IsValid())
	{
		// Orphan parameter graph
		return;
	}

	Fixup();

	OnLoaded.Broadcast();
}

void UVoxelTerminalGraph::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
void UVoxelTerminalGraph::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_OWN_MEMBER_NAME(bDisableNodeMerging))
	{
		GVoxelGraphTracker->NotifyEdGraphChanged(GetEdGraph());
	}
}
#endif