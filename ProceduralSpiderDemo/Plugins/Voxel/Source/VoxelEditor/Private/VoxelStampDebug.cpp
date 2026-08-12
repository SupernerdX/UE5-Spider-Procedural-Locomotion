// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampDebug.h"
#include "SVoxelStampDebug.h"

void FVoxelStampDebug::Open(
	const FVoxelState& State,
	const FVector& Position)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<SWindow> Window =
		SNew(SWindow)
		.Type(EWindowType::Normal)
		.bDragAnywhere(true)
		.IsTopmostWindow(true)
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FVector2D(1280, 720))
		.SupportsTransparency(EWindowTransparency::PerPixel);

	Window->SetContent(SNew(SVoxelStampDebug, State, Position));

	const TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	if (!ensure(RootWindow))
	{
		return;
	}

	FSlateApplication::Get().AddWindowAsNativeChild(Window, RootWindow.ToSharedRef());
	Window->BringToFront();
}
