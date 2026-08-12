// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"

export void VoxelNode_GenerateSurfacePoints2D_SplitPositions(
	const uniform double PositionX[],
	const uniform double PositionY[],
	const uniform int32 GradientNum,
	const uniform float HalfStep,
	uniform double OutPositionX[],
	uniform double OutPositionY[])
{
    FOREACH(GradientIndex, 0, GradientNum)
    {
        const varying int32 InputIndex = GradientIndex >> 2; // / 4
        const varying int32 Direction = GradientIndex & 3; // % 4

        const varying float DeltaX =
            Direction == 0
            ? -HalfStep
            : Direction == 1
            ? HalfStep
            : 0.f;

        const varying float DeltaY =
            Direction == 2
            ? -HalfStep
            : Direction == 3
            ? HalfStep
            : 0.f;

        IGNORE_PERF_WARNING
        OutPositionX[GradientIndex] = PositionX[InputIndex] + DeltaX;
        IGNORE_PERF_WARNING
        OutPositionY[GradientIndex] = PositionY[InputIndex] + DeltaY;
    }
}

export void VoxelNode_GenerateSurfacePoints2D_Collapse(
	const uniform float Values[],
	const uniform int32 Num,
	const uniform float Step,
	uniform float OutX[],
	uniform float OutY[],
	uniform float OutZ[],
	uniform float OutW[])
{
	FOREACH(Index, 0, Num)
    {
        IGNORE_PERF_WARNING
        const varying float MinX = Values[4 * Index + 0];
        IGNORE_PERF_WARNING
        const varying float MaxX = Values[4 * Index + 1];
        IGNORE_PERF_WARNING
        const varying float MinY = Values[4 * Index + 2];
        IGNORE_PERF_WARNING
        const varying float MaxY = Values[4 * Index + 3];

		// SDFMinX = Z - HeightMinX
		// SDFMaxX = Z - HeightMaxX
		// SDFMaxX - SDFMinX = HeightMinX - HeightMaxX
		const varying float DeltaX = MinX - MaxX;
		const varying float DeltaY = MinY - MaxY;
		// SDFMinZ = (Z - 1) - Height
		// SDFMaxZ = (Z + 1) - Height
		// SDFMaxZ - SDFMinZ = 2
		const varying float DeltaZ = Step;

		const varying float3 Gradient = normalize(MakeFloat3(DeltaX, DeltaY, DeltaZ));

		float4 ReturnValue;

		ReturnValue = MakeQuaternionFromZ(Gradient);

		OutX[Index] = ReturnValue.x;
		OutY[Index] = ReturnValue.y;
		OutZ[Index] = ReturnValue.z;
		OutW[Index] = ReturnValue.w;
	}
}