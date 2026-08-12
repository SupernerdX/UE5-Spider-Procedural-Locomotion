// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphContext.h"
#include "VoxelGraphQuery.h"
#include "VoxelCompiledGraph.h"
#include "VoxelCompiledTerminalGraph.h"

const uint32 GVoxelGraphContextTLS = FPlatformTLS::AllocTlsSlot();

FVoxelGraphContext::FVoxelGraphContext(
	const FVoxelGraphEnvironment& Environment,
	const FVoxelCompiledTerminalGraph& TerminalGraph,
	FVoxelDependencyCollector& DependencyCollector)
	: Environment(Environment)
	, TerminalGraph(TerminalGraph)
	, DependencyCollector(DependencyCollector)
	, PreviousTLS(FPlatformTLS::GetTlsValue(GVoxelGraphContextTLS))
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(Environment.RootCompiledGraph->FindTerminalGraph(TerminalGraph.TerminalGraphGuid) == &TerminalGraph);

	FPlatformTLS::SetTlsValue(GVoxelGraphContextTLS, this);

	Pages.Reserve(8);
	Pages.Emplace();
}

FVoxelGraphContext::~FVoxelGraphContext()
{
	VOXEL_FUNCTION_COUNTER();

	for (const FDeleter& Deleter : Deleters)
	{
		Deleter.Deleter(Deleter.Data);
	}

	Pages.Empty();

	check(FPlatformTLS::GetTlsValue(GVoxelGraphContextTLS) == this);
	FPlatformTLS::SetTlsValue(GVoxelGraphContextTLS, PreviousTLS);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphContext::Execute()
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_COUNTER_FNAME(Environment.Owner.GetFName());
	VOXEL_SCOPE_COUNTER_FNAME(Environment.RootCompiledGraph->Graph.GetFName());

	while (NextTask)
	{
		FVoxelGraphTask* Task = NextTask;
		NextTask = NextTask->NextTask;
		TaskToInsertAfter = nullptr;

		Task->Execute(Task->LambdaData);

		TaskPool.Add(Task);
	}
}

FVoxelGraphQueryImpl& FVoxelGraphContext::MakeQuery()
{
	FVoxelGraphQueryImpl& Query = *Allocate<FVoxelGraphQueryImpl>(
		*this,
		*Environment.RootCompiledGraph,
		TerminalGraph);

	Query.NameIndexToUniformParameter_Storage = Environment.DefaultNameIndexToUniformParameter;

	return Query;
}