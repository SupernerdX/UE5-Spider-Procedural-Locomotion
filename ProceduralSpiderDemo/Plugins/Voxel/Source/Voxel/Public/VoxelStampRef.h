// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStamp.h"
#include "VoxelStampRefInner.h"
#include "VoxelStampRef.generated.h"

class IVoxelStampComponentInterface;

template<typename>
struct TVoxelStampRefImpl;

template<typename Type>
using TVoxelStampRef = typename TVoxelStampRefImpl<Type>::Type;

template<typename>
struct TVoxelInstancedStampRefImpl;

template<typename Type>
using TVoxelInstancedStampRef = typename TVoxelInstancedStampRefImpl<Type>::Type;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define GENERATED_VOXEL_STAMP_REF_BODY_IMPL(ThisType, Name) \
	using StampType = Name; \
	\
	static ThisType New(const Name& StampToCopyFrom = Name()) \
	{ \
		return ReinterpretCastRef<ThisType>(FVoxelStampRef::New(StampToCopyFrom)); \
	} \
	explicit ThisType(const Super& StampRef) \
	{ \
		if (StampRef.IsA<Name>()) \
		{ \
			static_cast<Super&>(*this) = StampRef; \
		} \
	} \
	void VOXEL_APPEND_LINE(Dummy_ ## Name)() \
	{ \
		checkStatic(std::is_same_v<ThisType, VOXEL_THIS_TYPE>); \
	} \
	\
	FORCEINLINE Name* operator->() const \
	{ \
		check(IsA<Name>()); \
		return static_cast<Name*>(Super::operator->()); \
	} \
	\
	FORCEINLINE Name& operator*() const \
	{ \
		check(IsA<Name>()); \
		return static_cast<Name&>(Super::operator*()); \
	} \
	\
	FORCEINLINE TSharedPtr<Name> ToSharedPtr() const \
	{ \
		return static_cast<const FVoxelStampRef*>(this)->ToSharedPtr<Name>(); \
	} \
	FORCEINLINE TSharedRef<Name> ToSharedRef() const \
	{ \
		return static_cast<const FVoxelStampRef*>(this)->ToSharedRef<Name>(); \
	} \
	\
	ThisType MakeCopy() const \
	{ \
		return ReinterpretCastRef<ThisType>(Super::MakeCopy()); \
	}

#define GENERATED_VOXEL_STAMP_REF_PARENT_BODY(ThisType, Name) \
	GENERATED_VOXEL_STAMP_REF_BODY_IMPL(ThisType, Name) \
	ThisType() = default; \
	ThisType(ENoInit) \
	{ \
	}

#define GENERATED_VOXEL_STAMP_REF_BODY(ThisType, Name) \
	GENERATED_VOXEL_STAMP_REF_BODY_IMPL(ThisType, Name) \
	ThisType() \
	{ \
		static_assert(std::is_final_v<Name>, #Name " should be final"); \
		checkStatic(std::is_final_v<VOXEL_THIS_TYPE>); \
		const Name Stamp; \
		static_cast<FVoxelStampRef&>(*this) = FVoxelStampRef::New(Stamp); \
	} \
	ThisType(ENoInit) \
	{ \
	}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXEL_API FVoxelStampRefCopyHandler
{
	GENERATED_BODY()

	FVoxelStampRefCopyHandler& operator=(const FVoxelStampRefCopyHandler& InOther);
};

USTRUCT(BlueprintType, DisplayName = "Voxel Stamp", Category = "Voxel|Stamp", meta = (HasNativeMake = "/Script/Voxel.VoxelStamp_K2.Make", HasNativeBreak = "/Script/Voxel.VoxelStamp_K2.Break"))
struct VOXEL_API FVoxelStampRef
{
	GENERATED_BODY()

#if !CPP
	UPROPERTY()
	FVoxelStampRefCopyHandler Inner;
#endif

public:
	using StampType = FVoxelStamp;

	FVoxelStampRef() = default;

	static FVoxelStampRef New(const FVoxelStamp& StampToCopyFrom);

public:
	FORCEINLINE bool IsValid() const
	{
		return Inner->Stamp.IsValid();
	}
	FORCEINLINE UScriptStruct* GetStruct() const
	{
		const FVoxelStamp* Stamp = Inner->Stamp.Get();
		if (!Stamp)
		{
			return nullptr;
		}

		return Stamp->GetStruct();
	}
	FORCEINLINE FVoxelStructView GetStructView() const
	{
		return FVoxelStructView(GetStruct(), GetStampData());
	}
	FORCEINLINE bool IsSharedPtrUnique() const
	{
		return
			Inner.SharedRef.IsUnique() &&
			Inner->Stamp.IsUnique();
	}

public:
	FORCEINLINE operator bool() const
	{
		return IsValid();
	}

	FORCEINLINE FVoxelStamp* operator->() const
	{
		checkVoxelSlow(IsValid());
		checkVoxelSlow(Inner->Stamp->WeakStampRef == *this);
		return Inner->Stamp.operator->();
	}
	FORCEINLINE FVoxelStamp& operator*() const
	{
		checkVoxelSlow(IsValid());
		checkVoxelSlow(Inner->Stamp->WeakStampRef == *this);
		return Inner->Stamp.operator*();
	}
	FORCEINLINE void* GetStampData() const
	{
		return Inner->Stamp.Get();
	}

	FORCEINLINE bool operator==(const FVoxelStampRef& Other) const
	{
		return Inner == Other.Inner;
	}

public:
	FORCEINLINE bool IsA(const UScriptStruct* BaseStruct) const
	{
		const UScriptStruct* Struct = GetStruct();
		if (!Struct)
		{
			return false;
		}

		return Struct->IsChildOf(BaseStruct);
	}
	template<typename Type>
	FORCEINLINE bool IsA() const
	{
		return this->IsA(StaticStructFast<Type>());
	}

public:
	template<typename Type = FVoxelStamp>
	requires std::derived_from<Type, FVoxelStamp>
	FORCEINLINE TSharedPtr<Type> ToSharedPtr() const
	{
		return CastStruct<Type>(Inner->Stamp);
	}
	template<typename Type = FVoxelStamp>
	requires std::derived_from<Type, FVoxelStamp>
	FORCEINLINE TSharedRef<Type> ToSharedRef() const
	{
		return CastStructChecked<Type>(Inner->Stamp.ToSharedRef());
	}

public:
	template<typename Type>
	FORCEINLINE Type* As() const
	{
		if (!Inner->Stamp)
		{
			return nullptr;
		}

		return Inner->Stamp->As<Type>();
	}
	template<typename Type>
	FORCEINLINE Type& AsChecked() const
	{
		return Inner->Stamp->AsChecked<Type>();
	}

public:
	template<typename T>
	FORCEINLINE TVoxelStampRef<T> CastTo() const
	{
		if (!IsA<T>())
		{
			return TVoxelStampRef<T>(NoInit);
		}

		return ReinterpretCastRef<TVoxelStampRef<T>>(*this);
	}

public:
	FVoxelStampRef MakeCopy() const;
	TSharedPtr<const FVoxelStampRuntime> ResolveStampRuntime() const;

#if WITH_EDITOR
	void SetStruct_Editor(UScriptStruct* Struct);
	FSimpleMulticastDelegate& OnRefreshDetails_Editor() const;
#endif

public:
	FORCEINLINE bool IsRegistered() const
	{
		return Inner->Index.IsValid();
	}

	void Register(USceneComponent& Component) const;
	void Unregister() const;
	void Update() const;

public:
	static void BulkRegister(
		TConstVoxelArrayView<FVoxelStampRef> StampRefs,
		USceneComponent& Component);

	static void BulkUnregister(TConstVoxelArrayView<FVoxelStampRef> StampRefs);

public:
	//~ Begin TStructOpsTypeTraits Interface
	bool Serialize(FArchive& Ar);
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
	bool Identical(const FVoxelStampRef* Other, uint32 PortFlags) const;
	bool ExportTextItem(FString& ValueStr, const FVoxelStampRef& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const;
	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText, FArchive* InSerializingArchive = nullptr);
	void AddStructReferencedObjects(FReferenceCollector& Collector);
	void GetPreloadDependencies(TArray<UObject*>& OutDependencies) const;
	//~ End TStructOpsTypeTraits Interface

private:
	struct FInnerWrapper
	{
		TSharedRef<FVoxelStampRefInner> SharedRef = MakeShared<FVoxelStampRefInner>();

		FORCEINLINE FVoxelStampRefInner* operator->() const
		{
			// Inner properties might be reassigned on the game thread, this is unsafe to call async
			checkUObjectAccess();
			return SharedRef.operator->();
		}
		FORCEINLINE bool operator==(const FInnerWrapper& Other) const
		{
			return SharedRef == Other.SharedRef;
		}
	};
	FInnerWrapper Inner;

	explicit FVoxelStampRef(const TSharedRef<FVoxelStampRefInner>& Inner)
		: Inner(FInnerWrapper
		{
			Inner
		})
	{
	}

	friend struct FVoxelWeakStampRef;
};
checkStatic(sizeof(FVoxelStampRef) == 16);

template<>
struct TStructOpsTypeTraits<FVoxelStampRef> : TStructOpsTypeTraitsBase2<FVoxelStampRef>
{
	enum
	{
		// We handle copy through FVoxelStampRefCopyHandler
		WithCopy = false,
		WithSerializer = true,
		WithNetSerializer = true,
		WithIdentical = true,
		WithExportTextItem = true,
		WithImportTextItem = true,
		WithAddStructReferencedObjects = true,
		WithGetPreloadDependencies = true,
	};
};

template<>
struct TVoxelStampRefImpl<FVoxelStamp>
{
	using Type = FVoxelStampRef;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Will do a deep copy of the stamp on copy
USTRUCT()
struct VOXEL_API FVoxelInstancedStampRef : public FVoxelStampRef
{
	GENERATED_BODY()

	FVoxelInstancedStampRef() = default;
	FVoxelInstancedStampRef(const FVoxelInstancedStampRef& Other);

	FVoxelInstancedStampRef& operator=(const FVoxelInstancedStampRef& Other);
};

template<>
struct TStructOpsTypeTraits<FVoxelInstancedStampRef> : public TStructOpsTypeTraits<FVoxelStampRef>
{
	enum
	{
		// We handle copy in FVoxelInstancedStampRef
		WithCopy = true,
	};
};

template<>
struct TVoxelInstancedStampRefImpl<FVoxelStamp>
{
	using Type = FVoxelInstancedStampRef;
};