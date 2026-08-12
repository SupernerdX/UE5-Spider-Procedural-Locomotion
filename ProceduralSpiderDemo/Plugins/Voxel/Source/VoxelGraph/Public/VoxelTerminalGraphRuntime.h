// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSerializedGraph.h"
#include "VoxelTerminalGraphRuntime.generated.h"

class UVoxelGraph;
class UVoxelTerminalGraph;
class FVoxelGraphCompiler;
struct FOnVoxelGraphChanged;

namespace Voxel::Graph
{
	class FGraph;
}

class FVoxelCompiledTerminalGraph;

#if WITH_EDITOR
class IVoxelGraphEditorInterface
{
public:
	IVoxelGraphEditorInterface() = default;
	virtual ~IVoxelGraphEditorInterface() = default;

	virtual void CompileAll() = 0;
	virtual void ReconstructAllNodes(UVoxelTerminalGraph& TerminalGraph) = 0;
	virtual bool HasNode(const UVoxelTerminalGraph& TerminalGraph, const UScriptStruct* Struct) = 0;
	virtual bool HasFunctionInputDefault(const UVoxelTerminalGraph& TerminalGraph, const FGuid& Guid) = 0;
	virtual UEdGraph* CreateEdGraph(UVoxelTerminalGraph& TerminalGraph) = 0;
	// Should be free of side effect (outside of preview fixups & migrations), ie no messages
	virtual FVoxelSerializedGraph Translate(UVoxelTerminalGraph& TerminalGraph) = 0;
};

extern VOXELGRAPH_API IVoxelGraphEditorInterface* GVoxelGraphEditorInterface;
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Needed because struct reinstantiation doesn't call AddReferencedObjects on objects
USTRUCT()
struct FVoxelCompiledTerminalGraphRef
#if CPP
	: TVoxelOptional<TSharedPtr<FVoxelCompiledTerminalGraph>>
#endif
{
	GENERATED_BODY()

	//~ Begin TStructOpsTypeTraits Interface
	void AddStructReferencedObjects(FReferenceCollector& Collector);
	//~ End TStructOpsTypeTraits Interface
};

template<>
struct TStructOpsTypeTraits<FVoxelCompiledTerminalGraphRef> : TStructOpsTypeTraitsBase2<FVoxelCompiledTerminalGraphRef>
{
	enum
	{
		WithAddStructReferencedObjects = true
	};
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS(Within=VoxelTerminalGraph)
class VOXELGRAPH_API UVoxelTerminalGraphRuntime : public UObject
{
	GENERATED_BODY()

public:
	const UVoxelGraph& GetGraph() const;
	UVoxelTerminalGraph& GetTerminalGraph();
	const UVoxelTerminalGraph& GetTerminalGraph() const;

	const FVoxelSerializedGraph& GetSerializedGraph() const;

	void EnsureIsCompiled(
		bool bForce = false,
		const TFunction<void(FName, FVoxelGraphCompiler&)>& OnGetGraphAtPass = nullptr);
#if WITH_EDITOR
	void BindOnEdGraphChanged();
#endif

public:
	template<typename T>
	bool HasNode() const
	{
		return this->HasNode(StaticStructFast<T>());
	}
	bool HasNode(const UScriptStruct* Struct) const;
#if WITH_EDITOR
	bool HasFunctionInputDefault_EditorOnly(const FGuid& Guid) const;
#endif

public:
	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform) override;
#endif
	//~ End UObject Interface

public:
	FSimpleMulticastDelegate OnMessagesChanged;

	const TVoxelArray<TSharedRef<FVoxelMessage>>& GetRuntimeMessages() const
	{
		return RuntimeMessages;
	}
	const TVoxelArray<TSharedRef<FVoxelMessage>>& GetCompileMessages() const
	{
		return CompileMessages;
	}

	void AddMessage(const TSharedRef<FVoxelMessage>& Message);
	bool HasCompileMessages() const;

private:
	// Runtime messages that referenced this graph
	TVoxelArray<TSharedRef<FVoxelMessage>> RuntimeMessages;
	// Compile errors for the entire graph
	TVoxelArray<TSharedRef<FVoxelMessage>> CompileMessages;

	UPROPERTY(DuplicateTransient, NonTransactional)
	FVoxelSerializedGraph PrivateSerializedGraph;

#if WITH_EDITOR
	void Translate();
#endif

private:
#if WITH_EDITOR
	FSharedVoidPtr OnEdGraphChangedPtr;
	FSharedVoidPtr OnTranslatedPtr;
#endif

	UPROPERTY()
	FVoxelCompiledTerminalGraphRef CompiledGraph;

	TSharedPtr<FVoxelCompiledTerminalGraph> Compile(const TFunction<void(FName, FVoxelGraphCompiler&)>& OnGetGraphAtPass = nullptr);

	static TSharedPtr<FVoxelCompiledTerminalGraph> Compile(
		const FOnVoxelGraphChanged& OnTranslated,
		const FOnVoxelGraphChanged& OnForceRecompile,
		const UVoxelTerminalGraph& TerminalGraph,
		bool bEnableLogging,
		TVoxelArray<TSharedRef<FVoxelMessage>>& OutCompileMessages,
		const TFunction<void(FName, FVoxelGraphCompiler&)>& OnGetGraphAtPass = nullptr);

	friend class UVoxelGraph;
};