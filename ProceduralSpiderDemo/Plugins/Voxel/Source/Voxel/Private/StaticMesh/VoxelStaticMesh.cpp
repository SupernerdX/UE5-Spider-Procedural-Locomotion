// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "StaticMesh/VoxelStaticMesh.h"
#include "StaticMesh/VoxelStaticMeshCS.h"
#include "StaticMesh/VoxelStaticMeshData.h"
#include "VoxelBuffer.h"

#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "Engine/StaticMesh.h"
#include "Misc/ScopedSlowTask.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#include "Editor/EditorEngine.h"
#include "EditorReimportHandler.h"
#include "DerivedDataCacheInterface.h"
#endif

DEFINE_VOXEL_FACTORY(UVoxelStaticMesh);

#if WITH_EDITOR
VOXEL_RUN_ON_STARTUP_GAME()
{
	FReimportManager::Instance()->OnPostReimport().AddLambda([](const UObject* Asset, bool bSuccess)
	{
		ForEachObjectOfClass_Copy<UVoxelStaticMesh>([&](UVoxelStaticMesh& StaticMesh)
		{
			if (StaticMesh.Mesh == Asset)
			{
				StaticMesh.OnReimport();
			}
		});
	});
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelStaticMesh::BakeMetadatas_EditorOnly()
{
	VOXEL_FUNCTION_COUNTER();

	if (Metadatas.Num() == 0)
	{
		return;
	}

	UStaticMesh* LoadedMesh = Mesh.LoadSynchronous();
	if (!LoadedMesh)
	{
		VOXEL_MESSAGE(Info, "Failed to bake metadata: no static mesh");
		return;
	}

	const TSharedPtr<const FVoxelStaticMeshData> Data = GetData();
	if (!Data)
	{
		VOXEL_MESSAGE(Info, "Failed to bake metadata: failed to voxelize");
		return;
	}

	const FMeshDescription* MeshDescription = LoadedMesh->GetMeshDescription(0);
	if (!ensure(MeshDescription))
	{
		VOXEL_MESSAGE(Error, "Failed to read vertex data from {0}", LoadedMesh);
		return;
	}

	TVoxelArray<FVector4f> Vertices;
	TVoxelArray<FVector4f> TangentX;
	TVoxelArray<FVector4f> TangentY;
	TVoxelArray<FVector4f> TangentZ;
	TVoxelArray<FVector4f> Colors;
	TVoxelArray<TVoxelArray<FVector2f>> AllTextureCoordinates;
	{
		VOXEL_SCOPE_COUNTER("Copy mesh data");

		const FStaticMeshConstAttributes Attributes(*MeshDescription);
		const TVertexAttributesConstRef<FVector3f> VertexPositions = Attributes.GetVertexPositions();
		const TVertexInstanceAttributesConstRef<FVector3f> VertexTangent = Attributes.GetVertexInstanceTangents();
		const TVertexInstanceAttributesConstRef<FVector3f> VertexNormal = Attributes.GetVertexInstanceNormals();
		const TVertexInstanceAttributesConstRef<float> VertexBinormalSign = Attributes.GetVertexInstanceBinormalSigns();
		const TVertexInstanceAttributesConstRef<FVector4f> VertexColor = Attributes.GetVertexInstanceColors();

		const TVertexInstanceAttributesConstRef<FVector2f> InstanceUVs = Attributes.GetVertexInstanceUVs();
		const int32 NumUVLayers = InstanceUVs.GetNumChannels();

		FVoxelUtilities::SetNumFast(Vertices, Data->Points.Num());
		FVoxelUtilities::SetNumFast(TangentX, Data->Points.Num());
		FVoxelUtilities::SetNumFast(TangentY, Data->Points.Num());
		FVoxelUtilities::SetNumFast(TangentZ, Data->Points.Num());
		FVoxelUtilities::SetNumFast(Colors, Data->Points.Num());

		AllTextureCoordinates.SetNum(NumUVLayers);

		for (TVoxelArray<FVector2f>& TextureCoordinates : AllTextureCoordinates)
		{
			FVoxelUtilities::SetNumFast(TextureCoordinates, Data->Points.Num());
		}

		for (int32 Index = 0; Index < Data->Points.Num(); Index++)
		{
			const FVoxelStaticMeshPoint Point = Data->Points[Index];

			if (!ensure(MeshDescription->IsTriangleValid(Point.TriangleId)))
			{
				continue;
			}

			const TConstVoxelArrayView<const FVertexID> VertexIDs = MeshDescription->GetTriangleVertices(Point.TriangleId);
			const TConstVoxelArrayView<const FVertexInstanceID> VertexInstanceIDs = MeshDescription->GetTriangleVertexInstances(Point.TriangleId);
			checkVoxelSlow(VertexIDs.Num() == 3);
			checkVoxelSlow(VertexInstanceIDs.Num() == 3);

			const FVector3f Barycentric = Point.GetBarycentric();

			Vertices[Index] = FVector4f(
				VertexPositions[VertexIDs[0]] * Barycentric.X +
				VertexPositions[VertexIDs[1]] * Barycentric.Y +
				VertexPositions[VertexIDs[2]] * Barycentric.Z);

			TangentX[Index] = FVector4f((
				VertexTangent[VertexInstanceIDs[0]] * Barycentric.X +
				VertexTangent[VertexInstanceIDs[1]] * Barycentric.Y +
				VertexTangent[VertexInstanceIDs[2]] * Barycentric.Z).GetSafeNormal());

			TangentY[Index] = FVector4f((
				FVector3f::CrossProduct(VertexNormal[VertexInstanceIDs[0]], VertexTangent[VertexInstanceIDs[0]]) * VertexBinormalSign[VertexIDs[0]] * Barycentric.X +
				FVector3f::CrossProduct(VertexNormal[VertexInstanceIDs[1]], VertexTangent[VertexInstanceIDs[1]]) * VertexBinormalSign[VertexIDs[1]] * Barycentric.Y +
				FVector3f::CrossProduct(VertexNormal[VertexInstanceIDs[2]], VertexTangent[VertexInstanceIDs[2]]) * VertexBinormalSign[VertexIDs[2]] * Barycentric.Z).GetSafeNormal());

			TangentZ[Index] = FVector4f((
				VertexNormal[VertexInstanceIDs[0]] * Barycentric.X +
				VertexNormal[VertexInstanceIDs[1]] * Barycentric.Y +
				VertexNormal[VertexInstanceIDs[2]] * Barycentric.Z).GetSafeNormal());

			Colors[Index] = FVector4f(
				VertexColor[VertexInstanceIDs[0]] * Barycentric.X +
				VertexColor[VertexInstanceIDs[1]] * Barycentric.Y +
				VertexColor[VertexInstanceIDs[2]] * Barycentric.Z);

			for (int32 UVIndex = 0; UVIndex < NumUVLayers; UVIndex++)
			{
				AllTextureCoordinates[UVIndex][Index] =
					InstanceUVs.Get(VertexInstanceIDs[0], UVIndex) * Barycentric.X +
					InstanceUVs.Get(VertexInstanceIDs[1], UVIndex) * Barycentric.Y +
					InstanceUVs.Get(VertexInstanceIDs[2], UVIndex) * Barycentric.Z;
			}
		}
	}

	if (!ensure(Vertices.Num() > 0))
	{
		return;
	}

	for (int32 Index = 0; Index < Metadatas.Num(); Index++)
	{
		FVoxelStaticMeshMetadata& Metadata = Metadatas[Index];
		if (!Metadata.Metadata ||
			!Metadata.Material)
		{
			Metadata.Data.Buffer = {};
			continue;
		}

		const TSharedRef<FVoxelNotification> Notification = FVoxelNotification::Create("Baking metadata");
		Notification->MarkAsPending();

		FVoxelStaticMeshCS::Compute(
			*Metadata.Material,
			Metadata.Metadata->GetInnerType(),
			Vertices,
			TangentX,
			TangentY,
			TangentZ,
			Colors,
			AllTextureCoordinates,
			Metadata.Attribute)
		.Then_GameThread(MakeWeakObjectPtrLambda(this, [=, this](const TSharedPtr<FVoxelBuffer>& Buffer)
		{
			if (Buffer)
			{
				Notification->MarkAsCompletedAndExpire(5.f);
			}
			else
			{
				Notification->MarkAsFailedAndExpire(5.f);
			}

			if (!ensure(Metadatas.IsValidIndex(Index)))
			{
				return;
			}

			Metadatas[Index].Data.Buffer = Buffer;

			(void)MarkPackageDirty();

			OnChanged_EditorOnly.Broadcast();
		}));
	}
}

void UVoxelStaticMesh::CreateMeshData_EditorOnly()
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedPtr<const FVoxelStaticMeshData> PreviousData = PrivateData;

	ON_SCOPE_EXIT
	{
		for (FVoxelStaticMeshMetadata& Metadata : Metadatas)
		{
			if (!Metadata.Data.Buffer)
			{
				continue;
			}

			if (PrivateData &&
				PrivateData->Points.Num() == Metadata.Data.Buffer->Num_Slow())
			{
				continue;
			}

			Metadata.Data.Buffer = {};
		}

		if (PrivateData != PreviousData)
		{
			OnChanged_EditorOnly.Broadcast();
		}
	};

	UStaticMesh* LoadedMesh = Mesh.LoadSynchronous();
	if (!LoadedMesh)
	{
		PrivateData = {};
		return;
	}

	FString KeySuffix;

	const FStaticMeshSourceModel& SourceModel = LoadedMesh->GetSourceModel(0);
	if (ensure(SourceModel.GetMeshDescriptionBulkData()))
	{
		KeySuffix += "MD";
		KeySuffix += SourceModel.GetMeshDescriptionBulkData()->GetIdString();
	}

	{
		FVoxelWriter Writer;
		Writer << VoxelSize;
		Writer << BoundsExtension;
		Writer << VoxelizerSettings;

		KeySuffix += "_" + FVoxelUtilities::BlobToHex(Writer.Bytes);
	}

	const FString DerivedDataKey = FDerivedDataCacheInterface::BuildCacheKey(
		TEXT("VOXEL_STATIC_MESH"),
		TEXT("0BA28D61459848B69C2944A44530900A"),
		*KeySuffix);

	TArray<uint8> DerivedData;
	if (GetDerivedDataCacheRef().GetSynchronous(*DerivedDataKey, DerivedData, GetPathName()))
	{
		FMemoryReader Ar(DerivedData);

		const TSharedRef<FVoxelStaticMeshData> MeshData = MakeShared<FVoxelStaticMeshData>();
		MeshData->Serialize(Ar);

		PrivateData = MeshData;
		return;
	}

	LOG_VOXEL(Log, "Voxelizing %s", *GetPathName());

	FScopedSlowTask SlowTask(1.f, FText::FromString("Voxelizing " + GetPathName()));
	// Only allow cancelling in PostEditChangeProperty
	SlowTask.MakeDialog(false, true);
	SlowTask.EnterProgressFrame();

	const TSharedPtr<FVoxelStaticMeshData> MeshData = FVoxelStaticMeshData::VoxelizeMesh(
		*LoadedMesh,
		VoxelSize,
		BoundsExtension,
		VoxelizerSettings);

	if (!MeshData)
	{
		PrivateData = {};
		return;
	}

	FVoxelWriter Writer;
	ConstCast(*MeshData).Serialize(Writer);
	GetDerivedDataCacheRef().Put(*DerivedDataKey, Writer.Bytes, GetPathName());

	PrivateData = MeshData;
}

void UVoxelStaticMesh::OnReimport()
{
	VOXEL_FUNCTION_COUNTER();

	PrivateData.Reset();
	CreateMeshData_EditorOnly();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<const FVoxelStaticMeshData> UVoxelStaticMesh::GetData()
{
#if WITH_EDITOR
	if (!PrivateData)
	{
		CreateMeshData_EditorOnly();
	}
#endif
	return PrivateData;
}

TSharedPtr<const FVoxelStaticMeshData> UVoxelStaticMesh::GetData_NoPrepare()
{
	return PrivateData;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelStaticMesh::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);

	bool bCooked = Ar.IsCooking();
	Ar << bCooked;

	if (Ar.IsCountingMemory())
	{
		if (PrivateData)
		{
			Ar.CountBytes(
				PrivateData->GetAllocatedSize(),
				PrivateData->GetAllocatedSize());
		}

		return;
	}

	if (!bCooked ||
		IsTemplate())
	{
		return;
	}

	if (Ar.IsLoading())
	{
		const TSharedRef<FVoxelStaticMeshData> NewData = MakeShared<FVoxelStaticMeshData>();
		NewData->Serialize(Ar);
		PrivateData = NewData;

		if (PrivateData->Size.IsZero())
		{
			PrivateData.Reset();
		}
	}
#if WITH_EDITOR
	else if (Ar.IsSaving())
	{
		if (!PrivateData)
		{
			CreateMeshData_EditorOnly();
		}
		if (!PrivateData)
		{
			PrivateData = MakeShared<FVoxelStaticMeshData>();
			ensure(PrivateData->Size.IsZero());
		}
		ConstCast(*PrivateData).Serialize(Ar);
	}
#endif
}

#if WITH_EDITOR
void UVoxelStaticMesh::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
	{
		return;
	}

	// Recompute data
	// Do this next frame to be able to undo on cancel
	FVoxelUtilities::DelayedCall(MakeWeakObjectPtrLambda(this, [this]
	{
		FScopedSlowTask SlowTask(1.f, FText::FromString("Voxelizing " + GetPathName()));
		SlowTask.MakeDialog(true, true);
		SlowTask.EnterProgressFrame();

		CreateMeshData_EditorOnly();

		if (SlowTask.ShouldCancel())
		{
			ensure(GEditor->UndoTransaction());
			return;
		}

		BakeMetadatas_EditorOnly();

		OnChanged_EditorOnly.Broadcast();
	}));
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelStaticMesh::Migrate(
	TObjectPtr<UStaticMesh>& OldMesh,
	TObjectPtr<UVoxelStaticMesh>& NewMesh)
{
	VOXEL_FUNCTION_COUNTER();

	if (!OldMesh ||
		!ensure(!NewMesh))
	{
		return;
	}

	FString Path = "/Game/VSM_" + OldMesh->GetName();

	while (true)
	{
		UVoxelStaticMesh* Asset = LoadObject<UVoxelStaticMesh>(
			nullptr,
			*Path,
			nullptr,
			LOAD_NoWarn);

		if (!Asset)
		{
			break;
		}

		if (Asset->Mesh == OldMesh)
		{
			OldMesh = {};
			NewMesh = Asset;
			return;
		}

		Path += "_New";
	}

	UVoxelStaticMesh* Asset = FVoxelUtilities::CreateNewAsset_Direct<UVoxelStaticMesh>(Path, {}, {}, {});
	if (!ensure(Asset))
	{
		return;
	}

	FVoxelUtilities::DelayedCall([WeakAsset = MakeVoxelObjectPtr(Asset)]
	{
		const UVoxelStaticMesh* LocalAsset = WeakAsset.Resolve();
		if (!ensure(LocalAsset))
		{
			return;
		}

		UEditorLoadingAndSavingUtils::SavePackages(
			TArray<UPackage*>
			{
				LocalAsset->GetOutermost()
			},
			false);
	});

	Asset->Mesh = OldMesh;

	OldMesh = {};
	NewMesh = Asset;
}
#endif