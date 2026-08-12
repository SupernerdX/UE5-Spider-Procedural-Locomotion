// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelTemplateNode.h"
#include "VoxelObjectPinType.h"
#include "VoxelCompilationGraph.h"
#include "Nodes/VoxelNode_UFunction.h"
#include "FunctionLibrary/VoxelMathFunctionLibrary.h"
#include "FunctionLibrary/VoxelAutocastFunctionLibrary.h"

FVoxelTemplateNodeContext* GVoxelTemplateNodeContext = nullptr;

void FVoxelTemplateNodeUtilities::SetPinDimension(FVoxelPin& Pin, const int32 Dimension)
{
	Pin.SetType(GetVectorType(Pin.GetType(), Dimension));
}

TConstVoxelArrayView<FVoxelPinType> FVoxelTemplateNodeUtilities::GetFloatTypes()
{
	static const TArray<FVoxelPinType> Types =
	{
		FVoxelPinType::Make<float>(),
		FVoxelPinType::Make<FVector2D>(),
		FVoxelPinType::Make<FVector>(),
		FVoxelPinType::Make<FLinearColor>(),

		FVoxelPinType::Make<FVoxelFloatBuffer>(),
		FVoxelPinType::Make<FVoxelVector2DBuffer>(),
		FVoxelPinType::Make<FVoxelVectorBuffer>(),
		FVoxelPinType::Make<FVoxelLinearColorBuffer>()
	};
	return Types;
}

TConstVoxelArrayView<FVoxelPinType> FVoxelTemplateNodeUtilities::GetDoubleTypes()
{
	static const TArray<FVoxelPinType> Types =
	{
		FVoxelPinType::Make<double>(),
		FVoxelPinType::Make<FVoxelDoubleVector2D>(),
		FVoxelPinType::Make<FVoxelDoubleVector>(),
		FVoxelPinType::Make<FVoxelDoubleLinearColor>(),

		FVoxelPinType::Make<FVoxelDoubleBuffer>(),
		FVoxelPinType::Make<FVoxelDoubleVector2DBuffer>(),
		FVoxelPinType::Make<FVoxelDoubleVectorBuffer>(),
		FVoxelPinType::Make<FVoxelDoubleLinearColorBuffer>()
	};
	return Types;
}

TConstVoxelArrayView<FVoxelPinType> FVoxelTemplateNodeUtilities::GetInt32Types()
{
	static const TArray<FVoxelPinType> Types =
	{
		FVoxelPinType::Make<int32>(),
		FVoxelPinType::Make<FIntPoint>(),
		FVoxelPinType::Make<FIntVector>(),
		FVoxelPinType::Make<FIntVector4>(),

		FVoxelPinType::Make<FVoxelInt32Buffer>(),
		FVoxelPinType::Make<FVoxelIntPointBuffer>(),
		FVoxelPinType::Make<FVoxelIntVectorBuffer>(),
		FVoxelPinType::Make<FVoxelIntVector4Buffer>()
	};
	return Types;
}

TConstVoxelArrayView<FVoxelPinType> FVoxelTemplateNodeUtilities::GetInt64Types()
{
	static const TArray<FVoxelPinType> Types =
	{
		FVoxelPinType::Make<int64>(),
		FVoxelPinType::Make<FInt64Point>(),
		FVoxelPinType::Make<FInt64Vector>(),
		FVoxelPinType::Make<FInt64Vector4>(),

		FVoxelPinType::Make<FVoxelInt64Buffer>(),
		FVoxelPinType::Make<FVoxelInt64PointBuffer>(),
		FVoxelPinType::Make<FVoxelInt64VectorBuffer>(),
		FVoxelPinType::Make<FVoxelInt64Vector4Buffer>()
	};
	return Types;
}

TConstVoxelArrayView<FVoxelPinType> FVoxelTemplateNodeUtilities::GetObjectTypes()
{
	static TArray<FVoxelPinType> Types;
	if (Types.Num() == 0)
	{
		for (const auto& It : FVoxelObjectPinType::StructToPinType())
		{
			FVoxelPinType Type = FVoxelPinType::MakeStruct(ConstCast(It.Key));
			Types.Add(Type.GetInnerType());
			Types.Add(Type.GetBufferType().WithBufferArray(false));
		}
	}
	return Types;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelTemplateNodeUtilities::IsPinBool(const FPin* Pin)
{
	return IsBool(Pin->Type);
}

bool FVoxelTemplateNodeUtilities::IsPinByte(const FPin* Pin)
{
	return IsByte(Pin->Type);
}

bool FVoxelTemplateNodeUtilities::IsPinFloat(const FPin* Pin)
{
	return IsFloat(Pin->Type);
}

bool FVoxelTemplateNodeUtilities::IsPinDouble(const FPin* Pin)
{
	return IsDouble(Pin->Type);
}

bool FVoxelTemplateNodeUtilities::IsPinInt32(const FPin* Pin)
{
	return IsInt32(Pin->Type);
}

bool FVoxelTemplateNodeUtilities::IsPinInt64(const FPin* Pin)
{
	return IsInt64(Pin->Type);
}

bool FVoxelTemplateNodeUtilities::IsPinObject(const FPin* Pin)
{
	return IsObject(Pin->Type);
}

bool FVoxelTemplateNodeUtilities::IsPinFloatOrDouble(const FPin* Pin)
{
	return
		IsPinFloat(Pin) ||
		IsPinDouble(Pin);
}

bool FVoxelTemplateNodeUtilities::IsPinInt32OrInt64(const FPin* Pin)
{
	return
		IsPinInt32(Pin) ||
		IsPinInt64(Pin);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 FVoxelTemplateNodeUtilities::GetDimension(const FVoxelPinType& PinType)
{
	if (PinType.IsBuffer())
	{
		return GetDimension(PinType.GetInnerType());
	}

	if (PinType.Is<float>() ||
		PinType.Is<double>() ||
		PinType.Is<int32>() ||
		PinType.Is<int64>() ||
		PinType.Is<bool>() ||
		PinType.Is<uint8>())
	{
		return 1;
	}

	if (PinType.Is<FVector2D>() ||
		PinType.Is<FVoxelDoubleVector2D>() ||
		PinType.Is<FIntPoint>() ||
		PinType.Is<FInt64Point>())
	{
		return 2;
	}

	if (PinType.Is<FVector>() ||
		PinType.Is<FVoxelDoubleVector>() ||
		PinType.Is<FIntVector>() ||
		PinType.Is<FInt64Vector>())
	{
		return 3;
	}

	if (PinType.Is<FLinearColor>() ||
		PinType.Is<FVoxelDoubleLinearColor>() ||
		PinType.Is<FIntVector4>() ||
		PinType.Is<FInt64Vector4>())
	{
		return 4;
	}

	if (PinType.GetInnerExposedType().IsObject())
	{
		return 1;
	}

	ensure(false);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelPinType FVoxelTemplateNodeUtilities::GetVectorType(const FVoxelPinType& PinType, const int32 Dimension)
{
	if (PinType.IsBuffer())
	{
		const FVoxelPinType Type = GetVectorType(PinType.GetInnerType(), Dimension);
		return Type.GetBufferType();
	}

	if (PinType.Is<float>() ||
		PinType.Is<FVector2D>() ||
		PinType.Is<FVector>() ||
		PinType.Is<FLinearColor>())
	{
		switch (Dimension)
		{
		default: ensure(false);
		case 1: return FVoxelPinType::Make<float>();
		case 2: return FVoxelPinType::Make<FVector2D>();
		case 3: return FVoxelPinType::Make<FVector>();
		case 4: return FVoxelPinType::Make<FLinearColor>();
		}
	}

	if (PinType.Is<double>() ||
		PinType.Is<FVoxelDoubleVector2D>() ||
		PinType.Is<FVoxelDoubleVector>() ||
		PinType.Is<FVoxelDoubleLinearColor>())
	{
		switch (Dimension)
		{
		default: ensure(false);
		case 1: return FVoxelPinType::Make<double>();
		case 2: return FVoxelPinType::Make<FVoxelDoubleVector2D>();
		case 3: return FVoxelPinType::Make<FVoxelDoubleVector>();
		case 4: return FVoxelPinType::Make<FVoxelDoubleLinearColor>();
		}
	}

	if (PinType.Is<int32>() ||
		PinType.Is<FIntPoint>() ||
		PinType.Is<FIntVector>() ||
		PinType.Is<FIntVector4>())
	{
		switch (Dimension)
		{
		default: ensure(false);
		case 1: return FVoxelPinType::Make<int32>();
		case 2: return FVoxelPinType::Make<FIntPoint>();
		case 3: return FVoxelPinType::Make<FIntVector>();
		case 4: return FVoxelPinType::Make<FIntVector4>();
		}
	}

	if (PinType.Is<int64>() ||
		PinType.Is<FInt64Point>() ||
		PinType.Is<FInt64Vector>() ||
		PinType.Is<FInt64Vector4>())
	{
		switch (Dimension)
		{
		default: ensure(false);
		case 1: return FVoxelPinType::Make<int64>();
		case 2: return FVoxelPinType::Make<FInt64Point>();
		case 3: return FVoxelPinType::Make<FInt64Vector>();
		case 4: return FVoxelPinType::Make<FInt64Vector4>();
		}
	}

	ensure(false);
	return PinType;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelNode> FVoxelTemplateNodeUtilities::GetBreakNode(const FVoxelPinType& PinType)
{
	if (PinType.IsBuffer())
	{
		TSharedPtr<FVoxelNode> Node = GetBreakNode(PinType.GetInnerType());
		if (!ensure(Node))
		{
			return nullptr;
		}
		Node->PromotePin_Runtime(Node->GetUniqueInputPin(), PinType);
		return Node;
	}

	if (PinType.Is<FVector2D>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakVector2D));
	}
	else if (PinType.Is<FVector>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakVector));
	}
	else if (PinType.Is<FLinearColor>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakLinearColor));
	}

	if (PinType.Is<FVoxelDoubleVector2D>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakDoubleVector2D));
	}
	else if (PinType.Is<FVoxelDoubleVector>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakDoubleVector));
	}
	else if (PinType.Is<FVoxelDoubleLinearColor>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakDoubleLinearColor));
	}

	if (PinType.Is<FIntPoint>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakIntPoint));
	}
	else if (PinType.Is<FIntVector>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakIntVector));
	}
	else if (PinType.Is<FIntVector4>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakIntVector4));
	}

	if (PinType.Is<FInt64Point>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakInt64Point));
	}
	else if (PinType.Is<FInt64Vector>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakInt64Vector));
	}
	else if (PinType.Is<FInt64Vector4>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, BreakInt64Vector4));
	}

	ensure(false);
	return nullptr;
}

TSharedPtr<FVoxelNode> FVoxelTemplateNodeUtilities::GetMakeNode(const FVoxelPinType& PinType)
{
	if (PinType.IsBuffer())
	{
		TSharedPtr<FVoxelNode> Node = GetMakeNode(PinType.GetInnerType());
		if (!ensure(Node))
		{
			return nullptr;
		}
		Node->PromotePin_Runtime(Node->GetUniqueOutputPin(), PinType);
		return Node;
	}

	if (PinType.Is<FVector2D>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeVector2D));
	}
	else if (PinType.Is<FVector>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeVector));
	}
	else if (PinType.Is<FLinearColor>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeLinearColor));
	}

	if (PinType.Is<FVoxelDoubleVector2D>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeDoubleVector2D));
	}
	else if (PinType.Is<FVoxelDoubleVector>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeDoubleVector));
	}
	else if (PinType.Is<FVoxelDoubleLinearColor>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeDoubleLinearColor));
	}

	if (PinType.Is<FIntPoint>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeIntPoint));
	}
	else if (PinType.Is<FIntVector>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeIntVector));
	}
	else if (PinType.Is<FIntVector4>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeIntVector4));
	}

	if (PinType.Is<FInt64Point>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeInt64Point));
	}
	else if (PinType.Is<FInt64Vector>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeInt64Vector));
	}
	else if (PinType.Is<FInt64Vector4>())
	{
		return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeInt64Vector4));
	}

	ensure(false);
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelNode> FVoxelTemplateNodeUtilities::GetConvertToFloatNode(const FPin* Pin)
{
	const int32 Dimension = GetDimension(Pin->Type);

	const TSharedPtr<FVoxelNode> Node =
		INLINE_LAMBDA -> TSharedPtr<FVoxelNode>
		{
			if (IsPinInt32(Pin))
			{
				switch (Dimension)
				{
				default: ensure(false);
				case 1:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Int32ToFloat));
				}
				case 2:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, IntPointToVector2D));
				}
				case 3:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, IntVectorToVector));
				}
				case 4:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, IntVector4ToLinearColor));
				}
				}
			}
			else if (IsPinInt64(Pin))
			{
				switch (Dimension)
				{
				default: ensure(false);
				case 1:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Int64ToFloat));
				}
				case 2:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Int64PointToVector2D));
				}
				case 3:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Int64VectorToVector));
				}
				case 4:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Int64Vector4ToLinearColor));
				}
				}
			}
			else if (IsPinDouble(Pin))
			{
				switch (Dimension)
				{
				default: ensure(false);
				case 1:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, DoubleToFloat));
				}
				case 2:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, DoubleVector2DToVector2D));
				}
				case 3:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, DoubleVectorToVector));
				}
				case 4:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, DoubleLinearColorToLinearColor));
				}
				}
			}
			else
			{
				ensure(false);
				return nullptr;
			}
		};

	if (!Node)
	{
		return nullptr;
	}

	const bool bIsBuffer = Pin->Type.IsBuffer();
	for (FVoxelPin& VoxelPin : Node->GetPins())
	{
		Node->PromotePin_Runtime(VoxelPin, bIsBuffer ? VoxelPin.GetType().GetBufferType() : VoxelPin.GetType().GetInnerType());
	}

	return Node;
}

TSharedPtr<FVoxelNode> FVoxelTemplateNodeUtilities::GetConvertToDoubleNode(const FPin* Pin)
{
	const int32 Dimension = GetDimension(Pin->Type);

	const TSharedPtr<FVoxelNode> Node =
		INLINE_LAMBDA -> TSharedPtr<FVoxelNode>
		{
			if (IsPinInt32(Pin))
			{
				switch (Dimension)
				{
				default: ensure(false);
				case 1:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Int32ToDouble));
				}
				case 2:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, IntPointToDoubleVector2D));
				}
				case 3:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, IntVectorToDoubleVector));
				}
				case 4:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, IntVector4ToDoubleLinearColor));
				}
				}
			}
			else if (IsPinInt64(Pin))
			{
				switch (Dimension)
				{
				default: ensure(false);
				case 1:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Int64ToDouble));
				}
				case 2:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Int64PointToDoubleVector2D));
				}
				case 3:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Int64VectorToDoubleVector));
				}
				case 4:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Int64Vector4ToDoubleLinearColor));
				}
				}
			}
			else if (IsPinFloat(Pin))
			{
				switch (Dimension)
				{
				default: ensure(false);
				case 1:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, FloatToDouble));
				}
				case 2:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Vector2DToDoubleVector2D));
				}
				case 3:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, VectorToDoubleVector));
				}
				case 4:
				{
					return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, LinearColorToDoubleLinearColor));
				}
				}
			}
			else
			{
				ensure(false);
				return nullptr;
			}
		};

	if (!Node)
	{
		return nullptr;
	}

	const bool bIsBuffer = Pin->Type.IsBuffer();
	for (FVoxelPin& VoxelPin : Node->GetPins())
	{
		Node->PromotePin_Runtime(VoxelPin, bIsBuffer ? VoxelPin.GetType().GetBufferType() : VoxelPin.GetType().GetInnerType());
	}

	return Node;
}

TSharedPtr<FVoxelNode> FVoxelTemplateNodeUtilities::GetConvertToInt64Node(const FPin* Pin)
{
	const int32 Dimension = GetDimension(Pin->Type);

	const TSharedPtr<FVoxelNode> Node = INLINE_LAMBDA -> TSharedPtr<FVoxelNode>
	{
		switch (Dimension)
		{
		default: ensure(false);
		case 1:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, Int32ToInt64));
		}
		case 2:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, IntPointToInt64Point));
		}
		case 3:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, IntVectorToInt64Vector));
		}
		case 4:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelAutocastFunctionLibrary, IntVector4ToInt64Vector4));
		}
		}
	};

	if (!Node)
	{
		return nullptr;
	}

	const bool bIsBuffer = Pin->Type.IsBuffer();
	for (FVoxelPin& VoxelPin : Node->GetPins())
	{
		Node->PromotePin_Runtime(VoxelPin, bIsBuffer ? VoxelPin.GetType().GetBufferType() : VoxelPin.GetType().GetInnerType());
	}

	return Node;
}

TSharedPtr<FVoxelNode> FVoxelTemplateNodeUtilities::GetMakeNode(const FPin* Pin, const int32 Dimension)
{
	ensure(Dimension > 1);

	if (IsPinFloat(Pin))
	{
		switch (Dimension)
		{
		default: ensure(false);
		case 2:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeVector2D));
		}
		case 3:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeVector));
		}
		case 4:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeLinearColor));
		}
		}
	}
	else if (IsPinDouble(Pin))
	{
		switch (Dimension)
		{
		default: ensure(false);
		case 2:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeDoubleVector2D));
		}
		case 3:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeDoubleVector));
		}
		case 4:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeDoubleLinearColor));
		}
		}
	}
	else if (IsPinInt32(Pin))
	{
		switch (Dimension)
		{
		default: ensure(false);
		case 2:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeIntPoint));
		}
		case 3:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeIntVector));
		}
		case 4:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeIntVector4));
		}
		}
	}
	else if (IsPinInt64(Pin))
	{
		switch (Dimension)
		{
		default: ensure(false);
		case 2:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeInt64Point));
		}
		case 3:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeInt64Vector));
		}
		case 4:
		{
			return FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelMathFunctionLibrary, MakeInt64Vector4));
		}
		}
	}
	else
	{
		ensure(false);
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 FVoxelTemplateNodeUtilities::GetMaxDimension(const TArray<FPin*>& Pins)
{
	int32 HighestDimension = 0;
	for (const FPin* Pin : Pins)
	{
		HighestDimension = FMath::Max(HighestDimension, GetDimension(Pin->Type));
	}

	return HighestDimension;
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNodeUtilities::GetLinkedPin(FPin* Pin)
{
	ensure(Pin->GetLinkedTo().Num() == 1);
	return &Pin->GetLinkedTo()[0];
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNodeUtilities::ConvertToByte(FPin* Pin)
{
	ensure(IsPinByte(Pin));

	if (!Pin->Type.GetInnerType().GetEnum())
	{
		return Pin;
	}

	FVoxelPinType OutputType = FVoxelPinType::Make<uint8>();
	if (Pin->Type.IsBuffer())
	{
		OutputType = OutputType.GetBufferType();
	}

	FNode& Passthrough = Pin->Node.Graph.NewNode(ENodeType::Passthrough, GetNodeRef());
	FPin& InputPin = Passthrough.NewInputPin("ConvertToByteIn", Pin->Type, FVoxelPinValue(Pin->Type));
	FPin& OutputPin = Passthrough.NewOutputPin("ConvertToByteOut", OutputType);

	Pin->MakeLinkTo(InputPin);

	return &OutputPin;
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNodeUtilities::ConvertToFloat(FPin* Pin)
{
	if (IsPinFloat(Pin))
	{
		return Pin;
	}

	if (!ensure(
		IsPinInt32(Pin) ||
		IsPinInt64(Pin) ||
		IsPinDouble(Pin)))
	{
		return Pin;
	}

	const TSharedPtr<FVoxelNode> VoxelNode = GetConvertToFloatNode(Pin);
	if (!ensure(VoxelNode))
	{
		return Pin;
	}

	FVoxelPinType ResultType = GetVectorType(FVoxelPinType::Make<float>(), GetDimension(Pin->Type));
	if (Pin->Type.IsBuffer())
	{
		ResultType = ResultType.GetBufferType();
	}

	FNode& ConvNode = Pin->Node.Graph.NewNode(GetNodeRef());
	ConvNode.SetVoxelNode(VoxelNode.ToSharedRef());

	FPin* ResultPin = nullptr;
	for (const FVoxelPin& ConvNodePin : ConvNode.GetVoxelNode().GetPins())
	{
		if (ConvNodePin.bIsInput)
		{
			FPin& InputPin = ConvNode.NewInputPin(ConvNodePin.Name, Pin->Type);
			Pin->MakeLinkTo(InputPin);
		}
		else
		{
			FPin& OutputPin = ConvNode.NewOutputPin(ConvNodePin.Name, ResultType);
			ResultPin = &OutputPin;
		}
	}
	return ResultPin;
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNodeUtilities::ConvertToDouble(FPin* Pin)
{
	if (IsPinDouble(Pin))
	{
		return Pin;
	}

	if (!ensure(
		IsPinInt32(Pin) ||
		IsPinInt64(Pin) ||
		IsPinFloat(Pin)))
	{
		return Pin;
	}

	const TSharedPtr<FVoxelNode> VoxelNode = GetConvertToDoubleNode(Pin);
	if (!ensure(VoxelNode))
	{
		return Pin;
	}

	FVoxelPinType ResultType = GetVectorType(FVoxelPinType::Make<double>(), GetDimension(Pin->Type));
	if (Pin->Type.IsBuffer())
	{
		ResultType = ResultType.GetBufferType();
	}

	FNode& ConvNode = Pin->Node.Graph.NewNode(GetNodeRef());
	ConvNode.SetVoxelNode(VoxelNode.ToSharedRef());

	FPin* ResultPin = nullptr;
	for (const FVoxelPin& ConvNodePin : ConvNode.GetVoxelNode().GetPins())
	{
		if (ConvNodePin.bIsInput)
		{
			FPin& InputPin = ConvNode.NewInputPin(ConvNodePin.Name, Pin->Type);
			Pin->MakeLinkTo(InputPin);
		}
		else
		{
			FPin& OutputPin = ConvNode.NewOutputPin(ConvNodePin.Name, ResultType);
			ResultPin = &OutputPin;
		}
	}
	return ResultPin;
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNodeUtilities::ConvertToInt64(FPin* Pin)
{
	if (IsPinInt64(Pin))
	{
		return Pin;
	}

	if (!ensure(IsPinInt32(Pin)))
	{
		return Pin;
	}

	const TSharedPtr<FVoxelNode> VoxelNode = GetConvertToInt64Node(Pin);
	if (!ensure(VoxelNode))
	{
		return Pin;
	}

	FVoxelPinType ResultType = GetVectorType(FVoxelPinType::Make<int64>(), GetDimension(Pin->Type));
	if (Pin->Type.IsBuffer())
	{
		ResultType = ResultType.GetBufferType();
	}

	FNode& ConvNode = Pin->Node.Graph.NewNode(GetNodeRef());
	ConvNode.SetVoxelNode(VoxelNode.ToSharedRef());

	FPin* ResultPin = nullptr;
	for (const FVoxelPin& ConvNodePin : ConvNode.GetVoxelNode().GetPins())
	{
		if (ConvNodePin.bIsInput)
		{
			FPin& InputPin = ConvNode.NewInputPin(ConvNodePin.Name, Pin->Type);
			Pin->MakeLinkTo(InputPin);
		}
		else
		{
			FPin& OutputPin = ConvNode.NewOutputPin(ConvNodePin.Name, ResultType);
			ResultPin = &OutputPin;
		}
	}
	return ResultPin;
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNodeUtilities::ScalarToVector(FPin* Pin, const int32 HighestDimension)
{
	if (HighestDimension == 1 ||
		GetDimension(Pin->Type) != 1)
	{
		return Pin;
	}

	const TSharedPtr<FVoxelNode> VoxelNode = GetMakeNode(Pin, HighestDimension);
	if (!ensure(VoxelNode))
	{
		return Pin;
	}

	FNode& MakeNode = CreateNode(Pin, VoxelNode.ToSharedRef());
	ensure(MakeNode.GetInputPins().Num() == HighestDimension);

	for (FPin& MakeNodePin : MakeNode.GetInputPins())
	{
		Pin->MakeLinkTo(MakeNodePin);
	}

	return &MakeNode.GetOutputPin(0);
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNodeUtilities::ZeroExpandVector(FPin* Pin, const int32 HighestDimension)
{
	const int32 PinDimension = GetDimension(Pin->Type);

	if (PinDimension == HighestDimension)
	{
		return Pin;
	}

	const TSharedPtr<FVoxelNode> VoxelBreakNode = GetBreakNode(Pin->Type);
	if (!ensure(VoxelBreakNode))
	{
		return Pin;
	}

	const TSharedPtr<FVoxelNode> VoxelMakeNode = GetMakeNode(Pin, HighestDimension);
	if (!ensure(VoxelMakeNode))
	{
		return Pin;
	}

	TArray<FPin*> ScalarPins;
	{
		FNode& BreakNode = CreateNode(Pin, VoxelBreakNode.ToSharedRef());
		ensure(BreakNode.GetOutputPins().Num() == PinDimension);

		Pin->MakeLinkTo(BreakNode.GetInputPin(0));

		for (FPin& BreakNodePin : BreakNode.GetOutputPins())
		{
			ScalarPins.Add(&BreakNodePin);
		}
	}

	FNode& Passthrough = Pin->Node.Graph.NewNode(ENodeType::Passthrough, GetNodeRef());
	Passthrough.NewInputPin("ZeroScalarInput", ScalarPins[0]->Type, FVoxelPinValue(ScalarPins[0]->Type.GetInnerType()));
	FPin* ZeroScalarPin = &Passthrough.NewOutputPin("ZeroScalarOutput", ScalarPins[0]->Type);

	FNode& MakeNode = CreateNode(Pin, VoxelMakeNode.ToSharedRef());
	ensure(MakeNode.GetInputPins().Num() == HighestDimension);

	for (int32 Index = 0; Index < MakeNode.GetInputPins().Num(); Index++)
	{
		if (ScalarPins.IsValidIndex(Index))
		{
			ScalarPins[Index]->MakeLinkTo(MakeNode.GetInputPin(Index));
		}
		else
		{
			ZeroScalarPin->MakeLinkTo(MakeNode.GetInputPin(Index));
		}
	}

	return &MakeNode.GetOutputPin(0);
}

TArray<FVoxelTemplateNodeUtilities::FPin*> FVoxelTemplateNodeUtilities::BreakVector(FPin* Pin)
{
	const int32 PinDimension = GetDimension(Pin->Type);
	if (PinDimension == 1)
	{
		return { Pin };
	}
	ensure(PinDimension > 1);

	return BreakNode(Pin, GetBreakNode(Pin->Type), PinDimension);
}

TArray<FVoxelTemplateNodeUtilities::FPin*> FVoxelTemplateNodeUtilities::BreakNode(FPin* Pin, const TSharedPtr<FVoxelNode>& BreakVoxelNode, const int32 NumExpectedOutputPins)
{
	if (!ensure(BreakVoxelNode))
	{
		return {};
	}

	TArray<FPin*> ResultPins;

	if (Pin->Type.IsBuffer())
	{
		BreakVoxelNode->PromotePin_Runtime(BreakVoxelNode->GetUniqueInputPin(), Pin->Type);
	}

	FNode& BreakNode = CreateNode(Pin, BreakVoxelNode.ToSharedRef());
	ensure(BreakNode.GetPins().Num() == NumExpectedOutputPins + 1);

	Pin->MakeLinkTo(BreakNode.GetInputPin(0));

	for (FPin& BreakNodePin : BreakNode.GetOutputPins())
	{
		ResultPins.Add(&BreakNodePin);
	}

	return ResultPins;
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNodeUtilities::MakeVector(TArray<FPin*> Pins)
{
	if (Pins.Num() == 1)
	{
		return Pins[0];
	}
	ensure(Pins.Num() > 1);

	const TSharedPtr<FVoxelNode> VoxelNode = GetMakeNode(Pins[0], Pins.Num());
	if (!ensure(VoxelNode))
	{
		return {};
	}

	FNode& MakeNode = CreateNode(Pins[0], VoxelNode.ToSharedRef());
	ensure(MakeNode.GetInputPins().Num() == Pins.Num());

	for (int32 Index = 0; Index < Pins.Num(); Index++)
	{
		Pins[Index]->MakeLinkTo(MakeNode.GetInputPin(Index));
	}

	return &MakeNode.GetOutputPin(0);
}

bool FVoxelTemplateNodeUtilities::IsPinOfName(const FPin* Pin, const TSet<FName> Names)
{
	return Names.Contains(Pin->Name);
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNodeUtilities::MakeConstant(const FNode& Node, const FVoxelPinValue& Value)
{
	FNode& Passthrough = Node.Graph.NewNode(ENodeType::Passthrough, GetNodeRef());
	Passthrough.NewInputPin("ConstantInput", Value.GetType(), Value);
	return &Passthrough.NewOutputPin("ConstantOutput", Value.GetType());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTemplateNodeUtilities::FNode& FVoxelTemplateNodeUtilities::CreateNode(const FPin* Pin, const TSharedRef<FVoxelNode>& VoxelNode)
{
	const FNode& Node = Pin->Node;

	FNode& StructNode = Node.Graph.NewNode(GetNodeRef());

	for (FVoxelPin& StructPin : VoxelNode->GetPins())
	{
		if (StructPin.IsPromotable() ||
			EnumHasAllFlags(StructPin.Flags, EVoxelPinFlags::TemplatePin))
		{
			if (Pin->Type.IsBuffer())
			{
				StructPin.SetType(StructPin.GetType().GetBufferType());
			}
			else
			{
				StructPin.SetType(StructPin.GetType().GetInnerType());
			}
		}

		if (StructPin.bIsInput)
		{
			StructNode.NewInputPin(StructPin.Name, StructPin.GetType());
		}
		else
		{
			StructNode.NewOutputPin(StructPin.Name, StructPin.GetType());
		}
	}

	StructNode.SetVoxelNode(VoxelNode);
	return StructNode;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelTemplateNodeUtilities::Any(const TArray<FPin*>& Pins, const TFunctionRef<bool(FPin*)> Lambda)
{
	for (FPin* Pin : Pins)
	{
		if (Lambda(Pin))
		{
			return true;
		}
	}

	return false;
}

bool FVoxelTemplateNodeUtilities::All(const TArray<FPin*>& Pins, const TFunctionRef<bool(FPin*)> Lambda)
{
	for (FPin* Pin : Pins)
	{
		if (!Lambda(Pin))
		{
			return false;
		}
	}

	return true;
}

TArray<FVoxelTemplateNodeUtilities::FPin*> FVoxelTemplateNodeUtilities::Filter(const TArray<FPin*>& Pins, const TFunctionRef<bool(const FPin*)> Lambda)
{
	TArray<FPin*> Result;
	for (FPin* Pin : Pins)
	{
		if (Lambda(Pin))
		{
			Result.Add(Pin);
		}
	}

	return Result;
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNodeUtilities::Reduce(TArray<FPin*> Pins, TFunctionRef<FPin*(FPin*, FPin*)> Lambda)
{
	ensure(Pins.Num() > 0);

	while (Pins.Num() > 1)
	{
		FPin* PinB = Pins.Pop(EAllowShrinking::No);
		FPin* PinA = Pins.Pop(EAllowShrinking::No);
		Pins.Add(Lambda(PinA, PinB));
	}

	return Pins[0];
}

FVoxelTemplateNodeUtilities::FPin* FVoxelTemplateNodeUtilities::Call_Single(
	const UScriptStruct* NodeStruct,
	const TArray<FPin*>& Pins,
	const TOptional<FVoxelPinType> OutputPinType)
{
	if (!ensure(Pins.Num() > 0))
	{
		return nullptr;
	}

	TArray<TArray<FPin*>> MultiPins;
	for (auto Pin : Pins)
	{
		MultiPins.Add({ Pin });
	}

	TArray<FPin*> OutputPins = Call_Multi(NodeStruct, MultiPins, OutputPinType);

	if (!ensure(OutputPins.Num() == 1))
	{
		return nullptr;
	}

	return OutputPins[0];
}

TArray<FVoxelTemplateNodeUtilities::FPin*> FVoxelTemplateNodeUtilities::Call_Multi(
	const UScriptStruct* NodeStruct,
	const TArray<TArray<FPin*>>& Pins,
	const TOptional<FVoxelPinType> OutputPinType)
{
	int32 NumDimensions = -1;

	const FNode* TargetNode = nullptr;
	for (const TArray<FPin*>& PinsArray : Pins)
	{
		if (NumDimensions == -1)
		{
			NumDimensions = PinsArray.Num();
		}
		else
		{
			ensure(NumDimensions == PinsArray.Num());
		}

		for (const FPin* Pin : PinsArray)
		{
			if (!TargetNode)
			{
				TargetNode = &Pin->Node;
				break;
			}
		}
	}

	if (!ensure(TargetNode) ||
		!ensure(NumDimensions != -1))
	{
		return {};
	}

	TArray<FPin*> ResultPins;

	for (int32 DimensionIndex = 0; DimensionIndex < NumDimensions; DimensionIndex++)
	{
		FNode& StructNode = TargetNode->Graph.NewNode(GetNodeRef());
		const TSharedRef<FVoxelNode> VoxelNode = MakeSharedStruct<FVoxelNode>(NodeStruct);

		int32 PinIndex = 0;
		for (FVoxelPin& Pin : VoxelNode->GetPins())
		{
			if (!Pin.bIsInput)
			{
				FVoxelPinType Type = Pin.GetType();
				if (Type.IsWildcard() &&
					ensure(OutputPinType.IsSet()))
				{
					Type = OutputPinType.GetValue();
					VoxelNode->PromotePin_Runtime(Pin, Type);
				}

				ResultPins.Add(&StructNode.NewOutputPin(Pin.Name, Type));
				continue;
			}

			FPin* TargetPin = nullptr;
			if (ensure(Pins.IsValidIndex(PinIndex) &&
				Pins[PinIndex].IsValidIndex(DimensionIndex)))
			{
				TargetPin = Pins[PinIndex++][DimensionIndex];

				if (Pin.GetType().IsWildcard() ||
					EnumHasAllFlags(Pin.Flags, EVoxelPinFlags::TemplatePin))
				{
					VoxelNode->PromotePin_Runtime(Pin, TargetPin->Type);
				}
			}

			FPin& NewInputPin = StructNode.NewInputPin(Pin.Name, TargetPin ? TargetPin->Type : Pin.GetType());

			if (TargetPin)
			{
				TargetPin->MakeLinkTo(NewInputPin);
			}
		}

		StructNode.SetVoxelNode(VoxelNode);
		StructNode.Check();
	}

	return ResultPins;
}

TArray<FVoxelTemplateNodeUtilities::FPin*> FVoxelTemplateNodeUtilities::Call_NoInput(
	const FNode& TargetNode,
	const UScriptStruct* NodeStruct,
	const int32 MaxDimension,
	const TOptional<FVoxelPinType>& OutputPinType)
{
	TArray<FPin*> ResultPins;

	for (int32 DimensionIndex = 0; DimensionIndex < MaxDimension; DimensionIndex++)
	{
		FNode& StructNode = TargetNode.Graph.NewNode(GetNodeRef());
		const TSharedRef<FVoxelNode> VoxelNode = MakeSharedStruct<FVoxelNode>(NodeStruct);

		for (FVoxelPin& Pin : VoxelNode->GetPins())
		{
			if (!ensure(!Pin.bIsInput))
			{
				continue;
			}

			FVoxelPinType Type = Pin.GetType();
			if (Type.IsWildcard() &&
				ensure(OutputPinType.IsSet()))
			{
				Type = OutputPinType.GetValue();
				VoxelNode->PromotePin_Runtime(Pin, Type);
			}

			ResultPins.Add(&StructNode.NewOutputPin(Pin.Name, Type));
		}

		StructNode.SetVoxelNode(VoxelNode);
		StructNode.Check();
	}

	return ResultPins;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelTemplateNode::ExpandNode(FGraph& Graph, FNode& Node) const
{
	TArray<FPin*> InputPins = Node.GetInputPins().Array();
	InputPins = Apply(InputPins, &GetLinkedPin);

	TArray<FPin*> Pins = InputPins;
	Pins.Append(Node.GetOutputPins().Array());

	TArray<FPin*> OutputPins;
	ExpandPins(Node, InputPins, Pins, OutputPins);

	ensure(Node.GetOutputPins().Num() == OutputPins.Num());

	for (int32 Index = 0; Index < OutputPins.Num(); Index++)
	{
		const FPin& SourceOutputPin = Node.GetOutputPin(Index);
		FPin& TargetOutputPin = *OutputPins[Index];

		if (!ensure(SourceOutputPin.Type == TargetOutputPin.Type))
		{
			VOXEL_MESSAGE(Error, "{0}: internal error when expanding template node", Node);
			return;
		}

		SourceOutputPin.CopyOutputPinTo(TargetOutputPin);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelArray<FName> FVoxelTemplateNode::GetAllPins() const
{
	TVoxelArray<FName> AllPins;
	for (const FVoxelPin& PinIt : GetPins())
	{
		AllPins.Add(PinIt.Name);
	}
	return AllPins;
}

int32 FVoxelTemplateNode::GetMaxDimension(const TVoxelArray<FName>& PinNames) const
{
	int32 MaxTypeDimension = 0;
	for (const FName PinName : PinNames)
	{
		const FVoxelPin& Pin = FindPinChecked(PinName);
		MaxTypeDimension = FMath::Max(MaxTypeDimension, GetDimension(Pin.GetType()));
	}
	return MaxTypeDimension;
}

void FVoxelTemplateNode::FixupWildcards(const FVoxelPinType& NewType)
{
	for (FVoxelPin& Pin : GetPins())
	{
		if (Pin.GetType().IsWildcard())
		{
			Pin.SetType(NewType);
		}
	}
}

void FVoxelTemplateNode::FixupBuffers(
	const FVoxelPinType& NewType,
	const TVoxelArray<FName>& PinNames)
{
	for (const FName PinName : PinNames)
	{
		FVoxelPin& Pin = FindPinChecked(PinName);
		if (!Pin.IsPromotable())
		{
			continue;
		}

		Pin.SetType(NewType.IsBuffer() ? Pin.GetType().GetBufferType() : Pin.GetType().GetInnerType());
	}
}

void FVoxelTemplateNode::EnforceNoPrecisionLoss(
	const FVoxelPin& InPin,
	const FVoxelPinType& NewType,
	const TVoxelArray<FName>& PinNames)
{
	ON_SCOPE_EXIT
	{
		ensure(GetDimension(GetUniqueOutputPin().GetType()) == GetMaxDimension(PinNames));
	};

	if (InPin.bIsInput)
	{
		FVoxelPin& OutputPin = GetUniqueOutputPin();

		// Output needs to have a dimension same or equal as max input
		SetPinDimension(OutputPin, GetMaxDimension(PinNames));

		bool bAnyFloat = false;
		bool bAnyDouble = false;
		bool bAnyInt64 = false;
		for (const FName PinName : PinNames)
		{
			FVoxelPin& Pin = FindPinChecked(PinName);
			if (!Pin.bIsInput)
			{
				continue;
			}

			bAnyFloat |= IsFloat(Pin.GetType());
			bAnyDouble |= IsDouble(Pin.GetType());
			bAnyInt64 |= IsInt64(Pin.GetType());
		}

		if (bAnyDouble)
		{
			// If we have a double input, output needs to be double
			SetPinScalarType<double>(OutputPin);
			return;
		}

		if (bAnyInt64)
		{
			if (bAnyFloat)
			{
				// If we have a int64 input and a float input, output needs to be double
				SetPinScalarType<double>(OutputPin);
				return;
			}
			else
			{
				// If we have a int64 input and no float/double input, output needs to be int64
				SetPinScalarType<int64>(OutputPin);
				return;
			}
		}

		if (bAnyFloat)
		{
			// If we have a float input, output needs to be float
			SetPinScalarType<float>(OutputPin);
			return;
		}

		return;
	}

	// We are setting the output type

	// Reduce the dimension of inputs if they're too big
	const int32 NewDimension = GetDimension(NewType);
	for (const FName PinName : PinNames)
	{
		FVoxelPin& Pin = FindPinChecked(PinName);
		if (GetDimension(Pin.GetType()) > NewDimension)
		{
			SetPinDimension(Pin, NewDimension);
		}
	}

	if (IsFloat(NewType))
	{
		// Convert double to float and int64 to float
		for (const FName PinName : PinNames)
		{
			FVoxelPin& Pin = FindPinChecked(PinName);
			if (IsDouble(Pin.GetType()) ||
				IsInt64(Pin.GetType()))
			{
				SetPinScalarType<float>(Pin);
			}
		}
	}

	// Convert float, double and int64 to int
	if (IsInt32(NewType))
	{
		for (const FName PinName : PinNames)
		{
			FVoxelPin& Pin = FindPinChecked(PinName);
			if (IsFloat(Pin.GetType()) ||
				IsDouble(Pin.GetType()) ||
				IsInt64(Pin.GetType()))
			{
				SetPinScalarType<int32>(Pin);
			}
		}
	}

	// Convert float and double to int64
	if (IsInt64(NewType))
	{
		for (const FName PinName : PinNames)
		{
			FVoxelPin& Pin = FindPinChecked(PinName);
			if (IsFloat(Pin.GetType()) ||
				IsDouble(Pin.GetType()))
			{
				SetPinScalarType<int64>(Pin);
			}
		}
	}
}

void FVoxelTemplateNode::EnforceSameDimensions(
	const FVoxelPin& InPin,
	const FVoxelPinType& NewType,
	const TVoxelArray<FName>& PinNames)
{
	if (!InPin.IsPromotable())
	{
		return;
	}

	const int32 NewDimension = GetDimension(NewType);
	if (NewDimension != 1)
	{
		ensure(
			NewDimension == 2 ||
			NewDimension == 3 ||
			NewDimension == 4);

		// We have a vector pin, force all other vector pins to have the same size
		for (const FName PinName : PinNames)
		{
			FVoxelPin& Pin = FindPinChecked(PinName);
			if (!Pin.IsPromotable())
			{
				continue;
			}

			const int32 PinDimension = GetDimension(Pin.GetType());
			if (PinDimension == 1)
			{
				// Scalar, allowed
				continue;
			}

			SetPinDimension(Pin, NewDimension);
		}
	}
	else
	{
		if (!InPin.bIsInput)
		{
			// If we're setting the output to be scalar, everything needs to be scalar
			for (const FName PinName : PinNames)
			{
				FVoxelPin& Pin = FindPinChecked(PinName);
				if (!Pin.IsPromotable())
				{
					continue;
				}

				SetPinDimension(Pin, 1);
			}
		}
	}

	// Fixup output
	for (const FName PinName : PinNames)
	{
		FVoxelPin& Pin = FindPinChecked(PinName);
		if (Pin.bIsInput ||
			!Pin.IsPromotable())
		{
			continue;
		}

		SetPinDimension(Pin, GetMaxDimension(GetAllPins()));
	}
}