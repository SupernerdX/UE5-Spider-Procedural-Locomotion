// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBuffer.h"
#include "VoxelGraphQuery.h"
#include "VoxelBufferAccessor.h"
#include "VoxelRuntimePinValue.h"
#include "VoxelFunctionLibrary.generated.h"

struct FVoxelNode_UFunction;

struct VOXELGRAPH_API FVoxelFunctionLibraryRegistry
{
public:
	using FGetClassFuncPtr = UClass* (*)();
	using FNativeFuncPtr = void (*)(UVoxelFunctionLibrary& Context, TConstVoxelArrayView<FVoxelRuntimePinValue*> Values);

public:
	static void RegisterFunction(
		FGetClassFuncPtr Class,
		const TCHAR* FunctionName,
		FNativeFuncPtr NativeFunction);
	static FNativeFuncPtr FindFunction(const UFunction& Function);

private:
	static void RegisterDelayedFunctions();

	static TVoxelMap<const UFunction*, FNativeFuncPtr> FunctionToNativeCall;

	struct FDelayedFunction
	{
		FGetClassFuncPtr GetClassPtr = nullptr;
		const TCHAR* Name = nullptr;
		FNativeFuncPtr NativeFunction = nullptr;

		FDelayedFunction* Next = nullptr;
		FDelayedFunction(
			FGetClassFuncPtr GetClassPtr,
			const TCHAR* Name,
			FNativeFuncPtr NativeFunction);
	};
	static FDelayedFunction* GFirstDelayedFunction;
};

struct VOXELGRAPH_API FVoxelFunctionRegistration
{
	FVoxelFunctionRegistration(
		const FVoxelFunctionLibraryRegistry::FGetClassFuncPtr Class,
		const TCHAR* FunctionName,
		const FVoxelFunctionLibraryRegistry::FNativeFuncPtr NativeFunction)
	{
		FVoxelFunctionLibraryRegistry::RegisterFunction(
			Class,
			FunctionName,
			NativeFunction);
	}
};

UCLASS()
class VOXELGRAPH_API UVoxelFunctionLibrary
	: public UObject
	, public FVoxelBufferInitializers
{
	GENERATED_BODY()

public:
	FVoxelGraphQuery Query;

	void RaiseBufferError() const;
	TSharedRef<FVoxelMessageToken> CreateMessageToken() const;

private:
	const FVoxelNode_UFunction* PrivateNode = nullptr;

public:
	static FVoxelPinType MakeType(const FProperty& Property);

	struct FCachedFunction
	{
		const UFunction& Function;
		const FVoxelFunctionLibraryRegistry::FNativeFuncPtr NativeCall;
		explicit FCachedFunction(const UFunction& Function);
	};
	static void Call(
		const FVoxelNode_UFunction& Node,
		const FCachedFunction& CachedFunction,
		FVoxelGraphQuery InQuery,
		TConstVoxelArrayView<FVoxelRuntimePinValue*> Values);
};