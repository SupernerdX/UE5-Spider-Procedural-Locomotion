// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"

// ReSharper disable CppCStyleCast

const uniform float VoxelTextureFunctionLibrary_sRGBToLinearTable[256] =
{
	0.0f,
	0.000303526983548838f, 0.000607053967097675f, 0.000910580950646512f, 0.00121410793419535f, 0.00151763491774419f,
	0.00182116190129302f, 0.00212468888484186f, 0.0024282158683907f, 0.00273174285193954f, 0.00303526983548838f,
	0.00334653564113713f, 0.00367650719436314f, 0.00402471688178252f, 0.00439144189356217f, 0.00477695332960869f,
	0.005181516543916f, 0.00560539145834456f, 0.00604883284946662f, 0.00651209061157708f, 0.00699540999852809f,
	0.00749903184667767f, 0.00802319278093555f, 0.0085681254056307f, 0.00913405848170623f, 0.00972121709156193f,
	0.0103298227927056f, 0.0109600937612386f, 0.0116122449260844f, 0.012286488094766f, 0.0129830320714536f,
	0.0137020827679224f, 0.0144438433080002f, 0.0152085141260192f, 0.0159962930597398f, 0.0168073754381669f,
	0.0176419541646397f, 0.0185002197955389f, 0.0193823606149269f, 0.0202885627054049f, 0.0212190100154473f,
	0.0221738844234532f, 0.02315336579873f, 0.0241576320596103f, 0.0251868592288862f, 0.0262412214867272f,
	0.0273208912212394f, 0.0284260390768075f, 0.0295568340003534f, 0.0307134432856324f, 0.0318960326156814f,
	0.0331047661035236f, 0.0343398063312275f, 0.0356013143874111f, 0.0368894499032755f, 0.0382043710872463f,
	0.0395462347582974f, 0.0409151963780232f, 0.0423114100815264f, 0.0437350287071788f, 0.0451862038253117f,
	0.0466650857658898f, 0.0481718236452158f, 0.049706565391714f, 0.0512694577708345f, 0.0528606464091205f,
	0.0544802758174765f, 0.0561284894136735f, 0.0578054295441256f, 0.0595112375049707f, 0.0612460535624849f,
	0.0630100169728596f, 0.0648032660013696f, 0.0666259379409563f, 0.0684781691302512f, 0.070360094971063f,
	0.0722718499453493f, 0.0742135676316953f, 0.0761853807213167f, 0.0781874210336082f, 0.0802198195312533f,
	0.0822827063349132f, 0.0843762107375113f, 0.0865004612181274f, 0.0886555854555171f, 0.0908417103412699f,
	0.0930589619926197f, 0.0953074657649191f, 0.0975873462637915f, 0.0998987273569704f, 0.102241732185838f,
	0.104616483176675f, 0.107023102051626f, 0.109461709839399f, 0.1119324268857f, 0.114435372863418f,
	0.116970666782559f, 0.119538426999953f, 0.122138771228724f, 0.124771816547542f, 0.127437679409664f,
	0.130136475651761f, 0.132868320502552f, 0.135633328591233f, 0.138431613955729f, 0.141263290050755f,
	0.144128469755705f, 0.147027265382362f, 0.149959788682454f, 0.152926150855031f, 0.155926462553701f,
	0.158960833893705f, 0.162029374458845f, 0.16513219330827f, 0.168269398983119f, 0.171441099513036f,
	0.174647402422543f, 0.17788841473729f, 0.181164242990184f, 0.184474993227387f, 0.187820771014205f,
	0.191201681440861f, 0.194617829128147f, 0.198069318232982f, 0.201556252453853f, 0.205078735036156f,
	0.208636868777438f, 0.212230756032542f, 0.215860498718652f, 0.219526198320249f, 0.223227955893977f,
	0.226965872073417f, 0.23074004707378f, 0.23455058069651f, 0.238397572333811f, 0.242281120973093f,
	0.246201325201334f, 0.250158283209375f, 0.254152092796134f, 0.258182851372752f, 0.262250655966664f,
	0.266355603225604f, 0.270497789421545f, 0.274677310454565f, 0.278894261856656f, 0.283148738795466f,
	0.287440836077983f, 0.291770648154158f, 0.296138269120463f, 0.300543792723403f, 0.304987312362961f,
	0.309468921095997f, 0.313988711639584f, 0.3185467763743f, 0.323143207347467f, 0.32777809627633f,
	0.332451534551205f, 0.337163613238559f, 0.341914423084057f, 0.346704054515559f, 0.351532597646068f,
	0.356400142276637f, 0.361306777899234f, 0.36625259369956f, 0.371237678559833f, 0.376262121061519f,
	0.381326009488037f, 0.386429431827418f, 0.39157247577492f, 0.396755228735618f, 0.401977777826949f,
	0.407240209881218f, 0.41254261144808f, 0.417885068796976f, 0.423267667919539f, 0.428690494531971f,
	0.434153634077377f, 0.439657171728079f, 0.445201192387887f, 0.450785780694349f, 0.456411021020965f,
	0.462076997479369f, 0.467783793921492f, 0.473531493941681f, 0.479320180878805f, 0.485149937818323f,
	0.491020847594331f, 0.496932992791578f, 0.502886455747457f, 0.50888131855397f, 0.514917663059676f,
	0.520995570871595f, 0.527115123357109f, 0.533276401645826f, 0.539479486631421f, 0.545724458973463f,
	0.552011399099209f, 0.558340387205378f, 0.56471150325991f, 0.571124827003694f, 0.577580437952282f,
	0.584078415397575f, 0.590618838409497f, 0.597201785837643f, 0.603827336312907f, 0.610495568249093f,
	0.617206559844509f, 0.623960389083534f, 0.630757133738175f, 0.637596871369601f, 0.644479679329661f,
	0.651405634762384f, 0.658374814605461f, 0.665387295591707f, 0.672443154250516f, 0.679542466909286f,
	0.686685309694841f, 0.693871758534824f, 0.701101889159085f, 0.708375777101046f, 0.71569349769906f,
	0.723055126097739f, 0.730460737249286f, 0.737910405914797f, 0.745404206665559f, 0.752942213884326f,
	0.760524501766589f, 0.768151144321824f, 0.775822215374732f, 0.783537788566466f, 0.791297937355839f,
	0.799102735020525f, 0.806952254658248f, 0.81484656918795f, 0.822785751350956f, 0.830769873712124f,
	0.838799008660978f, 0.846873228412837f, 0.854992605009927f, 0.863157210322481f, 0.871367116049835f,
	0.879622393721502f, 0.887923114698241f, 0.896269350173118f, 0.904661171172551f, 0.913098648557343f,
	0.921581853023715f, 0.930110855104312f, 0.938685725169219f, 0.947306533426946f, 0.955973349925421f,
	0.964686244552961f, 0.973445287039244f, 0.982250546956257f, 0.991102093719252f, 1.0f
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FORCEINLINE varying float GetLinearValue(const varying int32 Index)
{
	IGNORE_PERF_WARNING
	return VoxelTextureFunctionLibrary_sRGBToLinearTable[Index];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FORCEINLINE varying int32 GetR_Impl(const varying int32 Color)
{
	return (Color >> 16) & 0xFF;
}
FORCEINLINE varying int32 GetG_Impl(const varying int32 Color)
{
	return (Color >> 8) & 0xFF;
}
FORCEINLINE varying int32 GetB_Impl(const varying int32 Color)
{
	return Color & 0xFF;
}
FORCEINLINE varying int32 GetA_Impl(const varying int32 Color)
{
	return ((uint32)Color) >> 24;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FORCEINLINE varying float GetR(const varying int32 Color)
{
	return GetR_Impl(Color);
}
FORCEINLINE varying float GetG(const varying int32 Color)
{
	return GetG_Impl(Color);
}
FORCEINLINE varying float GetB(const varying int32 Color)
{
	return GetB_Impl(Color);
}
FORCEINLINE varying float GetA(const varying int32 Color)
{
	return GetA_Impl(Color);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FORCEINLINE varying float GetR_Linear(const varying int32 Color)
{
	return GetLinearValue(GetR_Impl(Color));
}
FORCEINLINE varying float GetG_Linear(const varying int32 Color)
{
	return GetLinearValue(GetG_Impl(Color));
}
FORCEINLINE varying float GetB_Linear(const varying int32 Color)
{
	return GetLinearValue(GetB_Impl(Color));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelTextureFunctionLibrary_SampleNearest_Color(
	uniform float OutR[],
	uniform float OutG[],
	uniform float OutB[],
	uniform float OutA[],
	const uniform int32 Texture[],
	const uniform float PositionsX[],
	const uniform float PositionsY[],
	const uniform bool bConstPositionsX,
	const uniform bool bConstPositionsY,
	const uniform int32 Num,
	const uniform bool bSRGB,
	const uniform float Scale,
	const uniform int32 SizeX,
	const uniform int32 SizeY)
{
	const uniform float ScaleX = SizeX / Scale;
	const uniform float ScaleY = SizeY / Scale;

	const uniform int32 MaskX = SizeX - 1;
	const uniform int32 MaskY = SizeY - 1;

	const uniform bool bCanMask =
		IsPowerOfTwo(SizeX) &&
		IsPowerOfTwo(SizeY);

	FOREACH(Index, 0, Num)
	{
		const varying float PositionX = ScaleX * (bConstPositionsX ? PositionsX[0] : PositionsX[Index]);
		const varying float PositionY = ScaleY * (bConstPositionsY ? PositionsY[0] : PositionsY[Index]);

		const varying int32 MinX = (int32)round(PositionX);
		const varying int32 MinY = (int32)round(PositionY);

		varying int32 Color;
		if (bCanMask)
		{
			IGNORE_PERF_WARNING
			Color = Texture[(MinX & MaskX) + SizeX * (MinY & MaskY)];
		}
		else
		{
			IGNORE_PERF_WARNING
			Color = Texture[PositiveMod(MinX, SizeX) + SizeX * PositiveMod(MinY, SizeY)];
		}

		if (bSRGB)
		{
			OutR[Index] = GetR_Linear(Color);
			OutG[Index] = GetG_Linear(Color);
			OutB[Index] = GetB_Linear(Color);

		}
		else
		{
			OutR[Index] = GetR(Color) / 255.f;
			OutG[Index] = GetG(Color) / 255.f;
			OutB[Index] = GetB(Color) / 255.f;
		}

		OutA[Index] = GetA(Color) / 255.f;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelTextureFunctionLibrary_SampleBilinear_Color(
	uniform float OutR[],
	uniform float OutG[],
	uniform float OutB[],
	uniform float OutA[],
	const uniform int32 Texture[],
	const uniform float PositionsX[],
	const uniform float PositionsY[],
	const uniform bool bConstPositionsX,
	const uniform bool bConstPositionsY,
	const uniform int32 Num,
	const uniform bool bSRGB,
	const uniform float Scale,
	const uniform int32 SizeX,
	const uniform int32 SizeY)
{
	const uniform float ScaleX = SizeX / Scale;
	const uniform float ScaleY = SizeY / Scale;

	const uniform int32 MaskX = SizeX - 1;
	const uniform int32 MaskY = SizeY - 1;

	const uniform bool bCanMask =
		IsPowerOfTwo(SizeX) &&
		IsPowerOfTwo(SizeY);

	FOREACH(Index, 0, Num)
	{
		const varying float PositionX = ScaleX * (bConstPositionsX ? PositionsX[0] : PositionsX[Index]);
		const varying float PositionY = ScaleY * (bConstPositionsY ? PositionsY[0] : PositionsY[Index]);

		const varying float MinXf = floor(PositionX);
		const varying float MinYf = floor(PositionY);

		const varying float AlphaX = PositionX - MinXf;
		const varying float AlphaY = PositionY - MinYf;

		const varying int32 MinX = (int32)MinXf;
		const varying int32 MinY = (int32)MinYf;

		varying int32 Color00;
		varying int32 Color01;
		varying int32 Color10;
		varying int32 Color11;
		if (bCanMask)
		{
			IGNORE_PERF_WARNING
			Color00 = Texture[((MinX + 0) & MaskX) + SizeX * ((MinY + 0) & MaskY)];
			IGNORE_PERF_WARNING
			Color01 = Texture[((MinX + 1) & MaskX) + SizeX * ((MinY + 0) & MaskY)];
			IGNORE_PERF_WARNING
			Color10 = Texture[((MinX + 0) & MaskX) + SizeX * ((MinY + 1) & MaskY)];
			IGNORE_PERF_WARNING
			Color11 = Texture[((MinX + 1) & MaskX) + SizeX * ((MinY + 1) & MaskY)];
		}
		else
		{
			const varying int32 MinX0 = PositiveMod(MinX, SizeX);
			const varying int32 MinY0 = PositiveMod(MinY, SizeY);
			IGNORE_PERF_WARNING
			const varying int32 MinX1 = (MinX0 + 1) % SizeX;
			IGNORE_PERF_WARNING
			const varying int32 MinY1 = (MinY0 + 1) % SizeY;

			IGNORE_PERF_WARNING
			Color00 = Texture[MinX0 + SizeX * MinY0];
			IGNORE_PERF_WARNING
			Color01 = Texture[MinX1 + SizeX * MinY0];
			IGNORE_PERF_WARNING
			Color10 = Texture[MinX0 + SizeX * MinY1];
			IGNORE_PERF_WARNING
			Color11 = Texture[MinX1 + SizeX * MinY1];
		}

		if (bSRGB)
		{
			OutR[Index] = BilinearInterpolation(
				GetR_Linear(Color00),
				GetR_Linear(Color01),
				GetR_Linear(Color10),
				GetR_Linear(Color11),
				AlphaX,
				AlphaY);

			OutG[Index] = BilinearInterpolation(
				GetG_Linear(Color00),
				GetG_Linear(Color01),
				GetG_Linear(Color10),
				GetG_Linear(Color11),
				AlphaX,
				AlphaY);

			OutB[Index] = BilinearInterpolation(
				GetB_Linear(Color00),
				GetB_Linear(Color01),
				GetB_Linear(Color10),
				GetB_Linear(Color11),
				AlphaX,
				AlphaY);
		}
		else
		{
			OutR[Index] = BilinearInterpolation(
				GetR(Color00),
				GetR(Color01),
				GetR(Color10),
				GetR(Color11),
				AlphaX,
				AlphaY) / 255.f;

			OutG[Index] = BilinearInterpolation(
				GetG(Color00),
				GetG(Color01),
				GetG(Color10),
				GetG(Color11),
				AlphaX,
				AlphaY) / 255.f;

			OutB[Index] = BilinearInterpolation(
				GetB(Color00),
				GetB(Color01),
				GetB(Color10),
				GetB(Color11),
				AlphaX,
				AlphaY) / 255.f;
		}

		OutA[Index] = BilinearInterpolation(
			GetA(Color00),
			GetA(Color01),
			GetA(Color10),
			GetA(Color11),
			AlphaX,
			AlphaY) / 255.f;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelTextureFunctionLibrary_SampleBicubic_Color(
	uniform float OutR[],
	uniform float OutG[],
	uniform float OutB[],
	uniform float OutA[],
	const uniform int32 Texture[],
	const uniform float PositionsX[],
	const uniform float PositionsY[],
	const uniform bool bConstPositionsX,
	const uniform bool bConstPositionsY,
	const uniform int32 Num,
	const uniform bool bSRGB,
	const uniform float Scale,
	const uniform int32 SizeX,
	const uniform int32 SizeY)
{
	const uniform float ScaleX = SizeX / Scale;
	const uniform float ScaleY = SizeY / Scale;

	if (!IsPowerOfTwo(SizeX) ||
		!IsPowerOfTwo(SizeY))
	{
		FOREACH(Index, 0, Num)
		{
			const varying float PositionX = ScaleX * (bConstPositionsX ? PositionsX[0] : PositionsX[Index]);
			const varying float PositionY = ScaleY * (bConstPositionsY ? PositionsY[0] : PositionsY[Index]);

			const varying float MinXf = floor(PositionX);
			const varying float MinYf = floor(PositionY);

			const varying float AlphaX = PositionX - MinXf;
			const varying float AlphaY = PositionY - MinYf;

			const varying int32 MinX = (int32)MinXf;
			const varying int32 MinY = (int32)MinYf;

			const varying int32 MinX0 = PositiveMod(MinX - 1, SizeX);
			const varying int32 MinY0 = PositiveMod(MinY - 1, SizeY);
			IGNORE_PERF_WARNING
			const varying int32 MinX1 = (MinX0 + 1) % SizeX;
			IGNORE_PERF_WARNING
			const varying int32 MinY1 = (MinY0 + 1) % SizeY;
			IGNORE_PERF_WARNING
			const varying int32 MinX2 = (MinX0 + 2) % SizeX;
			IGNORE_PERF_WARNING
			const varying int32 MinY2 = (MinY0 + 2) % SizeY;
			IGNORE_PERF_WARNING
			const varying int32 MinX3 = (MinX0 + 3) % SizeX;
			IGNORE_PERF_WARNING
			const varying int32 MinY3 = (MinY0 + 3) % SizeY;

			IGNORE_PERF_WARNING
			const varying int32 Color00 = Texture[MinX0 + SizeX * MinY0];
			IGNORE_PERF_WARNING
			const varying int32 Color01 = Texture[MinX1 + SizeX * MinY0];
			IGNORE_PERF_WARNING
			const varying int32 Color02 = Texture[MinX2 + SizeX * MinY0];
			IGNORE_PERF_WARNING
			const varying int32 Color03 = Texture[MinX3 + SizeX * MinY0];
			IGNORE_PERF_WARNING
			const varying int32 Color10 = Texture[MinX0 + SizeX * MinY1];
			IGNORE_PERF_WARNING
			const varying int32 Color11 = Texture[MinX1 + SizeX * MinY1];
			IGNORE_PERF_WARNING
			const varying int32 Color12 = Texture[MinX2 + SizeX * MinY1];
			IGNORE_PERF_WARNING
			const varying int32 Color13 = Texture[MinX3 + SizeX * MinY1];
			IGNORE_PERF_WARNING
			const varying int32 Color20 = Texture[MinX0 + SizeX * MinY2];
			IGNORE_PERF_WARNING
			const varying int32 Color21 = Texture[MinX1 + SizeX * MinY2];
			IGNORE_PERF_WARNING
			const varying int32 Color22 = Texture[MinX2 + SizeX * MinY2];
			IGNORE_PERF_WARNING
			const varying int32 Color23 = Texture[MinX3 + SizeX * MinY2];
			IGNORE_PERF_WARNING
			const varying int32 Color30 = Texture[MinX0 + SizeX * MinY3];
			IGNORE_PERF_WARNING
			const varying int32 Color31 = Texture[MinX1 + SizeX * MinY3];
			IGNORE_PERF_WARNING
			const varying int32 Color32 = Texture[MinX2 + SizeX * MinY3];
			IGNORE_PERF_WARNING
			const varying int32 Color33 = Texture[MinX3 + SizeX * MinY3];

			if (bSRGB)
			{
				OutR[Index] = BicubicInterpolation(
					GetR_Linear(Color00),
					GetR_Linear(Color01),
					GetR_Linear(Color02),
					GetR_Linear(Color03),
					GetR_Linear(Color10),
					GetR_Linear(Color11),
					GetR_Linear(Color12),
					GetR_Linear(Color13),
					GetR_Linear(Color20),
					GetR_Linear(Color21),
					GetR_Linear(Color22),
					GetR_Linear(Color23),
					GetR_Linear(Color30),
					GetR_Linear(Color31),
					GetR_Linear(Color32),
					GetR_Linear(Color33),
					AlphaX,
					AlphaY);

				OutG[Index] = BicubicInterpolation(
					GetG_Linear(Color00),
					GetG_Linear(Color01),
					GetG_Linear(Color02),
					GetG_Linear(Color03),
					GetG_Linear(Color10),
					GetG_Linear(Color11),
					GetG_Linear(Color12),
					GetG_Linear(Color13),
					GetG_Linear(Color20),
					GetG_Linear(Color21),
					GetG_Linear(Color22),
					GetG_Linear(Color23),
					GetG_Linear(Color30),
					GetG_Linear(Color31),
					GetG_Linear(Color32),
					GetG_Linear(Color33),
					AlphaX,
					AlphaY);

				OutB[Index] = BicubicInterpolation(
					GetB_Linear(Color00),
					GetB_Linear(Color01),
					GetB_Linear(Color02),
					GetB_Linear(Color03),
					GetB_Linear(Color10),
					GetB_Linear(Color11),
					GetB_Linear(Color12),
					GetB_Linear(Color13),
					GetB_Linear(Color20),
					GetB_Linear(Color21),
					GetB_Linear(Color22),
					GetB_Linear(Color23),
					GetB_Linear(Color30),
					GetB_Linear(Color31),
					GetB_Linear(Color32),
					GetB_Linear(Color33),
					AlphaX,
					AlphaY);
			}
			else
			{
				OutR[Index] = BicubicInterpolation(
					GetR(Color00),
					GetR(Color01),
					GetR(Color02),
					GetR(Color03),
					GetR(Color10),
					GetR(Color11),
					GetR(Color12),
					GetR(Color13),
					GetR(Color20),
					GetR(Color21),
					GetR(Color22),
					GetR(Color23),
					GetR(Color30),
					GetR(Color31),
					GetR(Color32),
					GetR(Color33),
					AlphaX,
					AlphaY) / 255.f;

				OutG[Index] = BicubicInterpolation(
					GetG(Color00),
					GetG(Color01),
					GetG(Color02),
					GetG(Color03),
					GetG(Color10),
					GetG(Color11),
					GetG(Color12),
					GetG(Color13),
					GetG(Color20),
					GetG(Color21),
					GetG(Color22),
					GetG(Color23),
					GetG(Color30),
					GetG(Color31),
					GetG(Color32),
					GetG(Color33),
					AlphaX,
					AlphaY) / 255.f;

				OutB[Index] = BicubicInterpolation(
					GetB(Color00),
					GetB(Color01),
					GetB(Color02),
					GetB(Color03),
					GetB(Color10),
					GetB(Color11),
					GetB(Color12),
					GetB(Color13),
					GetB(Color20),
					GetB(Color21),
					GetB(Color22),
					GetB(Color23),
					GetB(Color30),
					GetB(Color31),
					GetB(Color32),
					GetB(Color33),
					AlphaX,
					AlphaY) / 255.f;
			}

			OutA[Index] = BicubicInterpolation(
				GetA(Color00),
				GetA(Color01),
				GetA(Color02),
				GetA(Color03),
				GetA(Color10),
				GetA(Color11),
				GetA(Color12),
				GetA(Color13),
				GetA(Color20),
				GetA(Color21),
				GetA(Color22),
				GetA(Color23),
				GetA(Color30),
				GetA(Color31),
				GetA(Color32),
				GetA(Color33),
				AlphaX,
				AlphaY) / 255.f;
		}

		return;
	}

	const uniform int32 MaskX = SizeX - 1;
	const uniform int32 MaskY = SizeY - 1;

	FOREACH(Index, 0, Num)
	{
		const varying float PositionX = ScaleX * (bConstPositionsX ? PositionsX[0] : PositionsX[Index]);
		const varying float PositionY = ScaleY * (bConstPositionsY ? PositionsY[0] : PositionsY[Index]);

		const varying float MinXf = floor(PositionX);
		const varying float MinYf = floor(PositionY);

		const varying float AlphaX = PositionX - MinXf;
		const varying float AlphaY = PositionY - MinYf;

		const varying int32 MinX = ((int32)MinXf) & MaskX;
		const varying int32 MinY = ((int32)MinYf) & MaskY;

		if (!all(
			0 <= MinX - 1 && MinX + 2 < SizeX &&
			0 <= MinY - 1 && MinY + 2 < SizeY))
		{
			IGNORE_PERF_WARNING
			const varying int32 Color00 = Texture[((MinX - 1) & MaskX) + SizeX * ((MinY - 1) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color01 = Texture[((MinX + 0) & MaskX) + SizeX * ((MinY - 1) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color02 = Texture[((MinX + 1) & MaskX) + SizeX * ((MinY - 1) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color03 = Texture[((MinX + 2) & MaskX) + SizeX * ((MinY - 1) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color10 = Texture[((MinX - 1) & MaskX) + SizeX * ((MinY + 0) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color11 = Texture[((MinX + 0) & MaskX) + SizeX * ((MinY + 0) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color12 = Texture[((MinX + 1) & MaskX) + SizeX * ((MinY + 0) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color13 = Texture[((MinX + 2) & MaskX) + SizeX * ((MinY + 0) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color20 = Texture[((MinX - 1) & MaskX) + SizeX * ((MinY + 1) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color21 = Texture[((MinX + 0) & MaskX) + SizeX * ((MinY + 1) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color22 = Texture[((MinX + 1) & MaskX) + SizeX * ((MinY + 1) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color23 = Texture[((MinX + 2) & MaskX) + SizeX * ((MinY + 1) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color30 = Texture[((MinX - 1) & MaskX) + SizeX * ((MinY + 2) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color31 = Texture[((MinX + 0) & MaskX) + SizeX * ((MinY + 2) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color32 = Texture[((MinX + 1) & MaskX) + SizeX * ((MinY + 2) & MaskY)];
			IGNORE_PERF_WARNING
			const varying int32 Color33 = Texture[((MinX + 2) & MaskX) + SizeX * ((MinY + 2) & MaskY)];

			if (bSRGB)
			{
				OutR[Index] = BicubicInterpolation(
					GetR_Linear(Color00),
					GetR_Linear(Color01),
					GetR_Linear(Color02),
					GetR_Linear(Color03),
					GetR_Linear(Color10),
					GetR_Linear(Color11),
					GetR_Linear(Color12),
					GetR_Linear(Color13),
					GetR_Linear(Color20),
					GetR_Linear(Color21),
					GetR_Linear(Color22),
					GetR_Linear(Color23),
					GetR_Linear(Color30),
					GetR_Linear(Color31),
					GetR_Linear(Color32),
					GetR_Linear(Color33),
					AlphaX,
					AlphaY);

				OutG[Index] = BicubicInterpolation(
					GetG_Linear(Color00),
					GetG_Linear(Color01),
					GetG_Linear(Color02),
					GetG_Linear(Color03),
					GetG_Linear(Color10),
					GetG_Linear(Color11),
					GetG_Linear(Color12),
					GetG_Linear(Color13),
					GetG_Linear(Color20),
					GetG_Linear(Color21),
					GetG_Linear(Color22),
					GetG_Linear(Color23),
					GetG_Linear(Color30),
					GetG_Linear(Color31),
					GetG_Linear(Color32),
					GetG_Linear(Color33),
					AlphaX,
					AlphaY);

				OutB[Index] = BicubicInterpolation(
					GetB_Linear(Color00),
					GetB_Linear(Color01),
					GetB_Linear(Color02),
					GetB_Linear(Color03),
					GetB_Linear(Color10),
					GetB_Linear(Color11),
					GetB_Linear(Color12),
					GetB_Linear(Color13),
					GetB_Linear(Color20),
					GetB_Linear(Color21),
					GetB_Linear(Color22),
					GetB_Linear(Color23),
					GetB_Linear(Color30),
					GetB_Linear(Color31),
					GetB_Linear(Color32),
					GetB_Linear(Color33),
					AlphaX,
					AlphaY);
			}
			else
			{
				OutR[Index] = BicubicInterpolation(
					GetR(Color00),
					GetR(Color01),
					GetR(Color02),
					GetR(Color03),
					GetR(Color10),
					GetR(Color11),
					GetR(Color12),
					GetR(Color13),
					GetR(Color20),
					GetR(Color21),
					GetR(Color22),
					GetR(Color23),
					GetR(Color30),
					GetR(Color31),
					GetR(Color32),
					GetR(Color33),
					AlphaX,
					AlphaY) / 255.f;

				OutG[Index] = BicubicInterpolation(
					GetG(Color00),
					GetG(Color01),
					GetG(Color02),
					GetG(Color03),
					GetG(Color10),
					GetG(Color11),
					GetG(Color12),
					GetG(Color13),
					GetG(Color20),
					GetG(Color21),
					GetG(Color22),
					GetG(Color23),
					GetG(Color30),
					GetG(Color31),
					GetG(Color32),
					GetG(Color33),
					AlphaX,
					AlphaY) / 255.f;

				OutB[Index] = BicubicInterpolation(
					GetB(Color00),
					GetB(Color01),
					GetB(Color02),
					GetB(Color03),
					GetB(Color10),
					GetB(Color11),
					GetB(Color12),
					GetB(Color13),
					GetB(Color20),
					GetB(Color21),
					GetB(Color22),
					GetB(Color23),
					GetB(Color30),
					GetB(Color31),
					GetB(Color32),
					GetB(Color33),
					AlphaX,
					AlphaY) / 255.f;
			}

			OutA[Index] = BicubicInterpolation(
				GetA(Color00),
				GetA(Color01),
				GetA(Color02),
				GetA(Color03),
				GetA(Color10),
				GetA(Color11),
				GetA(Color12),
				GetA(Color13),
				GetA(Color20),
				GetA(Color21),
				GetA(Color22),
				GetA(Color23),
				GetA(Color30),
				GetA(Color31),
				GetA(Color32),
				GetA(Color33),
				AlphaX,
				AlphaY) / 255.f;

			continue;
		}

		const varying float p0_Y_MultiplierVector = ((-0.5f * AlphaY + 1.0f) * AlphaY - 0.5f) * AlphaY + 0.0f;
		const varying float p1_Y_MultiplierVector = ((+1.5f * AlphaY - 2.5f) * AlphaY - 0.0f) * AlphaY + 1.0f;
		const varying float p2_Y_MultiplierVector = ((-1.5f * AlphaY + 2.0f) * AlphaY + 0.5f) * AlphaY + 0.0f;
		const varying float p3_Y_MultiplierVector = ((+0.5f * AlphaY - 0.5f) * AlphaY + 0.0f) * AlphaY + 0.0f;

		const varying int32 BaseIndexVector = (MinX - 1) + SizeX * (MinY - 1);

		varying float ResultR;
		varying float ResultG;
		varying float ResultB;
		varying float ResultA;

		// Read the current active lanes
		const uniform int64 LaneMask = lanemask_unsafe();

		// Reset active lanes
		unmasked_unsafe
		{
			UNROLL_N(TARGET_WIDTH)
			for (uniform int32 Lane = 0; Lane < programCount; Lane++)
			{
				if (!(LaneMask & (1u << Lane)))
				{
					continue;
				}

				const uniform int32 BaseIndex = extract(BaseIndexVector, Lane);
				const uniform int32 LaneMinX = extract(MinX, Lane);
				const uniform int32 LaneMinY = extract(MinY, Lane);
				const uniform float LaneAlphaX = extract(AlphaX, Lane);
				const uniform float p0_Y_Multiplier = extract(p0_Y_MultiplierVector, Lane);
				const uniform float p1_Y_Multiplier = extract(p1_Y_MultiplierVector, Lane);
				const uniform float p2_Y_Multiplier = extract(p2_Y_MultiplierVector, Lane);
				const uniform float p3_Y_Multiplier = extract(p3_Y_MultiplierVector, Lane);

				check(0 <= LaneMinX - 1 && LaneMinX + 2 < SizeX);
				check(0 <= LaneMinY - 1 && LaneMinY + 2 < SizeY);
				check(BaseIndex + 2 * SizeX + TARGET_WIDTH <= SizeX * SizeY);
#if TARGET_WIDTH == 8
				check(BaseIndex + 3 * SizeX - 4 + TARGET_WIDTH <= SizeX * SizeY);
#elif TARGET_WIDTH == 4
				check(BaseIndex + 3 * SizeX + TARGET_WIDTH <= SizeX * SizeY);
#endif

				const varying int32 Color0 = Texture[BaseIndex + 0 * SizeX + programIndex];
				const varying int32 Color1 = Texture[BaseIndex + 1 * SizeX + programIndex];
				const varying int32 Color2 = Texture[BaseIndex + 2 * SizeX + programIndex];
#if TARGET_WIDTH == 8
				const varying int32 Color3 = rotate(Texture[BaseIndex + 3 * SizeX - 4 + programIndex], 4);
#elif TARGET_WIDTH == 4
				// Don't access out of bounds
				const varying int32 Color3 = Texture[BaseIndex + 3 * SizeX + programIndex];
#endif

				const varying float A = MakeRegister4_ZeroPadded(+0.0f, +1.0f, +0.0f, +0.0f);
				const varying float B = MakeRegister4_ZeroPadded(-0.5f, -0.0f, +0.5f, +0.0f);
				const varying float C = MakeRegister4_ZeroPadded(+1.0f, -2.5f, +2.0f, -0.5f);
				const varying float D = MakeRegister4_ZeroPadded(-0.5f, +1.5f, -1.5f, +0.5f);

				const varying float p0123_X_Multiplier = A + LaneAlphaX * (B + LaneAlphaX * (C + LaneAlphaX * D));

				varying float R0;
				varying float R1;
				varying float R2;
				varying float R3;
				varying float G0;
				varying float G1;
				varying float G2;
				varying float G3;
				varying float B0;
				varying float B1;
				varying float B2;
				varying float B3;

				if (bSRGB)
				{
					R0 = GetR_Linear(Color0);
					R1 = GetR_Linear(Color1);
					R2 = GetR_Linear(Color2);
					R3 = GetR_Linear(Color3);
					G0 = GetG_Linear(Color0);
					G1 = GetG_Linear(Color1);
					G2 = GetG_Linear(Color2);
					G3 = GetG_Linear(Color3);
					B0 = GetB_Linear(Color0);
					B1 = GetB_Linear(Color1);
					B2 = GetB_Linear(Color2);
					B3 = GetB_Linear(Color3);
				}
				else
				{
					R0 = GetR(Color0);
					R1 = GetR(Color1);
					R2 = GetR(Color2);
					R3 = GetR(Color3);
					G0 = GetG(Color0);
					G1 = GetG(Color1);
					G2 = GetG(Color2);
					G3 = GetG(Color3);
					B0 = GetB(Color0);
					B1 = GetB(Color1);
					B2 = GetB(Color2);
					B3 = GetB(Color3);
				}

				ResultR = insert(ResultR, Lane, reduce_add(
					p0123_X_Multiplier * (
						p0_Y_Multiplier * R0 +
						p1_Y_Multiplier * R1 +
						p2_Y_Multiplier * R2 +
						p3_Y_Multiplier * R3)));

				ResultG = insert(ResultG, Lane, reduce_add(
					p0123_X_Multiplier * (
						p0_Y_Multiplier * G0 +
						p1_Y_Multiplier * G1 +
						p2_Y_Multiplier * G2 +
						p3_Y_Multiplier * G3)));

				ResultB = insert(ResultB, Lane, reduce_add(
					p0123_X_Multiplier * (
						p0_Y_Multiplier * B0 +
						p1_Y_Multiplier * B1 +
						p2_Y_Multiplier * B2 +
						p3_Y_Multiplier * B3)));

				ResultA = insert(ResultA, Lane, reduce_add(
					p0123_X_Multiplier * (
						p0_Y_Multiplier * GetA(Color0) +
						p1_Y_Multiplier * GetA(Color1) +
						p2_Y_Multiplier * GetA(Color2) +
						p3_Y_Multiplier * GetA(Color3))));
			}
		}

		if (bSRGB)
		{
			OutR[Index] = ResultR;
			OutG[Index] = ResultG;
			OutB[Index] = ResultB;
		}
		else
		{
			OutR[Index] = ResultR / 255.f;
			OutG[Index] = ResultG / 255.f;
			OutB[Index] = ResultB / 255.f;
		}

		OutA[Index] = ResultA / 255.f;
	}
}