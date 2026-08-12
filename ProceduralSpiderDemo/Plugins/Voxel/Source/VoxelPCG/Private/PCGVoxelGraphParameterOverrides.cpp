// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGVoxelGraphParameterOverrides.h"

void FPCGVoxelObjectSingleOverride::Initialize(const FPCGAttributePropertySelector& InputSelector, const FString& OutputProperty, const UStruct* TemplateClass, const UPCGData* SourceData, FPCGContext* Context)
{
	InputKeys = PCGAttributeAccessorHelpers::CreateConstKeys(SourceData, InputSelector);
	ObjectOverrideInputAccessor = PCGAttributeAccessorHelpers::CreateConstAccessor(SourceData, InputSelector);

	if (!ObjectOverrideInputAccessor.IsValid())
	{
		PCGLog::LogWarningOnGraph(FText::Format(NSLOCTEXT("PCGObjectPropertyOverride", "OverrideInputInvalid", "ObjectOverride for input '{0}' is invalid or unsupported. Check the attribute or property selection."), InputSelector.GetDisplayText()), Context);
		return;
	}

	const FPCGAttributePropertySelector OutputSelector = FPCGAttributePropertySelector::CreateSelectorFromString(OutputProperty);
	// TODO: Move implementation into a new helper: PCGAttributeAccessorHelpers::CreatePropertyAccessor(const FPCGAttributePropertySelector& Selector, UStruct* Class)
	const TArray<FString>& ExtraNames = OutputSelector.GetExtraNames();
	if (ExtraNames.IsEmpty())
	{
		ObjectOverrideOutputAccessor = PCGAttributeAccessorHelpers::CreatePropertyAccessor(FName(OutputProperty), TemplateClass);
	}
	else
	{
		TArray<FName> PropertyNames;
		PropertyNames.Reserve(ExtraNames.Num() + 1);
		PropertyNames.Add(OutputSelector.GetAttributeName());
		for (const FString& Name : ExtraNames)
		{
			PropertyNames.Add(FName(Name));
		}

		ObjectOverrideOutputAccessor = PCGAttributeAccessorHelpers::CreatePropertyChainAccessor(PropertyNames, TemplateClass);
	}

	if (!ObjectOverrideOutputAccessor.IsValid())
	{
		PCGLog::LogWarningOnGraph(FText::Format(NSLOCTEXT("PCGObjectPropertyOverride", "OverrideOutputInvalid", "ObjectOverride for object property '{0}' is invalid or unsupported. Check the attribute or property selection."), FText::FromString(OutputProperty)), Context);
		return;
	}

	if (!PCG::Private::IsBroadcastableOrConstructible(ObjectOverrideInputAccessor->GetUnderlyingType(), ObjectOverrideOutputAccessor->GetUnderlyingType()))
	{
		PCGLog::LogWarningOnGraph(
			FText::Format(
				NSLOCTEXT("PCGObjectPropertyOverride", "TypesIncompatible", "ObjectOverride cannot set input '{0}' to output '{1}'. Cannot convert type '{2}' to type '{3}'. Will be skipped."),
				InputSelector.GetDisplayText(),
				FText::FromString(OutputProperty),
				PCG::Private::GetTypeNameText(ObjectOverrideInputAccessor->GetUnderlyingType()),
				PCG::Private::GetTypeNameText(ObjectOverrideOutputAccessor->GetUnderlyingType())),
			Context);

		ObjectOverrideInputAccessor.Reset();
		ObjectOverrideOutputAccessor.Reset();
		return;
	}

	auto CreateGetterSetter = [this](auto Dummy)
	{
		using Type = decltype(Dummy);

		ObjectOverrideFunction = &FPCGVoxelObjectSingleOverride::ApplyImpl<Type>;
	};

	PCGMetadataAttribute::CallbackWithRightType(ObjectOverrideOutputAccessor->GetUnderlyingType(), CreateGetterSetter);
}

bool FPCGVoxelObjectSingleOverride::IsValid() const
{
	return InputKeys.IsValid() && ObjectOverrideInputAccessor.IsValid() && ObjectOverrideOutputAccessor.IsValid() && ObjectOverrideFunction;
}

bool FPCGVoxelObjectSingleOverride::Apply(int32 InputKeyIndex, IPCGAttributeAccessorKeys& OutputKey)
{
	return Invoke(ObjectOverrideFunction, this, InputKeyIndex, OutputKey);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FPCGVoxelGraphParameterSingleOverride::Initialize(
	const FPCGAttributePropertySelector& InputSelector,
	const FString& OutputProperty,
	const UPCGData* SourceData,
	const FPCGContext* Context)
{
	InputKeys = PCGAttributeAccessorHelpers::CreateConstKeys(SourceData, InputSelector);
	ObjectOverrideInputAccessor = PCGAttributeAccessorHelpers::CreateConstAccessor(SourceData, InputSelector);

	if (!ObjectOverrideInputAccessor.IsValid())
	{
		PCGLog::LogWarningOnGraph(FText::Format(NSLOCTEXT("PCGObjectPropertyOverride", "OverrideInputInvalid", "ObjectOverride for input '{0}' is invalid or unsupported. Check the attribute or property selection."), InputSelector.GetDisplayText()), Context);
		return;
	}

	ParameterName = FName(OutputProperty);

	auto CreateGetterSetter = [this](auto Dummy)
	{
		using Type = decltype(Dummy);

		ObjectOverrideFunction = &FPCGVoxelGraphParameterSingleOverride::ApplyImpl<Type>;
	};

	PCGMetadataAttribute::CallbackWithRightType(ObjectOverrideInputAccessor->GetUnderlyingType(), CreateGetterSetter);
}

bool FPCGVoxelGraphParameterSingleOverride::IsValid() const
{
	return
		InputKeys.IsValid() &&
		ObjectOverrideInputAccessor.IsValid() &&
		ObjectOverrideFunction != nullptr;
}

bool FPCGVoxelGraphParameterSingleOverride::Apply(
	int32 InputKeyIndex,
	IVoxelParameterOverridesOwner* Actor,
	const FPCGContext* Context)
{
	return Invoke(ObjectOverrideFunction, this, InputKeyIndex, Actor, Context);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FPCGVoxelGraphParameterOverrides::Initialize(
	const TArray<FPCGObjectPropertyOverrideDescription>& OverrideDescriptions,
	const UPCGData* SourceData,
	const FPCGContext* Context)
{
	ObjectSingleOverrides.Reserve(OverrideDescriptions.Num());

	for (int32 Index = 0; Index < OverrideDescriptions.Num(); Index++)
	{
		FPCGAttributePropertyInputSelector InputSelector = OverrideDescriptions[Index].InputSource.CopyAndFixLast(SourceData);

		if (InputSelector.GetSelection() == EPCGAttributePropertySelection::Attribute)
		{
			const UPCGMetadata* Metadata = SourceData ? SourceData->ConstMetadata() : nullptr;
			const FName AttributeName = InputSelector.GetAttributeName();

			if (!Metadata ||
				!Metadata->HasAttribute(AttributeName))
			{
				PCGLog::LogWarningOnGraph(FText::Format(NSLOCTEXT("PCGObjectPropertyOverride", "InvalidAttribute", "Tried to initialize ObjectOverride for input '{0}', but the attribute '{1}' does not exist."), InputSelector.GetDisplayText(), FText::FromName(AttributeName)), Context);
				continue;
			}
		}

		const FString& OutputProperty = OverrideDescriptions[Index].PropertyTarget;

		FPCGVoxelGraphParameterSingleOverride Override;
		Override.Initialize(InputSelector, OutputProperty, SourceData, Context);

		if (Override.IsValid())
		{
			ObjectSingleOverrides.Add(MoveTemp(Override));
		}
	}
}

bool FPCGVoxelGraphParameterOverrides::Apply(
	IVoxelParameterOverridesOwner* Actor,
	const int32 InputKeyIndex,
	const FPCGContext* Context)
{
	if (!Actor)
	{
		return false;
	}

	bool bAllSucceeded = true;
	for (FPCGVoxelGraphParameterSingleOverride& ObjectSingleOverride : ObjectSingleOverrides)
	{
		bAllSucceeded &= ObjectSingleOverride.Apply(InputKeyIndex, Actor, Context);
	}

	return bAllSucceeded;
}