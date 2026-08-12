// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.h"
#include "VoxelStamp.h"
#include "Shape/VoxelShape.h"
#include "Shape/VoxelShapeStamp.h"
#if WITH_EDITOR
#include "VoxelHeaderGenerator.h"
#include "SourceCodeNavigation.h"
#include "Shape/VoxelShapeStampRef.h"
#include "VoxelStampBlueprintFunctionLibrary.h"
#endif

#if WITH_EDITOR
VOXEL_RUN_ON_STARTUP_GAME()
{
	if (!FVoxelUtilities::IsDevWorkflow())
	{
		return;
	}

	TArray<UScriptStruct*> AllStructs = GetDerivedStructs<FVoxelShape>(false);
	AllStructs.RemoveAll([](const UScriptStruct* Struct)
	{
		return Struct->HasMetaDataHierarchical("Internal");
	});
	AllStructs.Sort([](const UScriptStruct& A, const UScriptStruct& B)
	{
		return A.GetName() < B.GetName();
	});

	{
		TVoxelMap<UScriptStruct*, FString> StructToHeader;
		for (UScriptStruct* Struct : AllStructs)
		{
			FString Header;
			if (!FSourceCodeNavigation::FindClassHeaderPath(Struct, Header))
			{
				continue;
			}

			StructToHeader.Add_EnsureNew(Struct, Header);
		}
	}

	bool bModified = false;
	ON_SCOPE_EXIT
	{
		if (bModified)
		{
			ensure(false);
			FPlatformMisc::RequestExit(true);
		}
	};

	const auto ForeachProperties = [](const UStruct* Struct, const bool bIncludeParents, auto Lambda)
	{
		for (const FProperty& Property : GetStructProperties(Struct, bIncludeParents ? EFieldIterationFlags::IncludeSuper : EFieldIterationFlags::None))
		{
			if (!Property.HasAllPropertyFlags(CPF_Edit) ||
				Property.HasMetaData("NoK2"))
			{
				continue;
			}

			if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				if (!StructProperty->Struct->GetBoolMetaDataHierarchical("BlueprintType"))
				{
					continue;
				}
			}

			FString PropertyName = Property.GetNameCPP();
			if (PropertyName == "VoxelLayers")
			{
				PropertyName = "Layers";
			}

			Lambda(Property, PropertyName);
		}
	};

	FVoxelShapeStamp ShapeStamp;
	TVoxelMap<FString, FVoxelHeaderGenerator> HeaderToFile;
	for (const UScriptStruct* Struct : AllStructs)
	{
		FString HeaderName;
		if (!FVoxelHeaderGenerator::GetHeaderName(Struct, HeaderName))
		{
			continue;
		}

		FVoxelHeaderGenerator& File = HeaderToFile.FindOrAdd_WithDefault(HeaderName, FVoxelHeaderGenerator(HeaderName + "_K2", Struct));
		
		File.AddInclude<FVoxelShapeStampRef>();
		File.AddInclude(Struct);

		FString NameWithoutVoxel = Struct->GetName();
		NameWithoutVoxel.RemoveFromStart("Voxel");
		const FString DisplayNameWithoutVoxel = FName::NameToDisplayString(NameWithoutVoxel, false);
		const FString Category = "Voxel|Stamp|Shape|" + DisplayNameWithoutVoxel.Replace(TEXT(" Shape"), TEXT(""));

		const FString StructName = Struct->GetStructCPPName();

		const TSharedRef<const FVoxelShape> Template = MakeSharedStruct<FVoxelShape>(Struct);

		FVoxelHeaderObject& Object = File.AddClass(Struct->GetName() + "_K2", true);
		Object.AddParent<UVoxelStampBlueprintFunctionLibrary>();

		{
			FVoxelHeaderFunction& Function = Object.AddFunction("Make");
			Function.AddMetadata(false, "BlueprintCallable");
			Function.AddMetadata(false, "Category", Category);
			Function.AddMetadata(false, "DisplayName", "Make " + FName::NameToDisplayString(Struct->GetName(), false) + " Stamp");
			Function.AddMetadata(true, "Keywords", "Construct");
			Function.AddMetadata(true, "Keywords", "Create");

			Function.AddArgument<FVoxelShapeStampRef>("Stamp").Ref();
			TArray<FString> ShapeArguments;
			TArray<FString> StampArguments;
			ForeachProperties(Struct, true, [&](const FProperty& Property, const FString& PropertyName)
			{
				Function.AddArgumentWithDefault(Property, &Template.Get(), nullptr, PropertyName);
				ShapeArguments.Add(PropertyName);
			});
			ForeachProperties(FVoxelShapeStamp::StaticStruct(), true, [&](const FProperty& Property, const FString& PropertyName)
			{
				Function.AddArgumentWithDefault(Property, &ShapeStamp, nullptr, PropertyName);
				StampArguments.Add(PropertyName);
			});

			{
				Function += "Stamp = FVoxelShapeStampRef::New();";

				for (const FString& PropertyName : StampArguments)
				{
					Function += "Stamp->" + PropertyName + " = " + PropertyName + ";";
				}

				Function += "";
				Function += "TVoxelInstancedStruct<" + StructName + "> Shape(" + StructName + "::StaticStruct());";

				for (const FString& PropertyName : ShapeArguments)
				{
					Function += "Shape->" + PropertyName + " = " + PropertyName + ";";
				}

				Function += "";
				Function += "Stamp->Shape = Shape;";
			}
		}

		{
			FVoxelHeaderFunction& Function = Object.AddFunction("Break");
			Function.AddMetadata(false, "BlueprintPure");
			Function.AddMetadata(false, "Category", Category);
			Function.AddMetadata(false, "DisplayName", "Break " + FName::NameToDisplayString(Struct->GetName(), false) + " Stamp");
			Function.AddMetadata(true, "Keywords", "Break");

			Function.AddArgument<FVoxelShapeStampRef>("Stamp").Ref();
			TArray<TPair<FString, FString>> ShapeArguments;
			TArray<TPair<FString, FString>> StampArguments;
			ForeachProperties(Struct, true, [&](const FProperty& Property, const FString& PropertyName)
			{
				Function.AddArgument(Property, PropertyName).Ref();
				ShapeArguments.Emplace(PropertyName, FVoxelUtilities::GetFunctionType(Property));
			});
			ForeachProperties(FVoxelShapeStamp::StaticStruct(), true, [&](const FProperty& Property, const FString& PropertyName)
			{
				Function.AddArgument(Property, PropertyName).Ref();
				StampArguments.Emplace(PropertyName, FVoxelUtilities::GetFunctionType(Property));
			});

			{
				for (const TPair<FString, FString>& It : StampArguments)
				{
					Function += It.Key + " = FVoxelUtilities::MakeSafe<" + It.Value + ">();";
				}
				for (const TPair<FString, FString>& It : ShapeArguments)
				{
					Function += It.Key + " = FVoxelUtilities::MakeSafe<" + It.Value + ">();";
				}

				Function += "";
				Function += "if (!Stamp.IsValid())";
				Function += "{";
				Function += "	VOXEL_MESSAGE(Error, \"Stamp is invalid\");";
				Function += "	return;";
				Function += "}";
				Function += "";
				Function += StructName + "* Shape = Stamp->Shape->As<" + StructName + ">();";
				Function += "if (!Shape)";
				Function += "{";
				Function += "	VOXEL_MESSAGE(Error, \"Stamp is invalid\");";
				Function += "	return;";
				Function += "}";
				Function += "";

				for (const TPair<FString, FString>& It : StampArguments)
				{
					Function += It.Key + " = Stamp->" + It.Key + ";";
				}

				for (const TPair<FString, FString>& It : ShapeArguments)
				{
					Function += It.Key + " = Shape->" + It.Key + ";";
				}
			}
		}

		{
			ForeachProperties(Struct, true, [&](const FProperty& Property, const FString& PropertyName)
			{
				FString FunctionName = PropertyName;
				if (Property.IsA<FBoolProperty>())
				{
					FunctionName.RemoveFromStart("b", ESearchCase::CaseSensitive);
				}

				const FString ToolTip = Property.GetToolTipText().ToString();

				FVoxelHeaderFunction& GetFunction = Object.AddFunction("Get" + FunctionName);

				GetFunction.AddComment("Get " + Property.GetDisplayNameText().ToString());
				if (!ToolTip.IsEmpty())
				{
					GetFunction.AddComment(ToolTip);
				}

				GetFunction.AddMetadata(false, "BlueprintPure", "");
				GetFunction.AddMetadata(false, "Category", Category);
				GetFunction.AddMetadata(false, "DisplayName", "Get " + FName::NameToDisplayString(FunctionName, false));

				GetFunction.AddArgument<FVoxelShapeStampRef>("Stamp")
				.AddMetadata(false, "Required", "");
				GetFunction.AddArgument(Property).Ref();

				{
					GetFunction += PropertyName + " = FVoxelUtilities::MakeSafe<" + FVoxelUtilities::GetFunctionType(Property) + ">();";
					GetFunction += "";
					GetFunction += "if (!Stamp.IsValid())";
					GetFunction += "{";
					GetFunction += "	VOXEL_MESSAGE(Error, \"Stamp is invalid\");";
					GetFunction += "	return;";
					GetFunction += "}";
					GetFunction += "";
					GetFunction += Struct->GetStructCPPName() + "* Shape = Stamp->Shape->As<" + Struct->GetStructCPPName() + ">();";
					GetFunction += "if (!Shape)";
					GetFunction += "{";
					GetFunction += "	VOXEL_MESSAGE(Error, \"Stamp is invalid\");";
					GetFunction += "	return;";
					GetFunction += "}";
					GetFunction += "";
					GetFunction += PropertyName + " = Shape->" + PropertyName + ";";
				}

				FVoxelHeaderFunction& SetFunction = Object.AddFunction("Set" + FunctionName);

				SetFunction.AddComment("Set " + Property.GetDisplayNameText().ToString());
				if (!ToolTip.IsEmpty())
				{
					SetFunction.AddComment(ToolTip);
					SetFunction.AddComment("");
				}
				SetFunction.AddComment("This will automatically refresh the stamp");

				SetFunction.AddMetadata(false, "BlueprintCallable", "");
				SetFunction.AddMetadata(false, "Category", Category);
				SetFunction.AddMetadata(false, "DisplayName", "Set " + FName::NameToDisplayString(FunctionName, false));

				SetFunction.AddArgument<FVoxelShapeStampRef>("Stamp")
				.AddMetadata(false, "Required", "");
				SetFunction.AddArgument<FVoxelShapeStampRef>("OutStamp")
				.AddMetadata(false, "DisplayName", "Stamp")
				.Ref();
				SetFunction.AddArgumentWithDefault(Property, &Template.Get(), nullptr);

				{
					SetFunction += "OutStamp = Stamp;";
					SetFunction += "";
					SetFunction += "if (!Stamp.IsValid())";
					SetFunction += "{";
					SetFunction += "	VOXEL_MESSAGE(Error, \"Stamp is invalid\");";
					SetFunction += "	return;";
					SetFunction += "}";
					SetFunction += "";
					SetFunction += Struct->GetStructCPPName() + "* Shape = Stamp->Shape->As<" + Struct->GetStructCPPName() + ">();";
					SetFunction += "if (!Shape)";
					SetFunction += "{";
					SetFunction += "	VOXEL_MESSAGE(Error, \"Stamp is invalid\");";
					SetFunction += "	return;";
					SetFunction += "}";
					SetFunction += "";
					SetFunction += "Shape->" + PropertyName + " = " + PropertyName + ";";
					SetFunction += "Stamp.Update();";
				}
			});
		}
	}

	for (const auto& It : HeaderToFile)
	{
		if (It.Value.CreateFile())
		{
			bModified = true;
		}
	}
}
#endif