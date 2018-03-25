#pragma pack_matrix(row_major)
#define MAXTBN float3x3(normalize(input.Tgt), normalize(input.Nml), normalize(input.Btgt));

cbuffer cbPerObject : register(b0)
{
	float4x4 wvpMatrix;
	float4x4 wMatrix;
	float4x4 wvMatrixI;
	float4x4 vMatrixI;
	float4x4 vMatrix;
	float4 camPos;
};

Texture2D t_albedo : register(t0);
Texture2D t_normal : register(t1);
Texture2D t_gloss_map : register(t2);
Texture2D t_specular_colour : register(t3);
//Texture2D t_mask : register(t4);
TextureCube t_environment_map : register(t5);
TextureCube AmbiTexture : register(t6);

SamplerState s_default : register(s0)
{
	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = WRAP;
	AddressV = WRAP;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//	Inputs/Outputs
////////////////////////////////////////////////////////////////////////////////////////////////////
struct VS_OUTPUT
{
	float4 Position	: SV_POSITION;
	float4 TexCoord	: TEXCOORD0;
	float3 I		: TEXCOORD1;
	float3 Tgt		: TEXCOORD2;
	float3 Btgt		: TEXCOORD3;
	float3 Nml		: TEXCOORD4;
	float3 Wpos		: TEXCOORD5;
	float4 Color    : TEXCOORD6;
};

//float3 _gamma(in float3 vLinear)
//{
//	return pow(max(vLinear, 0.0001f), 1.0f / 2.2f);
//}
//
float3 normalSwizzle(in float3 ref)
{
	return float3(ref.y, ref.x, ref.z);
}

float3 normalSwizzle_UPDATED(in float3 ref)
{
	return float3(ref.x, ref.z, ref.y);
}

//float3 texcoordEnvSwizzle(in float3 ref)
//{
//	return -float3(ref.x, -ref.z, ref.y);
//}
//
//float3 get_environment_colour_UPDATED(in float3 direction, in float lod)
//{
//	return t_environment_map.SampleLevel(s_default, texcoordEnvSwizzle(direction), lod).rgb;
//}

float4 ps_ndotl(in VS_OUTPUT input) : SV_TARGET
{
	const float3x3 basis = MAXTBN
	float4 norm = t_normal.Sample(s_default, input.TexCoord.xy).rgba;
	float3 Np = float3(1.0f, 1.0f, 1.0f);
	Np.r = norm.a;
	Np.g = Np.g - norm.g;
	Np.b = Np.b;
	float3 N = normalSwizzle(Np.rgb * 2.0f - 1.0f);

	//if (1.0)
	//{
	//	float3 N2 = normalSwizzle(tex2D(s_d_normal, input.TexCoord.xy*1.0).rgb * 2.0f - 1.0f);
	//	N = float3(N.x + (N2.x*1.0), N.y + (N2.y*1.0), N.z);
	//}

	float3 nN = ((normalize(mul(N, basis)))*0.5) + 0.5;
	return float4(nN.rgb, 1.0f);
}
