// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelFunctionLibrary.h"
#include "VoxelBuffer.h"
#include "VoxelGenericStructBuffer.h"
#include "VoxelNodeStats.h"
#include "Nodes/VoxelNode_UFunction.h"

TVoxelMap<const UFunction*, FVoxelFunctionLibraryRegistry::FNativeFuncPtr> FVoxelFunctionLibraryRegistry::FunctionToNativeCall;
FVoxelFunctionLibraryRegistry::FDelayedFunction* FVoxelFunctionLibraryRegistry::GFirstDelayedFunction;

void FVoxelFunctionLibraryRegistry::RegisterFunction(
	const FGetClassFuncPtr Class,
	const TCHAR* FunctionName,
	const FNativeFuncPtr NativeFunction)
{
	GFirstDelayedFunction = new FDelayedFunction(Class, FunctionName, NativeFunction);
}

FVoxelFunctionLibraryRegistry::FNativeFuncPtr FVoxelFunctionLibraryRegistry::FindFunction(const UFunction& Function)
{
	if (const FNativeFuncPtr NativeFunction = FunctionToNativeCall.FindRef(&Function))
	{
		return NativeFunction;
	}

	if (GFirstDelayedFunction)
	{
		RegisterDelayedFunctions();
		return FunctionToNativeCall.FindRef(&Function);
	}

	return nullptr;
}

void FVoxelFunctionLibraryRegistry::RegisterDelayedFunctions()
{
	const FDelayedFunction* ThisFunction = GFirstDelayedFunction;
	while (ThisFunction)
	{
		ON_SCOPE_EXIT
		{
			const FDelayedFunction* NextFunction = ThisFunction->Next;
			delete ThisFunction;
			ThisFunction = NextFunction;
		};

		const UClass* Class = ThisFunction->GetClassPtr();
		if (!ensure(Class))
		{
			continue;
		}

		const UFunction* Function = Class->FindFunctionByName(ThisFunction->Name);
		if (!ensure(Function))
		{
			continue;
		}

		FunctionToNativeCall.Add_EnsureNew(Function, ThisFunction->NativeFunction);
	}
}

FVoxelFunctionLibraryRegistry::FDelayedFunction::FDelayedFunction(
	const FGetClassFuncPtr GetClassPtr,
	const TCHAR* Name,
	const FNativeFuncPtr NativeFunction)
	: GetClassPtr(GetClassPtr)
	, Name(Name)
	, NativeFunction(NativeFunction)
	, Next(GFirstDelayedFunction)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelFunctionLibrary::RaiseBufferError() const
{
	PrivateNode->RaiseBufferError();
}

TSharedRef<FVoxelMessageToken> UVoxelFunctionLibrary::CreateMessageToken() const
{
	return PrivateNode->GetNodeRef().CreateMessageToken();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelPinType UVoxelFunctionLibrary::MakeType(const FProperty& Property)
{
	if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		if (StructProperty->Struct == StaticStructFast<FVoxelRuntimePinValue>())
		{
			return FVoxelPinType::MakeWildcard();
		}
		if (StructProperty->Struct->IsChildOf(StaticStructFast<FVoxelBuffer>()))
		{
			return FVoxelBuffer::FindInnerType(StructProperty->Struct).GetBufferType();
		}
	}

	return FVoxelPinType(Property);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelFunctionLibrary::FCachedFunction::FCachedFunction(const UFunction& Function)
	: Function(Function)
	, NativeCall(FVoxelFunctionLibraryRegistry::FindFunction(Function))
{
}

void UVoxelFunctionLibrary::Call(
	const FVoxelNode_UFunction& Node,
	const FCachedFunction& CachedFunction,
	const FVoxelGraphQuery InQuery,
	const TConstVoxelArrayView<FVoxelRuntimePinValue*> Values)
{
	FVoxelNodeStatScope StatScope(Node, 0);

	int32 Num = 1;
	if (StatScope.IsEnabled() ||
		AreVoxelStatsEnabled())
	{
		for (const FVoxelRuntimePinValue* Value : Values)
		{
			if (!Value ||
				!Value->IsValid())
			{
				continue;
			}

			if (Value->CanBeCastedTo<FVoxelBuffer>())
			{
				Num = FMath::Max(Num, Value->Get<FVoxelBuffer>().Num_Slow());
			}
		}

		StatScope.SetCount(Num);
	}

	{
		VOXEL_SCOPE_COUNTER_FORMAT("%s Num=%d", *CachedFunction.Function.GetName(), Num);
		VOXEL_SCOPE_COUNTER_FNAME(CachedFunction.Function.GetFName());

		TVoxelTypeCompatibleBytes<UVoxelFunctionLibrary> FunctionLibraryBytes;
#if VOXEL_DEBUG
		FMemory::Memzero(FunctionLibraryBytes);
#endif

		UVoxelFunctionLibrary& FunctionLibrary = FunctionLibraryBytes.GetValue();
		FunctionLibrary.Query = InQuery;
		FunctionLibrary.PrivateNode = &Node;

		if (!ensure(CachedFunction.NativeCall))
		{
			return;
		}

		CachedFunction.NativeCall(FunctionLibrary, Values);
	}

	if (!StatScope.IsEnabled() ||
		!CachedFunction.Function.GetReturnProperty())
	{
		return;
	}

	if (!ensure(Values.Num() > 0))
	{
		return;
	}

	const FVoxelRuntimePinValue* ReturnValue = Values.Last();
	if (!ReturnValue->IsBuffer())
	{
		return;
	}

	StatScope.SetCount(FMath::Max(Num, ReturnValue->Get<FVoxelBuffer>().Num_Slow()));
}