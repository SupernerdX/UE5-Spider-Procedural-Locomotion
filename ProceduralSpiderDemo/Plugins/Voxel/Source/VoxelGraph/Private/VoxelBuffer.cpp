// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelBuffer.h"
#include "VoxelBufferStruct.h"
#include "VoxelGenericStructBuffer.h"
#include "VoxelRuntimeStructBuffer.h"
#include "Buffer/VoxelNameBuffer.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Buffer/VoxelClassBuffer.h"

struct FVoxelBufferStatics
{
	bool bInitialized = false;
	TVoxelMap<FVoxelPinType, FVoxelBuffer::FMakeBuffer> InnerToMakeEmpty;
	TVoxelMap<FVoxelPinType, FVoxelBuffer::FMakeBuffer> InnerToMakeDefault;
	TVoxelMap<UScriptStruct*, FVoxelPinType> BufferToInner;

	FORCEINLINE void InitializeIfNeeded()
	{
		if (!bInitialized)
		{
			Initialize();
		}
	}
	void Initialize()
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());

		if (bInitialized)
		{
			return;
		}

		static bool bInitializing = false;
		check(!bInitializing);
		bInitializing = true;

		ON_SCOPE_EXIT
		{
			bInitialized = true;
		};

		TVoxelChunkedArray<UScriptStruct*> Structs;
		ForEachObjectOfClass<UScriptStruct>([&](UScriptStruct& Struct)
		{
			if (Struct.GetSuperStruct() != StaticStructFast<FVoxelBufferStruct>() &&
				Struct.GetSuperStruct() != StaticStructFast<FVoxelTerminalBuffer>())
			{
				return;
			}

			Structs.Add(&Struct);
		});

		InnerToMakeEmpty.Reserve(Structs.Num());
		InnerToMakeDefault.Reserve(Structs.Num());
		BufferToInner.Reserve(Structs.Num());

		for (UScriptStruct* Struct : Structs)
		{
			const TSharedRef<FVoxelBuffer> Buffer = MakeSharedStruct<FVoxelBuffer>(Struct);
			const FVoxelPinType InnerType = Buffer->GetInnerType();

			InnerToMakeEmpty.Add_CheckNew_EnsureNoGrow(InnerType, Buffer->Internal_GetMakeEmpty());
			InnerToMakeDefault.Add_CheckNew_EnsureNoGrow(InnerType, Buffer->Internal_GetMakeDefault());
			BufferToInner.Add_CheckNew_EnsureNoGrow(Struct, InnerType);
		}
	}
};
FVoxelBufferStatics GVoxelBufferStatics;

VOXEL_RUN_ON_STARTUP(Game, 999)
{
	GVoxelBufferStatics.InitializeIfNeeded();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelBuffer> FVoxelBuffer::MakeEmpty(const FVoxelPinType& InnerType)
{
	checkVoxelSlow(!InnerType.IsBuffer());

	switch (InnerType.GetInternalType())
	{
	default: checkf(false, TEXT("Invalid buffer type: %s"), *InnerType.ToString());
	case EVoxelPinInternalType::Bool: return MakeShared<FVoxelBoolBuffer>();
	case EVoxelPinInternalType::Float: return MakeShared<FVoxelFloatBuffer>();
	case EVoxelPinInternalType::Double: return MakeShared<FVoxelDoubleBuffer>();
	case EVoxelPinInternalType::UInt16: return MakeShared<FVoxelUInt16Buffer>();
	case EVoxelPinInternalType::Int32: return MakeShared<FVoxelInt32Buffer>();
	case EVoxelPinInternalType::Int64: return MakeShared<FVoxelInt64Buffer>();
	case EVoxelPinInternalType::Name: return MakeShared<FVoxelNameBuffer>();
	case EVoxelPinInternalType::Byte: return MakeShared<FVoxelByteBuffer>();
	case EVoxelPinInternalType::Class: return MakeShared<FVoxelClassBuffer>();
	case EVoxelPinInternalType::Struct:
	{
		GVoxelBufferStatics.InitializeIfNeeded();

		if (const FMakeBuffer MakeEmpty = GVoxelBufferStatics.InnerToMakeEmpty.FindRef(InnerType))
		{
			return MakeEmpty();
		}

		if (InnerType.IsUserDefinedStruct())
		{
			return MakeSharedCopy(FVoxelRuntimeStructBuffer::MakeEmpty(*InnerType.GetStruct()));
		}

		return MakeSharedCopy(FVoxelGenericStructBuffer::MakeEmpty(*InnerType.GetStruct()));
	}
	}
}

TSharedRef<FVoxelBuffer> FVoxelBuffer::MakeDefault(const FVoxelPinType& InnerType)
{
	checkVoxelSlow(!InnerType.IsBuffer());

	switch (InnerType.GetInternalType())
	{
	default: checkf(false, TEXT("Invalid buffer type: %s"), *InnerType.ToString());
	case EVoxelPinInternalType::Bool: return MakeShared<FVoxelBoolBuffer>(DefaultBuffer);
	case EVoxelPinInternalType::Float: return MakeShared<FVoxelFloatBuffer>(DefaultBuffer);
	case EVoxelPinInternalType::Double: return MakeShared<FVoxelDoubleBuffer>(DefaultBuffer);
	case EVoxelPinInternalType::UInt16: return MakeShared<FVoxelUInt16Buffer>(DefaultBuffer);
	case EVoxelPinInternalType::Int32: return MakeShared<FVoxelInt32Buffer>(DefaultBuffer);
	case EVoxelPinInternalType::Int64: return MakeShared<FVoxelInt64Buffer>(DefaultBuffer);
	case EVoxelPinInternalType::Name: return MakeShared<FVoxelNameBuffer>(DefaultBuffer);
	case EVoxelPinInternalType::Byte: return MakeShared<FVoxelByteBuffer>(DefaultBuffer);
	case EVoxelPinInternalType::Class: return MakeShared<FVoxelClassBuffer>(DefaultBuffer);
	case EVoxelPinInternalType::Struct:
	{
		GVoxelBufferStatics.InitializeIfNeeded();

		if (const FMakeBuffer MakeDefault = GVoxelBufferStatics.InnerToMakeDefault.FindRef(InnerType))
		{
			return MakeDefault();
		}

		if (InnerType.IsUserDefinedStruct())
		{
			return MakeSharedCopy(FVoxelRuntimeStructBuffer::MakeDefault(*InnerType.GetStruct()));
		}

		return MakeSharedCopy(FVoxelGenericStructBuffer::MakeDefault(*InnerType.GetStruct()));
	}
	}
}

TSharedRef<FVoxelBuffer> FVoxelBuffer::MakeConstant(const FVoxelRuntimePinValue& Value)
{
	const TSharedRef<FVoxelBuffer> Buffer = MakeEmpty(Value.GetType());
	Buffer->Allocate(1);
	Buffer->SetGeneric(0, Value);
	return Buffer;
}

TSharedRef<FVoxelBuffer> FVoxelBuffer::MakeZeroed(
	const FVoxelPinType& InnerType,
	const int32 Num)
{
	const TSharedRef<FVoxelBuffer> Buffer = MakeEmpty(InnerType);
	Buffer->AllocateZeroed(Num);
	return Buffer;
}

FVoxelPinType FVoxelBuffer::FindInnerType(UScriptStruct* BufferStruct)
{
	checkVoxelSlow(BufferStruct->IsChildOf(FVoxelBuffer::StaticStruct()));
	checkVoxelSlow(BufferStruct != FVoxelBuffer::StaticStruct());
	checkVoxelSlow(BufferStruct != FVoxelBufferStruct::StaticStruct());
	checkVoxelSlow(BufferStruct != FVoxelTerminalBuffer::StaticStruct());
	checkVoxelSlow(BufferStruct != FVoxelGenericStructBuffer::StaticStruct());

	GVoxelBufferStatics.InitializeIfNeeded();
	return GVoxelBufferStatics.BufferToInner[BufferStruct];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelBuffer> FVoxelBuffer::MakeDeepCopy() const
{
	VOXEL_FUNCTION_COUNTER();

	const int32 Num = Num_Slow();

	const TSharedRef<FVoxelBuffer> Result = MakeEmpty(GetInnerType());
	Result->Allocate(Num);
	Result->CopyFrom(*this, 0, 0, Num);
	return Result;
}

FVoxelRuntimePinValue FVoxelBuffer::GetGenericConstant() const
{
	checkVoxelSlow(IsConstant_Slow());
	return GetGeneric(0);
}

void FVoxelBuffer::SetAllGeneric(const FVoxelRuntimePinValue& Value)
{
	VOXEL_FUNCTION_COUNTER_NUM(Num_Slow());

	const TSharedRef<FVoxelBuffer> Constant = MakeConstant(Value);
	CopyFrom(*Constant);
}

void FVoxelBuffer::CopyFrom(const FVoxelBuffer& Other)
{
	CopyFrom(Other, 0, 0, Num_Slow());
}

void FVoxelBuffer::IndirectCopyFrom(
	const FVoxelBuffer& Source,
	const TVoxelOptional<FVoxelInt32Buffer>& SourceToThis)
{
	if (SourceToThis)
	{
		IndirectCopyFrom(Source, SourceToThis->View());
	}
	else
	{
		CopyFrom(Source);
	}
}

bool FVoxelBuffer::IsConstant_Slow() const
{
	return Num_Slow() == 1;
}

bool FVoxelBuffer::IsValidIndex_Slow(const int32 Index) const
{
	const int32 Num = Num_Slow();

	return 0 <= Index && (Num == 1 || Index < Num);
}

void FVoxelBuffer::Serialize(FArchive& Ar, TSharedPtr<FVoxelBuffer>& Buffer)
{
	VOXEL_FUNCTION_COUNTER();

	bool bIsNull = !Buffer.IsValid();
	Ar << bIsNull;

	if (bIsNull)
	{
		Buffer.Reset();
		return;
	}

	UScriptStruct* BufferStruct = nullptr;
	if (Ar.IsSaving())
	{
		BufferStruct = Buffer->GetStruct();
	}

	FVoxelUtilities::SerializeStruct(Ar, BufferStruct);

	if (!ensureVoxelSlow(BufferStruct))
	{
		return;
	}

	if (Ar.IsLoading())
	{
		Buffer = MakeSharedStruct<FVoxelBuffer>(BufferStruct);
	}

	Buffer->SerializeData(Ar);
}