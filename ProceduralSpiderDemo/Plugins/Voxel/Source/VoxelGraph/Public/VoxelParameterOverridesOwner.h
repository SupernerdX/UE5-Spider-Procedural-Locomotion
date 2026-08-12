// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValue.h"
#include "VoxelParameterOverridesOwner.generated.h"

class UVoxelGraph;
class FVoxelParameterView;
class FVoxelGraphParametersView;

USTRUCT()
struct VOXELGRAPH_API FVoxelParameterValueOverride
{
	GENERATED_BODY()

	UPROPERTY()
	bool bEnable = false;

	UPROPERTY()
	FVoxelPinValue Value;

	UPROPERTY()
	FName CachedName;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FString CachedCategory;
#endif

	bool operator==(const FVoxelParameterValueOverride& Other) const
	{
		return
			bEnable == Other.bEnable &&
			Value == Other.Value;
	}
};

USTRUCT()
struct VOXELGRAPH_API FVoxelParameterOverrides
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FGuid, FVoxelParameterValueOverride> GuidToValueOverride;

	uint64 GetHash() const;
	bool operator==(const FVoxelParameterOverrides& Other) const;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXELGRAPH_API IVoxelParameterOverridesOwner
{
public:
	IVoxelParameterOverridesOwner() = default;
	virtual ~IVoxelParameterOverridesOwner() = default;

	virtual UObject* _getUObject() const { return nullptr; }
	virtual TSharedPtr<IVoxelParameterOverridesOwner> GetShared() const = 0;

	virtual bool ShouldForceEnableOverride(const FGuid& Guid) const = 0;
	virtual UVoxelGraph* GetGraph() const = 0;
	virtual FVoxelParameterOverrides& GetParameterOverrides() = 0;

	virtual FProperty* GetParameterOverridesProperty() const { return nullptr; }

public:
	FORCEINLINE const FVoxelParameterOverrides& GetParameterOverrides() const
	{
		return ConstCast(this)->GetParameterOverrides();
	}

	FORCEINLINE TMap<FGuid, FVoxelParameterValueOverride>& GetGuidToValueOverride()
	{
		return GetParameterOverrides().GuidToValueOverride;
	}
	FORCEINLINE const TMap<FGuid, FVoxelParameterValueOverride>& GetGuidToValueOverride() const
	{
		return ConstCast(this)->GetGuidToValueOverride();
	}

public:
	// Call in PostInitProperties to ensure delegates are bound properly
	virtual void FixupParameterOverrides();

public:
	TSharedPtr<FVoxelGraphParametersView> GetParametersView() const;
	TSharedRef<FVoxelGraphParametersView> GetParametersView_ValidGraph() const;

public:
	bool HasParameter(FName Name) const;

	FVoxelPinValue GetParameter(
		FName Name,
		FString* OutError = nullptr) const;

	FVoxelPinValue GetParameterTyped(
		FName Name,
		const FVoxelPinType& Type,
		FString* OutError = nullptr) const;

	bool SetParameter(
		FName Name,
		const FVoxelPinValue& Value,
		FString* OutError = nullptr);

public:
	// Use GetParameterChecked if possible
	template<typename T, typename ReturnType = std::decay_t<decltype(std::declval<FVoxelPinValue>().Get<T>())>>
	TOptional<ReturnType> GetParameter(const FName Name, FString* OutError = nullptr)
	{
		const FVoxelPinValue Value = this->GetParameterTyped(Name, FVoxelPinType::Make<T>(), OutError);
		if (!Value.IsValid())
		{
			return {};
		}
		return Value.Get<T>();
	}
	template<typename T, typename ReturnType = std::decay_t<decltype(std::declval<FVoxelPinValue>().Get<T>())>>
	ReturnType GetParameterChecked(const FName Name)
	{
		FString Error;
		const TOptional<ReturnType> Value = this->GetParameter<T>(Name, &Error);
		if (!ensureMsgf(Value.IsSet(), TEXT("%s"), *Error))
		{
			return {};
		}
		return Value.GetValue();
	}

	// Use SetParameterChecked if possible
	template<typename T>
	requires IsSafeVoxelPinValue<T>
	bool SetParameter(const FName Name, const T& Value, FString* OutError = nullptr)
	{
		return this->SetParameter(Name, FVoxelPinValue::Make(Value), OutError);
	}
	template<typename T>
	requires IsSafeVoxelPinValue<T>
	void SetParameterChecked(const FName Name, const T& Value)
	{
		FString Error;
		if (!this->SetParameter(Name, FVoxelPinValue::Make(Value), &Error))
		{
			ensureMsgf(false, TEXT("%s"), *Error);
		}
	}
	template<typename T>
	requires IsSafeVoxelPinValue<T>
	bool SetParameter(const FName Name, const TArray<T>& Value, FString* OutError = nullptr)
	{
		FVoxelPinValue VoxelValue = FVoxelPinValue(FVoxelPinType::Make<T>().GetBufferType().WithBufferArray(true));
		for (const T& Item : Value)
		{
			VoxelValue.AddValue(FVoxelPinValue::Make(Item).AsTerminalValue());
		}

		return this->SetParameter(Name, VoxelValue, OutError);
	}
	template<typename T>
	requires IsSafeVoxelPinValue<T>
	void SetParameterChecked(const FName Name, const TArray<T>& Value)
	{
		FVoxelPinValue VoxelValue = FVoxelPinValue(FVoxelPinType::Make<T>().GetBufferType().WithBufferArray(true));
		for (const T& Item : Value)
		{
			VoxelValue.AddValue(FVoxelPinValue::Make(Item).AsTerminalValue());
		}

		FString Error;
		if (!this->SetParameter(Name, VoxelValue, &Error))
		{
			ensureMsgf(false, TEXT("%s"), *Error);
		}
	}

private:
#if WITH_EDITOR
	FSharedVoidPtr OnParameterChangedPtr;
#endif
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UINTERFACE()
class VOXELGRAPH_API UVoxelParameterOverridesObjectOwner : public UInterface
{
	GENERATED_BODY()
};

class VOXELGRAPH_API IVoxelParameterOverridesObjectOwner
	: public IInterface
#if CPP
	, public IVoxelParameterOverridesOwner
#endif
{
	GENERATED_BODY()

private:
	virtual TSharedPtr<IVoxelParameterOverridesOwner> GetShared() const final override
	{
		return nullptr;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXELGRAPH_API FVoxelParameterOverridesOwnerPtr
{
public:
	FVoxelParameterOverridesOwnerPtr() = default;
	FVoxelParameterOverridesOwnerPtr(IVoxelParameterOverridesOwner* Owner);

	FORCEINLINE bool IsValid() const
	{
		return
			WeakPtr.IsValid() ||
			ObjectPtr.IsValid_Slow();
	}
	FORCEINLINE IVoxelParameterOverridesOwner* Get() const
	{
		if (!IsValid())
		{
			return nullptr;
		}
		return OwnerPtr;
	}
	FORCEINLINE UObject* GetObject() const
	{
		return ObjectPtr.Resolve();
	}

	FORCEINLINE IVoxelParameterOverridesOwner* operator->() const
	{
		check(IsValid());
		return OwnerPtr;
	}
	FORCEINLINE IVoxelParameterOverridesOwner* operator*() const
	{
		check(IsValid());
		return OwnerPtr;
	}

private:
	IVoxelParameterOverridesOwner* OwnerPtr = nullptr;
	FWeakVoidPtr WeakPtr;
	FVoxelObjectPtr ObjectPtr;
};