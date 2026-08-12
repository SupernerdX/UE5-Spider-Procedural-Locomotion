// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampRef.h"
#include "VoxelStampManager.h"
#include "UObject/UObjectThreadContext.h"

FVoxelStampRefCopyHandler& FVoxelStampRefCopyHandler::operator=(const FVoxelStampRefCopyHandler& InOther)
{
	FVoxelStampRef& This = *reinterpret_cast<FVoxelStampRef*>(this);
	const FVoxelStampRef& Other = *reinterpret_cast<const FVoxelStampRef*>(&InOther);

	FUObjectThreadContext& Context = FUObjectThreadContext::Get();
	if (Context.TopInitializer())
	{
		// Object is being constructed or duplicated, make sure to not share the stamp variable
		This = Other.MakeCopy();
	}
	else
	{
		This = Other;
	}

	return *this;
}

FVoxelStampRef FVoxelStampRef::New(const FVoxelStamp& StampToCopyFrom)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelStampRef Result;
	Result.Inner->Stamp = StampToCopyFrom.MakeSharedCopy();
	Result.Inner->Stamp->WeakStampRef = Result;
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelStampRef FVoxelStampRef::MakeCopy() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!IsValid())
	{
		return {};
	}

	return New(*Inner->Stamp);
}

TSharedPtr<const FVoxelStampRuntime> FVoxelStampRef::ResolveStampRuntime() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Inner->Index.IsValid())
	{
		return {};
	}

	const TSharedPtr<FVoxelStampManager> StampManager = Inner->Index.GetWeakStampManager().Pin();
	if (!ensureVoxelSlow(StampManager))
	{
		return {};
	}

	return StampManager->ResolveStampRuntime(Inner->Index);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void FVoxelStampRef::SetStruct_Editor(UScriptStruct* Struct)
{
	VOXEL_FUNCTION_COUNTER();

	if (GetStruct() == Struct)
	{
		return;
	}

	// Ensure the current struct is backed up - it won't be on startup
	if (UScriptStruct* OldStruct = GetStruct())
	{
		if (Inner->StructToStamp_Editor.FindRef(OldStruct) != Inner->Stamp)
		{
			Inner->StructToStamp_Editor.Add_EnsureNew(OldStruct, Inner->Stamp);
		}
	}

	const TSharedPtr<FVoxelStamp> OldStamp = Inner->Stamp;

	TSharedPtr<FVoxelStamp>& NewStamp = Inner->StructToStamp_Editor.FindOrAdd(Struct);
	if (!NewStamp)
	{
		NewStamp = MakeSharedStruct<FVoxelStamp>(Struct);
	}
	Inner->Stamp = NewStamp;
	Inner->Stamp->WeakStampRef = *this;

	if (OldStamp)
	{
		// Copy shared properties from the previous stamp

		const UScriptStruct* CommonStruct = OldStamp->GetStruct();
		while (!NewStamp->IsA(CommonStruct))
		{
			CommonStruct = CastChecked<UScriptStruct>(CommonStruct->GetSuperStruct());
		}

		const TSharedRef<FVoxelStamp> Default = MakeSharedStruct<FVoxelStamp>(OldStamp->GetStruct());

		for (FProperty& Property : GetStructProperties(CommonStruct))
		{
			if (Property.HasMetaData("NoCopyEditor"))
			{
				continue;
			}

			if (Property.Identical_InContainer(OldStamp.Get(), &Default.Get()))
			{
				// Default, don't copy (we don't want to replace height stamps bounds extension for example)
				continue;
			}

			Property.CopyCompleteValue_InContainer(NewStamp.Get(), OldStamp.Get());
		}
	}

	Update();
}

FSimpleMulticastDelegate& FVoxelStampRef::OnRefreshDetails_Editor() const
{
	return Inner->OnRefreshDetails_Editor;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampRef::Register(USceneComponent& Component) const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!ensure(IsValid()) ||
		!ensure(!IsRegistered()))
	{
		return;
	}

	BulkRegister(
		MakeVoxelArrayView(*this),
		Component);
}

void FVoxelStampRef::Unregister() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!ensure(IsRegistered()))
	{
		return;
	}

	BulkUnregister(MakeVoxelArrayView(*this));
}

void FVoxelStampRef::Update() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!IsValid() ||
		!IsRegistered())
	{
		return;
	}

	USceneComponent* Component = (*this)->GetComponent().Resolve();
	if (!ensureVoxelSlow(Component))
	{
		Unregister();
		return;
	}

	FVoxelInvalidationScope Scope(Component);

	const TSharedPtr<FVoxelStampManager> StampManager = Inner->Index.GetWeakStampManager().Pin();
	if (!ensureVoxelSlow(StampManager))
	{
		Inner->Index = {};
		return;
	}

	StampManager->UpdateStamp(
		Inner->Index,
		*this,
		*Component);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampRef::BulkRegister(
	const TConstVoxelArrayView<FVoxelStampRef> StampRefs,
	USceneComponent& Component)
{
	VOXEL_FUNCTION_COUNTER_NUM(StampRefs.Num());
	check(IsInGameThread());

	TVoxelInlineArray<FVoxelStampRef, 1> StampRefsToRegister;
	StampRefsToRegister.Reserve(StampRefs.Num());

	for (int32 Index = 0; Index < StampRefs.Num(); Index++)
	{
		const FVoxelStampRef& StampRef = StampRefs[Index];
		if (!StampRef ||
			StampRef.IsRegistered())
		{
			continue;
		}

		StampRefsToRegister.Add_EnsureNoGrow(StampRef);
	}

	if (StampRefsToRegister.Num() == 0)
	{
		return;
	}

	TVoxelInlineArray<FVoxelStampIndex, 1> StampIndices;
	FVoxelUtilities::SetNum(StampIndices, StampRefsToRegister.Num());

	const TSharedRef<FVoxelStampManager> StampManager = FVoxelStampManager::Get(Component.GetWorld());

	StampManager->RegisterStamps(
		StampRefsToRegister,
		StampIndices,
		Component);

	for (int32 Index = 0; Index < StampRefsToRegister.Num(); Index++)
	{
		const FVoxelStampRef& StampRef = StampRefsToRegister[Index];

		checkVoxelSlow(!StampRef.Inner->Index.IsValid());
		StampRef.Inner->Index = StampIndices[Index];
	}
}

void FVoxelStampRef::BulkUnregister(const TConstVoxelArrayView<FVoxelStampRef> StampRefs)
{
	VOXEL_FUNCTION_COUNTER_NUM(StampRefs.Num());
	check(IsInGameThread());

	TWeakPtr<FVoxelStampManager> WeakStampManager;

	TVoxelInlineArray<FVoxelStampIndex, 1> StampIndices;
	StampIndices.Reserve(StampRefs.Num());

	for (const FVoxelStampRef& StampRef : StampRefs)
	{
		if (!StampRef ||
			!StampRef.IsRegistered())
		{
			continue;
		}

		const FVoxelStampIndex Index = StampRef.Inner->Index;
		StampRef.Inner->Index = {};

		if (StampIndices.Num() == 0)
		{
			WeakStampManager = Index.GetWeakStampManager();
		}
		else if (!ensureVoxelSlow(WeakStampManager == Index.GetWeakStampManager()))
		{
			const TSharedPtr<FVoxelStampManager> StampManager = WeakStampManager.Pin();
			if (ensureVoxelSlow(StampManager))
			{
				StampManager->UnregisterStamps(StampIndices);
			}
			StampIndices.Reset();

			WeakStampManager = Index.GetWeakStampManager();
		}

		checkVoxelSlow(!IsExplicitlyNull(WeakStampManager));
		checkVoxelSlow(WeakStampManager == Index.GetWeakStampManager());

		StampIndices.Add_EnsureNoGrow(Index);
	}

	if (StampIndices.Num() == 0)
	{
		return;
	}

	const TSharedPtr<FVoxelStampManager> StampManager = WeakStampManager.Pin();
	if (!ensureVoxelSlow(StampManager))
	{
		return;
	}

	StampManager->UnregisterStamps(StampIndices);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

using FVoxelStampRefCustomVersion = DECLARE_VOXEL_VERSION
(
	FirstVersion,
	CustomSerialization
);

constexpr FVoxelGuid GVoxelStampRefCustomVersionGUID = VOXEL_GUID("14111C4AF59049D9A4B5C3053D03FBED");

FCustomVersionRegistration GRegisterVoxelStampRefCustomVersionGUID(
	GVoxelStampRefCustomVersionGUID,
	FVoxelStampRefCustomVersion::LatestVersion,
	TEXT("VoxelStampRefVer"));

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelStampRef::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	SerializeVoxelVersion(Ar);

	Ar.UsingCustomVersion(GVoxelStampRefCustomVersionGUID);

	if (Ar.CustomVer(GVoxelStampRefCustomVersionGUID) < FVoxelStampRefCustomVersion::CustomSerialization)
	{
		return true;
	}

	if (Ar.IsLoading())
	{
		Inner->Load(Ar);

		if (Inner->Stamp)
		{
			Inner->Stamp->WeakStampRef = *this;
		}
	}
	else if (
		Ar.IsSaving() ||
		// Make sure object references are accounted for
		// This is critical for FindReferences or example packaging to work
		Ar.IsObjectReferenceCollector() ||
		Ar.IsCountingMemory())
	{
		Inner->Save(Ar);
	}

	return true;
}

bool FVoxelStampRef::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Inner->NetSerialize(Ar, Map, bOutSuccess);

	if (Ar.IsLoading())
	{
		if (Inner->Stamp)
		{
			Inner->Stamp->WeakStampRef = *this;
		}
	}

	return true;
}

bool FVoxelStampRef::Identical(const FVoxelStampRef* Other, const uint32 PortFlags) const
{
	if (!ensure(Other))
	{
		return false;
	}

	if (!IsValid() &&
		!Other->IsValid())
	{
		return true;
	}

	if (GetStruct() != Other->GetStruct())
	{
		return false;
	}

	const UScriptStruct* Struct = GetStruct();
	check(Struct);

	VOXEL_SCOPE_COUNTER_FORMAT("CompareScriptStruct %s", *Struct->GetName());
	return Struct->CompareScriptStruct(GetStampData(), Other->GetStampData(), PortFlags);
}

bool FVoxelStampRef::ExportTextItem(FString& ValueStr, const FVoxelStampRef& DefaultValue, UObject* Parent, const int32 PortFlags, UObject* ExportRootScope) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!IsValid())
	{
		ValueStr += "(None)";
		return true;
	}

	const UScriptStruct* Struct = GetStruct();
	check(Struct);

	ValueStr += "(" + Struct->GetPathName() + ",";

	// ALWAYS provide a defaults, otherwise FProperty::Identical assumes the default is 0
	const TSharedRef<FVoxelStamp> Defaults = MakeSharedStruct<FVoxelStamp>(Struct);

	Struct->ExportText(
		ValueStr,
		GetStampData(),
		Struct == DefaultValue.GetStruct() ? DefaultValue.GetStampData() : &Defaults.Get(),
		Parent,
		PortFlags,
		ExportRootScope);

	ValueStr += ")";
	return true;
}

bool FVoxelStampRef::ImportTextItem(const TCHAR*& Buffer, const int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText, FArchive* InSerializingArchive)
{
	VOXEL_FUNCTION_COUNTER();

	TSharedPtr<FVoxelStamp> DefaultStamp;
	if (Inner->Stamp)
	{
		DefaultStamp = Inner->Stamp->MakeSharedCopy();
	}

	Inner.SharedRef = MakeShared<FVoxelStampRefInner>();

	if (!ensureVoxelSlow(*Buffer == TEXT('(')))
	{
		return false;
	}
	Buffer++;

	if (*Buffer == TEXT(')'))
	{
		Buffer++;
		return true;
	}

	FString StructPath;
	{
		const TCHAR* Result = FPropertyHelpers::ReadToken(Buffer, StructPath, true);
		if (!Result)
		{
			return false;
		}
		Buffer = Result;
	}

	if (StructPath.Len() == 0 ||
		StructPath == "None")
	{
		if (!ensureVoxelSlow(*Buffer == TEXT(')')))
		{
			return false;
		}
		Buffer++;

		return true;
	}

	if (!ensureVoxelSlow(*Buffer == TEXT(',')))
	{
		return false;
	}
	Buffer++;

	const UScriptStruct* Struct = LoadObject<UScriptStruct>(nullptr, *StructPath);
	if (!ensure(Struct))
	{
		return false;
	}

	Inner->Stamp = MakeSharedStruct<FVoxelStamp>(
		Struct,
		DefaultStamp &&
		DefaultStamp->GetStruct() == Struct
		? DefaultStamp.Get()
		: nullptr);
	Inner->Stamp->WeakStampRef = *this;

	const TCHAR* Result = Struct->ImportText(
		Buffer,
		GetStampData(),
		Parent,
		PortFlags,
		ErrorText,
		[&]
		{
			return Struct->GetName();
		});

	if (!Result)
	{
		return false;
	}

	Buffer = Result;

	if (!ensureVoxelSlow(*Buffer == TEXT(')')))
	{
		return false;
	}
	Buffer++;

	return true;
}

void FVoxelStampRef::AddStructReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_FUNCTION_COUNTER();

	if (const TSharedPtr<FVoxelStamp> Stamp = Inner->Stamp)
	{
		TObjectPtr<UScriptStruct> Struct = Stamp->GetStruct();
		Collector.AddReferencedObject(Struct);
		check(Struct);

		FVoxelUtilities::AddStructReferencedObjects(Collector, *Stamp);
	}

#if WITH_EDITOR
	for (const auto& It : Inner->StructToStamp_Editor)
	{
		TObjectPtr<UScriptStruct> Struct = It.Key;
		Collector.AddReferencedObject(Struct);
		check(Struct);

		FVoxelUtilities::AddStructReferencedObjects(Collector, *It.Value);
	}
#endif
}

void FVoxelStampRef::GetPreloadDependencies(TArray<UObject*>& OutDependencies) const
{
	if (UScriptStruct* Struct = GetStruct())
	{
		OutDependencies.Add(Struct);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelInstancedStampRef::FVoxelInstancedStampRef(const FVoxelInstancedStampRef& Other)
{
	*this = Other;
}

FVoxelInstancedStampRef& FVoxelInstancedStampRef::operator=(const FVoxelInstancedStampRef& Other)
{
	VOXEL_FUNCTION_COUNTER();

	if (IsValid() &&
		GetStruct() == Other.GetStruct())
	{
		// Don't allocate a new shared ptr as that would break graph details
		GetStructView().CopyTo(Other.GetStructView());
	}
	else
	{
		static_cast<FVoxelStampRef&>(*this) = Other.MakeCopy();
	}

	return *this;
}