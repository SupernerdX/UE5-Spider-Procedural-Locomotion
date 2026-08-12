// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

#if WITH_EDITOR
class VOXELGRAPH_API FVoxelSourceParser : public FVoxelSingleton
{
public:
	//~ Begin FVoxelSingleton Interface
	virtual void Initialize() override;
	//~ End FVoxelSingleton Interface

	FString GetPinTooltip(const UScriptStruct* NodeStruct, FName PinName);
	FString GetPropertyDefault(const UFunction* Function, FName PropertyName);

private:
	struct FFunctionPath
	{
		FTopLevelAssetPath ClassPath;
		FName FunctionName;

		FFunctionPath() = default;
		explicit FFunctionPath(const UFunction* Function);
		explicit FFunctionPath(const FString& String);

		FString ToString() const;

		bool operator==(const FFunctionPath& Other) const
		{
			return
				ClassPath == Other.ClassPath &&
				FunctionName == Other.FunctionName;
		}
		friend FArchive& operator<<(FArchive& Ar, FFunctionPath& Path)
		{
			Ar << Path.ClassPath;
			Ar << Path.FunctionName;
			return Ar;
		}
		friend uint32 GetTypeHash(const FFunctionPath& Path)
		{
			return HashCombineFast(GetTypeHash(Path.ClassPath), GetTypeHash(Path.FunctionName));
		}
	};

	TVoxelMap<FTopLevelAssetPath, TVoxelMap<FName, FString>> NodePathToPinToTooltip;
	TVoxelMap<FFunctionPath, TVoxelMap<FName, FString>> FunctionPathToPropertyToDefault;

	void GeneratePinToTooltip(const UScriptStruct* NodeStruct);
	void GeneratePropertyToDefault(const UFunction* Function);

private:
	bool bLoadedFromDisk = false;
	FVoxelCriticalSection CriticalSection;

	void LoadFromDiskIfNeeded();
	void SaveToDisk();

	static FString GetJsonPath();
};
extern VOXELGRAPH_API FVoxelSourceParser* GVoxelSourceParser;
#endif