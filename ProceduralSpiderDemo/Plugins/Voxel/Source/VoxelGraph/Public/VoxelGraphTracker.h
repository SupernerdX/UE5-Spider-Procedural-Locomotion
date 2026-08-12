// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class UVoxelGraph;
class UVoxelTerminalGraph;

struct VOXELGRAPH_API FOnVoxelGraphChanged
{
public:
	FORCEINLINE static FOnVoxelGraphChanged Null()
	{
		return FOnVoxelGraphChanged();
	}
#if WITH_EDITOR
	template<typename ThisTypePtr>
	static FOnVoxelGraphChanged Make(
		const FSharedVoidPtr& OnChangedPtr,
		const ThisTypePtr& This,
		TFunction<void()>&& Lambda)
	{
		ensure(OnChangedPtr.IsValid());

		FOnVoxelGraphChanged OnChanged;
		if constexpr (
			std::derived_from<std::remove_pointer_t<ThisTypePtr>, UObject> ||
			std::derived_from<std::remove_pointer_t<ThisTypePtr>, IInterface>)
		{
			OnChanged.Delegate = MakeShared<FSimpleDelegate>(MakeWeakPtrDelegate(OnChangedPtr, MakeWeakObjectPtrLambda(This, MoveTemp(Lambda))));
		}
		else
		{
			OnChanged.Delegate = MakeShared<FSimpleDelegate>(MakeWeakPtrDelegate(OnChangedPtr, MakeWeakPtrLambda(This, MoveTemp(Lambda))));
		}
		return OnChanged;
	}
	template<typename ThisTypePtr>
	static FOnVoxelGraphChanged Make(
		const ThisTypePtr& This,
		TFunction<void()>&& Lambda)
	{
		FOnVoxelGraphChanged OnChanged;
		if constexpr (
			std::derived_from<std::remove_pointer_t<ThisTypePtr>, UObject> ||
			std::derived_from<std::remove_pointer_t<ThisTypePtr>, IInterface>)
		{
			OnChanged.Delegate = MakeShared<FSimpleDelegate>(FSimpleDelegate::CreateWeakLambda(This, MoveTemp(Lambda)));
		}
		else
		{
			OnChanged.Delegate = MakeShared<FSimpleDelegate>(MakeWeakPtrDelegate(This, MoveTemp(Lambda)));
		}
		return OnChanged;
	}
#endif

private:
	FOnVoxelGraphChanged() = default;

#if WITH_EDITOR
	TSharedPtr<const FSimpleDelegate> Delegate;

	friend class FVoxelGraphTracker;
#endif
};

#if WITH_EDITOR
class VOXELGRAPH_API FVoxelGraphTracker : public FVoxelSingleton
{
public:
	struct VOXELGRAPH_API FDelegateGroup
	{
	public:
		void Add(const FOnVoxelGraphChanged& OnChanged);

	private:
		TVoxelArray<TSharedPtr<const FSimpleDelegate>> Delegates;

		friend FVoxelGraphTracker;
	};

public:
	template<typename T>
	static const T& MakeAny()
	{
		return *reinterpret_cast<const T*>(-1);
	}

private:
	struct VOXELGRAPH_API FImpl
	{
	protected:
		void Broadcast(const UObject& Object);
		FDelegateGroup& Get(const UObject& Object);

	private:
		FDelegateGroup AnyDelegateGroup;
		TVoxelMap<FVoxelObjectPtr, FDelegateGroup> ObjectToDelegateGroup;
	};
	template<typename T>
	struct TImpl : FImpl
	{
	public:
		FORCEINLINE void Broadcast(const T& Object)
		{
			return FImpl::Broadcast(reinterpret_cast<const UObject&>(Object));
		}
		FORCEINLINE FDelegateGroup& Get(const T& Object)
		{
			return FImpl::Get(reinterpret_cast<const UObject&>(Object));
		}
	};

public:
	void Mute();
	void Unmute();
	void Flush();

private:
	int32 NumMuteScopes = 0;

public:
	FDelegateGroup& OnEdGraphChanged(const UEdGraph& EdGraph)
	{
		return OnEdGraphChangedImpl.Get(EdGraph);
	}
	FDelegateGroup& OnEdGraphNodeChanged(const UEdGraphNode& Node)
	{
		return OnEdGraphNodeChangedImpl.Get(Node);
	}

private:
	TImpl<UEdGraph> OnEdGraphChangedImpl;
	TImpl<UEdGraphNode> OnEdGraphNodeChangedImpl;

public:
	// Terminal graph added/removed
	FDelegateGroup& OnTerminalGraphChanged(const UVoxelGraph& Graph)
	{
		return OnTerminalGraphChangedImpl.Get(Graph);
	}
	FDelegateGroup& OnParameterChanged(const UVoxelGraph& Graph)
	{
		return OnParameterChangedImpl.Get(Graph);
	}
	FDelegateGroup& OnParameterValueChanged(const UVoxelGraph& Graph)
	{
		return OnParameterValueChangedImpl.Get(Graph);
	}

private:
	TImpl<UVoxelGraph> OnTerminalGraphChangedImpl;
	TImpl<UVoxelGraph> OnParameterChangedImpl;
	TImpl<UVoxelGraph> OnParameterValueChangedImpl;

public:
	FDelegateGroup& OnTerminalGraphTranslated(const UVoxelTerminalGraph& TerminalGraph)
	{
		return OnTerminalGraphTranslatedImpl.Get(TerminalGraph);
	}
	FDelegateGroup& OnTerminalGraphCompiled(const UVoxelTerminalGraph& TerminalGraph)
	{
		return OnTerminalGraphCompiledImpl.Get(TerminalGraph);
	}
	FDelegateGroup& OnBaseTerminalGraphsChanged(const UVoxelTerminalGraph& TerminalGraph)
	{
		return OnBaseTerminalGraphsChangedImpl.Get(TerminalGraph);
	}
	FDelegateGroup& OnTerminalGraphMetaDataChanged(const UVoxelTerminalGraph& TerminalGraph)
	{
		return OnTerminalGraphMetaDataChangedImpl.Get(TerminalGraph);
	}
	FDelegateGroup& OnInputChanged(const UVoxelTerminalGraph& TerminalGraph)
	{
		return OnInputChangedImpl.Get(TerminalGraph);
	}
	FDelegateGroup& OnOutputChanged(const UVoxelTerminalGraph& TerminalGraph)
	{
		return OnOutputChangedImpl.Get(TerminalGraph);
	}
	FDelegateGroup& OnLocalVariableChanged(const UVoxelTerminalGraph& TerminalGraph)
	{
		return OnLocalVariableChangedImpl.Get(TerminalGraph);
	}

private:
	TImpl<UVoxelTerminalGraph> OnTerminalGraphTranslatedImpl;
	TImpl<UVoxelTerminalGraph> OnTerminalGraphCompiledImpl;
	TImpl<UVoxelTerminalGraph> OnBaseTerminalGraphsChangedImpl;
	TImpl<UVoxelTerminalGraph> OnTerminalGraphMetaDataChangedImpl;
	TImpl<UVoxelTerminalGraph> OnInputChangedImpl;
	TImpl<UVoxelTerminalGraph> OnOutputChangedImpl;
	TImpl<UVoxelTerminalGraph> OnLocalVariableChangedImpl;

public:
	void NotifyEdGraphChanged(const UEdGraph& EdGraph);
	void NotifyEdGraphNodeChanged(const UEdGraphNode& EdGraphNode);

	void NotifyTerminalGraphTranslated(const UVoxelTerminalGraph& TerminalGraph);
	void NotifyTerminalGraphCompiled(const UVoxelTerminalGraph& TerminalGraph);
	void NotifyTerminalGraphMetaDataChanged(const UVoxelTerminalGraph& TerminalGraph);

	void NotifyBaseGraphChanged(UVoxelGraph& Graph);
	void NotifyTerminalGraphChanged(UVoxelGraph& Graph);
	void NotifyParameterChanged(UVoxelGraph& Graph);
	void NotifyParameterValueChanged(UVoxelGraph& Graph);

	void NotifyInputChanged(UVoxelTerminalGraph& TerminalGraph);
	void NotifyOutputChanged(UVoxelTerminalGraph& TerminalGraph);
	void NotifyLocalVariableChanged(UVoxelTerminalGraph& TerminalGraph);

public:
	//~ Begin FVoxelSingleton Interface
	virtual void Tick() override;
	//~ End FVoxelSingleton Interface

private:
	TVoxelSet<TSharedPtr<const FSimpleDelegate>> QueuedDelegates;
};
extern VOXELGRAPH_API FVoxelGraphTracker* GVoxelGraphTracker;
#endif