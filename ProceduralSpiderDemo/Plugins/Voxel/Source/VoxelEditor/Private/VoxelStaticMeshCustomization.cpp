// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "StaticMesh/VoxelStaticMesh.h"

VOXEL_CUSTOMIZE_CLASS(UVoxelStaticMesh)(IDetailLayoutBuilder& DetailLayout)
{
	DetailLayout.EditCategory("Config")
	.AddCustomRow(INVTEXT("Bake Metadatas"))
	.NameContent()
	[
		SNew(SVoxelDetailText)
		.Text(INVTEXT("Bake Metadatas"))
	]
	.ValueContent()
	[
		SNew(SButton)
		.Text(INVTEXT("Bake"))
		.ToolTipText(INVTEXT("Re-bake the mesh metadatas"))
		.HAlign(HAlign_Center)
		.OnClicked_Lambda([WeakObjects = GetWeakObjectsBeingCustomized(DetailLayout)]
		{
			for (const TVoxelObjectPtr<UVoxelStaticMesh>& WeakObject : WeakObjects)
			{
				UVoxelStaticMesh* Object = WeakObject.Resolve();
				if (!ensure(Object))
				{
					continue;
				}

				Object->BakeMetadatas_EditorOnly();
			}

			return FReply::Handled();
		})
	];
}