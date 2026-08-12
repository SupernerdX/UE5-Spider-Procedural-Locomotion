// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGApplyOnVoxelGraphSettings.h"
#include "PCGVoxelGraphParameterOverrides.h"
#include "VoxelParameterOverridesOwner.h"

TArray<FPCGPinProperties> UPCGApplyOnVoxelGraphSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Any, /*bAllowMultipleConnections=*/true, /*bAllowMultipleData=*/true);
	PinProperties.Emplace("Property Overrides", EPCGDataType::Any, /*bAllowMultipleConnections=*/true, /*bAllowMultipleData=*/true);
	PinProperties.Emplace(PCGPinConstants::UE_506_SWITCH(DefaultDependencyOnlyLabel, DefaultExecutionDependencyLabel), EPCGDataType::Any, /*bAllowMultipleConnections=*/true, /*bAllowMultipleData=*/true);
	return PinProperties;
}

TArray<FPCGPinProperties> UPCGApplyOnVoxelGraphSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Any);

	return PinProperties;
}

FPCGElementPtr UPCGApplyOnVoxelGraphSettings::CreateElement() const
{
	return MakeShared<FPCGApplyOnVoxelGraphElement>();
}

void UPCGApplyOnVoxelGraphSettings::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FPCGApplyOnVoxelGraphContext::InitializeAndRequestLoad(
	const FName InputPinName,
	const FPCGAttributePropertyInputSelector& InputAttributeSelector,
	const TArray<FSoftObjectPath>& StaticObjectPaths,
	const bool bPersistAllData,
	const bool bSilenceErrorOnEmptyObjectPath,
	const bool bSynchronousLoad)
{
	if (!WasLoadRequested())
	{
		const bool InPinIsConnected = Node && Node->IsInputPinConnected(InputPinName);

		// First gather all the soft objects paths to extract
		TArray<FSoftObjectPath> ObjectsToLoad;
		if (!InPinIsConnected)
		{
			PathsToObjectsAndDataIndex.Reserve(StaticObjectPaths.Num());
			for (int32 PathIndex = 0; PathIndex < StaticObjectPaths.Num(); ++PathIndex)
			{
				const FSoftObjectPath& Path = StaticObjectPaths[PathIndex];
				if (!Path.IsNull())
				{
					ObjectsToLoad.AddUnique(Path);
				}

				PathsToObjectsAndDataIndex.Emplace(Path, -1, PathIndex);
			}
		}
		else
		{
			const TArray<FPCGTaggedData> Inputs = InputData.GetInputsByPin(InputPinName);
			for (int32 Index = 0; Index < Inputs.Num(); ++Index)
			{
				const FPCGTaggedData& Input = Inputs[Index];

				if (!Input.Data)
				{
					PCGE_LOG_C(Error, GraphAndLog, this, FText::Format(INVTEXT("Invalid data for input {0}"), FText::AsNumber(Index)));
					continue;
				}

				const FPCGAttributePropertyInputSelector AttributeSelector = InputAttributeSelector.CopyAndFixLast(Input.Data);
				const TUniquePtr<const IPCGAttributeAccessor> Accessor = PCGAttributeAccessorHelpers::CreateConstAccessor(Input.Data, AttributeSelector);
				const TUniquePtr<const IPCGAttributeAccessorKeys> Keys = PCGAttributeAccessorHelpers::CreateConstKeys(Input.Data, AttributeSelector);

				if (!Accessor.IsValid() || !Keys.IsValid())
				{
					if (bPersistAllData)
					{
						// Special case for empty data. We need this case if we ever chain this node multiple times. An empty param (with no attributes and no entries) will generate another empty param.
						const UPCGMetadata* Metadata = Input.Data->ConstMetadata();
						if (Metadata && Metadata->GetAttributeCount() == 0 && Metadata->GetLocalItemCount() == 0)
						{
							// Emplace empty path for this input. Will generate an empty param data.
							PathsToObjectsAndDataIndex.Emplace(FSoftObjectPath(), Index, -1);
						}
					}

					if (!bSilenceErrorOnEmptyObjectPath)
					{
						PCGE_LOG_C(Error, GraphAndLog, this, FText::Format(INVTEXT("Attribute/Property '{0}' does not exist on input {1}"), AttributeSelector.GetDisplayText(), FText::AsNumber(Index)));
					}

					continue;
				}

				const int32 NumElementsToAdd = Keys->GetNum();
				if (NumElementsToAdd == 0)
				{
					continue;
				}

				// Extract value as String to validate that a path is empty or ill-formed (because any ill-formed path will be null).
				TArray<FString> InputValues;
				InputValues.SetNum(NumElementsToAdd);
				if (Accessor->GetRange(MakeArrayView(InputValues), 0, *Keys, EPCGAttributeAccessorFlags::AllowConstructible | EPCGAttributeAccessorFlags::AllowBroadcast))
				{
					PathsToObjectsAndDataIndex.Reserve(PathsToObjectsAndDataIndex.Num() + NumElementsToAdd);
					for (int32 i = 0; i < InputValues.Num(); ++i)
					{
						FString& StringPath = InputValues[i];
						// Empty SoftObjectPath can convert to string to None and is treated as empty, so check that one too.
						const bool PathIsEmpty = StringPath.IsEmpty() || StringPath.Equals(TEXT("None"), ESearchCase::CaseSensitive);
						if (PathIsEmpty && bPersistAllData)
						{
							PathsToObjectsAndDataIndex.Emplace(FSoftObjectPath(), Index, i);
						}

						FSoftObjectPath Path(std::move(StringPath));

						if (!Path.IsNull())
						{
							ObjectsToLoad.AddUnique(Path);
							PathsToObjectsAndDataIndex.Emplace(std::move(Path), Index, i);
						}
						else
						{
							if (!PathIsEmpty || !bSilenceErrorOnEmptyObjectPath)
							{
								PCGE_LOG_C(Error, GraphAndLog, this, FText::Format(INVTEXT("Value number {0} for Attribute/Property '{1}' on input {2} is not a valid path or is null. Will be ignored."), FText::AsNumber(i), AttributeSelector.GetDisplayText(), FText::AsNumber(Index)));
							}

							continue;
						}
					}
				}
				else
				{
					PCGE_LOG_C(Error, GraphAndLog, this, FText::Format(INVTEXT("Attribute/Property '{0}'({1}) is not convertible to a SoftObjectPath on input {2}"), AttributeSelector.GetDisplayText(), PCG::Private::GetTypeNameText(Accessor->GetUnderlyingType()), FText::AsNumber(Index)));
					continue;
				}
			}
		}

		if (!ObjectsToLoad.IsEmpty())
		{
			return RequestResourceLoad(this, std::move(ObjectsToLoad), !bSynchronousLoad);
		}
	}

	return true;
}

FPCGContext* FPCGApplyOnVoxelGraphElement::CreateContext()
{
	return new FPCGApplyOnVoxelGraphContext();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FPCGApplyOnVoxelGraphElement::PrepareDataInternal(FPCGContext* InContext) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGApplyOnActorElement::PrepareData);
	check(InContext);

	const UPCGApplyOnVoxelGraphSettings* Settings = InContext->GetInputSettings<UPCGApplyOnVoxelGraphSettings>();
	check(Settings);

	FPCGApplyOnVoxelGraphContext* ThisContext = static_cast<FPCGApplyOnVoxelGraphContext*>(InContext);
	return ThisContext->InitializeAndRequestLoad(PCGPinConstants::DefaultInputLabel,
		Settings->ObjectReferenceAttribute,
		{},
		/*bPersistAllData=*/false,
		Settings->bSilenceErrorOnEmptyObjectPath,
		Settings->bSynchronousLoad);
}

bool FPCGApplyOnVoxelGraphElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGApplyOnActorElement::Execute);

	check(Context);
	FPCGApplyOnVoxelGraphContext* ThisContext = static_cast<FPCGApplyOnVoxelGraphContext*>(Context);

	const UPCGApplyOnVoxelGraphSettings* Settings = Context->GetInputSettings<UPCGApplyOnVoxelGraphSettings>();
	check(Settings);

	TArray<TPair<UObject*, int32>> TargetObjectsAndIndices;

	const TArray<FPCGTaggedData> OverrideInputs = Context->InputData.GetInputsByPin("Property Overrides");
	if (OverrideInputs.Num() == 0)
	{
		return true;
	}

	int32 CurrentPathIndex = 0;
	while (CurrentPathIndex < ThisContext->PathsToObjectsAndDataIndex.Num())
	{
		TargetObjectsAndIndices.Reset();

		int32 InputIndex = ThisContext->PathsToObjectsAndDataIndex[CurrentPathIndex].Get<1>();
		while (CurrentPathIndex < ThisContext->PathsToObjectsAndDataIndex.Num() && InputIndex == ThisContext->PathsToObjectsAndDataIndex[CurrentPathIndex].Get<1>())
		{
			TargetObjectsAndIndices.Emplace(ThisContext->PathsToObjectsAndDataIndex[CurrentPathIndex].Get<0>().ResolveObject(), ThisContext->PathsToObjectsAndDataIndex[CurrentPathIndex].Get<2>());
			++CurrentPathIndex;
		}

		int32 PropertyInputIndex = InputIndex;
		if (OverrideInputs.Num() == 1)
		{
			PropertyInputIndex = 0;
		}
		else if (OverrideInputs.Num() > 1 && !OverrideInputs.IsValidIndex(PropertyInputIndex))
		{
			PCGLog::LogWarningOnGraph(FText::Format(INVTEXT("The data provided on pin '{0}' does not have a consistent size with the input index '{1}'. Will use the first one."), FText::FromString("Property Overrides"), FText::AsNumber(PropertyInputIndex)), Context);
			PropertyInputIndex = 0;
		}

		const UPCGData* OverrideData = OverrideInputs[PropertyInputIndex].Data;
		if (!OverrideData)
		{
			PCGLog::LogErrorOnGraph(INVTEXT("Invalid input data for Object Property Overrides pin."), Context);
			return false;
		}

		FPCGVoxelGraphParameterOverrides GraphOverrides;
		GraphOverrides.Initialize(Settings->PropertyOverrideDescriptions, OverrideData, Context);

		for (const TPair<UObject*, int32>& TargetObjectAndIndex : TargetObjectsAndIndices)
		{
			IVoxelParameterOverridesObjectOwner* TargetObject = Cast<IVoxelParameterOverridesObjectOwner>(TargetObjectAndIndex.Key);
			if (!TargetObject)
			{
				continue;
			}

			GraphOverrides.Apply(TargetObject, TargetObjectAndIndex.Value, Context);
		}
	}

	// Since we have only one output pin, there's no need to update the pins here.
	Context->OutputData.TaggedData = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);

	return true;
}