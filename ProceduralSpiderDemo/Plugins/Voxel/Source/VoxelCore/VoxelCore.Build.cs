// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

global using static VoxelGlobalMethods;

using System;
using System.IO;
using UnrealBuildTool;
using System.Reflection;
using System.Diagnostics;
using System.Collections.Generic;
using Microsoft.Extensions.Logging;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

internal static class VoxelGlobalMethods
{
	public static void SetupVoxelModule(ModuleRules ModuleRules)
	{
		new VoxelConfig(ModuleRules).Setup();
	}
}

internal class VoxelConfig
{
	public readonly ModuleRules ModuleRules;
	public ReadOnlyTargetRules Target => ModuleRules.Target;
	public string PluginDirectory => ModuleRules.PluginDirectory;
	public string PluginsDirectory => Path.Combine(ModuleRules.PluginDirectory, "..");
	public readonly bool DevWorkflow;
	public readonly bool EnableDebug;

	public VoxelConfig(ModuleRules ModuleRules)
	{
		this.ModuleRules = ModuleRules;

		DevWorkflow = File.Exists(Path.Combine(PluginsDirectory, "VoxelDevWorkflow.txt"));

		if (Target.Configuration == UnrealTargetConfiguration.Debug)
		{
			EnableDebug = true;
		}
		else if (File.Exists(Path.Combine(PluginsDirectory, "VoxelDebug.txt")))
		{
			EnableDebug = true;
		}
		else if (
			Target.Configuration == UnrealTargetConfiguration.DebugGame &&
			DevWorkflow)
		{
			EnableDebug = true;
		}
		else
		{
			EnableDebug = false;
		}
	}

	public void Setup()
	{
		ModuleRules.DefaultBuildSettings = BuildSettingsVersion.Latest;
		// Critical, among other things, to have the same PCG points on client & server
		ModuleRules.FPSemantics = FPSemanticsMode.Precise;
		ModuleRules.CppStandard = CppStandardVersion.Cpp20;
		ModuleRules.bAllowConfidentialPlatformDefines = true;

		string ModuleName = ModuleRules.GetType().Name;

		if (DevWorkflow)
		{
			ModuleRules.bUseUnity =
				Target.Configuration != UnrealTargetConfiguration.Debug &&
				Target.Configuration != UnrealTargetConfiguration.DebugGame;

			if (Target.Platform != UnrealTargetPlatform.Win64 &&
			    Target.Platform != UnrealTargetPlatform.Mac)
			{
				ModuleRules.bUseUnity = true;
			}
		}
		else
		{
			// Unoptimized voxel code is _really_ slow, hurting iteration times for projects with VP as a project plugin using DebugGame
			ModuleRules.OptimizeCode = ModuleRules.CodeOptimization.Always;
			ModuleRules.bUseUnity = true;
		}

		// Always set PrivatePCHHeaderFile due to FPSemantics
		if (ModuleName.EndsWith("Editor"))
		{
			ModuleRules.PrivatePCHHeaderFile = "../VoxelCoreEditor/Public/VoxelCoreEditorPCH.h";
		}
		else
		{
			ModuleRules.PrivatePCHHeaderFile = "../VoxelCore/Public/VoxelPCH.h";
		}

		if (File.Exists(PluginDirectory + "/CheckPackaging.txt"))
		{
			// No need to optimize if we're just checking compilation
			ModuleRules.OptimizeCode = ModuleRules.CodeOptimization.Never;
		}

		ModuleRules.PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"RHI",
			"Engine",
			"InputCore",
			"RenderCore",
			"DeveloperSettings",
		});

		if (ModuleName != "VoxelCore")
		{
			ModuleRules.PublicDependencyModuleNames.Add("VoxelCore");
		}

		if (ModuleName.EndsWith("Editor"))
		{
			ModuleRules.PublicDependencyModuleNames.AddRange(new string[]
			{
				"Slate",
				"SlateCore",
				"EditorStyle",
				"PropertyEditor",
				"EditorFramework",
			});

			if (ModuleName != "VoxelCoreEditor")
			{
				ModuleRules.PublicDependencyModuleNames.Add("VoxelCoreEditor");
			}
		}

		if (Target.bBuildEditor)
		{
			ModuleRules.PublicDependencyModuleNames.Add("UnrealEd");
		}

		ModuleRules.PrivateIncludePathModuleNames.Add("DerivedDataCache");

		if (!Target.bBuildRequiresCookedData)
		{
			ModuleRules.DynamicallyLoadedModuleNames.Add("DerivedDataCache");
		}

		if (DevWorkflow &&
		    !Target.Architectures.bIsMultiArch)
		{
			var ModulePath = Path.Combine(
				PluginDirectory,
				"Intermediate",
				"Build",
				Target.Platform.ToString(),
				Target.Architecture.ToString(),
				Target.bBuildEditor ? "UnrealEditor" : "UnrealGame",
				Target.Configuration.ToString(),
				ModuleName);

			// ISPC support for R#
			if (Directory.Exists(ModulePath))
			{
				ModuleRules.PublicIncludePaths.Add(ModulePath);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

internal class VoxelISPCCompiler
{
	public readonly ModuleRules ModuleRules;
	public ReadOnlyTargetRules Target => ModuleRules.Target;

	private static bool UsePowershell => BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Win64;

	public VoxelISPCCompiler(ModuleRules ModuleRules)
	{
		this.ModuleRules = ModuleRules;
	}

	public void Compile(UnrealArch Architecture)
	{
		{
			string EnginePath = new DirectoryInfo(ModuleRules.EngineDirectory).FullName;
			string PluginPath = new DirectoryInfo(ModuleRules.PluginDirectory).FullName;

			// Normalize the paths
			EnginePath = Path.GetFullPath(EnginePath.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar) + Path.DirectorySeparatorChar);
			PluginPath = Path.GetFullPath(PluginPath);

			if (PluginPath.StartsWith(EnginePath, StringComparison.OrdinalIgnoreCase))
			{
				// Plugin is in engine, don't compile anything as we could be a marketplace install
				return;
			}
		}

		string IntermediatePath = Path.Combine(
			ModuleRules.PluginDirectory,
			"Intermediate",
			"ISPC",
			Target.Platform.ToString(),
			Architecture.ToString(),
			Target.bBuildEditor ? "UnrealEditor" : "UnrealGame",
			Target.Configuration.ToString());

		string BuildScriptPath = Path.Combine(
			IntermediatePath,
			UsePowershell ? "Build.ps1" : "Build.sh");

		if (Target.bCompileISPC)
		{
			Directory.CreateDirectory(Path.GetDirectoryName(BuildScriptPath)!);
			File.Delete(BuildScriptPath);
			File.Create(BuildScriptPath);
			return;
		}

		string IncludesPath = Path.Combine(IntermediatePath, "Includes");
		Directory.CreateDirectory(IncludesPath);

		ModuleRules.PublicIncludePaths.Add(IncludesPath);

		string SourceDirectory = Path.Combine(ModuleRules.PluginDirectory, "Source");

		string[] HeaderFiles = Directory.GetFiles(
			SourceDirectory,
			"*.isph",
			SearchOption.AllDirectories);

		string BuildScript = File.ReadAllText(Path.Combine(
			ModuleRules.PluginDirectory,
			"Source",
			"VoxelCore",
			"Private",
			UsePowershell ? "BuildISPC-windows.txt" : "BuildISPC-linux.txt"));

		if (BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Mac)
		{
			BuildScript = BuildScript.Replace("stat -c %Y ", "stat -f \"%m\" ");
		}

		string ISPCPath;
		{
			string PlatformName;
			string ExecutableName;
			string DependencyPath;
			if (BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Win64)
			{
				PlatformName = "Windows";
				ExecutableName = "ispc.exe";
				DependencyPath = "UnrealEngine-28863921/076d2ee625039e05710c662ba62e32b614f2a565";
			}
			else if (BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Mac)
			{
				PlatformName = "Mac";
				ExecutableName = "ispc";
				DependencyPath = "UnrealEngine-28863921/61c092aceb40ae772a3ce1be72b3eed552c32e56";
			}
			else if (BuildHostPlatform.Current.Platform == UnrealTargetPlatform.Linux)
			{
				PlatformName = "Linux";
				ExecutableName = "ispc";
				DependencyPath = "UnrealEngine-32062198/5239059f3a68abfe6be8a8d51f1a83218d12e0a8";
			}
			else
			{
				throw new Exception("Unsupported host platform: " + BuildHostPlatform.Current.Platform);
			}

			ISPCPath = Path.Combine(
				ModuleRules.EngineDirectory,
				"Source",
				"ThirdParty",
				"Intel",
				"ISPC",
				"bin",
				PlatformName,
				ExecutableName);

			if (!File.Exists(ISPCPath))
			{
				ISPCPath = Path.Combine(
					ModuleRules.PluginDirectory,
					"Intermediate",
					"ISPC",
					"bin",
					PlatformName,
					ExecutableName);

				if (!File.Exists(ISPCPath))
				{
					Directory.CreateDirectory(Path.GetDirectoryName(ISPCPath)!);

					Console.WriteLine("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
					Console.WriteLine("!!!!!!!! ISPC not found, downloading !!!!!!!!");
					Console.WriteLine("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

					DownloadFile(
						"http://cdn.unrealengine.com/dependencies/" + DependencyPath,
						8,
						ISPCPath);
				}
			}
		}

		ISPCInfo ISPCInfo = GetISPCInfo(Architecture);
		{
			string ISPCArgs = $"--target=`\"{string.Join(",", ISPCInfo.Targets)}`\" --arch=`\"{ISPCInfo.Arch}`\" --target-os=`\"{ISPCInfo.TargetOS}`\"";
			if (!UsePowershell)
			{
				ISPCArgs = ISPCArgs.Replace("`", "\\");
			}

			if (BuildHostPlatform.Current.Platform != UnrealTargetPlatform.Win64)
			{
				ISPCArgs += " --pic";
			}

			string ISPCHeaders = string.Join(",", Array.ConvertAll(HeaderFiles, Header => $"\"{Header}\""));

			string ISPCIncludes = string.Join(" ", new HashSet<string>(Array.ConvertAll(
				HeaderFiles,
				Header => $"-I`\"{Directory.GetParent(Header)!.FullName}`\"")));

			if (!UsePowershell)
			{
				ISPCPath = ISPCPath.Replace(" ", "\\ ");
				ISPCHeaders = ISPCHeaders.Replace(",", " ");
				ISPCIncludes = ISPCIncludes.Replace("`", "\\");
			}

			BuildScript = BuildScript.Replace("ISPC_PATH", ISPCPath);
			BuildScript = BuildScript.Replace("ISPC_ARGS", $"\"{ISPCArgs}\"");
			BuildScript = BuildScript.Replace("ISPC_HEADERS", ISPCHeaders);
			BuildScript = BuildScript.Replace("ISPC_INCLUDES", $"\"{ISPCIncludes}\"");
		}

		string BuildLibraries = "";

		foreach (string ModulePath in Directory.GetDirectories(SourceDirectory))
		{
			string Module = Path.GetFileName(ModulePath)!;

			string ModuleSourceDirectory = Path.Combine(
				ModuleRules.PluginDirectory,
				"Source",
				Module);

			string[] SourceFiles = Directory.GetFiles(
				ModuleSourceDirectory,
				"*.ispc",
				SearchOption.AllDirectories);

			if (SourceFiles.Length == 0)
			{
				continue;
			}

			// Console.WriteLine("VOXEL: " + Module + " has ISPC files");

			string LibraryPath = Path.Combine(
				IntermediatePath,
				// Android needs a 'lib' prefix
				"lib" + Module + ".a");

			if (!File.Exists(LibraryPath))
			{
				// Create a dummy library to avoid UBT erroring out
				// This will be replaced by the pre-build step since the date is older than any ispc file

				Directory.CreateDirectory(Directory.GetParent(LibraryPath)!.FullName);
				File.Create(LibraryPath);
				File.SetCreationTime(LibraryPath, new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc));
				File.SetLastWriteTime(LibraryPath, new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc));
			}

			ModuleRules.PublicAdditionalLibraries.Add(LibraryPath);

			List<string> ObjectFiles = new List<string>();
			foreach (string SourceFile in SourceFiles)
			{
				string BaseObjectFile = Path.Combine(
					IntermediatePath,
					Path.GetFileNameWithoutExtension(SourceFile));

				string ObjectFile = BaseObjectFile + ".o";

				string GeneratedHeader = Path.Combine(
					IncludesPath,
					Path.GetFileName(SourceFile) + ".generated.h");

				if (UsePowershell)
				{
					BuildScript += $"\nBuild -SourceFile \"{SourceFile}\" -ObjectFile \"{ObjectFile}\" -GeneratedHeader \"{GeneratedHeader}\"";
				}
				else
				{
					BuildScript += $"\nbuild \"{SourceFile}\" \"{ObjectFile}\" \"{GeneratedHeader}\"";
				}

				ObjectFiles.Add(ObjectFile);

				foreach (string ISPCTarget in ISPCInfo.LinkTargets)
				{
					ObjectFiles.Add(BaseObjectFile + "_" + ISPCTarget + ".o");
				}
			}

			if (UsePowershell)
			{
				string ObjectFilesString = string.Join(",", Array.ConvertAll(ObjectFiles.ToArray(), ObjectFile => $"\"{ObjectFile}\""));
				BuildLibraries += $"\nBuildLibrary -TargetFile \"{LibraryPath}\" -ObjectFiles {ObjectFilesString}";
			}
			else
			{
				string ObjectFilesString = string.Join(" ", Array.ConvertAll(ObjectFiles.ToArray(), ObjectFile => $"\"{ObjectFile}\""));
				BuildLibraries += $"\nbuild_library \"{LibraryPath}\" {ObjectFilesString}";
			}
		}

		if (UsePowershell)
		{
			BuildScript += "\n\nFlushCommands\n";
			BuildScript += BuildLibraries;
			BuildScript += "\n\nFlushCommands";
		}
		else
		{
			BuildScript += "\n\nflush_commands\n";
			BuildScript += BuildLibraries;
			BuildScript += "\n\nflush_commands";
		}

		ModuleRules.Logger.Log(LogLevel.Debug, "Writing {Path}", BuildScriptPath);
		File.WriteAllText(BuildScriptPath, BuildScript.Replace("\r\n", "\n"));

		// Add to PreBuildSteps to run the build script even if UnrealBuildTool doesn't regenerate project files
		{
			FieldInfo FieldInfo = typeof(ReadOnlyTargetRules).GetField("Inner", BindingFlags.NonPublic | BindingFlags.Instance);
			TargetRules Rules = (TargetRules)FieldInfo!.GetValue(Target)!;

			string Command =
				UsePowershell
					? $"powershell -ExecutionPolicy Bypass -File \"{BuildScriptPath}\""
					: $"bash \"{BuildScriptPath}\"";

			if (!Rules.PreBuildSteps.Contains(Command))
			{
				Rules.PreBuildSteps.Add(Command);
			}
		}

		// Don't compile when generating project files
		foreach (string Argument in Environment.GetCommandLineArgs())
		{
			if (Argument.ToLower() == "-projectfiles")
			{
				return;
			}
		}

		// Trigger manually in case PreBuildSteps were already executed
		ExecuteScript(BuildScriptPath);
	}

	private struct ISPCInfo
	{
		public string TargetOS;
		public string Arch;
		public string[] Targets;
		public string[] LinkTargets;
	}
	private ISPCInfo GetISPCInfo(UnrealArch Architecture)
	{
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			return new ISPCInfo
			{
				Arch = "x86-64",
				TargetOS = "windows",
				Targets = new string[] { "avx512skx-i32x8", "avx2", "avx", "sse4" },
				LinkTargets = new string[] { "avx512skx", "avx2", "avx", "sse4" }
			};
		}

		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			return new ISPCInfo
			{
				Arch = "x86-64",
				TargetOS = "linux",
				Targets = new string[] { "avx512skx-i32x8", "avx2", "avx", "sse4" },
				LinkTargets = new string[] { "avx512skx", "avx2", "avx", "sse4" }
			};
		}

		if (Target.Platform == UnrealTargetPlatform.LinuxArm64)
		{
			return new ISPCInfo
			{
				Arch = "aarch64",
				TargetOS = "linux",
				Targets = new string[] { "neon" },
				LinkTargets = new string[] {}
			};
		}

		if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			if (Architecture == UnrealArch.X64)
			{
				return new ISPCInfo
				{
					Arch = "x86-64",
					TargetOS = "macos",
					Targets = new string[] { "avx512skx-i32x8", "avx2", "avx", "sse4" },
					LinkTargets = new string[] { "avx512skx", "avx2", "avx", "sse4" }
				};
			}
			if (Architecture == UnrealArch.Arm64)
			{
				return new ISPCInfo
				{
					Arch = "aarch64",
					TargetOS = "macos",
					Targets = new string[] { "neon" },
					LinkTargets = new string[] {}
				};
			}
		}

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			if (Architecture == UnrealArch.X64)
			{
				return new ISPCInfo
				{
					Arch = "x86-64",
					TargetOS = "android",
					Targets = new string[] { "avx512skx-i32x8", "avx2", "avx", "sse4" },
					LinkTargets = new string[] { "avx512skx", "avx2", "avx", "sse4" }
				};
			}
			if (Architecture == UnrealArch.Arm64)
			{
				return new ISPCInfo
				{
					Arch = "aarch64",
					TargetOS = "android",
					Targets = new string[] { "neon" },
					LinkTargets = new string[] {}
				};
			}
		}

		if (Target.Platform == UnrealTargetPlatform.IOS ||
		    Target.Platform == UnrealTargetPlatform.VisionOS)
		{
			return new ISPCInfo
			{
				Arch = "aarch64",
				TargetOS = "ios",
				Targets = new string[] { "neon" },
				LinkTargets = new string[] {}
			};
		}

		throw new Exception("Unsupported platform or architecture: " + Target.Platform + " " + Architecture);
	}

	private void DownloadFile(
		string Url,
		int Offset,
		string OutputPath)
	{
		string BasePath = Path.Combine(ModuleRules.PluginDirectory, "Intermediate", "Downloads");
		Directory.CreateDirectory(BasePath);

		string TempFile = Path.Combine(BasePath, "temp");
		string ScriptPath = Path.Combine(BasePath, UsePowershell ? "Download.ps1" : "Download.sh");

		File.Delete(ScriptPath);
		File.Delete(TempFile);

		if (UsePowershell)
		{
			File.WriteAllText(
				ScriptPath,
				$@"
				$ErrorActionPreference = 'Stop'
				$ProgressPreference = 'SilentlyContinue'
		        
				Invoke-WebRequest -Uri ""{Url}"" -OutFile ""{TempFile}""

				$InputStream = [System.IO.File]::OpenRead(""{TempFile}"")
				$GzipStream = [System.IO.Compression.GzipStream]::new($InputStream, [System.IO.Compression.CompressionMode]::Decompress)

				$OutputStream = New-Object System.IO.MemoryStream
			    $GzipStream.CopyTo($OutputStream)
			    $GzipStream.Close()

				$Bytes = $OutputStream.ToArray()
				$Bytes = $Bytes[{Offset}..($Bytes.Length - 1)]

				[System.IO.File]::WriteAllBytes(""{OutputPath}"", $Bytes)
			".Replace("\r\n", "\n"));
		}
		else
		{
			File.WriteAllText(
				ScriptPath,
				$@"
				set -e
				curl -o ""{TempFile}"" ""{Url}""
				gzip -d -c ""{TempFile}"" | tail -c +{Offset + 1} > ""{OutputPath}""
				chmod +x ""{OutputPath}""
			".Replace("\r\n", "\n"));
		}

		ExecuteScript(ScriptPath);
	}

	private static void ExecuteScript(string ScriptPath)
	{
		Process ScriptProcess = new Process
		{
			StartInfo = new ProcessStartInfo
			{
				FileName = UsePowershell ? "powershell.exe" : "bash",
				Arguments = UsePowershell ? $" -ExecutionPolicy Bypass -File \"{ScriptPath}\"" : $"\"{ScriptPath}\"",
				RedirectStandardOutput = true,
				RedirectStandardError = true,
				UseShellExecute = false,
				CreateNoWindow = true,
				WorkingDirectory = Path.GetDirectoryName(ScriptPath)!
			}
		};

		ScriptProcess.OutputDataReceived += (_, Args) =>
		{
			if (!string.IsNullOrEmpty(Args.Data))
			{
				Console.WriteLine(Args.Data);
			}
		};
		ScriptProcess.ErrorDataReceived += (_, Args) =>
		{
			if (!string.IsNullOrEmpty(Args.Data))
			{
				Console.WriteLine(Args.Data);
			}
		};

		ScriptProcess.Start();
		ScriptProcess.BeginOutputReadLine();
		ScriptProcess.BeginErrorReadLine();
		ScriptProcess.WaitForExit();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

public class VoxelCore : ModuleRules
{
	public VoxelCore(ReadOnlyTargetRules Target) : base(Target)
	{
		SetupVoxelModule(this);

		IWYUSupport = IWYUSupport.None;

		VoxelConfig Config = new VoxelConfig(this);
		if (Config.EnableDebug)
		{
			Console.WriteLine("VOXEL_DEBUG=1");
		}

		if (Config.EnableDebug)
		{
			PublicDefinitions.Add("VOXEL_DEBUG=1");
		}
		else
		{
			// TRICKY: VoxelBuilder relies on text being exactly "VOXEL_DEBUG=0"
			PublicDefinitions.Add("VOXEL_DEBUG=0");
		}

		PublicDefinitions.Add("VOXEL_DEV_WORKFLOW=" + (Config.DevWorkflow ? "1" : "0"));

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Chaos",
			"TraceLog",
			"Renderer",
			"Projects",
			"ApplicationCore",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"zlib",
			"UElibPNG",
			"Json",
			"JsonUtilities",
			"HTTP",
			"Slate",
			"SlateCore",
			"ChaosCore",
			"Landscape",
			"EventLoop",
			"MoviePlayer",
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"DesktopPlatform",
				"GraphEditor",
				"UATHelper",
				"MaterialEditor",
			});
		}

		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(GetModuleDirectory("Engine"), "Private"),
			Path.Combine(GetModuleDirectory("Engine"), "Internal"),
			Path.Combine(GetModuleDirectory("Renderer"), "Private"),
			Path.Combine(GetModuleDirectory("Renderer"), "Internal")
		});

		VoxelISPCCompiler Compiler = new VoxelISPCCompiler(this);

		foreach (UnrealArch Architecture in Target.Architectures.Architectures)
		{
			Compiler.Compile(Architecture);
		}
	}
}