// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGWaitForVoxelWorld.h"
#include "VoxelState.h"
#include "VoxelWorld.h"
#include "VoxelRuntime.h"
#include "VoxelPCGHelpers.h"
#include "PCGComponent.h"

TArray<FPCGPinProperties> UPCGWaitForVoxelWorldSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;

	Properties.Emplace_GetRef(
		PCGPinConstants::DefaultInputLabel,
		EPCGDataType::Any).SetRequiredPin();

	return Properties;
}

TArray<FPCGPinProperties> UPCGWaitForVoxelWorldSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;
	Properties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Any);
	return Properties;
}

#if VOXEL_ENGINE_VERSION >= 507
FPCGDataTypeIdentifier UPCGWaitForVoxelWorldSettings::GetCurrentPinTypesID(const UPCGPin* InPin) const
{
	check(InPin);

	// Non-dynamically-typed pins
	if (!InPin->IsOutputPin())
	{
		return InPin->Properties.AllowedTypes;
	}

	// Output pin narrows to union of inputs on first pin
	const FPCGDataTypeIdentifier InputTypeUnion = GetTypeUnionIDOfIncidentEdges(PCGPinConstants::DefaultInputLabel);
	return InputTypeUnion != EPCGDataType::None ? InputTypeUnion : FPCGDataTypeIdentifier(EPCGDataType::Any);
}
#else
EPCGDataType UPCGWaitForVoxelWorldSettings::GetCurrentPinTypes(const UPCGPin* InPin) const
{
 	check(InPin);

 	// Non-dynamically-typed pins
 	if (!InPin->IsOutputPin())
 	{
 		return InPin->Properties.AllowedTypes;
 	}

 	// Output pin narrows to union of inputs on first pin
 	const EPCGDataType InputTypeUnion = GetTypeUnionOfIncidentEdges(PCGPinConstants::DefaultInputLabel);
 	return InputTypeUnion != EPCGDataType::None ? InputTypeUnion : EPCGDataType::Any;
}
#endif

FPCGElementPtr UPCGWaitForVoxelWorldSettings::CreateElement() const
{
 	return MakeShared<FPCGWaitForVoxelWorldElement>();
}

void UPCGWaitForVoxelWorldSettings::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

FPCGContext* FPCGWaitForVoxelWorldElement::CreateContext()
{
	return new FPCGWaitForVoxelWorldContext();
}

bool FPCGWaitForVoxelWorldElement::IsCacheable(const UPCGSettings* InSettings) const
{
	return false;
}

bool FPCGWaitForVoxelWorldElement::CanExecuteOnlyOnMainThread(FPCGContext* Context) const
{
	return true;
}

bool FPCGWaitForVoxelWorldElement::PrepareDataInternal(FPCGContext* Context) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(Context->InputData.TaggedData.Num() == Context->InputData.DataCrcs.Num()))
	{
		return true;
	}

	for (int32 Index = 0; Index < Context->InputData.TaggedData.Num(); Index++)
	{
		FPCGTaggedData Data = Context->InputData.TaggedData[Index];
		Data.Pin = PCGPinConstants::DefaultOutputLabel;

		Context->OutputData.AddData(
			Data,
			Context->InputData.DataCrcs[Index]);
	}

	return true;
}

bool FPCGWaitForVoxelWorldElement::ExecuteInternal(FPCGContext* Context) const
{
	VOXEL_FUNCTION_COUNTER();

	const UPCGComponent* Component = GetPCGComponent(*Context);
	if (!ensure(Component))
	{
		return true;
	}

	UWorld* World = Component->GetWorld();
	if (!ensure(World))
	{
		return true;
	}

	for (const AVoxelWorld* VoxelWorld : TActorRange<AVoxelWorld>(World))
	{
		const TSharedPtr<FVoxelRuntime> Runtime = VoxelWorld->GetRuntime();
		if (!Runtime)
		{
			continue;
		}

		const TSharedPtr<FVoxelState> NewState = Runtime->GetNewState();
		if (!NewState ||
			// Will happen if the voxel world is waiting for PCG
			NewState->IsReadyToRender())
		{
			continue;
		}

		Context->bIsPaused = true;

		NewState->OnReadyToRender.AddLambda(MakeWeakPtrLambda(static_cast<FPCGWaitForVoxelWorldContext*>(Context)->SharedVoid, [Context]
		{
			Context->bIsPaused = false;
		}));

		return false;
	}

	return true;
}