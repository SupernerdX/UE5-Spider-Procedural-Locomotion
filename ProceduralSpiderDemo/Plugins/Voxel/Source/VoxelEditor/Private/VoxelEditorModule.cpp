// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "Interfaces/IMainFrameModule.h"
#include "ExampleContent/SVoxelAddContentDialog.h"

class IVoxelEditorModule : public IModuleInterface
{
public:
	int32 Version = 0;

	virtual void ShowContent() = 0;
};

class FVoxelEditorModule : public IVoxelEditorModule
{
public:
	virtual void ShowContent() override
	{
		const IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		const TSharedPtr<SWindow> ParentWindow = MainFrame.GetParentWindow();

		if (!ensure(ParentWindow))
		{
			return;
		}

		FSlateApplication::Get().AddWindowAsNativeChild(SNew(SVoxelAddContentDialog), ParentWindow.ToSharedRef());
	}
};
IMPLEMENT_MODULE(FVoxelEditorModule, VoxelEditor);