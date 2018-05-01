#pragma pack_matrix(row_major)

cbuffer cbPerObject : register(b0)
{
	float4x4 wvpMatrix;
	float4x4 wMatrix;
	float4x4 wvMatrixI;
	float4x4 vMatrixI;
	float4x4 vMatrix;
	float4 lightVec;
};

struct APP_INPUT
{
	float4 Position		: POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Bitangent	: BINORMAL;
	float4 TexCoord0	: TEXCOORD0;
	float4 TexCoord1	: TEXCOORD1;
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

VS_OUTPUT vs_main(in APP_INPUT input)
{
	VS_OUTPUT output;

	output.Position = mul(input.Position, wvpMatrix);
	output.TexCoord.x = input.TexCoord0.x;
	output.TexCoord.y = -input.TexCoord0.y;
	output.TexCoord.zw = input.TexCoord1.xy;

	output.TexCoord.y += 1;
	output.TexCoord.w += 1;

	output.Tgt = mul(float4(input.Tangent.xyz, 0.0f), wMatrix).xyz;
	output.Btgt = mul(float4(input.Bitangent.xyz, 0.0f), wMatrix).xyz;
	output.Nml = mul(float4(input.Normal.xyz, 0.0f), wMatrix).xyz;
	output.Wpos = mul(input.Position, wMatrix).xyz;

	return output;
}