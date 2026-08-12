// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGCallVoxelGraph.h"

#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "VoxelPointId.h"
#include "VoxelPCGGraph.h"
#include "VoxelPCGHelpers.h"
#include "VoxelPCGUtilities.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraph.h"
#include "VoxelParameterView.h"
#include "VoxelPCGFunctionLibrary.h"
#include "VoxelGraphParametersView.h"
#include "VoxelTerminalGraphRuntime.h"
#include "VoxelOutputNode_OutputPoints.h"
#include "VoxelGraphParametersViewContext.h"

#include "Buffer/VoxelNameBuffer.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Buffer/VoxelSoftObjectPathBuffer.h"
#include "Graphs/VoxelStampGraphParameters.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Scatter/VoxelScatterFunctionLibrary.h"

#include "PCGComponent.h"
#include "Helpers/PCGAsync.h"
#include "Data/PCGPointData.h"
#include "Helpers/PCGHelpers.h"
#include "Helpers/PCGSettingsHelpers.h"

UPCGCallVoxelGraphSettings::UPCGCallVoxelGraphSettings()
{
}

#if WITH_EDITOR
UObject* UPCGCallVoxelGraphSettings::GetJumpTargetForDoubleClick() const
{
	if (Graph)
	{
		return Graph;
	}

	return Super::GetJumpTargetForDoubleClick();
}
#endif

FString UPCGCallVoxelGraphSettings::GetAdditionalTitleInformation() const
{
	if (!Graph)
	{
		return "Missing Graph";
	}

#if WITH_EDITOR
	if (!Graph->DisplayNameOverride.IsEmpty())
	{
		return Graph->DisplayNameOverride;
	}
#endif

	return FName::NameToDisplayString(Graph->GetName(), false);
}

TArray<FPCGPinProperties> UPCGCallVoxelGraphSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;

	Properties.Emplace_GetRef(
		"Points",
		EPCGDataType::Point,
		true,
		true).SetRequiredPin();

	Properties.Emplace(
		"Bounding Shape",
		EPCGDataType::Spatial,
		false,
		false);

	return Properties;
}

TArray<FPCGPinProperties> UPCGCallVoxelGraphSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;
	Properties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);

	if (!Graph)
	{
		return Properties;
	}

	const UVoxelTerminalGraph* TerminalGraph = nullptr;
	for (UVoxelGraph* BaseGraph : Graph->GetBaseGraphs())
	{
		if (BaseGraph->HasMainTerminalGraph())
		{
			TerminalGraph = &BaseGraph->GetMainTerminalGraph();
			break;
		}
	}

	if (!TerminalGraph)
	{
		return Properties;
	}

	for (const auto& NodeIt : TerminalGraph->GetRuntime().GetSerializedGraph().NodeNameToNode)
	{
		const FVoxelOutputNode_OutputPoints* OutputPointsNode = NodeIt.Value.VoxelNode.GetPtr<FVoxelOutputNode_OutputPoints>();
		if (!OutputPointsNode)
		{
			continue;
		}

		for (const FName PinName : OutputPointsNode->PinNames)
		{
			Properties.Emplace(PinName, EPCGDataType::Point);
		}
	}

	return Properties;
}

#if WITH_EDITOR
TArray<FPCGSettingsOverridableParam> UPCGCallVoxelGraphSettings::GatherOverridableParams() const
{
	TArray<FPCGSettingsOverridableParam> OverridableParams = Super::GatherOverridableParams();
	if (const UScriptStruct* ScriptStruct = ParameterPinOverrides.GetPropertyBagStruct())
	{
		PCGSettingsHelpers::FPCGGetAllOverridableParamsConfig Config;
		Config.bExcludeSuperProperties = true;
		Config.ExcludePropertyFlags = CPF_DisableEditOnInstance;
		OverridableParams.Append(PCGSettingsHelpers::GetAllOverridableParams(ScriptStruct, Config));
	}

	return OverridableParams;
}
#endif

void UPCGCallVoxelGraphSettings::FixingOverridableParamPropertyClass(FPCGSettingsOverridableParam& Param) const
{
	if (!ParameterPinOverrides.IsValid() ||
		Param.PropertiesNames.IsEmpty())
	{
		Super::FixingOverridableParamPropertyClass(Param);
		return;
	}

	const UScriptStruct* ScriptStruct = ParameterPinOverrides.GetPropertyBagStruct();
	if (ScriptStruct &&
		ScriptStruct->FindPropertyByName(Param.PropertiesNames[0]))
	{
		Param.PropertyClass = ScriptStruct;
		return;
	}

	Super::FixingOverridableParamPropertyClass(Param);
}

FPCGElementPtr UPCGCallVoxelGraphSettings::CreateElement() const
{
	return MakeShared<FPCGCallVoxelGraphElement>();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UPCGCallVoxelGraphSettings::PostLoad()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostLoad();

	FixupParameterOverrides();

#if WITH_EDITOR
	FixupOnChangedDelegate();
#endif
}

void UPCGCallVoxelGraphSettings::PostInitProperties()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostInitProperties();

	FixupParameterOverrides();

#if WITH_EDITOR
	FixupOnChangedDelegate();
#endif
}

void UPCGCallVoxelGraphSettings::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
void UPCGCallVoxelGraphSettings::PostEditUndo()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditUndo();

	FixupParameterOverrides();
	FixupOnChangedDelegate();
}

void UPCGCallVoxelGraphSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	FixupParameterOverrides();
	FixupOnChangedDelegate();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelGraph* UPCGCallVoxelGraphSettings::GetGraph() const
{
	return Graph;
}

bool UPCGCallVoxelGraphSettings::ShouldForceEnableOverride(const FGuid& Guid) const
{
	return true;
}

FVoxelParameterOverrides& UPCGCallVoxelGraphSettings::GetParameterOverrides()
{
	return ParameterOverrides;
}

void UPCGCallVoxelGraphSettings::FixupParameterOverrides()
{
	IVoxelParameterOverridesObjectOwner::FixupParameterOverrides();

	TArray<FPropertyBagPropertyDesc> NewProperties;
	if (const TSharedPtr<FVoxelGraphParametersView> ParametersView = GetParametersView())
	{
		TSet<FName> UsedNames;
		for (const FGuid Guid : Graph->GetParameters())
		{
			const FVoxelParameterView* ParameterView = ParametersView->FindByGuid(Guid);
			if (!ParameterView)
			{
				continue;
			}

			FPropertyBagPropertyDesc PropertyDesc = ParameterView->GetType().GetPropertyBagDesc();
			if (PropertyDesc.ValueType == EPropertyBagPropertyType::None)
			{
				continue;
			}

			FName ParameterName = FName(SlugStringForValidName(ParameterView->GetName().ToString()));
			while (UsedNames.Contains(ParameterName))
			{
				ParameterName.SetNumber(ParameterName.GetNumber() + 1);
			}

			UsedNames.Add(ParameterName);

			PropertyDesc.ID = ParameterView->Guid;
			PropertyDesc.Name = FName(ParameterName);
#if WITH_EDITOR
			PropertyDesc.MetaData.Emplace("DisplayName", ParameterView->GetName().ToString());
			PropertyDesc.MetaData.Emplace("Tooltip", ParameterView->GetDescription());
#endif

			NewProperties.Add(PropertyDesc);
		}
	}

	if (NewProperties.Num() == ParameterPinOverrides.GetNumPropertiesInBag())
	{
		bool bIsSame = true;
		for (const FPropertyBagPropertyDesc& PropertyDesc : NewProperties)
		{
			const FPropertyBagPropertyDesc* ExistingPropertyDesc = ParameterPinOverrides.FindPropertyDescByName(PropertyDesc.Name);
			if (!ExistingPropertyDesc)
			{
				bIsSame = false;
				break;
			}

			if (PropertyDesc.ValueType != ExistingPropertyDesc->ValueType ||
				PropertyDesc.ValueTypeObject != ExistingPropertyDesc->ValueTypeObject ||
				PropertyDesc.ContainerTypes != ExistingPropertyDesc->ContainerTypes)
			{
				bIsSame = false;
				break;
			}

#if WITH_EDITOR
			if (PropertyDesc.MetaData.Num() != ExistingPropertyDesc->MetaData.Num())
			{
				bIsSame = false;
				break;
			}

			for (int32 Index = 0; Index < PropertyDesc.MetaData.Num(); Index++)
			{
				const auto& MetadataA = PropertyDesc.MetaData[Index];
				const auto& MetadataB = ExistingPropertyDesc->MetaData[Index];
				if (MetadataA.Key != MetadataB.Key ||
					MetadataA.Value != MetadataB.Value)
				{
					bIsSame = false;
					break;
				}
			}

			if (!bIsSame)
			{
				break;
			}
#endif
		}

		if (bIsSame)
		{
			return;
		}
	}

	ParameterPinOverrides.Reset();
	ParameterPinOverrides.AddProperties(NewProperties);

	InitializeCachedOverridableParams(true);

#if WITH_EDITOR
	OnSettingsChangedDelegate.Broadcast(this, EPCGChangeType::Settings);
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UPCGCallVoxelGraphSettings::FixupOnChangedDelegate()
{
	OnTerminalGraphChangedPtr.Reset();

	if (!Graph)
	{
		return;
	}

	OnTerminalGraphChangedPtr = MakeSharedVoid();

	for (UVoxelGraph* BaseGraph : Graph->GetBaseGraphs())
	{
		// If the base graph changes, reconstruct the node
		GVoxelGraphTracker->OnTerminalGraphChanged(*BaseGraph).Add(FOnVoxelGraphChanged::Make(OnTerminalGraphChangedPtr, this, [this]
		{
			OnSettingsChangedDelegate.Broadcast(this, EPCGChangeType::Settings);
		}));

		if (!BaseGraph->HasMainTerminalGraph())
		{
			continue;
		}

		GVoxelGraphTracker->OnEdGraphChanged(BaseGraph->GetMainTerminalGraph().GetEdGraph()).Add(FOnVoxelGraphChanged::Make(OnTerminalGraphChangedPtr, this, [this]
		{
			OnSettingsChangedDelegate.Broadcast(this, EPCGChangeType::Settings);
		}));
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void FPCGCallVoxelGraphContext::InitializeUserParametersStruct()
{
	ParameterPinOverrides.Reset();

	const UPCGCallVoxelGraphSettings* Settings = GetInputSettings<UPCGCallVoxelGraphSettings>();
	check(Settings);

	const TArray<FPCGSettingsOverridableParam>& OverridableParams = Settings->OverridableParams();
	const FConstStructView UserParametersView = Settings->ParameterPinOverrides.GetValue();

	if (OverridableParams.IsEmpty() ||
		!UserParametersView.IsValid())
	{
		return;
	}

	bool bHasParamConnected = false;
	for (const FPCGSettingsOverridableParam& OverridableParam : OverridableParams)
	{
		if (!InputData.GetParamsByPin(OverridableParam.Label).IsEmpty())
		{
			bHasParamConnected = true;
			break;
		}
	}

	if (!InputData.GetParamsByPin(PCGPinConstants::DefaultParamsLabel).IsEmpty())
	{
		bHasParamConnected = true;
	}

	if (!bHasParamConnected)
	{
		return;
	}

	ParameterPinOverrides.InitializeFromBagStruct(Settings->ParameterPinOverrides.GetPropertyBagStruct());

	const TSharedPtr<FVoxelGraphParametersView> ParametersView = Settings->GetParametersView();
	if (!ParametersView)
	{
		return;
	}

	for (const FVoxelParameterView* ParameterView : ParametersView->GetChildren())
	{
		const FVoxelParameter Parameter = ParameterView->GetParameter();
		const FPropertyBagPropertyDesc* Desc = ParameterPinOverrides.FindPropertyDescByName(Parameter.Name);
		if (!Desc)
		{
			continue;
		}

		void* TargetAddress = ParameterPinOverrides.GetMutableValue().GetMemory() + Desc->CachedProperty->GetOffset_ForInternal();
		ParameterView->GetDefaultValue().ExportToProperty(*Desc->CachedProperty, TargetAddress);;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void* FPCGCallVoxelGraphContext::GetUnsafeExternalContainerForOverridableParam(const FPCGSettingsOverridableParam& InParam)
{
	return ParameterPinOverrides.GetMutableValue().GetMemory();
}

void FPCGCallVoxelGraphContext::AddExtraStructReferencedObjects(FReferenceCollector& Collector)
{
	ParameterPinOverrides.AddStructReferencedObjects(Collector);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if VOXEL_ENGINE_VERSION >= 506
FPCGContext* FPCGCallVoxelGraphElement::Initialize(const FPCGInitializeElementParams& InParams)
#else
FPCGContext* FPCGCallVoxelGraphElement::Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node)
#endif
{
	FPCGCallVoxelGraphContext* Context = new FPCGCallVoxelGraphContext();

#if VOXEL_ENGINE_VERSION >= 506
	Context->InitFromParams(InParams);
#else
	Context->InputData = InputData;
	Context->SourceComponent = SourceComponent;
	Context->Node = Node;
#endif

	Context->InitializeUserParametersStruct();

	return Context;
}

bool FPCGCallVoxelGraphElement::IsCacheable(const UPCGSettings* InSettings) const
{
	return false;
}

bool FPCGCallVoxelGraphElement::PrepareDataInternal(FPCGContext* InContext) const
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());

	FPCGCallVoxelGraphContext* Context = static_cast<FPCGCallVoxelGraphContext*>(InContext);
	check(Context);

	if (Context->WasLoadRequested())
	{
		return true;
	}

	UPCGComponent* Component = GetPCGComponent(*Context);
	if (!ensure(Component))
	{
		return true;
	}

	const UPCGCallVoxelGraphSettings* Settings = Context->GetInputSettings<UPCGCallVoxelGraphSettings>();
	check(Settings);

	if (!Settings->Graph)
	{
		return true;
	}

	const FVoxelBox Bounds = INLINE_LAMBDA -> FVoxelBox
	{
		if (Context->InputData.GetInputCountByPin("Bounding Shape") == 0)
		{
			return {};
		}

		FBox Result;
		{
			bool bOutUnionWasCreated = false;
			const UPCGSpatialData* BoundingShape = Context->InputData.GetSpatialUnionOfInputsByPin(Context, "Bounding Shape", bOutUnionWasCreated);

			// Fallback to getting bounds from actor
			if (!BoundingShape)
			{
				ensure(!bOutUnionWasCreated);
				BoundingShape = Cast<UPCGSpatialData>(Component->GetActorPCGData());
			}

			if (!ensure(BoundingShape))
			{
				return {};
			}

			Result = BoundingShape->GetBounds();
		}

		if (!ensureVoxelSlow(Result.IsValid))
		{
			return {};
		}

		// Intersect with actor bounds
		{
			const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Component->GetActorPCGData());
			if (!ensure(SpatialData))
			{
				return {};
			}

			const FBox ActorBounds = SpatialData->GetBounds();
			if (!ensure(ActorBounds.IsValid))
			{
				return {};
			}

			if (!Result.Intersect(ActorBounds))
			{
				// Nothing to process
				return {};
			}

			Result = Result.Overlap(ActorBounds);
		}

		if (!ensure(Result.IsValid))
		{
			return {};
		}

		return FVoxelBox(Result);
	};

	Context->Bounds = Bounds;

	INLINE_LAMBDA
	{
		// When there's property overrides, Settings object is duplicated for each element, so can be changed
		if (Settings == Context->GetOriginalSettings<UPCGCallVoxelGraphSettings>() ||
			!Context->ParameterPinOverrides.IsValid())
		{
			return;
		}

		UPCGCallVoxelGraphSettings* MutableSettings = ConstCast(Settings);
		const FInstancedPropertyBag& PinOverrides = Context->ParameterPinOverrides;

		const TSharedPtr<FVoxelGraphParametersView> ParametersView = MutableSettings->GetParametersView();
		const UPropertyBag* PropertyBagStruct = PinOverrides.GetPropertyBagStruct();
		if (!PropertyBagStruct ||
			!PinOverrides.GetValue().IsValid() ||
			!ParametersView)
		{
			return;
		}

		for (const FPropertyBagPropertyDesc& PropertyDesc : PropertyBagStruct->GetPropertyDescs())
		{
			if (!ensure(PropertyDesc.CachedProperty))
			{
				continue;
			}

			FVoxelPinValue Value;

			// FInstancedPropertyBag::GetValueAddress
			if (const void* Memory = PinOverrides.GetValue().GetMemory() + PropertyDesc.CachedProperty->GetOffset_ForInternal())
			{
				Value = FVoxelPinValue::MakeFromProperty(*PropertyDesc.CachedProperty, Memory);
			}

			if (!Value.IsValid())
			{
				continue;
			}

			const FVoxelParameterView* ParameterView = ParametersView->FindByGuid(PropertyDesc.ID);
			if (!ParameterView)
			{
				continue;
			}

			MutableSettings->SetParameter(ParameterView->GetName(), Value);
		}
	};

	UWorld* World = Component->GetWorld();
	ensure(World);

	FTransform LocalToWorld;
	if (const AActor* TargetActor = Context->GetTargetActor(nullptr))
	{
		LocalToWorld = TargetActor->ActorToWorld();
	}

	Context->DependencyCollector = MakeShared<FVoxelDependencyCollector>(STATIC_FNAME("PCGCallVoxelGraph")); // GVoxelPCGTracker->GetDependencyTracker(*Context->SourceComponent);

	const TSharedPtr<FVoxelGraphEnvironment> Environment = FVoxelGraphEnvironment::Create(
		Component,
		*Settings,
		LocalToWorld,
		*Context->DependencyCollector);

	if (!Environment)
	{
		return true;
	}

	Context->Evaluator = FVoxelNodeEvaluator::Create<FVoxelOutputNode_OutputPoints>(Environment.ToSharedRef());

	if (!Context->Evaluator)
	{
		return true;
	}

#if !WITH_EDITOR
	if (Context->Node &&
		Context->Node->IsOutputPinConnected(PCGPinConstants::DefaultOutputLabel))
#endif
	{
		Context->PinsToQuery.Add_EnsureNew(VOXEL_PIN_NAME(FVoxelOutputNode_OutputPoints, PointsPin));
	}

	for (const FName PinName : Context->Evaluator->PinNames)
	{
#if !WITH_EDITOR
		if (Context->Node &&
			Context->Node->IsOutputPinConnected(PinName))
#endif
		{
			Context->PinsToQuery.Add(PinName);
		}
	}

	// All pins are disconnected (can only happen in runtime)
	if (Context->PinsToQuery.Num() == 0)
	{
		return true;
	}

	Context->Layers = FVoxelLayers::Get(World);
	Context->SurfaceTypeTable = FVoxelSurfaceTypeTable::Get();

	const TArray<FPCGTaggedData> PointDatas = Context->InputData.GetInputsByPin("Points");
	if (PointDatas.Num() == 0)
	{
		return true;
	}

	TSet<FName> AttributesToLoad;
	for (const FVoxelPCGObjectAttributeType& Type : Settings->ObjectAttributeToType)
	{
		// SoftObjectPath do not need to be loaded
		if (Type.Type.Is<FVoxelSoftObjectPath>())
		{
			continue;
		}

		AttributesToLoad.Add(Type.Attribute.GetName());
	}

	TSet<FSoftObjectPath> ObjectsToLoad;
	TArray<FPCGTaggedData>& Outputs = InContext->OutputData.TaggedData;
	for (const FPCGTaggedData& TaggedData : PointDatas)
	{
		const UPCGPointData* PointData = Cast<UPCGPointData>(TaggedData.Data);
		if (!PointData)
		{
			continue;
		}

		INLINE_LAMBDA
		{
			const UPCGMetadata* Metadata = PointData->Metadata;
			if (!Metadata ||
				AttributesToLoad.Num() == 0)
			{
				return;
			}

			const TConstVoxelArrayView<FPCGPoint> Points = PointData->GetPoints();
			for (const FName AttributeName : AttributesToLoad)
			{
				const FPCGMetadataAttributeBase* MetadataAttribute = Metadata->GetConstAttribute(AttributeName);
				if (!MetadataAttribute)
				{
					continue;
				}

				EPCGMetadataTypes AttributeType = EPCGMetadataTypes::Unknown;
				if (MetadataAttribute->GetTypeId() < static_cast<uint16>(EPCGMetadataTypes::Unknown))
				{
					AttributeType = static_cast<EPCGMetadataTypes>(MetadataAttribute->GetTypeId());
				}

				if (AttributeType != EPCGMetadataTypes::SoftObjectPath)
				{
					continue;
				}

				const auto CopyAttribute = [&]<typename T>(const FPCGMetadataAttribute<T>& Attribute)
				{
					if constexpr (std::is_same_v<T, FSoftObjectPath>)
					{
						for (int32 Index = 0; Index < Points.Num(); Index++)
						{
							const FPCGPoint& Point = Points[Index];

							const T& Value = Attribute.GetValueFromItemKey(Point.MetadataEntry);
							ObjectsToLoad.Add(Value);
						}
					}
					else
					{
						ensure(false);
					}
				};

				FVoxelPCGUtilities::SwitchOnAttribute(
					AttributeType,
					*MetadataAttribute,
					CopyAttribute);
			}
		};

		Context->InPointData.Add(PointData);

		TVoxelMap<FName, TVoxelObjectPtr<UPCGPointData>> OutputPins;
		for (FName PinName : Context->PinsToQuery)
		{
			UPCGPointData* OutPointData = NewObject<UPCGPointData>();
			OutPointData->InitializeFromData(PointData);
			FPCGTaggedData& OutTaggedData = Outputs.Emplace_GetRef();
			OutTaggedData.Tags = TaggedData.Tags;
			OutTaggedData.Data = OutPointData;
			if (PinName == VOXEL_PIN_NAME(FVoxelOutputNode_OutputPoints, PointsPin))
			{
				OutTaggedData.Pin = PCGPinConstants::DefaultOutputLabel;
			}
			else
			{
				OutTaggedData.Pin = PinName;
			}

			OutputPins.Add_EnsureNew(PinName, OutPointData);
		}

		Context->OutPointData.Add(OutputPins);
	}

	if (!ensure(Context->InPointData.Num() > 0) ||
		!ensure(Context->InPointData.Num() == Context->OutPointData.Num()))
	{
		return true;
	}

	if (ObjectsToLoad.Num() > 0)
	{
		return Context->RequestResourceLoad(Context, ObjectsToLoad.Array(), !Settings->bSynchronousLoad);
	}

	return true;
}

bool FPCGCallVoxelGraphElement::ExecuteInternal(FPCGContext* InContext) const
{
	VOXEL_FUNCTION_COUNTER();

	FPCGCallVoxelGraphContext* Context = static_cast<FPCGCallVoxelGraphContext*>(InContext);
	check(Context);

	if (!Context->Evaluator ||
		!ensureVoxelSlow(Context->InPointData.Num() == Context->OutPointData.Num()))
	{
		return true;
	}

	const UPCGCallVoxelGraphSettings* Settings = Context->GetInputSettings<UPCGCallVoxelGraphSettings>();
	check(Settings);

	if (Context->FuturePoints.Num() > 0)
	{
		// Flush any game task queued by FuturePoints
		Voxel::FlushGameTasks();

		for (const FVoxelFuture& Future : Context->FuturePoints)
		{
			if (!Future.IsComplete())
			{
				return false;
			}
		}

		return true;
	}

	ensure(!Context->bIsPaused);
	Context->bIsPaused = true;

	const TFunction<void(int32)> Resume = MakeWeakPtrLambda(Context->SharedVoid, [Context](int32 FutureIndex)
	{
		check(IsInGameThread());

		for (int32 Index = 0; Index < Context->FuturePoints.Num(); Index++)
		{
			if (Index == FutureIndex)
			{
				continue;
			}

			if (!Context->FuturePoints[Index].IsComplete())
			{
				return;
			}
		}

		ensure(Context->bIsPaused);
		Context->bIsPaused = false;
	});

	TMap<FName, FVoxelPinType> AttributeToObjectType;
	for (const FVoxelPCGObjectAttributeType& Type : Settings->ObjectAttributeToType)
	{
		if (Type.Type.Is<FVoxelSoftObjectPath>())
		{
			continue;
		}

		AttributeToObjectType.Add(Type.Attribute.GetName(), Type.Type);
	}

	for (int32 PointDataIndex = 0; PointDataIndex < Context->InPointData.Num(); PointDataIndex++)
	{
		TVoxelMap<FName, EPCGMetadataTypes> AttributeNameToType;
		TVoxelMap<FName, TSharedPtr<FVoxelBuffer>> AttributeNameToBuffer;

		if (const UPCGMetadata* Metadata = Context->InPointData[PointDataIndex]->Metadata)
		{
			TArray<FName> AttributeNames;
			TArray<EPCGMetadataTypes> AttributeTypes;
			Metadata->GetAttributes(AttributeNames, AttributeTypes);
			check(AttributeNames.Num() == AttributeTypes.Num());

			const TConstVoxelArrayView<FPCGPoint> Points = Context->InPointData[PointDataIndex]->GetPoints();

			for (int32 AttributeIndex = 0; AttributeIndex < AttributeNames.Num(); AttributeIndex++)
			{
				const FName AttributeName = AttributeNames[AttributeIndex];
				const EPCGMetadataTypes AttributeType = AttributeTypes[AttributeIndex];

				AttributeNameToType.Add_EnsureNew(AttributeName, AttributeType);

				const auto CopyAttribute = [&]<typename T>(const FPCGMetadataAttribute<T>& Attribute)
				{
					VOXEL_SCOPE_COUNTER_FORMAT("CopyAttribute %s", *AttributeName.ToString());

					const TSharedRef<FVoxelBuffer> Buffer = INLINE_LAMBDA -> TSharedRef<FVoxelBuffer>
					{
						if constexpr (
							std::is_same_v<T, bool> ||
							std::is_same_v<T, float> ||
							std::is_same_v<T, double> ||
							std::is_same_v<T, int32> ||
							std::is_same_v<T, int64>)
						{
							TVoxelBufferTypeSafe<T> Result;
							Result.Allocate(Points.Num());

							for (int32 Index = 0; Index < Points.Num(); Index++)
							{
								Result.Set(Index, Attribute.GetValueFromItemKey(Points[Index].MetadataEntry));
							}

							return MakeSharedCopy(MoveTemp(Result));
						}
						else if constexpr (
							std::is_same_v<T, FVector2D> ||
							std::is_same_v<T, FVector> ||
							std::is_same_v<T, FQuat> ||
							std::is_same_v<T, FTransform>)
						{
							TVoxelDoubleBufferTypeSafe<T> Result;
							Result.Allocate(Points.Num());

							for (int32 Index = 0; Index < Points.Num(); Index++)
							{
								Result.Set(Index, Attribute.GetValueFromItemKey(Points[Index].MetadataEntry));
							}

							return MakeSharedCopy(MoveTemp(Result));
						}
						else if constexpr (std::is_same_v<T, FVector4>)
						{
							FVoxelDoubleLinearColorBuffer Result;
							Result.Allocate(Points.Num());

							for (int32 Index = 0; Index < Points.Num(); Index++)
							{
								Result.Set(Index, FVoxelDoubleLinearColor(Attribute.GetValueFromItemKey(Points[Index].MetadataEntry)));
							}

							return MakeSharedCopy(MoveTemp(Result));
						}
						else if constexpr (
							std::is_same_v<T, FString> ||
							std::is_same_v<T, FName>)
						{
							FVoxelNameBuffer Result;
							Result.Allocate(Points.Num());

							for (int32 Index = 0; Index < Points.Num(); Index++)
							{
								Result.Set(Index, FName(Attribute.GetValueFromItemKey(Points[Index].MetadataEntry)));
							}

							return MakeSharedCopy(MoveTemp(Result));
						}
						else if constexpr (std::is_same_v<T, FRotator>)
						{
							FVoxelDoubleVectorBuffer Result;
							Result.Allocate(Points.Num());

							for (int32 Index = 0; Index < Points.Num(); Index++)
							{
								const FPCGPoint& Point = Points[Index];

								const FRotator& Value = Attribute.GetValueFromItemKey(Point.MetadataEntry);
								Result.Set(Index, FVector(Value.Pitch, Value.Yaw, Value.Roll));
							}

							return MakeSharedCopy(MoveTemp(Result));
						}
						else if constexpr (std::is_same_v<T, FSoftObjectPath>)
						{
							if (const FVoxelPinType* PinType = AttributeToObjectType.Find(AttributeName))
							{
								TSharedRef<FVoxelBuffer> Result = FVoxelBuffer::MakeEmpty(*PinType);
								Result->Allocate(Points.Num());

								for (int32 Index = 0; Index < Points.Num(); Index++)
								{
									const FPCGPoint& Point = Points[Index];

									UObject* Object = Attribute.GetValueFromItemKey(Point.MetadataEntry).ResolveObject();

									FVoxelPinValue PinValue = FVoxelPinValue::Make(Object);
									PinValue.Fixup(PinType->GetExposedType());

									Result->SetGeneric(Index, FVoxelPinType::MakeRuntimeValue(*PinType, PinValue, {}));
								}

								return Result;
							}
							FVoxelSoftObjectPathBuffer Result;
							Result.Allocate(Points.Num());

							for (int32 Index = 0; Index < Points.Num(); Index++)
							{
								const FPCGPoint& Point = Points[Index];

								const T& Value = Attribute.GetValueFromItemKey(Point.MetadataEntry);
								Result.Set(Index, Value);
							}

							return MakeSharedCopy(MoveTemp(Result));
						}
						else if constexpr (std::is_same_v<T, FSoftClassPath>)
						{
							FVoxelSoftClassPathBuffer Result;
							Result.Allocate(Points.Num());

							for (int32 Index = 0; Index < Points.Num(); Index++)
							{
								const FPCGPoint& Point = Points[Index];

								const T& Value = Attribute.GetValueFromItemKey(Point.MetadataEntry);
								Result.Set(Index, Value);
							}

							return MakeSharedCopy(MoveTemp(Result));
						}
						else
						{
							checkStatic(sizeof(T) == 0);
							return TSharedRef<FVoxelBuffer>();
						}
					};

					AttributeNameToBuffer.Add_EnsureNew(AttributeName, Buffer);
				};

				const FPCGMetadataAttributeBase* Attribute = Metadata->GetConstAttribute(AttributeName);
				if (!ensureVoxelSlow(Attribute))
				{
					continue;
				}

				FVoxelPCGUtilities::SwitchOnAttribute(
					AttributeType,
					*Attribute,
					CopyAttribute);
			}
		}

		Context->FuturePoints.Add(Voxel::AsyncTask([
			Evaluator = Context->Evaluator,
			DependencyCollector = Context->DependencyCollector.ToSharedRef(),
			Bounds = Context->Bounds,
			PinsToQuery = Context->PinsToQuery,
			Layers = Context->Layers.ToSharedRef(),
			SurfaceTypeTable = Context->SurfaceTypeTable.ToSharedRef(),
			Points = TVoxelArray<FPCGPoint>(Context->InPointData[PointDataIndex]->GetPoints()),
			AttributeNameToType = MoveTemp(AttributeNameToType),
			AttributeNameToBuffer = MoveTemp(AttributeNameToBuffer),
			PinToOutPointData = Context->OutPointData[PointDataIndex]]
		{
			return Compute(
				Evaluator,
				DependencyCollector,
				Bounds,
				PinsToQuery,
				Layers,
				SurfaceTypeTable,
				Points,
				AttributeNameToType,
				AttributeNameToBuffer,
				PinToOutPointData);
		})
		.Then_GameThread([=]
		{
			Resume(PointDataIndex);
		}));
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

FVoxelFuture FPCGCallVoxelGraphElement::Compute(
	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputPoints>& Evaluator,
	const TSharedRef<FVoxelDependencyCollector>& DependencyCollector,
	const FVoxelBox& Bounds,
	const TVoxelSet<FName>& PinsToQuery,
	const TSharedRef<FVoxelLayers>& Layers,
	const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
	const TVoxelArray<FPCGPoint>& Points,
	const TVoxelMap<FName, EPCGMetadataTypes>& AttributeNameToType,
	const TVoxelMap<FName, TSharedPtr<FVoxelBuffer>>& AttributeNameToBuffer,
	const TVoxelMap<FName, TVoxelObjectPtr<UPCGPointData>>& PinToOutPointData)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelPointSet> PointSet = MakeShared<FVoxelPointSet>();
	PointSet->SetNum(Points.Num());

	for (const auto& It : AttributeNameToBuffer)
	{
		PointSet->Add(It.Key, It.Value.ToSharedRef());
	}

	{
		VOXEL_SCOPE_COUNTER_NUM("Allocate points", Points.Num());

		FVoxelDoubleVectorBuffer Position;
		Position.Allocate(Points.Num());

		FVoxelQuaternionBuffer Rotation;
		Rotation.Allocate(Points.Num());

		FVoxelVectorBuffer Scale;
		Scale.Allocate(Points.Num());

		FVoxelFloatBuffer Density;
		Density.Allocate(Points.Num());

		FVoxelVectorBuffer BoundsMin;
		BoundsMin.Allocate(Points.Num());

		FVoxelVectorBuffer BoundsMax;
		BoundsMax.Allocate(Points.Num());

		FVoxelLinearColorBuffer Color;
		Color.Allocate(Points.Num());

		FVoxelFloatBuffer Steepness;
		Steepness.Allocate(Points.Num());

		FVoxelPointIdBuffer Id;
		Id.Allocate(Points.Num());

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			const FPCGPoint& Point = Points[Index];

			Position.Set(Index, Point.Transform.GetTranslation());
			Rotation.Set(Index, FQuat4f(Point.Transform.GetRotation()));
			Scale.Set(Index, FVector3f(Point.Transform.GetScale3D()));
			Density.Set(Index, Point.Density);

			BoundsMin.Set(Index, FVector3f(Point.BoundsMin));
			BoundsMax.Set(Index, FVector3f(Point.BoundsMax));

			Color.Set(Index, FLinearColor(Point.Color));
			Steepness.Set(Index, Point.Steepness);
			Id.Set(Index, FVoxelPointId(Point.Seed));
		}

		PointSet->Add(FVoxelPointAttributes::Position, MoveTemp(Position));
		PointSet->Add(FVoxelPointAttributes::Rotation, MoveTemp(Rotation));
		PointSet->Add(FVoxelPointAttributes::Scale, MoveTemp(Scale));
		PointSet->Add(FVoxelPointAttributes::Density, MoveTemp(Density));
		PointSet->Add(FVoxelPointAttributes::BoundsMin, MoveTemp(BoundsMin));
		PointSet->Add(FVoxelPointAttributes::BoundsMax, MoveTemp(BoundsMax));
		PointSet->Add(FVoxelPointAttributes::Color, MoveTemp(Color));
		PointSet->Add(FVoxelPointAttributes::Steepness, MoveTemp(Steepness));
		PointSet->Add(FVoxelPointAttributes::Id, MoveTemp(Id));
	}

	FVoxelGraphContext Context = Evaluator.MakeContext(*DependencyCollector);

	const FVoxelQuery VoxelQuery(
		0,
		*Layers,
		*SurfaceTypeTable,
		*DependencyCollector);

	FVoxelGraphQueryImpl& Query = Context.MakeQuery();
	Query.AddParameter<FVoxelGraphParameters::FQuery>(VoxelQuery);
	Query.AddParameter<FVoxelGraphParameters::FPointSet>().Value = PointSet;
	Query.AddParameter<FVoxelGraphParameters::FPointSetChunkBounds>(Bounds, false);

	TVoxelMap<FName, TVoxelArray<FPCGPoint>> PinToNewPoints;
	TVoxelMap<FName, TVoxelArray<TFunction<void(UPCGMetadata&, TVoxelArray<FPCGPoint>&)>>> PinToMetadataWriters;

	TVoxelArray<const FVoxelNode::TPinRef_Input<FVoxelPointSet>*> Pins;
	Pins.Add(&Evaluator->PointsPin);
	for (const FVoxelNode::TPinRef_Input<FVoxelPointSet>& Pin : Evaluator->InputPins)
	{
		Pins.Add(&Pin);
	}

	for (const FVoxelNode::TPinRef_Input<FVoxelPointSet>* Pin : Pins)
	{
		if (!PinsToQuery.Contains(Pin->GetName()))
		{
			continue;
		}

		TVoxelArray<FPCGPoint>& NewPoints = PinToNewPoints.Add_EnsureNew(Pin->GetName());
		TVoxelArray<TFunction<void(UPCGMetadata&, TVoxelArray<FPCGPoint>&)>>& MetadataWriters = PinToMetadataWriters.Add_EnsureNew(Pin->GetName());

		const TSharedRef<const FVoxelPointSet> NewPointSet = Pin->GetSynchronous(Query);

		{
			VOXEL_SCOPE_COUNTER_NUM("Convert points", NewPointSet->Num());

			FVoxelUtilities::SetNum(NewPoints, NewPointSet->Num());

 			if (const FVoxelDoubleVectorBuffer* Position = NewPointSet->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position))
 			{
 				VOXEL_SCOPE_COUNTER("Position");

				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					NewPoints[Index].Transform.SetTranslation((*Position)[Index]);
				}
 			}

 			if (const FVoxelQuaternionBuffer* Rotation = NewPointSet->Find<FVoxelQuaternionBuffer>(FVoxelPointAttributes::Rotation))
 			{
 				VOXEL_SCOPE_COUNTER("Rotation");

				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					NewPoints[Index].Transform.SetRotation(FQuat((*Rotation)[Index]));
				}
 			}

 			if (const FVoxelVectorBuffer* Scale = NewPointSet->Find<FVoxelVectorBuffer>(FVoxelPointAttributes::Scale))
 			{
 				VOXEL_SCOPE_COUNTER("Scale");

				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					NewPoints[Index].Transform.SetScale3D(FVector((*Scale)[Index]));
				}
 			}

 			if (const FVoxelFloatBuffer* Density = NewPointSet->Find<FVoxelFloatBuffer>(FVoxelPointAttributes::Density))
 			{
 				VOXEL_SCOPE_COUNTER("Density");

				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					NewPoints[Index].Density = (*Density)[Index];
				}
 			}

 			if (const FVoxelVectorBuffer* BoundsMin = NewPointSet->Find<FVoxelVectorBuffer>(FVoxelPointAttributes::BoundsMin))
 			{
 				VOXEL_SCOPE_COUNTER("BoundsMin");

				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					NewPoints[Index].BoundsMin = FVector((*BoundsMin)[Index]);
				}
 			}

 			if (const FVoxelVectorBuffer* BoundsMax = NewPointSet->Find<FVoxelVectorBuffer>(FVoxelPointAttributes::BoundsMax))
 			{
 				VOXEL_SCOPE_COUNTER("BoundsMax");

				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					NewPoints[Index].BoundsMax = FVector((*BoundsMax)[Index]);
				}
 			}

 			if (const FVoxelLinearColorBuffer* Color = NewPointSet->Find<FVoxelLinearColorBuffer>(FVoxelPointAttributes::Color))
 			{
 				VOXEL_SCOPE_COUNTER("Color");

				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					NewPoints[Index].Color = FVector4((*Color)[Index]);
				}
 			}

 			if (const FVoxelFloatBuffer* Steepness = NewPointSet->Find<FVoxelFloatBuffer>(FVoxelPointAttributes::Steepness))
 			{
 				VOXEL_SCOPE_COUNTER("Steepness");

				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					NewPoints[Index].Steepness = (*Steepness)[Index];
				}
 			}

 			if (const FVoxelPointIdBuffer* Id = NewPointSet->Find<FVoxelPointIdBuffer>(FVoxelPointAttributes::Id))
 			{
 				VOXEL_SCOPE_COUNTER("Id");

				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					NewPoints[Index].Seed = (*Id)[Index].PointId;
				}
 			}
		}

		for (const auto& AttributeIt : NewPointSet->GetAttributes())
		{
			FName AttributeName = AttributeIt.Key;
			const FVoxelBuffer& Attribute = *AttributeIt.Value.ToSharedRef();

			if (AttributeName == FVoxelPointAttributes::Position ||
				AttributeName == FVoxelPointAttributes::Rotation ||
				AttributeName == FVoxelPointAttributes::Scale ||
				AttributeName == FVoxelPointAttributes::Density ||
				AttributeName == FVoxelPointAttributes::BoundsMin ||
				AttributeName == FVoxelPointAttributes::BoundsMax ||
				AttributeName == FVoxelPointAttributes::Color ||
				AttributeName == FVoxelPointAttributes::Steepness ||
				AttributeName == FVoxelPointAttributes::Id)
			{
				continue;
			}

			if (!FPCGMetadataAttributeBase::IsValidName(AttributeName.ToString()))
			{
				FString SanitizedAttributeName = AttributeName.ToString();
				FPCGMetadataAttributeBase::SanitizeName(SanitizedAttributeName);
				AttributeName = FName(SanitizedAttributeName);
			}

			VOXEL_SCOPE_COUNTER_FORMAT("CopyAttribute %s", *AttributeName.ToString());

			if (AttributeNameToType.FindRef(AttributeName) == EPCGMetadataTypes::Rotator)
			{
				if (const FVoxelDoubleVectorBuffer* Buffer = Attribute.As<FVoxelDoubleVectorBuffer>())
				{
					TVoxelArray<FRotator> Values;
					FVoxelUtilities::SetNumFast(Values, NewPoints.Num());
					for (int32 Index = 0; Index < NewPoints.Num(); Index++)
					{
						Values[Index] = FRotator(
							Buffer->X[Index],
							Buffer->Y[Index],
							Buffer->Z[Index]);
					}

					MetadataWriters.Add([AttributeName, Values = MakeSharedCopy(MoveTemp(Values))](
						UPCGMetadata& Metadata,
						TVoxelArray<FPCGPoint>& InPoints)
					{
						FVoxelPCGUtilities::AddAttribute<FRotator>(Metadata, InPoints, AttributeName, *Values);
					});

					continue;
				}
			}

#if PLATFORM_ANDROID
			checkf(false, TEXT("This code does not compile on Android"));
#else
			const auto ProcessImpl = [&]<typename BufferType, typename Type>()
			{
				const BufferType& Buffer = Attribute.AsChecked<BufferType>();
				if (!ensure(Buffer.IsConstant() || Buffer.Num() == NewPoints.Num()))
				{
					return;
				}

				TVoxelArray<Type> Values;
				if constexpr (std::is_trivially_destructible_v<Type>)
				{
					FVoxelUtilities::SetNumFast(Values, NewPoints.Num());
				}
				else
				{
					FVoxelUtilities::SetNum(Values, NewPoints.Num());
				}

				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					Values[Index] = Type(Buffer[Index]);
				}

				MetadataWriters.Add([AttributeName, Values = MakeSharedCopy(MoveTemp(Values))](
					UPCGMetadata& Metadata,
					TVoxelArray<FPCGPoint>& InPoints)
				{
					FVoxelPCGUtilities::AddAttribute<Type>(Metadata, InPoints, AttributeName, *Values);
				});
			};

			{
				const auto Process = [&]<typename Type>()
				{
					if (!Attribute.IsA<TVoxelBufferType<Type>>())
					{
						return false;
					}

					ProcessImpl.operator()<TVoxelBufferType<Type>, Type>();
					return true;
				};

				if (Process.operator()<bool>() ||
					Process.operator()<float>() ||
					Process.operator()<double>() ||
					Process.operator()<int32>() ||
					Process.operator()<int64>())
				{
					continue;
				}
			}

			{
				const auto Process = [&]<typename Type>()
				{
					if (Attribute.IsA<TVoxelBufferType<Type>>())
					{
						ProcessImpl.operator()<TVoxelBufferType<Type>, Type>();
						return true;
					}

					if (Attribute.IsA<TVoxelDoubleBufferType<Type>>())
					{
						ProcessImpl.operator()<TVoxelDoubleBufferType<Type>, Type>();
						return true;
					}

					return false;
				};

				if (Process.operator()<FVector2D>() ||
					Process.operator()<FVector>() ||
					Process.operator()<FQuat>() ||
					Process.operator()<FTransform>())
				{
					continue;
				}
			}

			if (const FVoxelPointIdBuffer* Buffer = Attribute.As<FVoxelPointIdBuffer>())
			{
				TVoxelArray<int64> Values;
				FVoxelUtilities::SetNumFast(Values, NewPoints.Num());
				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					Values[Index] = (*Buffer)[Index].PointId;
				}

				MetadataWriters.Add([AttributeName, Values = MakeSharedCopy(MoveTemp(Values))](
					UPCGMetadata& Metadata,
					TVoxelArray<FPCGPoint>& InPoints)
				{
					FVoxelPCGUtilities::AddAttribute<int64>(Metadata, InPoints, AttributeName, *Values);
				});

				continue;
			}

			if (Attribute.IsA<FVoxelLinearColorBuffer>())
			{
				ProcessImpl.operator()<FVoxelLinearColorBuffer, FVector4>();
				continue;
			}
			if (Attribute.IsA<FVoxelDoubleLinearColorBuffer>())
			{
				ProcessImpl.operator()<FVoxelDoubleLinearColorBuffer, FVector4>();
				continue;
			}

			if (Attribute.IsA<FVoxelSoftObjectPathBuffer>())
			{
				ProcessImpl.operator()<FVoxelSoftObjectPathBuffer, FSoftObjectPath>();
				continue;
			}
			if (Attribute.IsA<FVoxelSoftClassPathBuffer>())
			{
				ProcessImpl.operator()<FVoxelSoftClassPathBuffer, FSoftClassPath>();
				continue;
			}

			if (const FVoxelNameBuffer* Buffer = Attribute.As<FVoxelNameBuffer>())
			{
				if (AttributeNameToType.FindRef(AttributeName) == EPCGMetadataTypes::String)
				{
					TVoxelArray<FString> Values;
					FVoxelUtilities::SetNum(Values, NewPoints.Num());
					for (int32 Index = 0; Index < NewPoints.Num(); Index++)
					{
						Values[Index] = FName((*Buffer)[Index]).ToString();
					}

					MetadataWriters.Add([AttributeName, Values = MakeSharedCopy(MoveTemp(Values))](
						UPCGMetadata& Metadata,
						TVoxelArray<FPCGPoint>& InPoints)
					{
						FVoxelPCGUtilities::AddAttribute<FString>(Metadata, InPoints, AttributeName, *Values);
					});

					continue;
				}

				TVoxelArray<FName> Values;
				FVoxelUtilities::SetNumFast(Values, NewPoints.Num());
				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					Values[Index] = FName((*Buffer)[Index]);
				}

				MetadataWriters.Add([AttributeName, Values = MakeSharedCopy(MoveTemp(Values))](
					UPCGMetadata& Metadata,
					TVoxelArray<FPCGPoint>& InPoints)
				{
					FVoxelPCGUtilities::AddAttribute<FName>(Metadata, InPoints, AttributeName, *Values);
				});

				continue;
			}

			if (const TSharedPtr<const FVoxelObjectPinType> ObjectPinType = FVoxelObjectPinType::StructToPinType().FindRef(Attribute.GetInnerType().GetStruct()))
			{
				TVoxelArray<TVoxelObjectPtr<UObject>> WeakValues;
				FVoxelUtilities::SetNum(WeakValues, NewPoints.Num());
				for (int32 Index = 0; Index < NewPoints.Num(); Index++)
				{
					WeakValues[Index] = ObjectPinType->GetWeakObject(Attribute.GetGeneric(Index).GetStructView());
				}

				MetadataWriters.Add([AttributeName, WeakValues = MakeSharedCopy(MoveTemp(WeakValues))](
					UPCGMetadata& Metadata,
					TVoxelArray<FPCGPoint>& InPoints)
				{
					checkVoxelSlow(IsInGameThread());

					TVoxelArray<FSoftObjectPath> Values;
					FVoxelUtilities::SetNum(Values, WeakValues->Num());
					for (int32 Index = 0; Index < WeakValues->Num(); Index++)
					{
						Values[Index] = FSoftObjectPath((*WeakValues)[Index].Resolve());
					}

					FVoxelPCGUtilities::AddAttribute<FSoftObjectPath>(Metadata, InPoints, AttributeName, Values);
				});

				continue;
			}

			VOXEL_MESSAGE(Error, "Unsupported type for PCG attributes: {0} {1}", AttributeName, Attribute.GetBufferType().ToString());
#endif
		}

	}

	return Voxel::GameTask([=, PinToNewPoints = MakeSharedCopy(MoveTemp(PinToNewPoints))]
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());

		for (auto& It : PinToOutPointData)
		{
			UPCGPointData* OutPointData = It.Value.Resolve();
			if (!ensureVoxelSlow(OutPointData))
			{
				continue;
			}

			TVoxelArray<FPCGPoint, TSizedDefaultAllocator<32>>* NewPoints = PinToNewPoints->Find(It.Key);
			const TVoxelArray<TFunction<void(UPCGMetadata&, TVoxelArray<FPCGPoint>&)>>* MetadataWriters = PinToMetadataWriters.Find(It.Key);
			if (!ensure(NewPoints) ||
				!ensure(MetadataWriters))
			{
				continue;
			}

			FVoxelPCGUtilities::AddPointsToMetadata(*OutPointData->Metadata, *NewPoints);

			for (const TFunction<void(UPCGMetadata&, TVoxelArray<FPCGPoint>&)>& Writer : *MetadataWriters)
			{
				Writer(*OutPointData->Metadata, *NewPoints);
			}

			OutPointData->GetMutablePoints() = MoveTemp(*NewPoints);
		}
	});
}