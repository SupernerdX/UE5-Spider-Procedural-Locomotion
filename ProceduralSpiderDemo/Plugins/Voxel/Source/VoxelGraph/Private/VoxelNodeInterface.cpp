// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNodeInterface.h"
#include "VoxelMessage.h"
#include "VoxelGraphQuery.h"
#include "VoxelMessageToken_GraphCallstack.h"

#if WITH_EDITOR
VOXEL_RUN_ON_STARTUP_GAME()
{
	GVoxelMessageManager->GatherCallstacks.Add([](const TSharedRef<FVoxelMessage>& Message)
	{
		const FVoxelGraphContext* Context = FVoxelGraphContext::Get();
		if (!Context)
		{
			return;
		}

		const FVoxelGraphCallstack* Callstack = Context->CurrentCallstack_EditorOnly;
		if (!ensureVoxelSlow(Callstack))
		{
			return;
		}

		const TSharedRef<FVoxelMessageToken_GraphCallstack> Token = MakeShared<FVoxelMessageToken_GraphCallstack>();
		Token->Callstack = MakeShared<FVoxelGraphSharedCallstack>(*Callstack);
		Token->Message = MakeSharedCopy(*Message);

		Message->AddText(" ");
		Message->AddToken(Token);

		if (!Context->Environment.Owner.IsExplicitlyNull())
		{
			Message->AddText(" ");
			Message->AddToken(FVoxelMessageTokenFactory::CreateObjectToken(Context->Environment.Owner));
		}
	});
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void IVoxelNodeInterface::RaiseBufferErrorStatic(const FVoxelGraphMergedNodeRef& Node)
{
	VOXEL_MESSAGE(Error, "{0}: Inputs have different buffer sizes", Node);
}