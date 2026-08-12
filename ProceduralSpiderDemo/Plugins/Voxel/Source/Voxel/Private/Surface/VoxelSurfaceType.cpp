// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSurfaceType.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "Surface/VoxelSmartSurfaceType.h"
#include "Surface/VoxelCubicSurfaceTypeAsset.h"
#include "VoxelInvalidationCallstack.h"

#if !UE_BUILD_SHIPPING
TVoxelArray<FVoxelObjectPtr> GVoxelDebugSurfaceTypes;
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
struct TVoxelSurfaceTypeImpl
{
	TVoxelObjectPtr<T> WeakSurfaceType;
	FName Name;
	FGuid Guid;

	void Update()
	{
		checkUObjectAccess();

		if (WeakSurfaceType.IsExplicitlyNull())
		{
			return;
		}

		const T* SurfaceType = WeakSurfaceType.Resolve();
		if (!ensure(SurfaceType))
		{
			return;
		}

		Name = SurfaceType->GetFName();
		Guid = SurfaceType->Guid;
	}
};

class FVoxelSurfaceTypeManager : public FVoxelSingleton
{
public:
	FVoxelSharedCriticalSection CriticalSection;

	TVoxelArray<TVoxelSurfaceTypeImpl<UVoxelSurfaceTypeAsset>> SurfaceTypeAssets_RequiresLock;
	TVoxelArray<TVoxelSurfaceTypeImpl<UVoxelSmartSurfaceType>> SmartSurfaceTypes_RequiresLock;
	TVoxelArray<TVoxelSurfaceTypeImpl<UVoxelCubicSurfaceTypeAsset>> CubicSurfaceTypes_RequiresLock;

	TVoxelMap<TObjectPtr<UVoxelSurfaceTypeAsset>, uint16> SurfaceTypeAssetToIndex_RequiresLock;
	TVoxelMap<TObjectPtr<UVoxelSmartSurfaceType>, uint16> SmartSurfaceTypeToIndex_RequiresLock;
	TVoxelMap<TObjectPtr<UVoxelCubicSurfaceTypeAsset>, uint16> CubicSurfaceTypeToIndex_RequiresLock;

	struct FSurfaceTypeData
	{
		FVoxelSurfaceType::EClass Type = {};
		uint16 Index = 0;
	};
	TVoxelMap<FGuid, FSurfaceTypeData> GuidToData_RequiresLock;

	TVoxelFixedBitArray<FVoxelSurfaceType::MaxIndex> ValidSurfaceTypeAssets;
	TVoxelFixedBitArray<FVoxelSurfaceType::MaxIndex> ValidSmartSurfaceTypes;
	TVoxelFixedBitArray<FVoxelSurfaceType::MaxIndex> ValidCubicSurfaceTypes;

public:
	FVoxelSurfaceTypeManager()
	{
		SurfaceTypeAssets_RequiresLock.Add({});
		SmartSurfaceTypes_RequiresLock.Add({});
		CubicSurfaceTypes_RequiresLock.Add({});

		ValidSurfaceTypeAssets.Add(false);
		ValidSmartSurfaceTypes.Add(false);
		ValidCubicSurfaceTypes.Add(false);
	}

	void GetIndex(
		UVoxelSurfaceTypeInterface* SurfaceTypeInterface,
		FVoxelSurfaceType::EClass& OutType,
		uint16& OutIndex,
		bool& bOutRegister)
	{
		VOXEL_FUNCTION_COUNTER();
		checkUObjectAccess();

		if (!SurfaceTypeInterface)
		{
			return;
		}

		if (!SurfaceTypeInterface->Guid.IsValid())
		{
			SurfaceTypeInterface->Guid = FGuid::NewGuid();
			SurfaceTypeInterface->MarkPackageDirty();
			if (!GIsEditor)
			{
				VOXEL_MESSAGE(
					Error,
					"Surface Type {0} does not have a valid GUID.",
					SurfaceTypeInterface->GetName());
			}
		}

		// Check for cubic surface type BEFORE surface type asset since cubic
		// inherits from interface directly (not from asset)
		if (UVoxelCubicSurfaceTypeAsset* CubicSurfaceType = Cast<UVoxelCubicSurfaceTypeAsset>(SurfaceTypeInterface))
		{
			OutType = FVoxelSurfaceType::EClass::CubicSurfaceType;

			{
				VOXEL_SCOPE_READ_LOCK(CriticalSection);

				if (const uint16* IndexPtr = CubicSurfaceTypeToIndex_RequiresLock.Find(CubicSurfaceType))
				{
					OutIndex = *IndexPtr;
					return;
				}
			}

			{
				VOXEL_SCOPE_WRITE_LOCK(CriticalSection);

				if (const uint16* IndexPtr = CubicSurfaceTypeToIndex_RequiresLock.Find(CubicSurfaceType))
				{
					OutIndex = *IndexPtr;
					return;
				}

				check(CubicSurfaceTypes_RequiresLock.Num() < FVoxelSurfaceType::MaxIndex);
				OutIndex = uint16(CubicSurfaceTypes_RequiresLock.Add(TVoxelSurfaceTypeImpl<UVoxelCubicSurfaceTypeAsset>
				{
					CubicSurfaceType
				}));
				CubicSurfaceTypes_RequiresLock[OutIndex].Update();

				CubicSurfaceTypeToIndex_RequiresLock.Add_EnsureNew(CubicSurfaceType, uint16(OutIndex));
				ensure(ValidCubicSurfaceTypes.Add(true) == OutIndex);

				if (const FSurfaceTypeData* ExistingData = GuidToData_RequiresLock.Find(CubicSurfaceType->Guid))
				{
					FString OtherName;
					if (ExistingData->Type == FVoxelSurfaceType::EClass::SurfaceTypeAsset)
					{
						OtherName = SurfaceTypeAssets_RequiresLock[ExistingData->Index].Name.ToString();
					}
					else if (ExistingData->Type == FVoxelSurfaceType::EClass::SmartSurfaceType)
					{
						OtherName = SmartSurfaceTypes_RequiresLock[ExistingData->Index].Name.ToString();
					}
					else
					{
						OtherName = CubicSurfaceTypes_RequiresLock[ExistingData->Index].Name.ToString();
					}

					VOXEL_MESSAGE(
						Error,
						"Surface Type {0} GUID is colliding with Surface Type asset {1}. Regenerate the GUID in one of the assets.",
						CubicSurfaceType->GetName(),
						OtherName);
				}
				else
				{
					GuidToData_RequiresLock.Add_EnsureNew(CubicSurfaceType->Guid, FSurfaceTypeData(OutType, OutIndex));
				}
			}

			bOutRegister = true;
			return;
		}

		if (UVoxelSurfaceTypeAsset* SurfaceTypeAsset = Cast<UVoxelSurfaceTypeAsset>(SurfaceTypeInterface))
		{
			OutType = FVoxelSurfaceType::EClass::SurfaceTypeAsset;

			{
				VOXEL_SCOPE_READ_LOCK(CriticalSection);

				if (const uint16* IndexPtr = SurfaceTypeAssetToIndex_RequiresLock.Find(SurfaceTypeAsset))
				{
					OutIndex = *IndexPtr;
					return;
				}
			}

			{
				VOXEL_SCOPE_WRITE_LOCK(CriticalSection);

				if (const uint16* IndexPtr = SurfaceTypeAssetToIndex_RequiresLock.Find(SurfaceTypeAsset))
				{
					OutIndex = *IndexPtr;
					return;
				}

				check(SurfaceTypeAssets_RequiresLock.Num() < FVoxelSurfaceType::MaxIndex);
				OutIndex = uint16(SurfaceTypeAssets_RequiresLock.Add(TVoxelSurfaceTypeImpl<UVoxelSurfaceTypeAsset>
				{
					SurfaceTypeAsset
				}));
				SurfaceTypeAssets_RequiresLock[OutIndex].Update();

				SurfaceTypeAssetToIndex_RequiresLock.Add_EnsureNew(SurfaceTypeAsset, uint16(OutIndex));
				ensure(ValidSurfaceTypeAssets.Add(true) == OutIndex);

				if (const FSurfaceTypeData* ExistingData = GuidToData_RequiresLock.Find(SurfaceTypeAsset->Guid))
				{
					FString OtherName;
					if (ExistingData->Type == FVoxelSurfaceType::EClass::SurfaceTypeAsset)
					{
						OtherName = SurfaceTypeAssets_RequiresLock[ExistingData->Index].Name.ToString();
					}
					else if (ExistingData->Type == FVoxelSurfaceType::EClass::SmartSurfaceType)
					{
						OtherName = SmartSurfaceTypes_RequiresLock[ExistingData->Index].Name.ToString();
					}
					else
					{
						OtherName = CubicSurfaceTypes_RequiresLock[ExistingData->Index].Name.ToString();
					}

					VOXEL_MESSAGE(
						Error,
						"Surface Type {0} GUID is colliding with Surface Type asset {1}. Regenerate the GUID in one of the assets.",
						SurfaceTypeAsset->GetName(),
						OtherName);
				}
				else
				{
					GuidToData_RequiresLock.Add_EnsureNew(SurfaceTypeAsset->Guid, FSurfaceTypeData(OutType, OutIndex));
				}
			}

			bOutRegister = true;
			return;
		}

		if (UVoxelSmartSurfaceType* SmartSurfaceType = Cast<UVoxelSmartSurfaceType>(SurfaceTypeInterface))
		{
			OutType = FVoxelSurfaceType::EClass::SmartSurfaceType;

			{
				VOXEL_SCOPE_READ_LOCK(CriticalSection);

				if (const uint16* IndexPtr = SmartSurfaceTypeToIndex_RequiresLock.Find(SmartSurfaceType))
				{
					OutIndex = *IndexPtr;
					return;
				}
			}

			{
				VOXEL_SCOPE_WRITE_LOCK(CriticalSection);

				if (const uint16* IndexPtr = SmartSurfaceTypeToIndex_RequiresLock.Find(SmartSurfaceType))
				{
					OutIndex = *IndexPtr;
					return;
				}

				check(SmartSurfaceTypes_RequiresLock.Num() < FVoxelSurfaceType::MaxIndex);
				OutIndex = uint16(SmartSurfaceTypes_RequiresLock.Add(TVoxelSurfaceTypeImpl<UVoxelSmartSurfaceType>
				{
					SmartSurfaceType
				}));
				SmartSurfaceTypes_RequiresLock[OutIndex].Update();

				SmartSurfaceTypeToIndex_RequiresLock.Add_EnsureNew(SmartSurfaceType, uint16(OutIndex));
				ensure(ValidSmartSurfaceTypes.Add(true) == OutIndex);

				if (const FSurfaceTypeData* ExistingData = GuidToData_RequiresLock.Find(SmartSurfaceType->Guid))
				{
					FString OtherName;
					if (ExistingData->Type == FVoxelSurfaceType::EClass::SurfaceTypeAsset)
					{
						OtherName = SurfaceTypeAssets_RequiresLock[ExistingData->Index].Name.ToString();
					}
					else if (ExistingData->Type == FVoxelSurfaceType::EClass::SmartSurfaceType)
					{
						OtherName = SmartSurfaceTypes_RequiresLock[ExistingData->Index].Name.ToString();
					}
					else
					{
						OtherName = CubicSurfaceTypes_RequiresLock[ExistingData->Index].Name.ToString();
					}

					VOXEL_MESSAGE(
						Error,
						"Surface Type {0} GUID is colliding with Surface Type asset {1}. Regenerate the GUID in one of the assets.",
						SmartSurfaceType->GetName(),
						OtherName);
				}
				else
				{
					GuidToData_RequiresLock.Add_EnsureNew(SmartSurfaceType->Guid, FSurfaceTypeData(OutType, OutIndex));
				}
			}

			bOutRegister = true;
			return;
		}
	}

	void OnSurfaceTypeRegistered(const FVoxelSurfaceType SurfaceType) const
	{
		ensureVoxelSlow(!CriticalSection.IsLocked_Write());

		Voxel::GameTask([SurfaceType]
		{
#if !UE_BUILD_SHIPPING
			if (!GVoxelDebugSurfaceTypes.IsValidIndex(SurfaceType.RawValue))
			{
				GVoxelDebugSurfaceTypes.SetNum(SurfaceType.RawValue + 1);
			}

			GVoxelDebugSurfaceTypes[SurfaceType.RawValue] = SurfaceType.GetSurfaceTypeInterface();
#endif

			FVoxelInvalidationScope Scope("AddSurface " + SurfaceType.GetName());

			FVoxelSurfaceTypeTable::Refresh();
		});
	}

public:
	//~ Begin FVoxelSingleton Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		VOXEL_FUNCTION_COUNTER();
		VOXEL_SCOPE_WRITE_LOCK(CriticalSection);

		for (auto It = SurfaceTypeAssetToIndex_RequiresLock.CreateIterator(); It; ++It)
		{
			TObjectPtr<UVoxelSurfaceTypeAsset> Type = It.Key();
			Collector.AddReferencedObject(Type);

			if (Type)
			{
				continue;
			}

			checkVoxelSlow(ValidSurfaceTypeAssets[It.Value()]);
			ValidSurfaceTypeAssets[It.Value()] = false;

			It.RemoveCurrent();
		}

		for (auto It = SmartSurfaceTypeToIndex_RequiresLock.CreateIterator(); It; ++It)
		{
			TObjectPtr<UVoxelSmartSurfaceType> Type = It.Key();
			Collector.AddReferencedObject(Type);

			if (Type)
			{
				continue;
			}

			checkVoxelSlow(ValidSmartSurfaceTypes[It.Value()]);
			ValidSmartSurfaceTypes[It.Value()] = false;

			It.RemoveCurrent();
		}

		for (auto It = CubicSurfaceTypeToIndex_RequiresLock.CreateIterator(); It; ++It)
		{
			TObjectPtr<UVoxelCubicSurfaceTypeAsset> Type = It.Key();
			Collector.AddReferencedObject(Type);

			if (Type)
			{
				continue;
			}

			checkVoxelSlow(ValidCubicSurfaceTypes[It.Value()]);
			ValidCubicSurfaceTypes[It.Value()] = false;

			It.RemoveCurrent();
		}
	}
	//~ End FVoxelSingleton Interface
};
FVoxelSurfaceTypeManager* GVoxelSurfaceTypeManager = new FVoxelSurfaceTypeManager();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSurfaceType::FVoxelSurfaceType(UVoxelSurfaceTypeInterface* SurfaceTypeInterface)
{
	EClass Type = EClass::SurfaceTypeAsset;
	uint16 Index = 0;
	bool bRegister = false;
	GVoxelSurfaceTypeManager->GetIndex(
		SurfaceTypeInterface, 
		Type, 
		Index,
		bRegister);

	InternalType = uint16(Type);
	InternalIndex = Index;

	if (bRegister)
	{
		GVoxelSurfaceTypeManager->OnSurfaceTypeRegistered(*this);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TVoxelSurfaceTypeImpl<UVoxelSurfaceTypeAsset>& FVoxelSurfaceType::GetSurfaceTypeAssetImpl() const
{
	VOXEL_SCOPE_READ_LOCK(GVoxelSurfaceTypeManager->CriticalSection);
	return GVoxelSurfaceTypeManager->SurfaceTypeAssets_RequiresLock[InternalIndex];
}

TVoxelSurfaceTypeImpl<UVoxelSmartSurfaceType>& FVoxelSurfaceType::GetSmartSurfaceTypeImpl() const
{
	VOXEL_SCOPE_READ_LOCK(GVoxelSurfaceTypeManager->CriticalSection);
	return GVoxelSurfaceTypeManager->SmartSurfaceTypes_RequiresLock[InternalIndex];
}

TVoxelSurfaceTypeImpl<UVoxelCubicSurfaceTypeAsset>& FVoxelSurfaceType::GetCubicSurfaceTypeImpl() const
{
	VOXEL_SCOPE_READ_LOCK(GVoxelSurfaceTypeManager->CriticalSection);
	return GVoxelSurfaceTypeManager->CubicSurfaceTypes_RequiresLock[InternalIndex];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelObjectPtr<UVoxelSurfaceTypeAsset> FVoxelSurfaceType::GetSurfaceTypeAsset() const
{
	if (IsNull() ||
		!ensureVoxelSlow(GetClass() == EClass::SurfaceTypeAsset))
	{
		return {};
	}

	return GetSurfaceTypeAssetImpl().WeakSurfaceType;
}

TVoxelObjectPtr<UVoxelSmartSurfaceType> FVoxelSurfaceType::GetSmartSurfaceType() const
{
	if (IsNull() ||
		!ensureVoxelSlow(GetClass() == EClass::SmartSurfaceType))
	{
		return {};
	}

	return GetSmartSurfaceTypeImpl().WeakSurfaceType;
}

TVoxelObjectPtr<UVoxelCubicSurfaceTypeAsset> FVoxelSurfaceType::GetCubicSurfaceTypeAsset() const
{
	if (IsNull() ||
		!ensureVoxelSlow(GetClass() == EClass::CubicSurfaceType))
	{
		return {};
	}

	return GetCubicSurfaceTypeImpl().WeakSurfaceType;
}

TVoxelObjectPtr<UVoxelSurfaceTypeInterface> FVoxelSurfaceType::GetSurfaceTypeInterface() const
{
	switch (GetClass())
	{
	case EClass::SurfaceTypeAsset: return GetSurfaceTypeAsset();
	case EClass::SmartSurfaceType: return GetSmartSurfaceType();
	case EClass::CubicSurfaceType: return GetCubicSurfaceTypeAsset();
	}

	return {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FName FVoxelSurfaceType::GetFName() const
{
	if (IsNull())
	{
		return {};
	}

	switch (GetClass())
	{
	case EClass::SurfaceTypeAsset: return GetSurfaceTypeAssetImpl().Name;
	case EClass::SmartSurfaceType: return GetSmartSurfaceTypeImpl().Name;
	case EClass::CubicSurfaceType: return GetCubicSurfaceTypeImpl().Name;
	}

	return {};
}

FString FVoxelSurfaceType::GetName() const
{
	if (IsNull())
	{
		return {};
	}

	switch (GetClass())
	{
	case EClass::SurfaceTypeAsset: return GetSurfaceTypeAssetImpl().Name.ToString();
	case EClass::SmartSurfaceType: return GetSmartSurfaceTypeImpl().Name.ToString();
	case EClass::CubicSurfaceType: return GetCubicSurfaceTypeImpl().Name.ToString();
	}

	return {};
}

FLinearColor FVoxelSurfaceType::GetDebugColor() const
{
	return FLinearColor::IntToDistinctColor(RawValue, 1.f, 0.75f, 90.f);
}

FGuid FVoxelSurfaceType::GetGuid() const
{
	if (IsNull())
	{
		return {};
	}

	switch (GetClass())
	{
	case EClass::SurfaceTypeAsset: return GetSurfaceTypeAssetImpl().Guid;
	case EClass::SmartSurfaceType: return GetSmartSurfaceTypeImpl().Guid;
	case EClass::CubicSurfaceType: return GetCubicSurfaceTypeImpl().Guid;
	}

	return {};
}

void operator<<(FArchive& Ar, FVoxelSurfaceType& SurfaceType)
{
	if (Ar.GetArchiveName().StartsWith("FVoxel"))
	{
		FGuid Guid;

		if (Ar.IsSaving())
		{
			Guid = SurfaceType.GetGuid();
		}

		Ar << Guid;

		if (Ar.IsLoading())
		{
			if (!FVoxelSurfaceType::FindFromGuid(
				Guid,
				SurfaceType))
			{
				Ar.SetError();
			}
		}

		return;
	}

	UVoxelSurfaceTypeInterface* SurfaceTypeObject = nullptr;

	if (Ar.IsSaving())
	{
		SurfaceTypeObject = SurfaceType.GetSurfaceTypeInterface().Resolve();
		ensureVoxelSlow(SurfaceTypeObject);
	}

	Ar << SurfaceTypeObject;

	if (Ar.IsLoading())
	{
		SurfaceType = FVoxelSurfaceType(SurfaceTypeObject);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSurfaceType::UpdateFromSourceObject() const
{
	if (!ensureVoxelSlow(!IsNull()))
	{
		return;
	}

	switch (GetClass())
	{
	case EClass::SurfaceTypeAsset: GetSurfaceTypeAssetImpl().Update(); break;
	case EClass::SmartSurfaceType: GetSmartSurfaceTypeImpl().Update(); break;
	case EClass::CubicSurfaceType: GetCubicSurfaceTypeImpl().Update(); break;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSurfaceType::ForeachSurfaceType(const TFunctionRef<void(FVoxelSurfaceType)> Lambda)
{
	VOXEL_FUNCTION_COUNTER();

	for (const int32 Index : GVoxelSurfaceTypeManager->ValidSurfaceTypeAssets.IterateSetBits())
	{
		FVoxelSurfaceType SurfaceType;
		SurfaceType.InternalType = uint16(EClass::SurfaceTypeAsset);
		SurfaceType.InternalIndex = uint16(Index);
		Lambda(SurfaceType);
	}

	for (const int32 Index : GVoxelSurfaceTypeManager->ValidSmartSurfaceTypes.IterateSetBits())
	{
		FVoxelSurfaceType SurfaceType;
		SurfaceType.InternalType = uint16(EClass::SmartSurfaceType);
		SurfaceType.InternalIndex = uint16(Index);
		Lambda(SurfaceType);
	}

	for (const int32 Index : GVoxelSurfaceTypeManager->ValidCubicSurfaceTypes.IterateSetBits())
	{
		FVoxelSurfaceType SurfaceType;
		SurfaceType.InternalType = uint16(EClass::CubicSurfaceType);
		SurfaceType.InternalIndex = uint16(Index);
		Lambda(SurfaceType);
	}
}

bool FVoxelSurfaceType::FindFromGuid(
	const FGuid Guid,
	FVoxelSurfaceType& OutSurfaceType)
{
	if (!ensure(Guid.IsValid()))
	{
		return true;
	}

	VOXEL_SCOPE_READ_LOCK(GVoxelSurfaceTypeManager->CriticalSection);

	const FVoxelSurfaceTypeManager::FSurfaceTypeData* Data = GVoxelSurfaceTypeManager->GuidToData_RequiresLock.Find(Guid);
	if (!ensureVoxelSlow(Data))
	{
		VOXEL_MESSAGE(
			Error,
			"Failed to find surface type with GUID {0}. If this is a valid path, make sure to reference this surface type in your scene.",
			Guid.ToString());
		return false;
	}

	OutSurfaceType.InternalType = uint16(Data->Type);
	OutSurfaceType.InternalIndex = Data->Index;
	return true;
}