// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPCGCallstack.h"
#include "VoxelPCGHelpers.h"
#include "VoxelMessageTokens.h"
#include "PCGContext.h"
#include "PCGComponent.h"

FVoxelPCGCallstack::FVoxelPCGCallstack(const FPCGContext& Context)
{
	checkUObjectAccess();

	Component = GetPCGComponent(Context);
	Node = Context.Node;

	Stack = Context.UE_506_SWITCH(Stack, GetStack()) ? *Context.UE_506_SWITCH(Stack, GetStack()) : FPCGStack();
	Stack.PushFrame(Context.Node);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint32 FVoxelMessageToken_PCGCallstack::GetHash() const
{
	return FVoxelUtilities::MurmurHashMulti(
		Callstack->Component,
		Callstack->Stack.GetCrc().GetValue());
}