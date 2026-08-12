// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Commandlets/Commandlet.h"
#include "VoxelGenerateNodeDocsCommandlet.generated.h"

UCLASS()
class UVoxelGenerateNodeDocsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	//~ Begin UCommandlet Interface
	virtual int32 Main(const FString& Params) override;
	//~ End UCommandlet Interface

private:
	const FString GitRepoPath = PLATFORM_MAC ? "/Users/victor/ROOT/VoxelPluginDocs" : "C:/ROOT/Temp/VoxelPluginDocs";
	const FString DocsPath = GitRepoPath / "2.0p1";

	static FString RunCmd(
		const FString& CommandLine,
		const FString& WorkingDirectory,
		bool bAllowFailure = false,
		TFunction<void()> OnFailure = nullptr);

private:
	void Generate(bool bIsGatheringTexture);

	struct FNode
	{
		FString DisplayName;
		FString Url;
		TMap<FString, TSharedPtr<FNode>> Children;
	};
	TSharedPtr<FNode> RootNode;
	TMap<FString, TArray64<uint8>> PreviousFiles;
};