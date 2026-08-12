// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGCreateVoxelSpline.h"
#include "VoxelPCGHelpers.h"
#include "PCGComponent.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSplineData.h"
#include "PCGManagedResource.h"
#include "Helpers/PCGHelpers.h"
#include "Helpers/PCGActorHelpers.h"
#include "VoxelStampActor.h"
#include "Spline/VoxelSplineComponent.h"
#include "Spline/VoxelHeightSplineStamp.h"
#include "Spline/VoxelVolumeSplineStamp.h"

UPCGCreateVoxelSplineSettings::UPCGCreateVoxelSplineSettings()
{
	Layer = UVoxelHeightLayer::Default();
}

#if WITH_EDITOR
void UPCGCreateVoxelSplineSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_OWN_MEMBER_NAME(bVolumeSpline))
	{
		if (!Layer ||
			Layer->IsA<UVoxelVolumeLayer>() != bVolumeSpline)
		{
			if (bVolumeSpline)
			{
				Layer = UVoxelVolumeLayer::Default();
			}
			else
			{
				Layer = UVoxelHeightLayer::Default();
			}
		}
	}
}
#endif

TArray<FPCGPinProperties> UPCGCreateVoxelSplineSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	FPCGPinProperties& InputPinProperty = PinProperties.Emplace_GetRef(
		PCGPinConstants::DefaultInputLabel,
		EPCGDataType::Spline);
	InputPinProperty.SetRequiredPin();

	PinProperties.Emplace(
		"Actor Points",
		EPCGDataType::Point,
		true,
		true,
		INVTEXT("Spline actor transforms, if not provided, will use owning actor transform"));

	return PinProperties;
}

TArray<FPCGPinProperties> UPCGCreateVoxelSplineSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Add(FPCGPinProperties{ PCGPinConstants::DefaultOutputLabel, EPCGDataType::Spline });

	return PinProperties;
}

FString UPCGCreateVoxelSplineSettings::GetAdditionalTitleInformation() const
{
	return bVolumeSpline ? "Volume Spline" : "Height Spline";
}

FPCGElementPtr UPCGCreateVoxelSplineSettings::CreateElement() const
{
	return MakeShared<FPCGCreateVoxelSplineElement>();
}

void UPCGCreateVoxelSplineSettings::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FPCGContext* FPCGCreateVoxelSplineElement::CreateContext()
{
	return new FPCGCreateVoxelSplineContext();
}

bool FPCGCreateVoxelSplineElement::CanExecuteOnlyOnMainThread(FPCGContext* Context) const
{
	return true;
}

bool FPCGCreateVoxelSplineElement::IsCacheable(const UPCGSettings* InSettings) const
{
	return false;
}

bool FPCGCreateVoxelSplineElement::PrepareDataInternal(FPCGContext* InContext) const
{
	return true;
}

bool FPCGCreateVoxelSplineElement::ExecuteInternal(FPCGContext* Context) const
{
	VOXEL_FUNCTION_COUNTER();

	const UPCGCreateVoxelSplineSettings* Settings = Context->GetInputSettings<UPCGCreateVoxelSplineSettings>();
	check(Settings);

	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData> ActorPoints = Context->InputData.GetInputsByPin("Actor Points");
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	UPCGComponent* Component = GetPCGComponent(*Context);
	if (!ensure(Component))
	{
		return true;
	}

	int32 NumActorPoints = 0;
	for (int32 PointSetIndex = 0; PointSetIndex < ActorPoints.Num(); PointSetIndex++)
	{
		NumActorPoints += Cast<UPCGPointData>(ActorPoints[PointSetIndex].Data)->GetPoints().Num();
	}

	for (int32 Index = 0; Index < Inputs.Num(); Index++)
	{
		FPCGTaggedData& Input = Inputs[Index];
		UPCGSplineData* SplineData = ConstCast(Cast<UPCGSplineData>(Input.Data));
		if (!SplineData)
		{
			PCGE_LOG(Error, GraphAndLog, INVTEXT("Invalid input data"));
			continue;
		}

		AActor* TargetActor = Context->GetTargetActor(nullptr);
		if (!TargetActor)
		{
			PCGE_LOG(Error, GraphAndLog, INVTEXT("Invalid target actor. Ensure TargetActor member is initialized when creating SplineData."));
			continue;
		}

		FTransform ActorTransform = TargetActor->GetTransform();
		if (NumActorPoints > 0)
		{
			int32 ActorPointIndex = 0;
			if (NumActorPoints > 1)
			{
				if (NumActorPoints == Inputs.Num())
				{
					ActorPointIndex = Index;
				}
				else
				{
					PCGLog::LogWarningOnGraph(FText::Format(INVTEXT("The data provided on pin '{0}' does not have a consistent size with the input index '{1}'. Will use the first one."), FText::FromString("Actor Points"), FText::AsNumber(Index)), Context);
				}
			}

			int32 PointIndex = 0;
			for (int32 PointSetIndex = 0; PointSetIndex < ActorPoints.Num(); PointSetIndex++)
			{
				const TArray<FPCGPoint>& Points = Cast<UPCGPointData>(ActorPoints[PointSetIndex].Data)->GetPoints();
				if (Points.IsValidIndex(ActorPointIndex - PointIndex))
				{
					ActorTransform = Points[ActorPointIndex - PointIndex].Transform;
					break;
				}

				PointIndex += Points.Num();
			}
		}

		FActorSpawnParameters ActorSpawnParams;
		AActor* Actor = UPCGActorHelpers::SpawnDefaultActor(
			TargetActor->GetWorld(),
			TargetActor->GetLevel(),
			AVoxelStampActor::StaticClass(),
			ActorTransform,
			ActorSpawnParams);

		if (!Actor)
		{
			PCGE_LOG(Error, GraphAndLog, INVTEXT("Failed to create actor to hold the spline"));
			continue;
		}

		UPCGManagedActors* ManagedActors = NewObject<UPCGManagedActors>(Component);

		Actor->Tags = TargetActor->Tags;
		Actor->Tags.AddUnique(PCGHelpers::DefaultPCGActorTag);
		PCGHelpers::AttachToParent(Actor, TargetActor, Settings->AttachOptions, Context);
		SplineData->TargetActor = Actor;

		ManagedActors->UE_506_SWITCH(GeneratedActors, GetMutableGeneratedActors()).Add(Actor);
		Component->AddToManagedResources(ManagedActors);

		AVoxelStampActor* StampActor = Cast<AVoxelStampActor>(Actor);
		if (!StampActor)
		{
			PCGE_LOG(Error, GraphAndLog, INVTEXT("Invalid target actor. Ensure TargetActor member is of correct class."));
			continue;
		}

		{
			UVoxelSplineComponent* SplineComponent = NewObject<UVoxelSplineComponent>(StampActor, NAME_None, RF_Transactional);
			if (!ensure(SplineComponent))
			{
				continue;
			}

			StampActor->AddInstanceComponent(SplineComponent);

			SplineComponent->SetupAttachment(Actor->GetRootComponent());
			SplineComponent->SetRelativeTransform(FTransform::Identity);
			SplineComponent->RegisterComponent();

			SplineData->ApplyTo(SplineComponent);
		}

		if (Settings->bVolumeSpline)
		{
			FVoxelVolumeSplineStamp Stamp;
			Stamp.Graph = Settings->VolumeGraph;
			Stamp.BlendMode = Settings->VolumeBlendMode;
			Stamp.Smoothness = Settings->Smoothness;
			Stamp.Priority = Settings->Priority;
			Stamp.Behavior = Settings->StampBehavior;
			Stamp.Layer = Cast<UVoxelVolumeLayer>(Settings->Layer);
			StampActor->SetStamp(Stamp);
		}
		else
		{
			FVoxelHeightSplineStamp Stamp;
			Stamp.Graph = Settings->HeightGraph;
			Stamp.BlendMode = Settings->HeightBlendMode;
			Stamp.Smoothness = Settings->Smoothness;
			Stamp.Priority = Settings->Priority;
			Stamp.Behavior = Settings->StampBehavior;
			Stamp.Layer = Cast<UVoxelHeightLayer>(Settings->Layer);
			StampActor->SetStamp(Stamp);
		}

		FPCGTaggedData& Output = Outputs.Add_GetRef(Input);
		Output.Data = SplineData;
	}

	// Pass-through settings & exclusions
	Context->OutputData.TaggedData.Append(Context->InputData.GetAllSettings());

	return true;
}