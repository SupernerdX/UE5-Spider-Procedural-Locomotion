// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPin.h"
#include "VoxelNodeStats.h"
#include "VoxelPinTypeSet.h"
#include "VoxelGraphQuery.h"
#include "VoxelGraphNodeRef.h"
#include "VoxelNodeInterface.h"
#include "VoxelNodeSerializedData.h"
#include "VoxelDefaultNodeDefinition.h"
#include "VoxelNode.generated.h"

class UVoxelGraph;

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelNode
	: public FVoxelVirtualStruct
#if CPP
	, public IVoxelNodeInterface
#endif
{
	GENERATED_BODY()
	DECLARE_VIRTUAL_STRUCT_PARENT(FVoxelNode, GENERATED_VOXEL_NODE_BODY)

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	FGuid NodeGuid;

public:
	FVoxelNode() = default;
	FVoxelNode(const FVoxelNode& Other) = delete;
	FVoxelNode& operator=(const FVoxelNode& Other);

	VOXEL_COUNT_INSTANCES();

	//~ Begin TStructOpsTypeTraits Interface
	bool Serialize(FArchive& Ar);
	void AddStructReferencedObjects(FReferenceCollector& Collector);
	//~ End TStructOpsTypeTraits Interface

	//~ Begin IVoxelNodeInterface Interface
	FORCEINLINE virtual const FVoxelGraphMergedNodeRef& GetNodeRef() const final override
	{
		ensureVoxelSlow(!PrivateNodeRef.Node.IsExplicitlyNull());
		return PrivateNodeRef;
	}
	//~ End IVoxelNodeInterface Interface

public:
	struct FValue;

	template<typename>
	struct TValue;

	template<typename>
	struct TIsValue;

	struct FPinRef;
	struct FPinRef_Input;
	struct FPinRef_Output;
	struct FVariadicPinRef_Input;

	template<typename>
	struct TPinRef_Input;

	template<typename>
	struct TPinRef_Output;

	template<typename>
	struct TTemplatePinRef_Input;

	template<typename>
	struct TTemplatePinRef_Output;

	template<typename>
	struct TVariadicPinRef_Input;

	template<typename>
	struct TVariadicTemplatePinRef_Input;

public:
#if WITH_EDITOR
	virtual UStruct& GetMetadataContainer() const;

	virtual FString GetCategory() const;
	virtual FString GetDisplayName() const;
	virtual FString GetTooltip() const;
#endif

	virtual bool ShowPromotablePinsAsWildcards() const
	{
		return true;
	}
	virtual bool IsPureNode() const
	{
		return false;
	}
	virtual bool HasGuid() const
	{
		return false;
	}
	virtual bool CanBeQueried() const
	{
		return false;
	}
	virtual bool CanBeDuplicated() const
	{
		return true;
	}
	virtual bool CanBeDeleted() const
	{
		return true;
	}
	virtual bool CanPasteHere(
		const UVoxelGraph& Graph,
		const UVoxelTerminalGraph* TerminalGraph) const;
	virtual bool IsDesignedForGraph(const UVoxelGraph& Graph, FString* OutWarning = nullptr) const;

	struct FInitializer
	{
		FInitializer() = default;
		virtual ~FInitializer() = default;

		virtual void ClearPinRefs() = 0;
		virtual void InitializePinRef(FPinRef_Input& PinRef) = 0;
		virtual void InitializePinRef(FPinRef_Output& PinRef) = 0;
	};
	virtual void Initialize(FInitializer& Initializer) {}
	virtual void PostInitialize(
		const TVoxelMap<FName, FPinRef_Input*>& NameToInputPinRefs,
		const TVoxelMap<FName, FPinRef_Output*>& NameToOutputPinRefs) {}

	virtual void Compute(FVoxelGraphQuery Query) const VOXEL_PURE_VIRTUAL();

	virtual void ComputeNoCachePin(
		FVoxelGraphQuery Query,
		int32 PinIndex) const VOXEL_PURE_VIRTUAL();

	virtual uint32 GetNodeHash() const;
	virtual bool IsNodeIdentical(const FVoxelNode& Other) const;

public:
	virtual void PreSerialize() override;
	virtual void PostSerialize();
	void PostSerialize(const FArchive& Ar);

 #if WITH_EDITOR
	enum class EPostEditChange : uint8
	{
		None,
		Reconstruct
	};
 	virtual EPostEditChange PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
 	{
 		return EPostEditChange::None;
 	}
 #endif

private:
	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion,
		AddIsValid
	);

	UPROPERTY()
	int32 Version = FVersion::FirstVersion;

public:
	FSimpleMulticastDelegate OnNodeRecreateRequested;

#if WITH_EDITOR
	using FDefinition = FVoxelDefaultNodeDefinition;
	virtual TSharedRef<FVoxelDefaultNodeDefinition> GetNodeDefinition();
#endif

#if WITH_EDITOR
	// Pin will always be a promotable pin
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const;
	virtual FString GetPinPromotionWarning(const FVoxelPin& Pin, const FVoxelPinType& NewType) const { return {}; }
	virtual void PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType);
	virtual void PostReconstructNode();
#endif
	virtual void PromotePin_Runtime(FVoxelPin& Pin, const FVoxelPinType& NewType);

	bool AreTemplatePinsBuffers() const;
	TOptional<bool> AreTemplatePinsBuffersImpl() const;

#if WITH_EDITOR
	void RequestRenamePin(FName PinName);
	bool IsPinHidden(const FVoxelPin& Pin) const;
	FString GetPinDefaultValue(const FVoxelPin& Pin) const;
	void UpdatePropertyBoundDefaultValue(const FVoxelPin& Pin, const FVoxelPinValue& NewValue);
#endif

public:
	template<typename T>
	struct TPinIterator
	{
		TVoxelMap<FName, TSharedPtr<FVoxelPin>>::FConstIterator Iterator;

		TPinIterator(TVoxelMap<FName, TSharedPtr<FVoxelPin>>::FConstIterator&& Iterator)
			: Iterator(Iterator)
		{
		}

		TPinIterator& operator++()
		{
			++Iterator;
			return *this;
		}
		explicit operator bool() const
		{
			return bool(Iterator);
		}
		T& operator*() const
		{
			return *Iterator.Value();
		}
		bool operator!=(decltype(nullptr)) const
		{
			return bool(Iterator);
		}
	};
	template<typename T>
	struct TPinView
	{
		const TVoxelMap<FName, TSharedPtr<FVoxelPin>>& Pins;

		TPinView() = default;
		TPinView(const TVoxelMap<FName, TSharedPtr<FVoxelPin>>& Pins)
			: Pins(Pins)
		{
		}

		int32 Num() const { return Pins.Num(); }
		TPinIterator<T> begin() const { return TPinIterator<T>{ Pins.begin() }; }
		decltype(nullptr) end() const { return nullptr; }
	};

	TPinView<FVoxelPin> GetPins()
	{
		FlushDeferredPins();
		return TPinView<FVoxelPin>(PrivateNameToPin);
	}
	TPinView<const FVoxelPin> GetPins() const
	{
		FlushDeferredPins();
		return TPinView<const FVoxelPin>(PrivateNameToPin);
	}

	const TVoxelMap<FName, TSharedPtr<FVoxelPin>>& GetNameToPin();
	const TVoxelMap<FName, TSharedPtr<const FVoxelPin>>& GetNameToPin() const;

	TSharedPtr<FVoxelPin> FindPin(FName Name);
	TSharedPtr<const FVoxelPin> FindPin(FName Name) const;

	FVoxelPin& FindPinChecked(FName Name);
	const FVoxelPin& FindPinChecked(FName Name) const;

	FVoxelPin& GetPin(const FPinRef& Pin);
	const FVoxelPin& GetPin(const FPinRef& Pin) const;

	FVoxelPin& GetUniqueInputPin();
	FVoxelPin& GetUniqueOutputPin();

	const FVoxelPin& GetUniqueInputPin() const
	{
		return ConstCast(this)->GetUniqueInputPin();
	}
	const FVoxelPin& GetUniqueOutputPin() const
	{
		return ConstCast(this)->GetUniqueOutputPin();
	}

public:
	FName Variadic_AddPin(FName VariadicPinName, FName PinName = {});
	FName Variadic_InsertPin(FName VariadicPinName, int32 Position);
	FName Variadic_AddPin(const FPinRef&, FName = {}) = delete;

	TVoxelArray<FName> GetVariadicPinPinNames(const FVariadicPinRef_Input& VariadicPin) const;

private:
	void FixupVariadicPinNames(FName VariadicPinName);

protected:
	FName CreatePin(
		const FVoxelPinType& Type,
		bool bIsInput,
		FName Name,
		const FVoxelPinMetadata& Metadata = {},
		EVoxelPinFlags Flags = EVoxelPinFlags::None,
		int32 MinVariadicNum = 0);

	void RemovePin(FName Name);

protected:
	FPinRef_Input CreateInputPin(
		const FVoxelPinType& Type,
		FName Name,
		const FVoxelPinMetadata& Metadata = {},
		EVoxelPinFlags Flags = EVoxelPinFlags::None);

	FPinRef_Output CreateOutputPin(
		const FVoxelPinType& Type,
		FName Name,
		const FVoxelPinMetadata& Metadata = {},
		EVoxelPinFlags Flags = EVoxelPinFlags::None);

protected:
	template<typename Type>
	TPinRef_Input<Type> CreateInputPin(
		const FName Name,
		const FVoxelPinMetadata& Metadata,
		const EVoxelPinFlags Flags = EVoxelPinFlags::None)
	{
		return TPinRef_Input<Type>(this->CreateInputPin(
			FVoxelPinType::Make<Type>(),
			Name,
			Metadata,
			Flags));
	}
	template<typename Type>
	TPinRef_Output<Type> CreateOutputPin(
		const FName Name,
		const FVoxelPinMetadata& Metadata,
		const EVoxelPinFlags Flags = EVoxelPinFlags::None)
	{
		return TPinRef_Output<Type>(this->CreateOutputPin(
			FVoxelPinType::Make<Type>(),
			Name,
			Metadata,
			Flags));
	}

protected:
	template<typename Type>
	TTemplatePinRef_Input<Type> CreateTemplateInputPin(
		const FName Name,
		const FVoxelPinMetadata& Metadata,
		const EVoxelPinFlags Flags = EVoxelPinFlags::None)
	{
		return TTemplatePinRef_Input<Type>(this->CreateInputPin(
			FVoxelPinType::Make<Type>(),
			Name,
			Metadata,
			Flags | EVoxelPinFlags::TemplatePin));
	}
	template<typename Type>
	TTemplatePinRef_Output<Type> CreateTemplateOutputPin(
		const FName Name,
		const FVoxelPinMetadata& Metadata,
		const EVoxelPinFlags Flags = EVoxelPinFlags::None)
	{
		return TTemplatePinRef_Output<Type>(this->CreateOutputPin(
			FVoxelPinType::Make<Type>(),
			Name,
			Metadata,
			Flags | EVoxelPinFlags::TemplatePin));
	}

protected:
	FVariadicPinRef_Input CreateVariadicInputPin(
		const FVoxelPinType& Type,
		FName Name,
		const FVoxelPinMetadata& Metadata,
		int32 MinNum,
		EVoxelPinFlags Flags = EVoxelPinFlags::None);

	template<typename Type>
	TVariadicPinRef_Input<Type> CreateVariadicInputPin(
		const FName Name,
		const FVoxelPinMetadata& Metadata,
		const int32 MinNum,
		const EVoxelPinFlags Flags = EVoxelPinFlags::None)
	{
		return TVariadicPinRef_Input<Type>(this->CreateVariadicInputPin(FVoxelPinType::Make<Type>(), Name, Metadata, MinNum, Flags));
	}
	template<typename Type>
	TVariadicTemplatePinRef_Input<Type> CreateVariadicTemplateInputPin(
		const FName Name,
		const FVoxelPinMetadata& Metadata,
		const int32 MinNum,
		const EVoxelPinFlags Flags = EVoxelPinFlags::None)
	{
		return TVariadicTemplatePinRef_Input<Type>(this->CreateVariadicInputPin(
			FVoxelPinType::Make<Type>(),
			Name,
			Metadata,
			MinNum,
			Flags | EVoxelPinFlags::TemplatePin));
	}

protected:
	struct FDeferredPin
	{
		FName VariadicPinName;
		int32 MinVariadicNum = 0;

		FName Name;
		bool bIsInput = false;
		float SortOrder = 0.f;
		EVoxelPinFlags Flags = {};
		FVoxelPinType BaseType;
		FVoxelPinType ChildType;
		FVoxelPinMetadata Metadata;

		bool IsVariadicRoot() const
		{
			return EnumHasAllFlags(Flags, EVoxelPinFlags::VariadicPin);
		}
		bool IsVariadicChild() const
		{
			return !VariadicPinName.IsNone();
		}
	};
	struct FVariadicPin
	{
		const FDeferredPin PinTemplate;
		TVoxelArray<FName> Pins;

		explicit FVariadicPin(const FDeferredPin& PinTemplate)
			: PinTemplate(PinTemplate)
		{
		}
	};

	FORCEINLINE void RegisterPinRef(const FPinRef_Input& PinRef)
	{
		const int64 Offset = reinterpret_cast<const uint8*>(&PinRef) - reinterpret_cast<const uint8*>(this);
		checkVoxelSlow(0 <= Offset && Offset < 4096);
		PrivateInputPinRefOffsets.Add(Offset);
	}
	FORCEINLINE void RegisterPinRef(const FPinRef_Output& PinRef)
	{
		const int64 Offset = reinterpret_cast<const uint8*>(&PinRef) - reinterpret_cast<const uint8*>(this);
		checkVoxelSlow(0 <= Offset && Offset < 4096);
		PrivateOutputPinRefOffsets.Add(Offset);
	}
	FORCEINLINE void RegisterVariadicPinRef(const FVariadicPinRef_Input& PinRef)
	{
		const int64 Offset = reinterpret_cast<const uint8*>(&PinRef) - reinterpret_cast<const uint8*>(this);
		checkVoxelSlow(0 <= Offset && Offset < 4096);
		PrivateVariadicPinRefOffsets.Add(Offset);
	}

private:
	bool bEditorDataRemoved = false;

	int32 SortOrderCounter = 1;

	bool bIsDeferringPins = true;
	TVoxelArray<FDeferredPin> DeferredPins;

	TVoxelInlineArray<int32, 8> PrivateInputPinRefOffsets;
	TVoxelInlineArray<int32, 8> PrivateOutputPinRefOffsets;
	TVoxelInlineArray<int32, 8> PrivateVariadicPinRefOffsets;

	TVoxelMap<FName, FDeferredPin> PrivateNameToPinBackup;
	TVoxelArray<FName> PrivatePinsOrder;
	int32 DisplayLastPins = 0;
	TVoxelMap<FName, TSharedPtr<FVoxelPin>> PrivateNameToPin;
	TVoxelMap<FName, TSharedPtr<FVariadicPin>> PrivateNameToVariadicPin;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Voxel", Transient)
	TArray<FVoxelNodeExposedPinValue> ExposedPinValues;

	TSet<FName> ExposedPins;
	FSimpleMulticastDelegate OnExposedPinsUpdated;

	FName PinToRename;
	FSharedVoidPtr OnRequestRenameDelayPtr;
#endif

	FORCEINLINE void FlushDeferredPins() const
	{
		ensure(!bEditorDataRemoved);

		if (bIsDeferringPins)
		{
			ConstCast(this)->FlushDeferredPinsImpl();
		}
	}
	void FlushDeferredPinsImpl();
	void RegisterPin(FDeferredPin Pin, bool bApplyMinNum = true);
	void SortPins();
	void SortVariadicPinNames(FName VariadicPinName);

private:
	UPROPERTY()
	FVoxelNodeSerializedData SerializedDataProperty;

	FVoxelNodeSerializedData GetSerializedData() const;
	void LoadSerializedData(const FVoxelNodeSerializedData& SerializedData);

public:
	void InitializeNodeRuntime(
		int32 NodeIndex,
		const FVoxelGraphMergedNodeRef& NodeRef,
		TVoxelMap<FName, FPinRef_Input*>& OutNameToInputPinRefs,
		TVoxelMap<FName, FPinRef_Output*>& OutNameToOutputPinRefs);

	bool HasEditorData() const
	{
		return !bEditorDataRemoved;
	}

	virtual void ComputeIfNeeded(
		FVoxelGraphQuery Query,
		int32 PinIndex) const;

protected:
	void EnsureOutputValuesAreSet(FVoxelGraphQuery Query) const;

private:
	int32 PrivateNodeIndex = -1;
	FVoxelGraphMergedNodeRef PrivateNodeRef;
	TVoxelArray<FPinRef_Output*> PrivateOutputPinRefs;
	TOptional<bool> CachedAreTemplatePinsBuffers;

private:
	friend FVoxelNode_CallFunction;
	friend class SVoxelGraphNode;
	friend class FVoxelDefaultNodeDefinition;
	friend class FVoxelGraphNode_Struct_Customization;
	friend class FVoxelGraphNodeVariadicPinCustomization;
};

template<typename T>
requires std::derived_from<T, FVoxelNode>
struct TStructOpsTypeTraits<T> : TStructOpsTypeTraitsBase2<T>
{
	enum
	{
		WithSerializer = true,
		WithPostSerialize = true,
		WithAddStructReferencedObjects = true,
	};
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define GENERATED_VOXEL_NODE_DEFINITION_BODY(NodeType) \
	NodeType& Node; \
	explicit FDefinition(NodeType& Node) \
		: Super::FDefinition(Node) \
		, Node(Node) \
	{}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
#define IMPL_GENERATED_VOXEL_NODE_EDITOR_BODY() \
	virtual TSharedRef<FVoxelDefaultNodeDefinition> GetNodeDefinition() override { return MakeShared<FDefinition>(*this); }
#else
#define IMPL_GENERATED_VOXEL_NODE_EDITOR_BODY()
#endif

#define GENERATED_VOXEL_NODE_BODY() \
	GENERATED_VIRTUAL_STRUCT_BODY(FVoxelNode) \
	using FVoxelNode::PostSerialize; \
	IMPL_GENERATED_VOXEL_NODE_EDITOR_BODY()

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define VOXEL_PIN_NAME(NodeType, PinName) \
	([]() -> FName \
	 { \
		static const FName StaticName = NodeType().PinName.GetName(); \
		return StaticName; \
	 }())

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if CPP
#include "VoxelNodeValue.h"
#include "VoxelNodePinRef.h"
#include "VoxelNodePinDecl.h"
#include "VoxelNodeHelpers.h"
#endif