// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelPointUtilities.h"
#include "VoxelPointId.h"
#include "VoxelPointSet.h"
#include "Nodes/VoxelVectorNodes.h"
#include "Buffer/VoxelIntegerBuffers.h"
#include "Utilities/VoxelBufferMathUtilities.h"
#include "Metadata/Accessors/PCGAttributeExtractor.h"
#include "VoxelPointUtilitiesImpl.ispc.generated.h"

TSharedPtr<const FVoxelBuffer> FVoxelPointUtilities::FindExtractedAttribute(
	const FVoxelPointSet& PointSet,
	const FName AttributeName,
	FName& OutName,
	TVoxelArray<FString>& OutExtractors)
{
	VOXEL_FUNCTION_COUNTER();

	FString FullName = AttributeName.ToString();
	int32 DotIndex = -1;
	while (FullName.FindLastChar('.', DotIndex))
	{
		OutExtractors.Insert(FullName.RightChop(DotIndex + 1), 0);
		FullName.LeftInline(DotIndex);

		if (const TSharedPtr<const FVoxelBuffer> Attribute = PointSet.FindShared(FName(FullName)))
		{
			OutName = FName(FullName);
			return Attribute;
		}
	}

	OutName = FName(FullName);
	return nullptr;
}

TSharedPtr<const FVoxelBuffer> FVoxelPointUtilities::ParseAttribute(
	const FVoxelPointSet& PointSet,
	const FVoxelPinType& Type,
	FName AttributeName,
	const FVoxelGraphMergedNodeRef& NodeRef)
{
	if (AttributeName.IsNone())
	{
		return nullptr;
	}

	if (const TSharedPtr<const FVoxelBuffer> Attribute = PointSet.FindShared(AttributeName))
	{
		if (Type.IsValid_Fast() &&
			!Attribute->GetInnerType().CanBeCastedTo(Type.GetInnerType()))
		{
			VOXEL_MESSAGE(Error, "{0}: Found attribute named {1} of type {2}, cannot cast to {3}",
				NodeRef,
				AttributeName,
				Attribute->GetInnerType().ToString(),
				Type.GetInnerType().ToString());
			return nullptr;
		}

		return Attribute;
	}

	FName Name;
	TVoxelArray<FString> Extractors;
	const TSharedPtr<const FVoxelBuffer> Attribute = FindExtractedAttribute(
		PointSet,
		AttributeName,
		Name,
		Extractors);

	if (!Attribute)
	{
		VOXEL_MESSAGE(Error, "{0}: No attribute named {1} found", NodeRef, AttributeName);
		return nullptr;
	}

	const TSharedPtr<const FVoxelBuffer> Buffer = ExtractAttribute(
		Type,
		Attribute.ToSharedRef(),
		Extractors,
		Name,
		NodeRef);

	if (!ensure(Buffer))
	{
		VOXEL_MESSAGE(Error, "{0}: Failed to extract {1}", NodeRef, Name);
		return nullptr;
	}

	return Buffer;
}

TSharedPtr<const FVoxelBuffer> FVoxelPointUtilities::ExtractAttribute(
	const FVoxelPinType& Type,
	const TSharedRef<const FVoxelBuffer>& Buffer,
	const TConstVoxelArrayView<FString> Extractors,
	FName AttributeName,
	const FVoxelGraphMergedNodeRef& NodeRef)
{
	VOXEL_FUNCTION_COUNTER();

	TSharedPtr<const FVoxelBuffer> ExtractedBuffer = Buffer;
	for (int32 Index = 0; Index < Extractors.Num(); Index++)
	{
		const FString& Extractor = Extractors[Index];

		const TSharedPtr<const FVoxelBuffer> NewValue = INLINE_LAMBDA -> TSharedPtr<const FVoxelBuffer>
		{
#define CALL_EXTRACTOR(BufferType, ExtractorFunc) \
			if (Buffer->GetStruct() == StaticStructFast<BufferType>()) \
			{ \
				return ExtractorFunc( \
					StaticCastSharedPtr<const BufferType>(ExtractedBuffer).ToSharedRef(), \
					Extractor, \
					Index + 1 >= Extractors.Num() ? Type : FVoxelPinType(), \
					AttributeName, \
					NodeRef); \
			}

			CALL_EXTRACTOR(FVoxelLinearColorBuffer, FVoxelPointUtilities::ExtractVector);
			CALL_EXTRACTOR(FVoxelVectorBuffer, FVoxelPointUtilities::ExtractVector);
			CALL_EXTRACTOR(FVoxelVector2DBuffer, FVoxelPointUtilities::ExtractVector);
			CALL_EXTRACTOR(FVoxelIntVector4Buffer, FVoxelPointUtilities::ExtractVector);
			CALL_EXTRACTOR(FVoxelIntVectorBuffer, FVoxelPointUtilities::ExtractVector);
			CALL_EXTRACTOR(FVoxelIntPointBuffer, FVoxelPointUtilities::ExtractVector);
			CALL_EXTRACTOR(FVoxelQuaternionBuffer, FVoxelPointUtilities::ExtractQuat);
			CALL_EXTRACTOR(FVoxelTransformBuffer, FVoxelPointUtilities::ExtractTransform);

#undef CALL_EXTRACTOR

			VOXEL_MESSAGE(
				Error,
				"{0}: Cannot extract data from attribute {1} with type {2}",
				NodeRef,
				AttributeName,
				Type.ToString());

			return {};
		};

		if (!NewValue)
		{
			return nullptr;
		}

		ExtractedBuffer = NewValue;
	}

	return ExtractedBuffer;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPointUtilities::SetExtractedAttribute(
	FVoxelBuffer& Attribute,
	const TSharedRef<const FVoxelBuffer>& NewValue,
	const TArray<FString>& Extractors,
	const int32 ExtractorIndex,
	const FName AttributeName,
	const FVoxelGraphMergedNodeRef& NodeRef)
{
	VOXEL_FUNCTION_COUNTER();

	if (ExtractorIndex >= Extractors.Num())
	{
		return;
	}

	const FVoxelPinType Type = Attribute.GetBufferType();
	const bool bIsLastExtractor = ExtractorIndex + 1 == Extractors.Num();
	const FString& CurrentExtractor = Extractors[ExtractorIndex];
	if (FVoxelTransformBuffer* TransformBuffer = Attribute.As<FVoxelTransformBuffer>())
	{
		if (CurrentExtractor == PCGAttributeExtractorConstants::TransformLocation)
		{
			if (bIsLastExtractor)
			{
				if (const FVoxelVectorBuffer* VectorBuffer = NewValue->As<FVoxelVectorBuffer>())
				{
					TransformBuffer->Translation.X = VectorBuffer->X;
					TransformBuffer->Translation.Y = VectorBuffer->Y;
					TransformBuffer->Translation.Z = VectorBuffer->Z;
					return;
				}

				VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot set extracted component {3} to type {4}",
					NodeRef,
					AttributeName,
					Type.ToString(),
					CurrentExtractor,
					NewValue->GetBufferType().ToString());
				return;
			}

			SetExtractedAttribute(
				TransformBuffer->Translation,
				NewValue,
				Extractors,
				ExtractorIndex + 1,
				AttributeName,
				NodeRef);
			return;
		}
		if (CurrentExtractor == PCGAttributeExtractorConstants::TransformScale)
		{
			if (bIsLastExtractor)
			{
				if (const FVoxelVectorBuffer* VectorBuffer = NewValue->As<FVoxelVectorBuffer>())
				{
					TransformBuffer->Scale.X = VectorBuffer->X;
					TransformBuffer->Scale.Y = VectorBuffer->Y;
					TransformBuffer->Scale.Z = VectorBuffer->Z;
					return;
				}

				VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot set extracted component {3} to type {4}",
					NodeRef,
					AttributeName,
					Type.ToString(),
					CurrentExtractor,
					NewValue->GetBufferType().ToString());
				return;
			}

			SetExtractedAttribute(
				TransformBuffer->Scale,
				NewValue,
				Extractors,
				ExtractorIndex + 1,
				AttributeName,
				NodeRef);
			return;
		}
		if (CurrentExtractor == PCGAttributeExtractorConstants::TransformRotation)
		{
			if (bIsLastExtractor)
			{
				if (const FVoxelQuaternionBuffer* QuaternionBuffer = NewValue->As<FVoxelQuaternionBuffer>())
				{
					TransformBuffer->Rotation = *QuaternionBuffer;
					return;
				}

				VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot set extracted component {3} to type {4}",
					NodeRef,
					AttributeName,
					Type.ToString(),
					CurrentExtractor,
					NewValue->GetBufferType().ToString());
				return;
			}

			SetExtractedAttribute(
				TransformBuffer->Rotation,
				NewValue,
				Extractors,
				ExtractorIndex + 1,
				AttributeName,
				NodeRef);
			return;
		}
	}

	if (FVoxelQuaternionBuffer* QuaternionBuffer = Attribute.As<FVoxelQuaternionBuffer>())
	{
		if (CurrentExtractor == PCGAttributeExtractorConstants::RotatorPitch)
		{
			if (const FVoxelFloatBuffer* FloatBuffer = NewValue->As<FVoxelFloatBuffer>())
			{
				FVoxelVectorBuffer RotatorBuffer = FVoxelBufferMathUtilities::MakeEulerFromQuaternion(*QuaternionBuffer);
				RotatorBuffer.X = *FloatBuffer;
				*QuaternionBuffer = FVoxelBufferMathUtilities::MakeQuaternionFromEuler(RotatorBuffer);
				return;
			}

			VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot set extracted component {3} to type {4}",
				NodeRef,
				AttributeName,
				Type.ToString(),
				CurrentExtractor,
				NewValue->GetBufferType().ToString());
			return;
		}
		if (CurrentExtractor == PCGAttributeExtractorConstants::RotatorYaw)
		{
			if (const FVoxelFloatBuffer* FloatBuffer = NewValue->As<FVoxelFloatBuffer>())
			{
				FVoxelVectorBuffer RotatorBuffer = FVoxelBufferMathUtilities::MakeEulerFromQuaternion(*QuaternionBuffer);
				RotatorBuffer.Y = *FloatBuffer;
				*QuaternionBuffer = FVoxelBufferMathUtilities::MakeQuaternionFromEuler(RotatorBuffer);
				return;
			}

			VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot set extracted component {3} to type {4}",
				NodeRef,
				AttributeName,
				Type.ToString(),
				CurrentExtractor,
				NewValue->GetBufferType().ToString());
			return;
		}
		if (CurrentExtractor == PCGAttributeExtractorConstants::RotatorRoll)
		{
			if (const FVoxelFloatBuffer* FloatBuffer = NewValue->As<FVoxelFloatBuffer>())
			{
				FVoxelVectorBuffer RotatorBuffer = FVoxelBufferMathUtilities::MakeEulerFromQuaternion(*QuaternionBuffer);
				RotatorBuffer.Z = *FloatBuffer;
				*QuaternionBuffer = FVoxelBufferMathUtilities::MakeQuaternionFromEuler(RotatorBuffer);
				return;
			}

			VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot set extracted component {3} to type {4}",
				NodeRef,
				AttributeName,
				Type.ToString(),
				CurrentExtractor,
				NewValue->GetBufferType().ToString());
			return;
		}
	}

	if (Type.Is<FVoxelVector2DBuffer>() ||
		Type.Is<FVoxelVectorBuffer>() ||
		Type.Is<FVoxelLinearColorBuffer>() ||
		Type.Is<FVoxelIntPointBuffer>() ||
		Type.Is<FVoxelIntVectorBuffer>() ||
		Type.Is<FVoxelIntVector4Buffer>())
	{
		const static TVoxelMap<char, uint8> CharToComponentIndex
		{
			{'X', 0},
			{'x', 0},
			{'R', 0},
			{'r', 0},
			{'Y', 1},
			{'y', 1},
			{'G', 1},
			{'g', 1},
			{'Z', 2},
			{'z', 2},
			{'B', 2},
			{'b', 2},
			{'W', 3},
			{'w', 3},
			{'A', 3},
			{'A', 3},
		};

		const bool bIsFloat = FVoxelTemplateNodeUtilities::IsFloat(Type);
		if (bIsFloat != FVoxelTemplateNodeUtilities::IsFloat(NewValue->GetBufferType()))
		{
			VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot extract {3} to type {4}",
				NodeRef,
				AttributeName,
				Type.ToString(),
				CurrentExtractor,
				NewValue->GetBufferType().ToString());
			return;
		}

		TVoxelInlineArray<FVoxelTerminalBuffer*, 4> AttributeComponentBuffers;

		// Prepare components
		INLINE_LAMBDA
		{
#define INTERNAL_GET_ATTRIBUTE_COMPONENT_BUFFER(Name) AttributeComponentBuffers.Add(&TypedBuffer->Name);
#define GET_ATTRIBUTE_STORAGE_COMPONENTS(Type, ...) \
			if (Type* TypedBuffer = Attribute.As<Type>()) \
			{ \
				VOXEL_FOREACH(INTERNAL_GET_ATTRIBUTE_COMPONENT_BUFFER, ##__VA_ARGS__) \
				return; \
			}

			GET_ATTRIBUTE_STORAGE_COMPONENTS(FVoxelLinearColorBuffer, R, G, B, A);
			GET_ATTRIBUTE_STORAGE_COMPONENTS(FVoxelVectorBuffer, X, Y, Z);
			GET_ATTRIBUTE_STORAGE_COMPONENTS(FVoxelVector2DBuffer, X, Y);
			GET_ATTRIBUTE_STORAGE_COMPONENTS(FVoxelIntVector4Buffer, X, Y, Z, W);
			GET_ATTRIBUTE_STORAGE_COMPONENTS(FVoxelIntVectorBuffer, X, Y, Z);
			GET_ATTRIBUTE_STORAGE_COMPONENTS(FVoxelIntPointBuffer, X, Y);
		};

#undef GET_ATTRIBUTE_STORAGE_COMPONENTS
#undef INTERNAL_GET_ATTRIBUTE_COMPONENT_BUFFER

		TVoxelInlineArray<const FVoxelTerminalBuffer*, 4> NewValueBuffers;

		// Prepare new value buffers
		INLINE_LAMBDA
		{
#define INTERNAL_GET_NEW_VALUE_COMPONENT_BUFFER(Name) NewValueBuffers.Add(&TypedBuffer->Name);
#define GET_NEW_VALUE_COMPONENTS(Type, ...) \
			if (const Type* TypedBuffer = NewValue->As<Type>()) \
			{ \
				VOXEL_FOREACH(INTERNAL_GET_NEW_VALUE_COMPONENT_BUFFER, ##__VA_ARGS__) \
				return; \
			}

			GET_NEW_VALUE_COMPONENTS(FVoxelLinearColorBuffer, R, G, B, A);
			GET_NEW_VALUE_COMPONENTS(FVoxelVectorBuffer, X, Y, Z);
			GET_NEW_VALUE_COMPONENTS(FVoxelVector2DBuffer, X, Y);
			if (const FVoxelFloatBuffer* TypedBuffer = NewValue->As<FVoxelFloatBuffer>())
			{
				NewValueBuffers.Add(TypedBuffer);
				return;
			};
			GET_NEW_VALUE_COMPONENTS(FVoxelIntVector4Buffer, X, Y, Z, W);
			GET_NEW_VALUE_COMPONENTS(FVoxelIntVectorBuffer, X, Y, Z);
			GET_NEW_VALUE_COMPONENTS(FVoxelIntPointBuffer, X, Y);
			if (const FVoxelInt32Buffer* TypedBuffer = NewValue->As<FVoxelInt32Buffer>())
			{
				NewValueBuffers.Add(TypedBuffer);
				return;
			};
		};

#undef GET_NEW_VALUE_COMPONENTS
#undef INTERNAL_GET_NEW_VALUE_COMPONENT_BUFFER

		// Select components
		TVoxelInlineArray<FVoxelTerminalBuffer*, 8> OrderedComponents;
		for (int32 Index = 0; Index < CurrentExtractor.Len(); Index++)
		{
			const uint8* ComponentIndexPtr = CharToComponentIndex.Find(CurrentExtractor[Index]);
			if (!ComponentIndexPtr ||
				*ComponentIndexPtr >= AttributeComponentBuffers.Num())
			{
				VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot extract {3} component {4}",
					NodeRef,
					AttributeName,
					Type.ToString(),
					CurrentExtractor,
					CurrentExtractor.Mid(Index, 1));
				return;
			}

			OrderedComponents.Add(AttributeComponentBuffers[*ComponentIndexPtr]);
		}

		// If float/int32 is passed, set all components to its value
		if (NewValueBuffers.Num() == 1)
		{
			for (int32 Index = 0; Index < OrderedComponents.Num(); Index++)
			{
				if (bIsFloat)
				{
					OrderedComponents[Index]->AsChecked<FVoxelFloatBuffer>() = NewValueBuffers[0]->AsChecked<FVoxelFloatBuffer>();
				}
				else
				{
					OrderedComponents[Index]->AsChecked<FVoxelInt32Buffer>() = NewValueBuffers[0]->AsChecked<FVoxelInt32Buffer>();
				}
			}
		}
		else
		{
			const int32 NumStorages = FMath::Min(NewValueBuffers.Num(), OrderedComponents.Num());
			for (int32 Index = 0; Index < NumStorages; Index++)
			{
				if (bIsFloat)
				{
					*OrderedComponents[Index]->As<FVoxelFloatBuffer>() = NewValueBuffers[Index]->AsChecked<FVoxelFloatBuffer>();
				}
				else
				{
					*OrderedComponents[Index]->As<FVoxelInt32Buffer>() = NewValueBuffers[Index]->AsChecked<FVoxelInt32Buffer>();
				}
			}
		}
		return;
	}

	VOXEL_MESSAGE(Error, "{0}: Invalid extractor {1}, for attribute {2} with type {3}",
		NodeRef,
		CurrentExtractor,
		AttributeName,
		Type.ToString());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<const FVoxelBuffer> FVoxelPointUtilities::ExtractVectorComponent(
	const FVoxelBuffer& Buffer,
	const FString& Extractor,
	const FVoxelPinType& ReturnPinType,
	const FName AttributeName,
	const FVoxelGraphMergedNodeRef& NodeRef)
{
	VOXEL_FUNCTION_COUNTER();

	static const TVoxelMap<char, uint8> CharToComponentIndex
	{
		{'X', 0},
		{'x', 0},
		{'R', 0},
		{'r', 0},
		{'Y', 1},
		{'y', 1},
		{'G', 1},
		{'g', 1},
		{'Z', 2},
		{'z', 2},
		{'B', 2},
		{'b', 2},
		{'W', 3},
		{'w', 3},
		{'A', 3},
		{'A', 3},
	};

	static TVoxelArray<FVoxelPinType> FloatPinTypes =
	{
		FVoxelPinType::Make<FVoxelFloatBuffer>(),
		FVoxelPinType::Make<FVoxelVector2DBuffer>(),
		FVoxelPinType::Make<FVoxelVectorBuffer>(),
		FVoxelPinType::Make<FVoxelLinearColorBuffer>(),
	};

	static TVoxelArray<FVoxelPinType> Int32PinTypes =
	{
		FVoxelPinType::Make<FVoxelInt32Buffer>(),
		FVoxelPinType::Make<FVoxelIntPointBuffer>(),
		FVoxelPinType::Make<FVoxelIntVectorBuffer>(),
		FVoxelPinType::Make<FVoxelIntVector4Buffer>(),
	};

	if (Extractor.Len() > 4)
	{
		VOXEL_MESSAGE(Error, "{0}: Invalid extractor {1}, for attribute {2} with type {3}",
			NodeRef,
			Extractor,
			AttributeName,
			Buffer.GetBufferType().ToString());
		return {};
	}

	const bool bIsFloat = FVoxelTemplateNode::IsFloat(Buffer.GetBufferType());
	if (!bIsFloat &&
		!FVoxelTemplateNode::IsInt32(Buffer.GetBufferType()))
	{
		VOXEL_MESSAGE(Error, "{0}: Invalid extractor {1}, for attribute {2} with type {3}",
			NodeRef,
			Extractor,
			AttributeName,
			Buffer.GetBufferType().ToString());
		return {};
	}

	const TVoxelArray<FVoxelPinType>& PossibleTypes = bIsFloat ? FloatPinTypes : Int32PinTypes;
	TVoxelInlineArray<const FVoxelTerminalBuffer*, 4> ComponentBuffers;

	// Prepare components
	INLINE_LAMBDA
	{
#define INTERNAL_GET_COMPONENT_BUFFER(Name) ComponentBuffers.Add(&TypedBuffer->Name);
#define GET_STORAGE_COMPONENTS(Type, ...) \
		if (const Type* TypedBuffer = Buffer.As<Type>()) \
		{ \
			VOXEL_FOREACH(INTERNAL_GET_COMPONENT_BUFFER, ##__VA_ARGS__) \
			return; \
		}

		GET_STORAGE_COMPONENTS(FVoxelLinearColorBuffer, R, G, B, A);
		GET_STORAGE_COMPONENTS(FVoxelVectorBuffer, X, Y, Z);
		GET_STORAGE_COMPONENTS(FVoxelVector2DBuffer, X, Y);
		GET_STORAGE_COMPONENTS(FVoxelIntVector4Buffer, X, Y, Z, W);
		GET_STORAGE_COMPONENTS(FVoxelIntVectorBuffer, X, Y, Z);
		GET_STORAGE_COMPONENTS(FVoxelIntPointBuffer, X, Y);

#undef GET_STORAGE_COMPONENTS
#undef INTERNAL_GET_COMPONENT_BUFFER
	};

	// Select components
	TVoxelInlineArray<const FVoxelTerminalBuffer*, 8> OrderedComponents;
	for (int32 Index = 0; Index < Extractor.Len(); Index++)
	{
		const uint8* ComponentIndexPtr = CharToComponentIndex.Find(Extractor[Index]);
		if (!ComponentIndexPtr ||
			*ComponentIndexPtr >= ComponentBuffers.Num())
		{
			VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot extract {3} component {4}",
				NodeRef,
				AttributeName,
				Buffer.GetBufferType().ToString(),
				Extractor,
				Extractor.Mid(Index, 1));
			return {};
		}

		OrderedComponents.Add(ComponentBuffers[*ComponentIndexPtr]);
	}

	if (ReturnPinType.IsValid_Fast() &&
		ReturnPinType != PossibleTypes[OrderedComponents.Num() - 1])
	{
		VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot extract {3} to type {4}",
			NodeRef,
			AttributeName,
			ReturnPinType.ToString(),
			Extractor,
			PossibleTypes[OrderedComponents.Num() - 1].ToString());
		return {};
	}

#define INTERNAL_SET_STORAGE(Name) Result.Name = OrderedComponents[--TempIndex]->AsChecked<decltype(Result.Name)>();
#define APPLY_STORAGE(ResultType, ...) \
	if (OrderedComponents.Num() == PREPROCESSOR_VA_ARG_COUNT(__VA_ARGS__)) \
	{ \
		ResultType Result; \
		int32 TempIndex = PREPROCESSOR_VA_ARG_COUNT(__VA_ARGS__); \
		VOXEL_FOREACH(INTERNAL_SET_STORAGE, ##__VA_ARGS__) \
		return MakeSharedCopy(MoveTemp(Result)); \
	}

	if (bIsFloat)
	{
		if (OrderedComponents.Num() == 1)
		{
			return MakeSharedCopy(*OrderedComponents[0]);
		}
		APPLY_STORAGE(FVoxelVector2DBuffer, X, Y)
		APPLY_STORAGE(FVoxelVectorBuffer, X, Y, Z)
		APPLY_STORAGE(FVoxelLinearColorBuffer, R, G, B, A)
	}
	else
	{
		if (OrderedComponents.Num() == 1)
		{
			return MakeSharedCopy(*OrderedComponents[0]);
		}
		APPLY_STORAGE(FVoxelIntPointBuffer, X, Y)
		APPLY_STORAGE(FVoxelIntVectorBuffer, X, Y, Z)
		APPLY_STORAGE(FVoxelIntVector4Buffer, X, Y, Z, W)
	}

#undef APPLY_STORAGE
#undef INTERNAL_SET_STORAGE

	VOXEL_MESSAGE(Error, "{0}: Invalid extractor {1}, for attribute {2} with type {3}",
		NodeRef,
		Extractor,
		AttributeName,
		Buffer.GetBufferType().ToString());
	return {};
}

#define VOXEL_EXTRACT_ATTRIBUTE(ExpectedPinType, ExpectedExtractor, ...) \
	if (Extractor != ExpectedExtractor VOXEL_FOREACH_SUFFIX(&& Extractor !=, ##__VA_ARGS__)) \
	{} \
	else if ( \
		ReturnPinType.IsValid_Fast() && \
		ReturnPinType != FVoxelPinType::Make<ExpectedPinType>()) \
	{ \
		VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot extract {3} to type {4}", \
			NodeRef, \
			AttributeName, \
			ReturnPinType.ToString(), \
			Extractor, \
			FVoxelPinType::Make<ExpectedPinType>().ToString()); \
		return {}; \
	} \
	else

TSharedPtr<const FVoxelBuffer> FVoxelPointUtilities::ExtractVector(
	const TSharedRef<const FVoxelBuffer>& Buffer,
	const FString& Extractor,
	const FVoxelPinType& ReturnPinType,
	const FName AttributeName,
	const FVoxelGraphMergedNodeRef& NodeRef)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelPinType BufferType = Buffer->GetBufferType();
	if (BufferType.Is<FVoxelVectorBuffer>() ||
		BufferType.Is<FVoxelVector2DBuffer>() ||
		BufferType.Is<FVoxelIntPointBuffer>() ||
		BufferType.Is<FVoxelIntVectorBuffer>())
	{
		VOXEL_EXTRACT_ATTRIBUTE(FVoxelFloatBuffer, PCGAttributeExtractorConstants::VectorLength, PCGAttributeExtractorConstants::VectorSize)
		{
			if (BufferType.Is<FVoxelVector2DBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::Length(Buffer->AsChecked<FVoxelVector2DBuffer>()));
			}
			if (BufferType.Is<FVoxelVectorBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::Length(Buffer->AsChecked<FVoxelVectorBuffer>()));
			}
			if (BufferType.Is<FVoxelIntPointBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::Length(Buffer->AsChecked<FVoxelIntPointBuffer>()));
			}
			if (BufferType.Is<FVoxelIntVectorBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::Length(Buffer->AsChecked<FVoxelIntVectorBuffer>()));
			}

			ensure(false);
			return {};
		}
		VOXEL_EXTRACT_ATTRIBUTE(FVoxelFloatBuffer, PCGAttributeExtractorConstants::VectorSquaredLength)
		{
			if (BufferType.Is<FVoxelVector2DBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::SquaredLength(Buffer->AsChecked<FVoxelVector2DBuffer>()));
			}
			if (BufferType.Is<FVoxelVectorBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::SquaredLength(Buffer->AsChecked<FVoxelVectorBuffer>()));
			}
			if (BufferType.Is<FVoxelIntPointBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::SquaredLength(Buffer->AsChecked<FVoxelIntPointBuffer>()));
			}
			if (BufferType.Is<FVoxelIntVectorBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::SquaredLength(Buffer->AsChecked<FVoxelIntVectorBuffer>()));
			}

			ensure(false);
			return {};
		}

		if (Extractor == PCGAttributeExtractorConstants::VectorNormalized)
		{
			const int32 Dimension = FVoxelTemplateNodeUtilities::GetDimension(BufferType);

			if (ReturnPinType.IsValid_Fast())
			{
				const FVoxelPinType OutputType =
					Dimension == 3
					? FVoxelPinType::Make<FVoxelVectorBuffer>()
					: FVoxelPinType::Make<FVoxelVector2DBuffer>();

				if (ReturnPinType.IsValid_Fast() &&
					ReturnPinType != OutputType)
				{
					VOXEL_MESSAGE(Error, "{0}: Found attribute named {1} of type {2}, cannot cast to {3}",
						NodeRef,
						AttributeName,
						OutputType.ToString(),
						ReturnPinType.ToString());

					VOXEL_MESSAGE(Error, "{0}: Attribute named {1} of type {2}, cannot extract {3} to type {4}",
						NodeRef,
						AttributeName,
						ReturnPinType.ToString(),
						Extractor,
						OutputType.ToString());

					return {};
				}
			}

			if (BufferType.Is<FVoxelVector2DBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::Normalize(Buffer->AsChecked<FVoxelVector2DBuffer>()));
			}
			if (BufferType.Is<FVoxelVectorBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::Normalize(Buffer->AsChecked<FVoxelVectorBuffer>()));
			}
			if (BufferType.Is<FVoxelIntPointBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::Normalize(Buffer->AsChecked<FVoxelIntPointBuffer>()));
			}
			if (BufferType.Is<FVoxelIntVectorBuffer>())
			{
				return MakeSharedCopy(FVoxelBufferMathUtilities::Normalize(Buffer->AsChecked<FVoxelIntVectorBuffer>()));
			}

			ensure(false);
			return {};
		}
	}

	return ExtractVectorComponent(
		*Buffer,
		Extractor,
		ReturnPinType,
		AttributeName,
		NodeRef);
}

TSharedPtr<const FVoxelBuffer> FVoxelPointUtilities::ExtractQuat(
	const TSharedRef<const FVoxelQuaternionBuffer>& Buffer,
	const FString& Extractor,
	const FVoxelPinType& ReturnPinType,
	const FName AttributeName,
	const FVoxelGraphMergedNodeRef& NodeRef)
{
	VOXEL_FUNCTION_COUNTER();

	VOXEL_EXTRACT_ATTRIBUTE(FVoxelVectorBuffer, PCGAttributeExtractorConstants::RotatorUp)
	{
		return MakeSharedCopy(FVoxelBufferMathUtilities::RotateVector(*Buffer, FVector3f::UpVector));
	}

	VOXEL_EXTRACT_ATTRIBUTE(FVoxelVectorBuffer, PCGAttributeExtractorConstants::RotatorForward)
	{
		return MakeSharedCopy(FVoxelBufferMathUtilities::RotateVector(*Buffer, FVector3f::ForwardVector));
	}

	VOXEL_EXTRACT_ATTRIBUTE(FVoxelVectorBuffer, PCGAttributeExtractorConstants::RotatorRight)
	{
		return MakeSharedCopy(FVoxelBufferMathUtilities::RotateVector(*Buffer, FVector3f::RightVector));
	}

	VOXEL_EXTRACT_ATTRIBUTE(FVoxelFloatBuffer, PCGAttributeExtractorConstants::RotatorPitch)
	{
		return MakeSharedCopy(FVoxelBufferMathUtilities::MakeEulerFromQuaternion(*Buffer).X);
	}

	VOXEL_EXTRACT_ATTRIBUTE(FVoxelFloatBuffer, PCGAttributeExtractorConstants::RotatorYaw)
	{
		return MakeSharedCopy(FVoxelBufferMathUtilities::MakeEulerFromQuaternion(*Buffer).Y);
	}

	VOXEL_EXTRACT_ATTRIBUTE(FVoxelFloatBuffer, PCGAttributeExtractorConstants::RotatorRoll)
	{
		return MakeSharedCopy(FVoxelBufferMathUtilities::MakeEulerFromQuaternion(*Buffer).Z);
	}

	VOXEL_MESSAGE(Error, "{0}: Invalid extractor {1}, for attribute {2} with type {3}",
		NodeRef,
		Extractor,
		AttributeName,
		Buffer->GetBufferType().ToString());
	return {};
}

TSharedPtr<const FVoxelBuffer> FVoxelPointUtilities::ExtractTransform(
	const TSharedRef<const FVoxelTransformBuffer>& Buffer,
	const FString& Extractor,
	const FVoxelPinType& ReturnPinType,
	const FName AttributeName,
	const FVoxelGraphMergedNodeRef& NodeRef)
{
	VOXEL_FUNCTION_COUNTER();

	VOXEL_EXTRACT_ATTRIBUTE(FVoxelVectorBuffer, PCGAttributeExtractorConstants::TransformLocation)
	{
		return MakeSharedCopy(Buffer->Translation);
	}

	VOXEL_EXTRACT_ATTRIBUTE(FVoxelQuaternionBuffer, PCGAttributeExtractorConstants::TransformRotation)
	{
		return MakeSharedCopy(Buffer->Rotation);
	}

	VOXEL_EXTRACT_ATTRIBUTE(FVoxelVectorBuffer, PCGAttributeExtractorConstants::TransformScale)
	{
		return MakeSharedCopy(Buffer->Scale);
	}

	VOXEL_MESSAGE(Error, "{0}: Invalid extractor {1}, for attribute {2} with type {3}",
		NodeRef,
		Extractor,
		AttributeName,
		Buffer->GetBufferType().ToString());

	return {};
}

#undef VOXEL_EXTRACT_ATTRIBUTE

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSeedBuffer FVoxelPointUtilities::PointIdToSeed(const FVoxelPointIdBuffer& Buffer, const FVoxelSeed& Seed)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelSeedBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelPointUtilities_PointIdToSeed(
		ReinterpretCastPtr<uint64>(Buffer.GetData()),
		Seed.Seed,
		ReinterpretCastPtr<uint32>(Result.GetData()),
		Buffer.Num());

	return Result;
}