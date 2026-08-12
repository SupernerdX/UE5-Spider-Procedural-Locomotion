// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

namespace Voxel::Network
{

class FPeer;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if VOXEL_DEBUG
struct INode
{
	INode() = default;
	virtual ~INode() = default;

	VOXEL_COUNT_INSTANCES();
};
#endif

template<typename T>
struct TNode
#if VOXEL_DEBUG
	: private INode
#endif
{
public:
	TNode() = default;

	// Remove element from list and transfer ownership
	FORCEINLINE TUniquePtr<T> List_Remove()
	{
		Prev->Next = Next;
		Next->Prev = Prev;

		Next = nullptr;
		Prev = nullptr;

		return TUniquePtr<T>(static_cast<T*>(this));
	}

	// Insert element before this
	FORCEINLINE T* List_InsertBefore(TUniquePtr<T> Element)
	{
		checkVoxelSlow(Element);

		Element->Prev = Prev;
		Element->Next = this;

		Prev->Next = Element.Get();
		Prev = Element.Get();

		return Element.Release();
	}

	// Insert element after this
	FORCEINLINE T* List_InsertAfter(TUniquePtr<T> Element)
	{
		checkVoxelSlow(Element);

		Element->Prev = this;
		Element->Next = Next;

		Next->Prev = Element.Get();
		Next = Element.Get();

		return Element.Release();
	}

	// Move range [First, Last] to before this
	FORCEINLINE void List_MoveBefore(T* First, T* Last)
	{
		// Remove range from current list
		First->Prev->Next = Last->Next;
		Last->Next->Prev = First->Prev;

		// Insert range before Position
		First->Prev = Prev;
		Last->Next = this;

		Prev->Next = First;
		Prev = Last;
	}

	FORCEINLINE TNode<T>* List_Next() const
	{
		return Next;
	}
	FORCEINLINE TNode<T>* List_Prev() const
	{
		return Prev;
	}

private:
	TNode<T>* Next = nullptr;
	TNode<T>* Prev = nullptr;

	template<typename>
	friend class TList;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
class TList
{
public:
	FORCEINLINE TList()
	{
		Sentinel.Next = &Sentinel;
		Sentinel.Prev = &Sentinel;
	}
	FORCEINLINE ~TList()
	{
		Clear();
	}
	UE_NONCOPYABLE(TList);

	FORCEINLINE void Clear()
	{
		while (!IsEmpty())
		{
			Sentinel.Next->List_Remove();
		}
	}

	FORCEINLINE bool IsEmpty() const
	{
		checkVoxelSlow((Sentinel.Next == &Sentinel) == (Sentinel.Prev == &Sentinel));
		return Sentinel.Next == &Sentinel;
	}

	FORCEINLINE T& Front()
	{
		checkVoxelSlow(!IsEmpty());
		return static_cast<T&>(*Sentinel.Next);
	}
	FORCEINLINE T& Back()
	{
		checkVoxelSlow(!IsEmpty());
		return static_cast<T&>(*Sentinel.Prev);
	}

	FORCEINLINE TUniquePtr<T> PopFront()
	{
		return Front().List_Remove();
	}
	FORCEINLINE TUniquePtr<T> PopBack()
	{
		return Back().List_Remove();
	}

	FORCEINLINE T* PushFront(TUniquePtr<T> Element)
	{
		return Sentinel.List_InsertAfter(MoveTemp(Element));
	}
	FORCEINLINE T* PushBack(TUniquePtr<T> Element)
	{
		return Sentinel.List_InsertBefore(MoveTemp(Element));
	}

	int32 Num_Slow() const
	{
		VOXEL_FUNCTION_COUNTER();

		int32 Count = 0;
		for (const T& It : *this)
		{
			Count++;
		}
		return Count;
	}

	bool Contains(const T* Node) const
	{
		VOXEL_FUNCTION_COUNTER();

		for (const T& It : *this)
		{
			if (&It == Node)
			{
				return true;
			}
		}

		return false;
	}

public:
	template<bool bReversed, bool bIsConst>
	class TIterator
	{
	public:
		using Type = std::conditional_t<bIsConst, const T, T>;

		FORCEINLINE TIterator(TNode<T>* Node, const TNode<T>* Sentinel)
			: Node(Node)
			, Sentinel(Sentinel)
		{
		}

		FORCEINLINE Type& Get() const
		{
			checkVoxelSlow(Node);
			checkVoxelSlow(Node != Sentinel);
			checkVoxelSlow(!NextNode_IfCurrentRemoved);
			return static_cast<T&>(*Node);
		}

		FORCEINLINE Type* operator->() const
		{
			return &Get();
		}
		FORCEINLINE Type& operator*() const
		{
			return Get();
		}

		FORCEINLINE TIterator& operator++()
		{
			if (NextNode_IfCurrentRemoved)
			{
				checkVoxelSlow(!Node);
				Node = NextNode_IfCurrentRemoved;
				NextNode_IfCurrentRemoved = nullptr;
			}
			else
			{
				checkVoxelSlow(Node);

				if constexpr (bReversed)
				{
					Node = Node->Prev;
				}
				else
				{
					Node = Node->Next;
				}
			}
			return *this;
		}

		FORCEINLINE bool operator!=(decltype(nullptr)) const
		{
			return Node != Sentinel;
		}
		FORCEINLINE explicit operator bool() const
		{
			return Node != Sentinel;
		}

		FORCEINLINE TUniquePtr<T> RemoveCurrent()
		{
			checkStatic(!bIsConst);
			checkVoxelSlow(Node);
			checkVoxelSlow(Node != Sentinel);
			checkVoxelSlow(!NextNode_IfCurrentRemoved);

			if constexpr (bReversed)
			{
				NextNode_IfCurrentRemoved = Node->Prev;
			}
			else
			{
				NextNode_IfCurrentRemoved = Node->Next;
			}

			TUniquePtr<T> Removed = Node->List_Remove();
			Node = nullptr;
			return Removed;
		}

	private:
		TNode<T>* Node = nullptr;
		const TNode<T>* const Sentinel;
		TNode<T>* NextNode_IfCurrentRemoved = nullptr;
	};

	using FIterator = TIterator<false, false>;
	using FReverseIterator = TIterator<true, false>;

	using FConstIterator = TIterator<false, true>;
	using FConstReverseIterator = TIterator<true, true>;

	FORCEINLINE FIterator begin()
	{
		return CreateIterator();
	}
	FORCEINLINE FConstIterator begin() const
	{
		return CreateIterator();
	}
	FORCEINLINE decltype(nullptr) end() const
	{
		return nullptr;
	}

public:
	template<bool bIsConst>
	class TReverseView
	{
	public:
		using ListType = std::conditional_t<bIsConst, const TList<T>, TList<T>>;

		FORCEINLINE explicit TReverseView(ListType& List)
			: List(List)
		{
		}

		FORCEINLINE TIterator<true, bIsConst> begin() const
		{
			return List.CreateReverseIterator();
		}
		FORCEINLINE decltype(nullptr) end() const
		{
			return nullptr;
		}

	private:
		ListType& List;
	};

	FORCEINLINE TReverseView<false> Reverse()
	{
		return TReverseView<false>(*this);
	}
	FORCEINLINE TReverseView<true> Reverse() const
	{
		return TReverseView<true>(*this);
	}

public:
	FORCEINLINE FIterator CreateIterator()
	{
		return FIterator(Sentinel.Next, &Sentinel);
	}
	FORCEINLINE FReverseIterator CreateReverseIterator()
	{
		return FReverseIterator(Sentinel.Prev, &Sentinel);
	}

	FORCEINLINE FConstIterator CreateIterator() const
	{
		return FConstIterator(Sentinel.Next, &Sentinel);
	}
	FORCEINLINE FConstReverseIterator CreateReverseIterator() const
	{
		return FConstReverseIterator(Sentinel.Prev, &Sentinel);
	}

private:
	TNode<T> Sentinel;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FTimespan
{
	int32 Milliseconds = 0;

	FTimespan() = default;

	FORCEINLINE static constexpr FTimespan FromMilliseconds(const int32 Milliseconds)
	{
		FTimespan Result;
		Result.Milliseconds = Milliseconds;
		return Result;
	}
	FORCEINLINE static constexpr FTimespan FromSeconds(const int32 Seconds)
	{
		return FromMilliseconds(1000 * Seconds);
	}

	FORCEINLINE bool IsNull() const
	{
		return Milliseconds == 0;
	}
	FORCEINLINE double Seconds() const
	{
		return Milliseconds / 1000.;
	}

	FORCEINLINE FTimespan Abs() const
	{
		return FromMilliseconds(FMath::Abs(Milliseconds));
	}

	FORCEINLINE bool operator<(const FTimespan Other) const
	{
		return Milliseconds < Other.Milliseconds;
	}
	FORCEINLINE bool operator>(const FTimespan Other) const
	{
		return Milliseconds > Other.Milliseconds;
	}
	FORCEINLINE bool operator<=(const FTimespan Other) const
	{
		return Milliseconds <= Other.Milliseconds;
	}
	FORCEINLINE bool operator>=(const FTimespan Other) const
	{
		return Milliseconds >= Other.Milliseconds;
	}

	FORCEINLINE FTimespan& operator+=(const FTimespan Other)
	{
		Milliseconds += Other.Milliseconds;
		return *this;
	}
	FORCEINLINE FTimespan& operator-=(const FTimespan Other)
	{
		Milliseconds -= Other.Milliseconds;
		return *this;
	}
	FORCEINLINE FTimespan& operator*=(const int32 Other)
	{
		Milliseconds *= Other;
		return *this;
	}
	FORCEINLINE FTimespan& operator/=(const int32 Other)
	{
		Milliseconds /= Other;
		return *this;
	}

	FORCEINLINE FTimespan operator+(const FTimespan Other) const
	{
		return FTimespan(*this) += Other;
	}
	FORCEINLINE FTimespan operator-(const FTimespan Other) const
	{
		return FTimespan(*this) -= Other;
	}
	FORCEINLINE FTimespan operator*(const int32 Other) const
	{
		return FTimespan(*this) *= Other;
	}
	FORCEINLINE FTimespan operator/(const int32 Other) const
	{
		return FTimespan(*this) /= Other;
	}
};

FORCEINLINE FTimespan operator*(const int32 Multiplier, const FTimespan Timespan)
{
	return Timespan * Multiplier;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FTime
{
	uint32 Milliseconds = 0;

	FTime() = default;

	static void SetNow_TestOnly(TVoxelOptional<FTime> NewTime);
	static FTime Now();

	FORCEINLINE bool IsNull() const
	{
		return Milliseconds == 0;
	}

	FORCEINLINE bool operator==(const FTime Other) const
	{
		return Milliseconds == Other.Milliseconds;
	}

	FORCEINLINE bool operator<(const FTime Other) const
	{
		return Milliseconds < Other.Milliseconds;
	}
	FORCEINLINE bool operator>(const FTime Other) const
	{
		return Milliseconds > Other.Milliseconds;
	}
	FORCEINLINE bool operator<=(const FTime Other) const
	{
		return Milliseconds <= Other.Milliseconds;
	}
	FORCEINLINE bool operator>=(const FTime Other) const
	{
		return Milliseconds >= Other.Milliseconds;
	}

	FORCEINLINE FTime operator+(const FTimespan Other) const
	{
		FTime Result;
		Result.Milliseconds = Milliseconds + Other.Milliseconds;
		return Result;
	}
};

FORCEINLINE FTimespan operator-(const FTime A, const FTime B)
{
	return FTimespan::FromMilliseconds(int32(int64(A.Milliseconds) - int64(B.Milliseconds)));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FPackedTime
{
	uint16 PackedMilliseconds = 0;

	FPackedTime() = default;

	FORCEINLINE static FPackedTime Pack(const FTime Time)
	{
		FPackedTime Result;
		Result.PackedMilliseconds = uint16(Time.Milliseconds);
		return Result;
	}
	FORCEINLINE FTime Unpack(const FTime CurrentTime) const
	{
		FTime Result;
		Result.Milliseconds = PackedMilliseconds | int32(CurrentTime.Milliseconds & 0xFFFF0000);

		// Check if bit 15 differs (indicates wraparound)
		const bool bPackedHasBit15 = (Result.Milliseconds & 0x8000) != 0;
		const bool bCurrentHasBit15 = (CurrentTime.Milliseconds & 0x8000) != 0;

		if (bPackedHasBit15 && !bCurrentHasBit15)
		{
			// Packed time had bit 15 set, but current doesn't - wrapped around
			Result.Milliseconds -= 0x10000;
		}

		return Result;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

using FSendPacket = TFunction<void(TConstVoxelArrayView<uint8> Data)>;
using FOnMessageReceived = TFunction<void(uint8 ChannelId, TVoxelArray<uint8>&& Data)>;

struct FContext
{
	FTime Time;

	int32 MTU = 1472;
	bool bAllowPings = true;
    // Max downstream bandwidth of the host in bytes/second;
	int32 IncomingBandwidthLimit = 100 * 1024 * 1024;
    // Max upstream bandwidth of the host in bytes/second;
	int32 OutgoingBandwidthLimit = 100 * 1024 * 1024;
};

}