// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValue.h"
#include "VoxelTerminalGraph.generated.h"

class UVoxelGraph;
class UVoxelEdGraph;
class UVoxelTerminalGraphRuntime;

USTRUCT()
struct VOXELGRAPH_API FVoxelGraphMetadata
{
	GENERATED_BODY()

	UPROPERTY()
	FString DisplayName;

	UPROPERTY()
	FString Category;

	UPROPERTY()
	FString Description;
};

USTRUCT()
struct VOXELGRAPH_API FVoxelGraphProperty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	FName Name;

	UPROPERTY(EditAnywhere, Category = "Config")
	FVoxelPinType Type;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Config")
	FString Category;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (MultiLine = true))
	FString Description;
#endif
};

USTRUCT()
struct VOXELGRAPH_API FVoxelGraphFunctionInput : public FVoxelGraphProperty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Default Value")
	bool bNoDefault = false;

	UPROPERTY(EditAnywhere, Category = "Default Value", meta = (EditCondition = "!bNoDefault"))
	FVoxelPinValue DefaultPinValue;

	void Fixup();
};

USTRUCT()
struct VOXELGRAPH_API FVoxelGraphFunctionOutput : public FVoxelGraphProperty
{
	GENERATED_BODY()
};

USTRUCT()
struct VOXELGRAPH_API FVoxelGraphLocalVariable : public FVoxelGraphProperty
{
	GENERATED_BODY()
};

USTRUCT()
struct VOXELGRAPH_API FVoxelGraphPreviewConfig
{
	GENERATED_BODY()

	UPROPERTY()
	EVoxelAxis Axis = EVoxelAxis::Z;

	UPROPERTY()
	int32 Resolution = 512;

	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	UPROPERTY()
	double Zoom = 1.;

	float GetAxisLocation() const
	{
		switch (Axis)
		{
		default: return 0.f;
		case EVoxelAxis::X: return Position.X;
		case EVoxelAxis::Y: return Position.Y;
		case EVoxelAxis::Z: return Position.Z;
		}
	}
	void SetAxisLocation(const float Value)
	{
		switch (Axis)
		{
		default: ensure(false);
		case EVoxelAxis::X: Position.X = Value; break;
		case EVoxelAxis::Y: Position.Y = Value; break;
		case EVoxelAxis::Z: Position.Z = Value; break;
		}
	}
};

UCLASS(Within=VoxelGraph)
class VOXELGRAPH_API UVoxelTerminalGraph : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	// Only used if this is in a function library
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bExposeToLibrary = true;

	// This function will also appear in search results when the user searches for words that match the keyword list.
	UPROPERTY(EditAnywhere, Category = "Config")
	FString Keywords;

	// This function can only be called from the graph types specified in the whitelist. If the list is empty, it can be called anywhere.
	UPROPERTY(EditAnywhere, Category = "Config", meta = (GetOptions = "GetWhitelistGraphTypes"))
	TArray<FString> WhitelistedTypes;

	UPROPERTY()
	FVoxelGraphPreviewConfig PreviewConfig;
#endif

	// Will disable common nodes sub-tree merging
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	bool bDisableNodeMerging = false;

public:
	UVoxelGraph& GetGraph();
	const UVoxelGraph& GetGraph() const;

	FGuid GetGuid() const;
	void SetGuid_Hack(const FGuid& Guid);

#if WITH_EDITOR
	FString GetDisplayName() const;
	FVoxelGraphMetadata GetMetadata() const;
	void UpdateMetadata(TFunctionRef<void(FVoxelGraphMetadata&)> Lambda);
	bool CanBePlaced(const UVoxelGraph& Graph) const;

	UFUNCTION()
	TArray<FString> GetWhitelistGraphTypes() const;
#endif

#if WITH_EDITOR
	void SetEdGraph_Hack(UEdGraph* NewEdGraph);
	void SetDisplayName_Hack(const FString& Name);

	UEdGraph& GetEdGraph();
	const UEdGraph& GetEdGraph() const;

	template<typename T = UVoxelEdGraph>
	T& GetTypedEdGraph()
	{
		return *CastChecked<T>(&GetEdGraph());
	}
	template<typename T = UVoxelEdGraph>
	const T& GetTypedEdGraph() const
	{
		return *CastChecked<T>(&GetEdGraph());
	}
#endif

public:
	const FVoxelGraphFunctionInput* FindInput(const FGuid& Guid) const;
	const FVoxelGraphFunctionOutput* FindOutput(const FGuid& Guid) const;
	const FVoxelGraphFunctionOutput* FindOutputByName(const FName& Name, FGuid& OutGuid) const;
	const FVoxelGraphLocalVariable* FindLocalVariable(const FGuid& Guid) const;

	const FVoxelGraphFunctionInput& FindInputChecked(const FGuid& Guid) const;
	const FVoxelGraphFunctionOutput& FindOutputChecked(const FGuid& Guid) const;
	const FVoxelGraphLocalVariable& FindLocalVariableChecked(const FGuid& Guid) const;

	TVoxelSet<FGuid> GetFunctionInputs() const;
	TVoxelSet<FGuid> GetFunctionOutputs() const;
	TVoxelSet<FGuid> GetLocalVariables() const;

	bool IsInheritedInput(const FGuid& Guid) const;
	bool IsInheritedOutput(const FGuid& Guid) const;

public:
#if WITH_EDITOR
	void AddFunctionInput(const FGuid& Guid, const FVoxelGraphFunctionInput& Input);
	void AddFunctionOutput(const FGuid& Guid, const FVoxelGraphFunctionOutput& Output);
	void AddLocalVariable(const FGuid& Guid, const FVoxelGraphLocalVariable& LocalVariable);

	void RemoveFunctionInput(const FGuid& Guid);
	void RemoveFunctionOutput(const FGuid& Guid);
	void RemoveLocalVariable(const FGuid& Guid);

	void UpdateFunctionInput(const FGuid& Guid, TFunctionRef<void(FVoxelGraphFunctionInput&)> Update);
	void UpdateFunctionOutput(const FGuid& Guid, TFunctionRef<void(FVoxelGraphFunctionOutput&)> Update);
	void UpdateLocalVariable(const FGuid& Guid, TFunctionRef<void(FVoxelGraphLocalVariable&)> Update);

	void ReorderFunctionInputs(TConstVoxelArrayView<FGuid> NewGuids);
	void ReorderFunctionOutputs(TConstVoxelArrayView<FGuid> NewGuids);
	void ReorderLocalVariables(TConstVoxelArrayView<FGuid> NewGuids);
#endif

private:
	UPROPERTY()
	FGuid PrivateGuid;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FVoxelGraphMetadata PrivateMetadata;
#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UEdGraph> EdGraph;
#endif

	UPROPERTY()
	TMap<FGuid, FVoxelGraphFunctionInput> GuidToFunctionInput;

	UPROPERTY()
	TMap<FGuid, FVoxelGraphFunctionOutput> GuidToFunctionOutput;

	UPROPERTY()
	TMap<FGuid, FVoxelGraphLocalVariable> GuidToLocalVariable;

public:
	UVoxelTerminalGraph();

	void Fixup();
	bool IsFunction() const;
	bool IsMainTerminalGraph() const;
	bool IsEditorTerminalGraph() const;
	// Isn't an inherited terminal graph
	bool IsTopmostTerminalGraph() const;

	FORCEINLINE UVoxelTerminalGraphRuntime& GetRuntime() const
	{
		return *Runtime;
	}

public:
	// Includes self
	TVoxelInlineSet<UVoxelTerminalGraph*, 8> GetBaseTerminalGraphs();
	TVoxelInlineSet<const UVoxelTerminalGraph*, 8> GetBaseTerminalGraphs() const;

	// Includes self, slow
	TVoxelSet<UVoxelTerminalGraph*> GetChildTerminalGraphs_LoadedOnly();
	TVoxelSet<const UVoxelTerminalGraph*> GetChildTerminalGraphs_LoadedOnly() const;

public:
	FSimpleMulticastDelegate OnLoaded;

	//~ Begin UObject Interface
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

private:
	UPROPERTY()
	TObjectPtr<UVoxelTerminalGraphRuntime> Runtime;

	friend class UVoxelTerminalGraphRuntime;
};