// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinType.h"
#include "VoxelObjectPinType.generated.h"

USTRUCT()
struct VOXELGRAPH_API FVoxelObjectPinType
{
	GENERATED_BODY()

	FVoxelObjectPinType() = default;
	virtual ~FVoxelObjectPinType() = default;

	virtual UClass* GetClass() const VOXEL_PURE_VIRTUAL({});
	virtual UScriptStruct* GetStruct() const VOXEL_PURE_VIRTUAL({});

	virtual TVoxelArray<UClass*> GetAllowedClasses() const;

	virtual TVoxelObjectPtr<UObject> GetWeakObject(FConstVoxelStructView Struct) const VOXEL_PURE_VIRTUAL({});
	virtual FVoxelInstancedStruct GetStruct(UObject* Object) const VOXEL_PURE_VIRTUAL({});

	UObject* GetObject(FConstVoxelStructView Struct) const;

	static void RegisterPinType(const TSharedRef<const FVoxelObjectPinType>& PinType);
	static const TVoxelMap<const UScriptStruct*, TSharedPtr<const FVoxelObjectPinType>>& StructToPinType();
	static const TVoxelMap<const UClass*, TVoxelArray<TSharedPtr<const FVoxelObjectPinType>>>& ClassToPinTypes();
};

#define DECLARE_VOXEL_OBJECT_PIN_TYPE(Type) \
	template<> \
	inline constexpr bool IsVoxelObjectStruct<Type> = true;

#define DEFINE_VOXEL_OBJECT_PIN_TYPE(StructType, ObjectType) \
	void Dummy1() { checkStatic(std::is_same_v<VOXEL_THIS_TYPE, StructType ## PinType>); } \
	void Dummy2() { static_assert(IsVoxelObjectStruct<StructType>, "Object pin type not declared: use DECLARE_VOXEL_OBJECT_PIN_TYPE(YourType);"); } \
	virtual UClass* GetClass() const override { return ObjectType::StaticClass(); } \
	virtual UScriptStruct* GetStruct() const override { return StructType::StaticStruct(); } \
	virtual TVoxelObjectPtr<UObject> GetWeakObject(const FConstVoxelStructView Struct) const override \
	{ \
		if (!ensure(Struct.IsValid()) || \
			!ensure(Struct.IsA<StructType>())) \
		{ \
			return nullptr; \
		} \
		\
		TVoxelObjectPtr<ObjectType> Object; \
		Convert(true, Object, *reinterpret_cast<ObjectType*>(-1), ConstCast(Struct.Get<StructType>())); \
		return Object; \
	} \
	virtual FVoxelInstancedStruct GetStruct(UObject* Object) const override \
	{ \
		check(IsInGameThread()); \
		FVoxelInstancedStruct Struct = FVoxelInstancedStruct::Make<StructType>(); \
		if (!Object || \
			!ensure(Object->IsA<ObjectType>())) \
		{ \
			return Struct; \
		} \
		TVoxelObjectPtr<ObjectType> DummyObject; \
		Convert(false, DummyObject, *CastChecked<ObjectType>(Object), Struct.Get<StructType>()); \
		return Struct; \
	} \
	static void Convert(const bool bSetObject, TVoxelObjectPtr<ObjectType>& OutObject, ObjectType& InObject, StructType& Struct)