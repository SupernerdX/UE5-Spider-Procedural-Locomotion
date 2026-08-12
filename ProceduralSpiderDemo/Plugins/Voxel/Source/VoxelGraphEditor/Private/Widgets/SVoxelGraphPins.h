// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "SVoxelGraphPin.h"
#include "KismetPins/SGraphPinBool.h"
#include "KismetPins/SGraphPinClass.h"
#include "KismetPins/SGraphPinColor.h"
#include "KismetPins/SGraphPinString.h"
#include "KismetPins/SGraphPinVector.h"
#include "KismetPins/SGraphPinInteger.h"
#include "KismetPins/SGraphPinVector2D.h"

class SVoxelGraphPinBool : public SGraphPinBool
{
public:
	VOXEL_SLATE_ARGS()
	{
		
	};

	void Construct(const FArguments& Args, UEdGraphPin* InGraphPinObj)
	{
		SGraphPinBool::Construct({}, InGraphPinObj);
		SVoxelGraphPin::ConstructDebugHighlight(this);
	}

	virtual TSharedRef<SWidget> GetLabelWidget(const FName& InPinLabelStyle) override
	{
		return SVoxelGraphPin::InitializeEditableLabel(
			SharedThis(this),
			SGraphPinBool::GetLabelWidget(InPinLabelStyle));
	}
};

class SVoxelGraphPinInteger : public SGraphPinInteger
{
public:
	VOXEL_SLATE_ARGS()
	{
		
	};

	void Construct(const FArguments& Args, UEdGraphPin* InGraphPinObj)
	{
		SGraphPinInteger::Construct({}, InGraphPinObj);
		SVoxelGraphPin::ConstructDebugHighlight(this);
	}

	virtual TSharedRef<SWidget> GetLabelWidget(const FName& InPinLabelStyle) override
	{
		return SVoxelGraphPin::InitializeEditableLabel(
			SharedThis(this),
			SGraphPinInteger::GetLabelWidget(InPinLabelStyle));
	}
};

class SVoxelGraphPinString : public SGraphPinString
{
public:
	VOXEL_SLATE_ARGS()
	{
		
	};

	void Construct(const FArguments& Args, UEdGraphPin* InGraphPinObj)
	{
		SGraphPinString::Construct({}, InGraphPinObj);
		SVoxelGraphPin::ConstructDebugHighlight(this);
	}

	virtual TSharedRef<SWidget> GetLabelWidget(const FName& InPinLabelStyle) override
	{
		return SVoxelGraphPin::InitializeEditableLabel(
			SharedThis(this),
			SGraphPinString::GetLabelWidget(InPinLabelStyle));
	}
};

template <typename NumericType>
class SVoxelGraphPinVector2D : public SGraphPinVector2D<NumericType>
{
public:
	VOXEL_SLATE_ARGS()
	{
		
	};

	void Construct(const FArguments& Args, UEdGraphPin* InGraphPinObj)
	{
		SGraphPinVector2D<NumericType>::Construct({}, InGraphPinObj);
		SVoxelGraphPin::ConstructDebugHighlight(this);
	}

	virtual TSharedRef<SWidget> GetLabelWidget(const FName& InPinLabelStyle) override
	{
		return SVoxelGraphPin::InitializeEditableLabel(
			SVoxelGraphPinVector2D<NumericType>::SharedThis(this),
			SGraphPinVector2D<NumericType>::GetLabelWidget(InPinLabelStyle));
	}
};

template <typename NumericType>
class SVoxelGraphPinVector : public SGraphPinVector<NumericType>
{
public:
	VOXEL_SLATE_ARGS()
	{
		
	};

	void Construct(const FArguments& Args, UEdGraphPin* InGraphPinObj)
	{
		SGraphPinVector<NumericType>::Construct({}, InGraphPinObj);
		SVoxelGraphPin::ConstructDebugHighlight(this);
	}

	virtual TSharedRef<SWidget> GetLabelWidget(const FName& InPinLabelStyle) override
	{
		return SVoxelGraphPin::InitializeEditableLabel(
			SVoxelGraphPinVector<NumericType>::SharedThis(this),
			SGraphPinVector<NumericType>::GetLabelWidget(InPinLabelStyle));
	}
};

class SVoxelGraphPinColor : public SGraphPinColor
{
public:
	VOXEL_SLATE_ARGS()
	{
		
	};

	void Construct(const FArguments& Args, UEdGraphPin* InGraphPinObj)
	{
		SGraphPinColor::Construct({}, InGraphPinObj);
		SVoxelGraphPin::ConstructDebugHighlight(this);
	}

	virtual TSharedRef<SWidget> GetLabelWidget(const FName& InPinLabelStyle) override
	{
		return SVoxelGraphPin::InitializeEditableLabel(
			SharedThis(this),
			SGraphPinColor::GetLabelWidget(InPinLabelStyle));
	}
};

class SVoxelGraphPinClass : public SGraphPinClass
{
public:
	VOXEL_SLATE_ARGS()
	{
		
	};

	void Construct(const FArguments& Args, UEdGraphPin* InGraphPinObj)
	{
		SGraphPinClass::Construct({}, InGraphPinObj);
		SVoxelGraphPin::ConstructDebugHighlight(this);
	}

	virtual TSharedRef<SWidget> GetLabelWidget(const FName& InPinLabelStyle) override
	{
		return SVoxelGraphPin::InitializeEditableLabel(
			SharedThis(this),
			SGraphPinClass::GetLabelWidget(InPinLabelStyle));
	}
};

template <typename NumericType>
class SVoxelGraphPinNum : public SGraphPinNum<NumericType>
{
public:
	VOXEL_SLATE_ARGS()
	{
		
	};

	void Construct(const FArguments& Args, UEdGraphPin* InGraphPinObj)
	{
		SGraphPinNum<NumericType>::Construct({}, InGraphPinObj);
		SVoxelGraphPin::ConstructDebugHighlight(this);
	}

	virtual TSharedRef<SWidget> GetLabelWidget(const FName& InPinLabelStyle) override
	{
		return SVoxelGraphPin::InitializeEditableLabel(
			SVoxelGraphPinNum<NumericType>::SharedThis(this),
			SGraphPinNum<NumericType>::GetLabelWidget(InPinLabelStyle));
	}
};