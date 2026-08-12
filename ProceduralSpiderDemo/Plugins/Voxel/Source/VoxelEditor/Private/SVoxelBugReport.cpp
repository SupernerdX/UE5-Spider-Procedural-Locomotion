// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelBugReport.h"
#include "HttpModule.h"
#include "PlatformHttp.h"
#include "VoxelBugReport.h"
#include "VoxelZipWriter.h"
#include "VoxelPluginVersion.h"
#include "Misc/ScopedSlowTask.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void SVoxelBugReport::Construct(
	const FArguments& Args,
	const TSharedRef<FVoxelBugReport>& InBugReport)
{
	ConstCast(BugReport) = InBugReport;

	OnCloseWindow = Args._OnCloseWindow;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SEditableTextBox)
			.HintText(INVTEXT("My bug report"))
			.Text_Lambda([this]
			{
				return FText::FromString(Name);
			})
			.OnTextChanged_Lambda([&](const FText& NewName)
			{
				Name = NewName.ToString();
			})
		]
		+ SVerticalBox::Slot()
		[
			SAssignNew(TreeView, STreeView<TSharedPtr<FPath>>)
			.TreeItemsSource(&RootPaths)
			.SelectionMode(ESelectionMode::Single)
			.OnGenerateRow_Lambda([this](const TSharedPtr<FPath>& Path, const TSharedRef<STableViewBase>& OwnerTable)
			{
				return
					SNew(STableRow<TSharedPtr<FPath>>, OwnerTable)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						[
							SNew(SBox)
							.MinDesiredWidth(150.0f)
							[
								SNew(STextBlock)
								.Text(FText::FromName(Path->Name))
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.MinDesiredWidth(75.0f)
							.Visibility(Path->Size > 0 ? EVisibility::Visible : EVisibility::Hidden)
							[
								SNew(STextBlock)
								.Text(FText::FromString(FVoxelUtilities::BytesToString(Path->Size)))
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SCheckBox)
							.Visibility(Path->Size > 0 ? EVisibility::Visible : EVisibility::Hidden)
							.IsChecked_Lambda([=, this]
							{
								return PackagesToSkip.Contains(Path->PackageName) ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
							})
							.OnCheckStateChanged_Lambda([=, this](const ECheckBoxState NewState)
							{
								if (NewState == ECheckBoxState::Checked)
								{
									PackagesToSkip.Remove(Path->PackageName);
								}
								else
								{
									PackagesToSkip.Add(Path->PackageName);
								}

								UpdateTotalSize();
							})
						]
					];
			})
			.OnGetChildren_Lambda([this](const TSharedPtr<FPath>& Path, TArray<TSharedPtr<FPath>>& OutChildren)
			{
				OutChildren = Path->NameToChild.ValueArray();
			})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.MinDesiredWidth(150.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]
					{
						return FText::FromString("Total Size: " + FVoxelUtilities::BytesToString(TotalSize));
					})
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.MinDesiredWidth(150.0f)
				[
					SNew(SButton)
					.Text(INVTEXT("Remove textures"))
					.ToolTipText(INVTEXT("Remove all textures to reduce size"))
					.OnClicked_Lambda([this]
					{
						for (const FName PackagePath : BugReport->PackagePaths)
						{
							UObject* Object = FSoftObjectPath(PackagePath.ToString()).TryLoad();
							if (Cast<UTexture>(Object))
							{
								PackagesToSkip.Add(PackagePath);
							}
						}

						UpdateTotalSize();

						return FReply::Handled();
					})
				]
			]
			+ SHorizontalBox::Slot()
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.MinDesiredWidth(150.0f)
				[
					SNew(SButton)
					.Text(INVTEXT("Submit"))
					.ToolTipText(INVTEXT("Uploads this content to submit a bug report report"))
					.OnClicked_Lambda([this]
					{
						Submit();
						return FReply::Handled();
					})
				]
			]
		]
	];

	UpdatePaths();
	UpdateTotalSize();

	const TFunction<void(const TSharedPtr<FPath>&)> Traverse = [&](const TSharedPtr<FPath>& Path)
	{
		TreeView->SetItemExpansion(Path, true);

		for (const auto& It : Path->NameToChild)
		{
			Traverse(It.Value);
		}
	};

	for (const TSharedPtr<FPath>& RootPath : RootPaths)
	{
		Traverse(RootPath);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelBugReport::FPath::Add(const TConstVoxelArrayView<FString> Path)
{
	const FName ChildName = FName(Path[0]);

	TSharedPtr<FPath>& Child = NameToChild.FindOrAdd(ChildName);
	if (!Child)
	{
		const FString PackagePath = PackageName.ToString() / ChildName.ToString();

		Child = MakeShared<FPath>();
		Child->PackageName = FName(PackageName.ToString() / ChildName.ToString());
		Child->Name = ChildName;
		Child->bIsFile = Path.Num() == 1;

		if (Child->bIsFile)
		{
			const FString DiskPath = Child->GetDiskPath();
			if (!DiskPath.IsEmpty())
			{
				Child->Size = IFileManager::Get().FileSize(*DiskPath);
			}
		}
	}

	if (Path.Num() > 1)
	{
		Child->Add(Path.RightOf(1));
	}
}

FString SVoxelBugReport::FPath::GetDiskPath() const
{
	if (!ensure(bIsFile))
	{
		return {};
	}

	FString DiskPath = PackageName.ToString();
	if (DiskPath.RemoveFromStart("/Config/"))
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectConfigDir() / DiskPath);
	}

	ensure(DiskPath.RemoveFromStart("/Game/"));

	const FString AssetPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / DiskPath + ".uasset");
	if (IFileManager::Get().FileExists(*AssetPath))
	{
		return AssetPath;
	}

	const FString MapPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / DiskPath + ".umap");
	if (IFileManager::Get().FileExists(*MapPath))
	{
		return MapPath;
	}

	return {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelBugReport::Submit()
{
	VOXEL_FUNCTION_COUNTER();

	if (Name.Len() < 4)
	{
		VOXEL_MESSAGE(Error, "Name is too short");
		return;
	}

	TVoxelArray64<uint8> ZipData;
	const TSharedRef<FVoxelZipWriter> Writer = FVoxelZipWriter::Create(ZipData);

	TVoxelArray<FString> DiskPaths;
	const TFunction<void(const TSharedPtr<FPath>&)> Traverse = [&](const TSharedPtr<FPath>& Path)
	{
		for (const auto& It : Path->NameToChild)
		{
			Traverse(It.Value);
		}

		if (PackagesToSkip.Contains(Path->PackageName) ||
			!Path->bIsFile)
		{
			return;
		}

		const FString DiskPath = Path->GetDiskPath();
		if (!ensureVoxelSlow(!DiskPath.IsEmpty()))
		{
			return;
		}

		DiskPaths.Add(DiskPath);
	};

	for (const TSharedPtr<FPath>& RootPath : RootPaths)
	{
		Traverse(RootPath);
	}

	if (DiskPaths.Num() == 0)
	{
		return;
	}

	FScopedSlowTask SlowTask(DiskPaths.Num(), INVTEXT("Creating zip..."));
	SlowTask.MakeDialog(true, true);

	for (const FString& DiskPath : DiskPaths)
	{
		SlowTask.EnterProgressFrame();

		if (SlowTask.ShouldCancel())
		{
			return;
		}

		TArray64<uint8> FileData;
		if (!ensureVoxelSlow(FFileHelper::LoadFileToArray(FileData, *DiskPath)))
		{
			VOXEL_MESSAGE(Error, "Failed to load {0}", DiskPath);
			return;
		}

		FString RelativePath = DiskPath;
		ensure(RelativePath.RemoveFromStart(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir())));

		Writer->WriteCompressed(RelativePath, FileData);
	}

	if (!ensure(Writer->Finalize()))
	{
		VOXEL_MESSAGE(Error, "Failed to create zip");
		return;
	}

	ensureVoxelSlow(FFileHelper::SaveArrayToFile(ZipData, *(FPaths::ProjectSavedDir() / "VoxelPluginBugReport.zip")));

	if (ZipData.Num() > int64(1) * 1024 * 1024 * 1024)
	{
		VOXEL_MESSAGE(Error, "Zip is too big: needs to be under 1GB, is {0}", FVoxelUtilities::BytesToString(ZipData.Num()));
		return;
	}

	const TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	const TSharedRef<FVoxelNotification> Notification = FVoxelNotification::Create("Uploading bug report");

	Notification->AddButton(
		"Cancel",
		"Cancel upload",
		[WeakRequest = MakeWeakPtr(Request)]
		{
			if (const TSharedPtr<IHttpRequest> PinnedRequest = WeakRequest.Pin())
			{
				PinnedRequest->CancelRequest();
			}
		});

	const int64 NumBytes = ZipData.Num();

	// To understand the need of the Boundary string, see https://www.w3.org/Protocols/rfc1341/7_2_Multipart.html
	const FString BoundaryLabel = "b5621670aba342069d1577a569e8ad9a-" + FString::FromInt(FMath::Rand());

	const FString BoundaryBegin =
		"\r\n--" + BoundaryLabel + "\r\n" +
		"Content-Disposition: form-data; name=\"data\"; filename=\"filename.zip\"\r\n" +
		"Content-Type: application/octet-stream\r\n\r\n";

	const FString BoundaryEnd = "\r\n--" + BoundaryLabel + "--\r\n";

	TArray<uint8> CombinedContent;
	{
		CombinedContent.Reserve(ZipData.Num() + 8192);

		{
			FTCHARToUTF8 Converted(*BoundaryBegin);
			CombinedContent.Append(reinterpret_cast<const uint8*>(Converted.Get()), Converted.Length());
		}

		CombinedContent.Append(ZipData);

		{
			FTCHARToUTF8 Converted(*BoundaryEnd);
			CombinedContent.Append(reinterpret_cast<const uint8*>(Converted.Get()), Converted.Length());
		}
	}

	Request->SetURL(FString() +
		"https://reports.voxelplugin.com/upload" +
		"?token=811qfkdjp0htrx9YbggHBYU9a4Ykf5oMOdPo8jgUoaxY71opxmH9DwWMSrIJL2op" +
		"&name=" + FPlatformHttp::UrlEncode(Name + " | " + FEngineVersion::Current().ToString(EVersionComponent::Patch) + " | " + FVoxelUtilities::GetPluginVersion().ToString_UserFacing()));

	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", FString("multipart/form-data; boundary=") + BoundaryLabel);
	Request->SetContent(MoveTemp(CombinedContent));
	Request->OnRequestProgress64().BindLambda([=](FHttpRequestPtr, const uint64 BytesSent, uint64)
	{
		Notification->SetSubText(
			FVoxelUtilities::BytesToString(BytesSent) + " of " +
			FVoxelUtilities::BytesToString(NumBytes) + " uploaded");
	});
	Request->OnProcessRequestComplete().BindLambda([=, OnCloseWindow = OnCloseWindow](FHttpRequestPtr, const FHttpResponsePtr Response, const bool bConnectedSuccessfully)
	{
		Notification->ExpireAndFadeout();

		if (!bConnectedSuccessfully ||
			!Response ||
			Response->GetResponseCode() != 200)
		{
			if (Response)
			{
				VOXEL_MESSAGE(Error, "Failed to upload zip: {0} {1}", Response->GetResponseCode(), Response->GetContentAsString());
			}
			else
			{
				VOXEL_MESSAGE(Error, "Failed to upload zip: failed to connect");
			}
			return;
		}

		const FString ContentString = Response->GetContentAsString();

		FPlatformApplicationMisc::ClipboardCopy(*ContentString);

		FMessageDialog::Open(EAppMsgType::Ok,
			FText::FromString("Upload successful!\n\nID (copied to clipboard): " + ContentString +
				"\n\nPlease share this on the Voxel Plugin discord in #support"),
			INVTEXT("Report Voxel Plugin bug"));

		OnCloseWindow.ExecuteIfBound();
	});
	Request->ProcessRequest();
}

void SVoxelBugReport::UpdatePaths()
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelSet<FString> ConfigPaths;
	IFileManager::Get().IterateDirectoryRecursively(*FPaths::ProjectConfigDir(), [&](const TCHAR* Path, const bool bIsDirectory)
	{
		if (bIsDirectory)
		{
			return true;
		}

		ConfigPaths.Add(FPaths::ConvertRelativePathToFull(Path));
		return true;
	});

	ConfigPaths.Sort();

	const TSharedRef<FPath> RootConfigPath = MakeShared<FPath>();
	RootConfigPath->PackageName = "/Config";
	RootConfigPath->Name = "Config";

	for (FString ConfigPath : ConfigPaths)
	{
		ensure(ConfigPath.RemoveFromStart(FPaths::ConvertRelativePathToFull(FPaths::ProjectConfigDir())));

		TArray<FString> Path;
		ConfigPath.ParseIntoArray(Path, TEXT("/"));

		RootConfigPath->Add(MakeVoxelArrayView(Path));
	}

	const TSharedRef<FPath> RootPath = MakeShared<FPath>();
	RootPath->PackageName = "/Game";
	RootPath->Name = "Game";

	for (const FName PackagePath : BugReport->PackagePaths)
	{
		TArray<FString> Path;
		PackagePath.ToString().ParseIntoArray(Path, TEXT("/"));

		if (!ensure(Path.Num() > 1) ||
			!ensure(Path[0] == "Game"))
		{
			continue;
		}

		RootPath->Add(MakeVoxelArrayView(Path).RightOf(1));
	}

	RootPaths = { RootConfigPath , RootPath };
}

void SVoxelBugReport::UpdateTotalSize()
{
	VOXEL_FUNCTION_COUNTER();

	TotalSize = 0;

	const TFunction<void(const TSharedPtr<FPath>&)> Traverse = [&](const TSharedPtr<FPath>& Path)
	{
		if (!PackagesToSkip.Contains(Path->PackageName) &&
			Path->Size > 0)
		{
			TotalSize += Path->Size;
		}

		for (const auto& It : Path->NameToChild)
		{
			Traverse(It.Value);
		}
	};

	for (const TSharedPtr<FPath>& RootPath : RootPaths)
	{
		Traverse(RootPath);
	}
}