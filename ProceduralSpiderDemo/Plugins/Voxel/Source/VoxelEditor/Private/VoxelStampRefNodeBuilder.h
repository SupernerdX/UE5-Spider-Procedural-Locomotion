// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class FVoxelStampRefNodeBuilder
	: public IDetailCustomNodeBuilder
	, public TSharedFromThis<FVoxelStampRefNodeBuilder>
{
public:
	const TSharedRef<IPropertyHandle> StructHandle;

	explicit FVoxelStampRefNodeBuilder(const TSharedRef<IPropertyHandle>& StructProperty)
		: StructHandle(StructProperty)
	{
	}

	void Initialize();

public:
	//~ Begin IDetailCustomNodeBuilder interface
	virtual void SetOnRebuildChildren(FSimpleDelegate NewOnRebuildChildren) override;
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildBuilder) override;
	virtual void Tick(float DeltaTime) override;
	virtual bool RequiresTick() const override { return true; }
	virtual bool InitiallyCollapsed() const override { return false; }
	virtual FName GetName() const override;
	//~ End IDetailCustomNodeBuilder interface

private:
	TVoxelArray<UScriptStruct*> GetStructs() const;

	FSimpleDelegate OnRebuildChildren;
	TVoxelArray<UScriptStruct*> LastStructs;
	bool bDisableObjectCountLimit = false;
};