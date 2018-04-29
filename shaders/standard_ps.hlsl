#pragma pack_matrix(row_major)
#define MAXTBN float3x3(normalize(input.Tgt), normalize(input.Nml), normalize(input.Btgt));

cbuffer cbPerObject : register(b0)
{
	float4x4 wvpMatrix;
	float4x4 wMatrix;
	float4x4 wvMatrixI;
	float4x4 vMatrixI;
	float4x4 vMatrix;
	float4 lightVec;
};

Texture2D t_albedo : register(t0);
Texture2D t_normal : register(t1);
Texture2D t_gloss_map : register(t2);
Texture2D t_specular : register(t3);

SamplerState s_default : register(s0)
{
	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VS_OUTPUT
{
	float4 Position	: SV_POSITION;
	float4 TexCoord	: TEXCOORD;
	float3 Wpos		: POSITION;
	float3 Nml		: NORMAL;
	float3 Btgt		: BINORMAL;
	float3 Tgt		: TANGENT;
};

struct Material
{
	float3  Diffuse;
	float3  Specular;
	float3  Normal;
	float   Smoothness;
	float   Reflectivity;
};

float4 ps_main( in VS_OUTPUT input ) : SV_TARGET
{
	float3 eye_vector = normalize(vMatrixI[3].xyz - input.Wpos);
	float3 light_vector = normalize(lightVec.xyz - input.Wpos);

	float4 diffuse = t_albedo.Sample(s_default, input.TexCoord.xy);
	float4 specular = t_specular.Sample(s_default, input.TexCoord.xy);
	float smoothness = t_gloss_map.Sample(s_default, input.TexCoord.xy).x;
	specular.a = t_gloss_map.Sample(s_default, input.TexCoord.xy).y;
	clip(diffuse.a - 0.5f);

	float3x3 basis = MAXTBN
	float4 norm = t_normal.Sample(s_default, input.TexCoord.xy).rgba;
	norm.r = norm.a;
	norm.g = 1.0f - norm.g;
	norm.b = 1.0f - norm.b;
	float3 N = (norm.rgb * 2.0f - 1.0f).xzy;
	float3 normal = normalize(mul(normalize(N), basis));
	normal.x = -normal.x;

	float diffuseFactor = dot(light_vector, normal);
	float4 diff = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 spec = float4(1.0f, 1.0f, 1.0f, 1.0f);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-light_vector, normal);
		float specFactor = pow(max(dot(v, eye_vector), 0.0f), specular.a);

		diff = diffuseFactor * diffuse;
		spec = specFactor * specular;
	}
	else
		return float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 litColour = diff + spec * smoothness * specular.a;

	return litColour;
}