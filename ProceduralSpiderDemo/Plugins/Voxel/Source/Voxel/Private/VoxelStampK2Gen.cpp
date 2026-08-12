// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.h"
#include "VoxelStamp.h"
#if WITH_EDITOR
#include "VoxelHeaderGenerator.h"
#include "SourceCodeNavigation.h"
#include "VoxelStampBlueprintFunctionLibrary.h"
#endif

#if WITH_EDITOR
VOXEL_RUN_ON_STARTUP_GAME()
{
	if (!FVoxelUtilities::IsDevWorkflow())
	{
		return;
	}

	TArray<UScriptStruct*> AllStructs = GetDerivedStructs<FVoxelStamp>(false);
	AllStructs.RemoveAll([](const UScriptStruct* Struct)
	{
		return false;
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

	TVoxelMap<FString, FString> StampRefToPath;
	for (const UScriptStruct* Struct : AllStructs)
	{
		FString HeaderName;
		FString Path;
		if (!FVoxelHeaderGenerator::GetHeaderName(Struct, HeaderName) ||
			!FVoxelHeaderGenerator::GetPath(Struct, Path))
		{
			continue;
		}

		Path.RemoveFromEnd(".h");
		StampRefToPath.Add_EnsureNew(Struct->GetStructCPPName() + "Ref", Path + "Ref" + ".h");
	}

	TVoxelMap<FString, FVoxelHeaderGenerator> HeaderToFile;
	for (const UScriptStruct* Struct : AllStructs)
	{
		FString HeaderName;
		if (!FVoxelHeaderGenerator::GetHeaderName(Struct, HeaderName))
		{
			continue;
		}

		const bool bIsFinal = Struct->GetSuperStruct() != FVoxelStamp::StaticStruct();

		const FString Name = Struct->GetName();
		const FString DisplayName = Struct->GetDisplayNameText().ToString();

		FString NameWithoutVoxel = Name;
		NameWithoutVoxel.RemoveFromStart("Voxel");

		const FString DisplayNameWithoutVoxel = FName::NameToDisplayString(NameWithoutVoxel, false);
		const FString Category = "Voxel|Stamp|" + DisplayNameWithoutVoxel.Replace(TEXT(" Stamp"), TEXT(""));
		const FString InstancedCategory = "Voxel|Instanced Stamp|" + DisplayNameWithoutVoxel.Replace(TEXT(" Stamp"), TEXT(""));

		FVoxelHeaderGenerator& StampRefFile = HeaderToFile.FindOrAdd_WithDefault(HeaderName + "Ref", FVoxelHeaderGenerator(HeaderName + "Ref", Struct));
		if (bIsFinal)
		{
			FString* Path = StampRefToPath.Find("F" + Struct->GetSuperStruct()->GetName() + "Ref");
			if (!ensure(Path))
			{
				continue;
			}

			StampRefFile.AddInclude(*Path);
		}
		else
		{
			StampRefFile.AddInclude<FVoxelStampRef>();
		}

		StampRefFile.AddInclude(Struct);

		{
			FVoxelHeaderObject& Object = StampRefFile.AddStruct(Struct->GetName() + "Ref", true);
			Object.SetFinal(bIsFinal);
			Object.AddMetadata(false, "BlueprintType");
			Object.AddMetadata(false, "DisplayName", DisplayName);
			Object.AddMetadata(false, "Category", Category);
			Object.AddMetadata(true, "HasNativeMake", Struct->GetOutermost()->GetName() + "." + Name + "_K2.Make");
			Object.AddMetadata(true, "HasNativeBreak", Struct->GetOutermost()->GetName() + "." + Name + "_K2.Break");
			Object.AddParent("F" + Struct->GetSuperStruct()->GetName() + "Ref");
			Object += FString(bIsFinal ? "GENERATED_VOXEL_STAMP_REF_BODY" : "GENERATED_VOXEL_STAMP_REF_PARENT_BODY") + "(F" + Struct->GetName() + "Ref, " + Struct->GetStructCPPName() + ")";

			FVoxelHeaderObject& TraitsObject = StampRefFile.AddStruct("TStructOpsTypeTraits<" + Struct->GetStructCPPName() + "Ref" + ">", false);
			TraitsObject.AddTemplate("");
			TraitsObject.AddParent("TStructOpsTypeTraits<FVoxelStampRef>");

			FVoxelHeaderObject& ImplObject = StampRefFile.AddStruct("TVoxelStampRefImpl<" + Struct->GetStructCPPName() + ">", false);
			ImplObject.AddTemplate("");
			ImplObject += "using Type = " + Struct->GetStructCPPName() + "Ref;";
		}

		{
			FString InstancedStampName = Struct->GetName();
			ensure(InstancedStampName.RemoveFromEnd("Stamp"));
			InstancedStampName += "InstancedStamp";

			FString InstancedParentStampName = Struct->GetSuperStruct()->GetName();
			ensure(InstancedParentStampName.RemoveFromEnd("Stamp"));
			InstancedParentStampName += "InstancedStamp";

			FVoxelHeaderObject& Object = StampRefFile.AddStruct(InstancedStampName + "Ref", true);
			Object.SetFinal(bIsFinal);
			// Object.AddMetadata(false, "BlueprintType");
			// Object.AddMetadata(false, "DisplayName", DisplayName);
			// Object.AddMetadata(false, "Category", InstancedCategory);
			// Object.AddMetadata(true, "HasNativeMake", Struct->GetOutermost()->GetName() + "." + Name + "_K2.Make");
			// Object.AddMetadata(true, "HasNativeBreak", Struct->GetOutermost()->GetName() + "." + Name + "_K2.Break");
			Object.AddParent("F" + InstancedParentStampName + "Ref");
			Object += FString(bIsFinal ? "GENERATED_VOXEL_STAMP_REF_BODY" : "GENERATED_VOXEL_STAMP_REF_PARENT_BODY") + "(F" + InstancedStampName + "Ref, " + Struct->GetStructCPPName() + ")";

			FVoxelHeaderObject& TraitsObject = StampRefFile.AddStruct("TStructOpsTypeTraits<F" + InstancedStampName + "Ref" + ">", false);
			TraitsObject.AddTemplate("");
			TraitsObject.AddParent("TStructOpsTypeTraits<FVoxelInstancedStampRef>");

			FVoxelHeaderObject& ImplObject = StampRefFile.AddStruct("TVoxelInstancedStampRefImpl<" + Struct->GetStructCPPName() + ">", false);
			ImplObject.AddTemplate("");
			ImplObject += "using Type = F" + InstancedStampName + "Ref;";
		}

		{
			FVoxelHeaderGenerator& LibraryFile = HeaderToFile.FindOrAdd_WithDefault(HeaderName + "_K2", FVoxelHeaderGenerator(HeaderName + "_K2", Struct));

			LibraryFile.AddInclude(HeaderName + "Ref.h");

			const FSharedVoidRef Template = MakeSharedStruct(Struct);

			FVoxelHeaderObject& Object = LibraryFile.AddClass(Struct->GetName() + "_K2", true);
			Object.AddParent<UVoxelStampBlueprintFunctionLibrary>();

			{
				FVoxelHeaderFunction& Function = Object.AddFunction("CastTo" + NameWithoutVoxel);
				Function.AddMetadata(false, "BlueprintCallable");
				Function.AddMetadata(false, "Category", "Voxel|Stamp|Casting");
				Function.AddMetadata(true, "ExpandEnumAsExecs", "Result");

				Function.ReturnType("F" + Name + "Ref");

				Function.AddArgument<FVoxelStampRef>("Stamp").Const();
				Function.AddArgument<EVoxelStampCastResult>("Result").Ref();

				Function += "return CastToStampImpl<F" + Name + ">(Stamp, Result);";
			}

			{
				FVoxelHeaderFunction& Function = Object.AddFunction("MakeCopy");

				Function.AddComment("Make a copy of this stamp");
				Function.AddComment("You can then call Set XXXX on the copy without having the original stamp be modified");

				Function.AddMetadata(false, "BlueprintPure");
				Function.AddMetadata(false, "Category", Category);
				Function.AddMetadata(false, "DisplayName", "Get " + DisplayNameWithoutVoxel);

				Function.AddArgument("Stamp", "F" + Name + "Ref").Const();
				Function.AddArgument("Copy", "F" + Name + "Ref").Ref();

				Function += "Copy = F" + Name + "Ref(Stamp.MakeCopy());";
			}

			if (Struct->HasMetaData("Abstract"))
			{
				FVoxelHeaderFunction& MakeFunction = Object.AddFunction("Make");
				MakeFunction.AddMetadata(false, "BlueprintCallable");
				MakeFunction.AddMetadata(false, "BlueprintInternalUseOnly");

				MakeFunction.AddArgument("Stamp", "F" + Name + "Ref").Ref();

				MakeFunction += "Stamp = {};";

				FVoxelHeaderFunction& BreakFunction = Object.AddFunction("Break");
				BreakFunction.AddMetadata(false, "BlueprintCallable");
				BreakFunction.AddMetadata(false, "BlueprintInternalUseOnly");

				BreakFunction.AddArgument("Stamp", "F" + Name + "Ref").Const();
			}
			else if (!Struct->HasMetaData("NoK2Make"))
			{
				// Make
				{
					FVoxelHeaderFunction& Function = Object.AddFunction("Make");
					Function.AddMetadata(false, "BlueprintCallable");
					Function.AddMetadata(false, "Category", Category);
					Function.AddMetadata(false, "DisplayName", "Make " + DisplayName);
					Function.AddMetadata(true, "Keywords", "Construct");
					Function.AddMetadata(true, "Keywords", "Create");

					Function.AddArgument("Stamp", "F" + Name + "Ref").Ref();

					TArray<FString> Arguments;
					ForeachProperties(Struct, true, [&](const FProperty& Property, const FString& PropertyName)
					{
						Function.AddArgumentWithDefault(Property, &Template.Get(), nullptr, PropertyName);
						Arguments.Add(PropertyName);
					});

					{
						Function += "Stamp = F" + Name + "Ref::New();";
						for (const FString& PropertyName : Arguments)
						{
							Function += "Stamp->" + PropertyName + " = " + PropertyName + ";";
						}
					}
				}

				// Break
				{
					FVoxelHeaderFunction& Function = Object.AddFunction("Break");

					Function.AddMetadata(false, "BlueprintPure");
					Function.AddMetadata(false, "Category", Category);
					Function.AddMetadata(false, "DisplayName", "Break " + DisplayName);
					Function.AddMetadata(true, "Keywords", "Break");

					Function.AddArgument("Stamp", "F" + Name + "Ref").Const();

					TArray<TPair<FString, FString>> Arguments;
					ForeachProperties(Struct, true, [&](const FProperty& Property, const FString& PropertyName)
					{
						FVoxelHeaderFunctionArgument& Param = Function.AddArgument(Property, PropertyName);
						Param.Ref();

						Arguments.Add({ PropertyName, FVoxelUtilities::GetFunctionType(Property) });
					});

					{
						for (const TPair<FString, FString>& It : Arguments)
						{
							Function += It.Key +" = FVoxelUtilities::MakeSafe<" + It.Value + ">();";
						}

						Function += "";
						Function += "if (!Stamp.IsValid())";
						Function += "{";
						Function += "	VOXEL_MESSAGE(Error, \"Stamp is invalid\");";
						Function += "	return;";
						Function += "}";
						Function += "";

						for (const TPair<FString, FString>& It : Arguments)
						{
							Function += It.Key + " = Stamp->" + It.Key + ";";
						}
					}
				}
			}

			{
				ForeachProperties(Struct, false, [&](const FProperty& Property, const FString& PropertyName)
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

					GetFunction.AddArgument("Stamp", "F" + Name + "Ref")
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
						GetFunction += PropertyName + " = Stamp->" + PropertyName + ";";
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

					SetFunction.AddArgument("Stamp", "F" + Name + "Ref")
					.AddMetadata(false, "Required", "");
					SetFunction.AddArgument("OutStamp", "F" + Name + "Ref")
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
						SetFunction += "Stamp->" + PropertyName + " = " + PropertyName + ";";
						SetFunction += "Stamp.Update();";
					}
				});
			}
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