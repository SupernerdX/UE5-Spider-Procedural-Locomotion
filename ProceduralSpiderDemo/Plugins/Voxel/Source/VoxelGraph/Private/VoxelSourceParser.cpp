// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSourceParser.h"
#include "VoxelNode.h"
#include "VoxelFunctionLibrary.h"
#include "Dom/JsonObject.h"
#if WITH_EDITOR
#include "SourceCodeNavigation.h"
#include "Interfaces/IPluginManager.h"
#endif

#if WITH_EDITOR
FVoxelSourceParser* GVoxelSourceParser = new FVoxelSourceParser();

void FVoxelSourceParser::Initialize()
{
	if (FVoxelUtilities::IsDevWorkflow())
	{
		FVoxelUtilities::DelayedCall([this]
		{
			SaveToDisk();
		});
	}
	else
	{
		LoadFromDiskIfNeeded();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FString FVoxelSourceParser::GetPinTooltip(const UScriptStruct* NodeStruct, const FName PinName)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);

	LoadFromDiskIfNeeded();

	while (
		NodeStruct &&
		NodeStruct != StaticStructFast<FVoxelNode>())
	{
		const TVoxelMap<FName, FString>* PinToTooltip = NodePathToPinToTooltip.Find(NodeStruct->GetStructPathName());
		if (!PinToTooltip)
		{
			GeneratePinToTooltip(NodeStruct);
			PinToTooltip = NodePathToPinToTooltip.Find(NodeStruct->GetStructPathName());
		}

		if (ensureVoxelSlow(PinToTooltip))
		{
			if (const FString* Tooltip = PinToTooltip->Find(PinName))
			{
				return *Tooltip;
			}
		}

		NodeStruct = Cast<UScriptStruct>(NodeStruct->GetSuperStruct());
	}

	return PinName.ToString();
}

FString FVoxelSourceParser::GetPropertyDefault(const UFunction* Function, const FName InPropertyName)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);

	LoadFromDiskIfNeeded();

	const FFunctionPath FunctionPath(Function);

	const TVoxelMap<FName, FString>* PropertyToDefault = FunctionPathToPropertyToDefault.Find(FunctionPath);
	if (!PropertyToDefault)
	{
		GeneratePropertyToDefault(Function);
		PropertyToDefault = FunctionPathToPropertyToDefault.Find(FunctionPath);
	}

	if (!ensureVoxelSlow(PropertyToDefault))
	{
		return {};
	}

	const FString* Default = PropertyToDefault->Find(InPropertyName);
	if (!ensureVoxelSlow(Default))
	{
		return {};
	}

	return *Default;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSourceParser::FFunctionPath::FFunctionPath(const UFunction* Function)
	: ClassPath(Function->GetOuterUClass()->GetStructPathName())
	, FunctionName(Function->GetFName())
{
}

FVoxelSourceParser::FFunctionPath::FFunctionPath(const FString& String)
{
	int32 Index;
	if (!ensure(String.FindLastChar(TEXT(':'), Index)))
	{
		return;
	}

	ClassPath = FTopLevelAssetPath(String.Left(Index));
	FunctionName = FName(String.RightChop(Index + 1));
}

FString FVoxelSourceParser::FFunctionPath::ToString() const
{
	return ClassPath.ToString() + ":" + FunctionName.ToString();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSourceParser::GeneratePinToTooltip(const UScriptStruct* NodeStruct)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelMap<FName, FString>& PinToTooltip = NodePathToPinToTooltip.Add_EnsureNew(NodeStruct->GetStructPathName());

	FString HeaderPath;
	if (!FSourceCodeNavigation::FindClassHeaderPath(NodeStruct, HeaderPath))
	{
		ensure(!FVoxelUtilities::IsDevWorkflow());
		return;
	}

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *HeaderPath))
	{
		ensure(!FVoxelUtilities::IsDevWorkflow());
		return;
	}

	// __LINE__ starts at 1
	Lines.Insert(FString(), 0);

	for (FString& Line : Lines)
	{
		Line.TrimStartAndEndInline();
	}

	const TSharedRef<FVoxelNode> NodeInstance = MakeSharedStruct<FVoxelNode>(NodeStruct);
	for (const FVoxelPin& Pin : NodeInstance->GetPins())
	{
		if (Pin.Metadata.Line == 0 ||
			Pin.Metadata.Struct != NodeStruct)
		{
			continue;
		}

		if (!ensure(Lines.IsValidIndex(Pin.Metadata.Line)))
		{
			continue;
		}

		int32 FirstLine = Pin.Metadata.Line - 1;
		while (
			Lines.IsValidIndex(FirstLine) &&
			Lines[FirstLine].StartsWith(TEXT("//")))
		{
			FirstLine--;
		}
		FirstLine++;

		if (FirstLine == Pin.Metadata.Line)
		{
			continue;
		}

		FString Comment;
		for (int32 Index = FirstLine; Index < Pin.Metadata.Line; Index++)
		{
			FString Line = Lines[Index];
			Line.RemoveFromStart(TEXT("//"));

			if (!Comment.IsEmpty())
			{
				Comment += TEXT("\n");
			}
			Comment += Line.TrimStartAndEnd();
		}

		PinToTooltip.Add_EnsureNew(Pin.Name, Comment);
	}
}

void FVoxelSourceParser::GeneratePropertyToDefault(const UFunction* Function)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelMap<FName, FString>& PropertyToDefault = FunctionPathToPropertyToDefault.Add_EnsureNew(FFunctionPath(Function));

	FString HeaderPath;
	if (!FSourceCodeNavigation::FindClassHeaderPath(Function, HeaderPath))
	{
		ensure(!FVoxelUtilities::IsDevWorkflow());
		return;
	}

	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *HeaderPath))
	{
		ensure(!FVoxelUtilities::IsDevWorkflow());
		return;
	}

	const FString FunctionName = Function->GetName();

	const auto FindStartIndex = [&](const ESearchDir::Type SearchDir)
	{
		int32 Index = -1;
		do
		{
			Index = Text.Find(
				FunctionName,
				ESearchCase::CaseSensitive,
				SearchDir,
				Index == -1 ? -1 : SearchDir == ESearchDir::FromStart
				? Index + FunctionName.Len()
				: Index - FunctionName.Len());

			if (!ensure(Index != -1))
			{
				return -1;
			}

			if (!Text.IsValidIndex(Index - 1) ||
				Text[Index - 1] != TEXT(' '))
			{
				continue;
			}

			if (!Text.IsValidIndex(Index + FunctionName.Len()) ||
				Text[Index + FunctionName.Len()] != TEXT('('))
			{
				continue;
			}

			break;
		} while (true);

		return Index;
	};

	int32 FunctionStartIndex = FindStartIndex(ESearchDir::FromStart);
	const int32 OtherFunctionStartIndex = FindStartIndex(ESearchDir::FromEnd);
	if (!ensure(FunctionStartIndex != -1) ||
		!ensure(FunctionStartIndex == OtherFunctionStartIndex))
	{
		return;
	}

	int32 FunctionEndIndex = -1;
	{
		int32 NumScopes = 0;
		for (int32 Index = FunctionStartIndex; Index < Text.Len(); Index++)
		{
			const TCHAR Char = Text[Index];
			if (Char == TEXT('('))
			{
				NumScopes++;

				if (NumScopes == 1)
				{
					FunctionStartIndex = Index;
				}
			}
			else if (Char == TEXT(')'))
			{
				NumScopes--;

				if (NumScopes == 0)
				{
					FunctionEndIndex = Index;
					break;
				}
			}
		}
	}
	if (!ensure(FunctionEndIndex != -1))
	{
		return;
	}

	ensure(Text[FunctionStartIndex] == TEXT('('));
	ensure(Text[FunctionEndIndex] == TEXT(')'));

	const FString FunctionDeclaration = Text.Mid(FunctionStartIndex + 1, FunctionEndIndex - FunctionStartIndex - 1);

	TArray<FString> Properties;
	{
		int32 NumParenthesisScopes = 0;
		int32 NumBraceScopes = 0;

		FString Property;
		for (const TCHAR Char : FunctionDeclaration)
		{
			Property += Char;

			if (Char == TEXT('('))
			{
				NumParenthesisScopes++;
			}
			else if (Char == TEXT(')'))
			{
				NumParenthesisScopes--;
				ensure(NumParenthesisScopes >= 0);
			}
			else if (Char == TEXT('{'))
			{
				NumBraceScopes++;
			}
			else if (Char == TEXT('}'))
			{
				NumBraceScopes--;
				ensure(NumBraceScopes >= 0);
			}

			if (NumParenthesisScopes > 0 ||
				NumBraceScopes > 0)
			{
				continue;
			}

			if (Char == TEXT(','))
			{
				ensure(Property.RemoveFromEnd(TEXT(",")));
				Properties.Add(Property);
				Property.Reset();
			}
		}

		if (!Property.IsEmpty())
		{
			Properties.Add(Property);
		}
		else
		{
			ensure(Properties.Num() == 0);
		}
	}

	for (FString& Property : Properties)
	{
		Property.TrimStartAndEndInline();

		if (Property.RemoveFromStart(TEXT("UPARAM")))
		{
			Property.TrimStartInline();
			if (!ensure(Property[0] == TEXT('(')))
			{
				continue;
			}

			int32 Index = 1;
			int32 NumScopes = 1;
			while (
				ensure(Property.IsValidIndex(Index)) &&
				NumScopes > 0)
			{
				if (Property[Index] == TEXT('('))
				{
					NumScopes++;
				}
				if (Property[Index] == TEXT(')'))
				{
					NumScopes--;
				}
				Index++;
			}
			Property.RemoveAt(0, Index + 1);
		}
		ensure(!Property.Contains(TEXT("UPARAM"), ESearchCase::CaseSensitive));

		Property.TrimStartInline();
		Property.RemoveFromStart(TEXT("const"));
		Property.TrimStartInline();

		// Remove type
		{
			int32 Index = 0;
			while (
				ensure(Property.IsValidIndex(Index)) &&
				(FChar::IsIdentifier(Property[Index]) || Property[Index] == TEXT(':')))
			{
				Index++;
			}
			Property.RemoveAt(0, Index);
		}

		Property.TrimStartInline();
		Property.RemoveFromStart(TEXT("&"));
		Property.TrimStartInline();

		FString PropertyName;
		{
			int32 Index = 0;
			while (
				Property.IsValidIndex(Index) &&
				FChar::IsIdentifier(Property[Index]))
			{
				PropertyName += Property[Index];
				Index++;
			}
			Property.RemoveAt(0, Index);
		}

		Property.TrimStartInline();
		Property.RemoveFromStart(TEXT("="));
		Property.TrimStartInline();

		PropertyToDefault.Add_EnsureNew(FName(PropertyName), Property);
	}

	TVoxelArray<FProperty*> FunctionProperties = GetFunctionProperties(Function).Array();
	FunctionProperties.RemoveSwap(Function->GetReturnProperty());

	ensure(PropertyToDefault.Num() == FunctionProperties.Num());
	for (const FProperty* Property : FunctionProperties)
	{
		ensure(PropertyToDefault.Contains(Property->GetFName()));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSourceParser::LoadFromDiskIfNeeded()
{
	VOXEL_FUNCTION_COUNTER();

	if (FVoxelUtilities::IsDevWorkflow())
	{
		// Built on the fly
		return;
	}

	if (bLoadedFromDisk)
	{
		return;
	}
	bLoadedFromDisk = true;

	FString JsonString;
	if (!ensureVoxelSlow(FFileHelper::LoadFileToString(JsonString, *GetJsonPath())))
	{
		return;
	}

	const TSharedPtr<FJsonObject> Json = FVoxelUtilities::StringToJson(JsonString);
	if (!ensureVoxelSlow(Json))
	{
		return;
	}

	const TSharedPtr<FJsonObject> NodePathToPinToTooltipObject = Json->GetObjectField(TEXT("NodePathToPinToTooltip"));
	const TSharedPtr<FJsonObject> FunctionPathToPropertyToDefaultObject = Json->GetObjectField(TEXT("FunctionPathToPropertyToDefault"));
	if (!ensureVoxelSlow(NodePathToPinToTooltipObject) ||
		!ensureVoxelSlow(FunctionPathToPropertyToDefaultObject))
	{
		return;
	}

	for (const auto& NodeIt : NodePathToPinToTooltipObject->Values)
	{
		if (!ensureVoxelSlow(NodeIt.Value) ||
			!ensureVoxelSlow(NodeIt.Value->AsObject()))
		{
			continue;
		}

		TVoxelMap<FName, FString>& PinToTooltip = NodePathToPinToTooltip.Add_EnsureNew(FTopLevelAssetPath(NodeIt.Key));

		for (auto& PinIt : NodeIt.Value->AsObject()->Values)
		{
			if (!ensureVoxelSlow(PinIt.Value) ||
				!ensureVoxelSlow(PinIt.Value->Type == EJson::String))
			{
				continue;
			}

			PinToTooltip.Add_EnsureNew(FName(PinIt.Key), PinIt.Value->AsString());
		}
	}

	for (const auto& FunctionIt : FunctionPathToPropertyToDefaultObject->Values)
	{
		if (!ensureVoxelSlow(FunctionIt.Value) ||
			!ensureVoxelSlow(FunctionIt.Value->AsObject()))
		{
			continue;
		}

		TVoxelMap<FName, FString>& PropertyToDefault = FunctionPathToPropertyToDefault.Add_EnsureNew(FFunctionPath(FunctionIt.Key));

		for (auto& PropertyIt : FunctionIt.Value->AsObject()->Values)
		{
			if (!ensureVoxelSlow(PropertyIt.Value) ||
				!ensureVoxelSlow(PropertyIt.Value->Type == EJson::String))
			{
				continue;
			}

			PropertyToDefault.Add_EnsureNew(FName(PropertyIt.Key), PropertyIt.Value->AsString());
		}
	}
}

void FVoxelSourceParser::SaveToDisk()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	for (const UScriptStruct* Struct : GetDerivedStructs<FVoxelNode>())
	{
		if (!NodePathToPinToTooltip.Contains(Struct->GetStructPathName()))
		{
			GeneratePinToTooltip(Struct);
		}
	}

	for (const UClass* Class : GetDerivedClasses<UVoxelFunctionLibrary>())
	{
		for (const UFunction* Function : GetClassFunctions(Class))
		{
			if (!FunctionPathToPropertyToDefault.Contains(FFunctionPath(Function)))
			{
				GeneratePropertyToDefault(Function);
			}
		}
	}

	NodePathToPinToTooltip.KeySort([](const FTopLevelAssetPath& A, const FTopLevelAssetPath& B)
	{
		return A.Compare(B) < 0;
	});
	FunctionPathToPropertyToDefault.KeySort([](const FFunctionPath& A, const FFunctionPath& B)
	{
		if (A.ClassPath != B.ClassPath)
		{
			return A.ClassPath.Compare(B.ClassPath) < 0;
		}

		return A.FunctionName.Compare(B.FunctionName) < 0;
	});

	const TSharedRef<FJsonObject> NodePathToPinToTooltipObject = MakeShared<FJsonObject>();
	const TSharedRef<FJsonObject> FunctionPathToPropertyToDefaultObject = MakeShared<FJsonObject>();

	for (const auto& NodeIt : NodePathToPinToTooltip)
	{
		const TSharedRef<FJsonObject> PinToTooltip = MakeShared<FJsonObject>();

		for (auto& PinIt : NodeIt.Value)
		{
			PinToTooltip->SetStringField(PinIt.Key.ToString(), PinIt.Value);
		}

		NodePathToPinToTooltipObject->SetObjectField(NodeIt.Key.ToString(), PinToTooltip);
	}

	for (const auto& FunctionIt : FunctionPathToPropertyToDefault)
	{
		const TSharedRef<FJsonObject> PropertyToDefault = MakeShared<FJsonObject>();

		for (auto& PropertyIt : FunctionIt.Value)
		{
			PropertyToDefault->SetStringField(PropertyIt.Key.ToString(), PropertyIt.Value);
		}

		FunctionPathToPropertyToDefaultObject->SetObjectField(FunctionIt.Key.ToString(), PropertyToDefault);
	}

	const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetObjectField(TEXT("NodePathToPinToTooltip"), NodePathToPinToTooltipObject);
	Json->SetObjectField(TEXT("FunctionPathToPropertyToDefault"), FunctionPathToPropertyToDefaultObject);

	const FString NewFile = FVoxelUtilities::JsonToString(Json, true);

	FString ExistingFile;
	FFileHelper::LoadFileToString(ExistingFile, *GetJsonPath());

	// Normalize line endings
	ExistingFile.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

	if (ExistingFile.Equals(NewFile))
	{
		return;
	}

	ensure(FFileHelper::SaveStringToFile(NewFile, *GetJsonPath()));
}

FString FVoxelSourceParser::GetJsonPath()
{
	return FVoxelUtilities::GetPlugin().GetBaseDir() / "Source" / "VoxelGraph" / "Private" / "VoxelSourceParser.json";
}
#endif