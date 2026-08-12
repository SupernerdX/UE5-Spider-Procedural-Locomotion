// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphEditorUtilities.h"

#if WITH_EDITOR
TDelegate<void(const TVoxelSet<const UEdGraphNode*>&)> FVoxelGraphEditorUtilities::OnSelectNodes;
#endif