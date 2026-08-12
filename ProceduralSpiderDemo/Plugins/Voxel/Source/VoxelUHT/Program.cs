// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using EpicGames.Core;
using EpicGames.UHT.Tables;
using EpicGames.UHT.Types;
using EpicGames.UHT.Utils;
using EpicGames.UHT.Exporters.CodeGen;

[UnrealHeaderTool]
public static class Program
{
	[UhtExporter(
		Name = "VoxelGraph",
		Options = UhtExporterOptions.Default | UhtExporterOptions.CompileOutput,
		ModuleName = "VoxelGraph",
		CppFilters = new string[] { "*.GeneratedVoxel.cpp" })]
	private static void Main(
		IUhtExportFactory Factory)
	{
		new VoxelFunctionLibraryCodeGenerator(Factory).Generate();
	}
}

class VoxelFunctionLibraryCodeGenerator
{
	private IUhtExportFactory Factory;
	private UhtClass? VoxelFunctionLibrary { get; set; } = null;
	private UhtScriptStruct? VoxelRuntimePinValue { get; set; } = null;

	public VoxelFunctionLibraryCodeGenerator(IUhtExportFactory Factory)
	{
		this.Factory = Factory;
	}

	public void Generate()
	{
		VoxelFunctionLibrary = Factory.Session.FindType(null, UhtFindOptions.SourceName | UhtFindOptions.Class, "UVoxelFunctionLibrary") as UhtClass;
		VoxelRuntimePinValue = Factory.Session.FindType(null, UhtFindOptions.SourceName | UhtFindOptions.ScriptStruct, "FVoxelRuntimePinValue") as UhtScriptStruct;

		List<Task?> Tasks = new();
		foreach (UhtHeaderFile Header in Factory.Session.SortedHeaderFiles)
		{
			List<UhtClass> FunctionLibraries = [];
			foreach (UhtType Child in Header.Children)
			{
				if (Child is UhtClass Class)
				{
					if (Class == VoxelFunctionLibrary)
					{
						continue;
					}

					if (Class.IsChildOf(VoxelFunctionLibrary))
					{
						FunctionLibraries.Add(Class);
					}
				}
			}

			if (FunctionLibraries.Count > 0)
			{
				Tasks.Add(Factory.CreateTask(_ => { GenerateHeader(Header, FunctionLibraries); }));
			}
		}

		// Wait for all the classes to export
		Task[]? WaitTasks = Tasks.Where(X => X != null).Cast<Task>().ToArray();
		if (WaitTasks.Length > 0)
		{
			Task.WaitAll(WaitTasks);
		}
	}

	private void GenerateHeader(
		UhtHeaderFile Header,
		List<UhtClass> FunctionLibraries)
	{
		using BorrowStringBuilder Borrower = new(StringBuilderCache.Big);
		
		StringBuilder Builder = Borrower.StringBuilder;
		Builder.Append("// Copyright Voxel Plugin SAS. All Rights Reserved.\r\n\r\n");
#if UE_5_6_OR_LATER
		Builder.Append("#include \"" + Factory.GetModuleShortestIncludePath(Header.Module, Header.FilePath) + "\"\r\n");
#else
		Builder.Append("#include \"" + Path.GetRelativePath(Header.Module.Module.IncludeBase, Header.FilePath).Replace('\\', '/') + "\"\r\n");
#endif

		foreach (UhtClass FunctionLibrary in FunctionLibraries)
		{
			Builder.Append("\r\n");
			GenerateClass(
				Builder,
				FunctionLibrary);
		}

		if (Header.Module.Module.SaveExportedHeaders)
		{
			// Can't use Factory.MakePath because it will reset to ModuleName (specified in attribute) path
			string FileName = Path.Combine(
				Header.Module.Module.OutputDirectory,
				Header.FileNameWithoutExtension) + ".GeneratedVoxel.cpp";
			Factory.CommitOutput(
				FileName,
				Builder);
		}
	}

	private void GenerateClass(
		StringBuilder Builder,
		UhtClass FunctionLibrary)
	{
		Builder.Append("struct Z_Voxel_" + FunctionLibrary.SourceName + "\r\n");
		Builder.Append("{\r\n");
		foreach (UhtFunction Function in FunctionLibrary.Functions)
		{
			Builder.Append("\tstatic void Exec" + Function.SourceName + "(UVoxelFunctionLibrary& BaseFunctionLibrary, const TConstVoxelArrayView<FVoxelRuntimePinValue*> Values)\r\n");
			Builder.Append("\t{\r\n");
			Builder.Append("\t\t" + FunctionLibrary.SourceName + "& FunctionLibrary = static_cast<" + FunctionLibrary.SourceName + "&>(BaseFunctionLibrary);\r\n");

			GenerateFunction(
				Builder,
				Function);

			Builder.Append("\t}\r\n");
		}
		
		Builder.Append("\r\n");

		foreach (UhtFunction Function in FunctionLibrary.Functions)
		{
			Builder.Append("\tinline static const FVoxelFunctionRegistration Register_" + Function.SourceName + " {\r\n");
			Builder.Append("\t\t&" + FunctionLibrary.SourceName + "::StaticClass,\r\n");
			Builder.Append("\t\tTEXT(\"" + Function.SourceName + "\"),\r\n");
			Builder.Append("\t\t&Exec" + Function.SourceName + " };\r\n");
		}

		Builder.Append("};\r\n");
	}

	private void GenerateFunction(
		StringBuilder Builder,
		UhtFunction Function)
	{
		StringBuilder Arguments = new StringBuilder();

		int Index = 0;
		int ReturnPropertyIndex = -1;
		foreach (UhtProperty Property in Function.Properties)
		{
			if (!IsValidArgument(
				    Builder,
				    Function,
				    Property))
			{
				return;
			}

			if (Property == Function.ReturnProperty)
			{
				ReturnPropertyIndex = Index++;
				continue;
			}

			if (Arguments.Length > 0)
			{
				Arguments.Append(",\r\n\t\t\t");
			}

			if (Property is UhtStructProperty StructProperty)
			{
				if (StructProperty.ScriptStruct == VoxelRuntimePinValue)
				{
					Arguments.Append("*Values[" + Index++ + "]");
					continue;
				}
			}

			string PropertyType = GetPropertyType(Property);
			if (Property.RefQualifier == UhtPropertyRefQualifier.NonConstRef)
			{
				Builder.Append("\t\t" + PropertyType + "& " + Property.StrippedEngineName + " = ConstCast(Values[" + Index++ + "]->Get<" + PropertyType + ">());\r\n");

				Arguments.Append(Property.StrippedEngineName);
				continue;
			}

			Arguments.Append("Values[" + Index++ + "]->Get<" + PropertyType + ">()");
		}

		string Call = "FunctionLibrary." + Function.SourceName + "(";
		if (Arguments.Length > 0)
		{
			Call += "\r\n\t\t\t" + Arguments;
		}
		Call += ")";
		

		if (!Function.HasReturnProperty)
		{
			Builder.Append("\t\t" + Call + ";\r\n");
			return;
		}

#if !UE_5_6_OR_LATER
		if (Function.ReturnProperty == null)
		{
			Builder.Append("\t\t" + Call + ";\r\n");
			return;
		}
#endif

		if (Function.ReturnProperty is UhtStructProperty ReturnStructProperty)
		{
			if (ReturnStructProperty.ScriptStruct == VoxelRuntimePinValue)
			{
				Builder.Append("\t\tConstCast(*Values[" + ReturnPropertyIndex + "]) = " + Call + ";\r\n");
				return;
			}
		}

		Builder.Append("\t\tConstCast(Values[" + ReturnPropertyIndex + "]->Get<" + GetPropertyType(Function.ReturnProperty) + ">()) = " + Call + ";\r\n");
	}

	private bool IsValidArgument(StringBuilder Builder, UhtFunction Function, UhtProperty Property)
	{
		if (Property is UhtArrayProperty)
		{
			Builder.Append("\t\tstatic_assert(false, \"Voxel Functions do not support TArray. Use an existing Voxel Buffer or create a new one. [" + Property.StrippedEngineName + "]\");\r\n");
			return false;
		}
		if (Property is UhtMapProperty)
		{
			Builder.Append("\t\tstatic_assert(false, \"Voxel Functions do not support TMap [" + Property.StrippedEngineName + "]\");\r\n");
			return false;
		}
		if (Property is UhtSetProperty)
		{
			Builder.Append("\t\tstatic_assert(false, \"Voxel Functions do not support TSet [" + Property.StrippedEngineName + "]\");\r\n");
			return false;
		}

		if (Property is UhtStructProperty &&
		    Property.RefQualifier == UhtPropertyRefQualifier.None &&
		    Property != Function.ReturnProperty)
		{
			Builder.Append("\t\tstatic_assert(false, \"All structs should be passed by ref in voxel functions [" + Property.StrippedEngineName + "]\");\r\n");
			return false;
		}

		return true;
	}

	private string GetPropertyType(
		UhtProperty Property)
	{
		if (Property is UhtEnumProperty EnumProperty)
		{
			if (EnumProperty.Enum.UnderlyingType == UhtEnumUnderlyingType.Int)
			{
				return "uint8";
			}

#if UE_5_7_OR_LATER
				return EnumProperty.Enum.FullyQualifiedCppType;
#else
				return EnumProperty.Enum.CppType;
#endif
		}

		StringBuilder Builder = new StringBuilder();
		Property.AppendText(Builder, UhtPropertyTextType.GenericFunctionArgOrRetVal);

		return Builder.ToString();
	}
}