// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelParameterOverridesOwner.h"
#include "Metadata/PCGObjectPropertyOverride.h"

struct FPCGVoxelObjectSingleOverride
{
	/** Initialize the single object override. Call before using Apply(InputKeyIndex, OutputKey). */
	void Initialize(const FPCGAttributePropertySelector& InputSelector, const FString& OutputProperty, const UStruct* TemplateClass, const UPCGData* SourceData, FPCGContext* Context);

	/** Returns true if initialization succeeded in creating the accessors and accessor keys. */
	bool IsValid() const;

	/** Applies a single property override to the object by reading from the InputAccessor at the given KeyIndex, and writing to the OutputKey which represents the object property. */
	bool Apply(int32 InputKeyIndex, IPCGAttributeAccessorKeys& OutputKey);

private:
	TUniquePtr<const IPCGAttributeAccessorKeys> InputKeys;
	TUniquePtr<const IPCGAttributeAccessor> ObjectOverrideInputAccessor;
	TUniquePtr<IPCGAttributeAccessor> ObjectOverrideOutputAccessor;

	// InputKeyIndex, OutputKeys
	using ApplyOverrideFunction = bool(FPCGVoxelObjectSingleOverride::*)(int32, IPCGAttributeAccessorKeys&);
	ApplyOverrideFunction ObjectOverrideFunction = nullptr;

	template <typename Type>
	bool ApplyImpl(int32 InputKeyIndex, IPCGAttributeAccessorKeys& OutputKey)
	{
		if (!IsValid())
		{
			return false;
		}

		Type Value{};
		check(ObjectOverrideInputAccessor.IsValid());
		if (ObjectOverrideInputAccessor->Get<Type>(Value, InputKeyIndex, *InputKeys.Get(), EPCGAttributeAccessorFlags::AllowBroadcast | EPCGAttributeAccessorFlags::AllowConstructible))
		{
			check(ObjectOverrideOutputAccessor.IsValid());
			if (ObjectOverrideOutputAccessor->Set<Type>(Value, OutputKey))
			{
				return true;
			}
		}

		return false;
	}
};

/**
* Represents a set of property overrides for the provided object. Provide a SourceData to read from, and a collection of ObjectPropertyOverrides matching the TemplateObject's class properties.
*/
template <typename T>
struct FPCGVoxelObjectOverrides
{
	FPCGVoxelObjectOverrides(T* TemplateObject) : OutputKey(TemplateObject)
	{}

	/** Initialize the object overrides. Call before using Apply(InputKeyIndex). */
	void Initialize(const TArray<FPCGObjectPropertyOverrideDescription>& OverrideDescriptions, const UStruct* Type, const UPCGData* SourceData, FPCGContext* Context)
	{
		if (!Type)
		{
			PCGLog::LogErrorOnGraph(NSLOCTEXT("PCGObjectPropertyOverride", "InitializeOverrideFailedNoObject", "Failed to initialize property overrides. No template object was provided."), Context);
			return;
		}

		ObjectSingleOverrides.Reserve(OverrideDescriptions.Num());

		for (int32 i = 0; i < OverrideDescriptions.Num(); ++i)
		{
			FPCGAttributePropertyInputSelector InputSelector = OverrideDescriptions[i].InputSource.CopyAndFixLast(SourceData);

			if (InputSelector.GetSelection() == EPCGAttributePropertySelection::Attribute)
			{
				const UPCGMetadata* Metadata = SourceData ? SourceData->ConstMetadata() : nullptr;
				const FName AttributeName = InputSelector.GetAttributeName();

				if (!Metadata || !Metadata->HasAttribute(AttributeName))
				{
					PCGLog::LogWarningOnGraph(FText::Format(NSLOCTEXT("PCGObjectPropertyOverride", "InvalidAttribute", "Tried to initialize ObjectOverride for input '{0}', but the attribute '{1}' does not exist."), InputSelector.GetDisplayText(), FText::FromName(AttributeName)), Context);
					continue;
				}
			}

			const FString& OutputProperty = OverrideDescriptions[i].PropertyTarget;

			FPCGVoxelObjectSingleOverride Override;
			Override.Initialize(InputSelector, OutputProperty, Type, SourceData, Context);

			if (Override.IsValid())
			{
				ObjectSingleOverrides.Add(std::move(Override));
			}
			else
			{
				PCGLog::LogErrorOnGraph(FText::Format(NSLOCTEXT("PCGObjectPropertyOverride", "InitializeOverrideFailed", "Failed to initialize override '{0}' for property {1} on object '{2}'."), InputSelector.GetDisplayText(), FText::FromString(OutputProperty), FText::FromName(Type->GetFName())), Context);
			}
		}
	}

	/** Applies each property override to the object by reading from the InputAccessor at the given KeyIndex, and writing to the OutputKey which represents the object property. */
	bool Apply(int32 InputKeyIndex)
	{
		bool bAllSucceeded = true;

		for (FPCGVoxelObjectSingleOverride& ObjectSingleOverride : ObjectSingleOverrides)
		{
			bAllSucceeded &= ObjectSingleOverride.Apply(InputKeyIndex, OutputKey);
		}

		return bAllSucceeded;
	}

private:
	FPCGAttributeAccessorKeysSingleObjectPtr<T> OutputKey;
	TArray<FPCGVoxelObjectSingleOverride> ObjectSingleOverrides;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FPCGVoxelGraphParameterSingleOverride
{
	void Initialize(
		const FPCGAttributePropertySelector& InputSelector,
		const FString& OutputProperty,
		const UPCGData* SourceData,
		const FPCGContext* Context);
	bool IsValid() const;
	bool Apply(
		int32 InputKeyIndex,
		IVoxelParameterOverridesOwner* Actor,
		const FPCGContext* Context);

private:
	TUniquePtr<const IPCGAttributeAccessorKeys> InputKeys;
	TUniquePtr<const IPCGAttributeAccessor> ObjectOverrideInputAccessor;
	FName ParameterName;

	using ApplyOverrideFunction = bool(FPCGVoxelGraphParameterSingleOverride::*)(int32, IVoxelParameterOverridesOwner*, const FPCGContext*);
	ApplyOverrideFunction ObjectOverrideFunction = nullptr;

	template <typename Type>
	bool ApplyImpl(
		const int32 InputKeyIndex,
		IVoxelParameterOverridesOwner* Actor,
		const FPCGContext* Context)
	{
		if (!IsValid())
		{
			return false;
		}

		Type Value{};
		check(ObjectOverrideInputAccessor.IsValid());
		if (ObjectOverrideInputAccessor->Get<Type>(Value, InputKeyIndex, *InputKeys.Get(), EPCGAttributeAccessorFlags::AllowBroadcast | EPCGAttributeAccessorFlags::AllowConstructible))
		{
			FString Error;
			if constexpr (std::is_same_v<Type, FString>)
			{
				if (Actor->SetParameter(ParameterName, FName(Value), &Error))
				{
					return true;
				}
			}
			else
			{
				if (Actor->SetParameter(ParameterName, Value, &Error))
				{
					return true;
				}
			}

			PCGLog::LogWarningOnGraph(FText::FromString(Error), Context);
		}

		return false;
	}
};

struct FPCGVoxelGraphParameterOverrides
{
	void Initialize(
		const TArray<FPCGObjectPropertyOverrideDescription>& OverrideDescriptions,
		const UPCGData* SourceData,
		const FPCGContext* Context);
	bool Apply(
		IVoxelParameterOverridesOwner* Actor,
		int32 InputKeyIndex,
		const FPCGContext* Context);

private:
	TArray<FPCGVoxelGraphParameterSingleOverride> ObjectSingleOverrides;
};