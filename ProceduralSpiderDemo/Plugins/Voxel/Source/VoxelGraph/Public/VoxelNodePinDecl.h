// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPin.h"
#include "VoxelPinValue.h"

#if WITH_EDITOR
class VOXELGRAPH_API FVoxelNodeDefaultValueHelper
{
public:
	template<typename PinType, typename ValueType>
	static FString Get(PinType*, ValueType Value)
	{
		if constexpr (std::is_same_v<ValueType, decltype(nullptr)>)
		{
			return {};
		}
		else if constexpr (
			std::is_void_v<PinType> &&
			std::is_same_v<ValueType, FString>)
		{
			// Generic default
			return Value;
		}
		else if constexpr (
			std::is_same_v<PinType, FVoxelWildcard> ||
			std::is_same_v<PinType, FVoxelWildcardBuffer>)
		{
			// Wildcards can take any default value
			return FVoxelPinValue::Make(Value).ExportToString();
		}
		else if constexpr (IsVoxelBuffer<PinType>)
		{
			// Buffer can be initialized by their uniform
			checkStatic(std::is_same_v<TVoxelBufferInnerType<PinType>, ValueType>);
			return FVoxelPinValue::Make(Value).ExportToString();
		}
		else if constexpr (IsVoxelObjectStruct<PinType>)
		{
			// Object need to be initialized by object paths
			return FVoxelNodeDefaultValueHelper::MakeObject(FVoxelPinType::Make<PinType>(), Value);
		}
		else if constexpr (
			std::is_same_v<PinType, FBodyInstance> &&
			std::is_same_v<ValueType, ECollisionEnabled::Type>)
		{
			return FVoxelNodeDefaultValueHelper::MakeBodyInstance(Value);
		}
		else
		{
			// Try to convert
			return FVoxelPinValue::Make(PinType(Value)).ExportToString();
		}
	}

private:
	static FString MakeObject(const FVoxelPinType& RuntimeType, const FString& Path);
	static FString MakeBodyInstance(ECollisionEnabled::Type CollisionEnabled);
};
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace FVoxelPinMetadataBuilder
{
	struct HidePin {};
	struct ArrayPin {};
	struct PositionPin {};
	struct SplineKeyPin {};
	struct DisplayLast {};
	struct NoCache {};
	struct HideDefault {};
	struct ShowInDetail {};
	struct AdvancedDisplay {};

	namespace Internal
	{
		struct None {};

		template<typename T>
		struct TStringParam
		{
#if WITH_EDITOR
			FString Value;
#endif

			template<typename CharType>
			explicit TStringParam(const CharType* Value)
#if WITH_EDITOR
				: Value(Value)
#endif
			{
			}
			explicit TStringParam(const FString& Value)
#if WITH_EDITOR
				: Value(Value)
#endif
			{
			}

			T operator()() const { return ReinterpretCastRef<const T&>(*this); }
		};
	}

	struct DisplayName : Internal::TStringParam<DisplayName> { using TStringParam::TStringParam; };
	struct Tooltip : Internal::TStringParam<Tooltip> { using TStringParam::TStringParam; };
	struct Category : Internal::TStringParam<Category> { using TStringParam::TStringParam; };
	struct Custom
	{
#if WITH_EDITOR
		FName Name;
		FString Value;
#endif

		template<typename CharType>
		explicit Custom(const CharType* Name, const CharType* Value)
#if WITH_EDITOR
			: Name(Name)
			, Value(Value)
#endif
		{
		}
		explicit Custom(const FName& Name, const FString& Value)
#if WITH_EDITOR
			: Name(Name)
			, Value(Value)
#endif
		{
		}
		Custom operator()() const { return *this; }
	};

	template<typename Type>
	struct TBuilder
	{
		static void MakeImpl(FVoxelPinMetadata&, Internal::None) {}

		static void MakeImpl(FVoxelPinMetadata& Metadata, DisplayName Value)
		{
#if WITH_EDITOR
			ensure(Metadata.DisplayName.IsEmpty());
			Metadata.DisplayName = Value.Value;
#endif
		}
		static void MakeImpl(FVoxelPinMetadata& Metadata, Category Value)
		{
#if WITH_EDITOR
			ensure(Metadata.Category.IsEmpty());
			Metadata.Category = Value.Value;
#endif
		}
		static void MakeImpl(FVoxelPinMetadata& Metadata, Tooltip Value)
		{
#if WITH_EDITOR
			ensure(!Metadata.Tooltip.IsSet());
			Metadata.Tooltip = Value.Value;
#endif
		}

		static void MakeImpl(FVoxelPinMetadata& Metadata, HidePin)
		{
			ensure(!Metadata.bHidePin);
			Metadata.bHidePin = true;
		}
		static void MakeImpl(FVoxelPinMetadata& Metadata, ArrayPin)
		{
			ensure(!Metadata.bArrayPin);
			Metadata.bArrayPin = true;
		}
		static void MakeImpl(FVoxelPinMetadata& Metadata, PositionPin)
		{
			ensure(!Metadata.bPositionPin);
			Metadata.bPositionPin = true;
		}
		static void MakeImpl(FVoxelPinMetadata& Metadata, SplineKeyPin)
		{
			ensure(!Metadata.bSplineKeyPin);
			Metadata.bSplineKeyPin = true;
		}
		static void MakeImpl(FVoxelPinMetadata& Metadata, DisplayLast)
		{
			ensure(!Metadata.bDisplayLast);
			Metadata.bDisplayLast = true;
		}
		static void MakeImpl(FVoxelPinMetadata& Metadata, NoCache)
		{
			ensure(!Metadata.bNoCache);
			Metadata.bNoCache = true;
		}
		static void MakeImpl(FVoxelPinMetadata& Metadata, HideDefault)
		{
			ensure(!Metadata.bHideDefault);
			Metadata.bHideDefault = true;
		}
		static void MakeImpl(FVoxelPinMetadata& Metadata, ShowInDetail)
		{
			ensure(!Metadata.bShowInDetail);
			Metadata.bShowInDetail = true;
		}
		static void MakeImpl(FVoxelPinMetadata& Metadata, AdvancedDisplay)
		{
#if WITH_EDITOR
			ensure(Metadata.Category.IsEmpty());
			Metadata.Category = "Advanced";
#endif
		}

		static void MakeImpl(FVoxelPinMetadata& Metadata, Custom Value)
		{
#if WITH_EDITOR
			Metadata.CustomMetadata.Add_EnsureNew(Value.Name, Value.Value);
#endif
		}

		template<typename T>
		static constexpr bool IsValid = std::is_void_v<decltype(TBuilder::MakeImpl(std::declval<FVoxelPinMetadata&>(), std::declval<T>()))>;

		template<typename T, typename... ArgTypes>
		requires (... && IsValid<ArgTypes>)
		static FVoxelPinMetadata Make(
			const int32 Line,
			UScriptStruct* Struct,
			const T& DefaultValue,
			ArgTypes&&... Args)
		{
			FVoxelPinMetadata Metadata;
#if WITH_EDITOR
			ensure(Metadata.DefaultValue.IsEmpty());
			Metadata.DefaultValue = FVoxelNodeDefaultValueHelper::Get(static_cast<Type*>(nullptr), DefaultValue);
			Metadata.Line = Line;
			Metadata.Struct = Struct;
#endif
			VOXEL_FOLD_EXPRESSION(TBuilder::MakeImpl(Metadata, Args));
			return Metadata;
		}
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define INTERNAL_VOXEL_PIN_METADATA_FOREACH(X) FVoxelPinMetadataBuilder::X()

#define VOXEL_PIN_METADATA_IMPL(Type, Line, Struct, Default, ...) \
	FVoxelPinMetadataBuilder::TBuilder<Type>::Make(Line, Struct, Default, VOXEL_FOREACH_COMMA(INTERNAL_VOXEL_PIN_METADATA_FOREACH, __VA_ARGS__))

#define VOXEL_PIN_METADATA(Type, Default, ...) VOXEL_PIN_METADATA_IMPL(Type, 0, nullptr, Default, Internal::None, ##__VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define INTERNAL_DECLARE_VOXEL_PIN(Type, Name) \
	INTELLISENSE_ONLY(void VOXEL_APPEND_LINE(__DummyFunction)(FPinRef Name) { struct FVoxelWildcard {}; struct FVoxelWildcardBuffer {}; sizeof(Type); (void)Name; })

#define INTERNAL_REGISTER_VOXEL_PIN(Name) \
	VOXEL_ON_CONSTRUCT() \
	{ \
		RegisterPinRef(Name ## Pin); \
	};

#define VOXEL_TYPED_INPUT_PIN(InType, InPinType, Name, Default, ...) \
	INTERNAL_DECLARE_VOXEL_PIN(InType, Name); \
	TPinRef_Input<InType> Name ## Pin = TPinRef_Input<InType>(CreateInputPin(InPinType, STATIC_FNAME(#Name), VOXEL_PIN_METADATA_IMPL(InType, __LINE__, StaticStruct(), Default, Internal::None, ##__VA_ARGS__))); \
	INTERNAL_REGISTER_VOXEL_PIN(Name)

#define VOXEL_INPUT_PIN(InType, Name, Default, ...) \
	INTERNAL_DECLARE_VOXEL_PIN(InType, Name); \
	TPinRef_Input<InType> Name ## Pin = CreateInputPin<InType>(STATIC_FNAME(#Name), VOXEL_PIN_METADATA_IMPL(InType, __LINE__, StaticStruct(), Default, Internal::None, ##__VA_ARGS__)); \
	INTERNAL_REGISTER_VOXEL_PIN(Name)

#define VOXEL_VARIADIC_INPUT_PIN(InType, Name, Default, MinNum, ...) \
	INTERNAL_DECLARE_VOXEL_PIN(InType, Name); \
	TVariadicPinRef_Input<InType> Name ## Pins = CreateVariadicInputPin<InType>(STATIC_FNAME(#Name), VOXEL_PIN_METADATA_IMPL(InType, __LINE__, StaticStruct(), Default, Internal::None, ##__VA_ARGS__), MinNum); \
	VOXEL_ON_CONSTRUCT() \
	{ \
		RegisterVariadicPinRef(Name ## Pins); \
	};

#define VOXEL_OUTPUT_PIN(InType, Name, ...) \
	INTERNAL_DECLARE_VOXEL_PIN(InType, Name); \
	TPinRef_Output<InType> Name ## Pin = CreateOutputPin<InType>(STATIC_FNAME(#Name), VOXEL_PIN_METADATA_IMPL(InType, __LINE__, StaticStruct(), nullptr, Internal::None, ##__VA_ARGS__)); \
	INTERNAL_REGISTER_VOXEL_PIN(Name)

///////////////////////////////////////////////////////////////////////////////
/////////////////// Template pins can be scalar or buffer /////////////////////
///////////////////////////////////////////////////////////////////////////////

#define VOXEL_TEMPLATE_INPUT_PIN(InType, Name, Default, ...) \
	INTERNAL_DECLARE_VOXEL_PIN(InType, Name); \
	TTemplatePinRef_Input<InType> Name ## Pin = ( \
		[] { checkStatic(!IsVoxelBuffer<InType>); }, \
		CreateTemplateInputPin<InType>(STATIC_FNAME(#Name), VOXEL_PIN_METADATA_IMPL(InType, __LINE__, StaticStruct(), Default, Internal::None, ##__VA_ARGS__))); \
	INTERNAL_REGISTER_VOXEL_PIN(Name)

#define VOXEL_TEMPLATE_VARIADIC_INPUT_PIN(InType, Name, Default, MinNum, ...) \
	INTERNAL_DECLARE_VOXEL_PIN(InType, Name); \
	TVariadicTemplatePinRef_Input<InType> Name ## Pins = ( \
		[] { checkStatic(!IsVoxelBuffer<InType>); }, \
		CreateVariadicTemplateInputPin<InType>(STATIC_FNAME(#Name), VOXEL_PIN_METADATA_IMPL(InType, __LINE__, StaticStruct(), Default, Internal::None, ##__VA_ARGS__), MinNum)); \
	VOXEL_ON_CONSTRUCT() \
	{ \
		RegisterVariadicPinRef(Name ## Pins); \
	};

#define VOXEL_TEMPLATE_OUTPUT_PIN(InType, Name, ...) \
	INTERNAL_DECLARE_VOXEL_PIN(InType, Name); \
	TTemplatePinRef_Output<InType> Name ## Pin = ( \
		[] { checkStatic(!IsVoxelBuffer<InType>); }, \
		CreateTemplateOutputPin<InType>(STATIC_FNAME(#Name), VOXEL_PIN_METADATA_IMPL(InType, __LINE__, StaticStruct(), nullptr, Internal::None, ##__VA_ARGS__))); \
	INTERNAL_REGISTER_VOXEL_PIN(Name)