#pragma pack_matrix(row_major)
#define MAXTBN float3x3(normalize(input.Tgt), normalize(input.Nml), normalize(input.Btgt));

float pi = 3.14159265;
float one_over_pi = 1 / 3.14159265;
float real_approx_zero = 0.001f;
float Tone_Map_Black = 0.001;
float Tone_Map_White = 10.0f;
float low_tones_scurve_bias = 0.33f;
float high_tones_scurve_bias = 0.66f;

int i_alpha_mode = 0;
//float4 vec4_colour_0 = { 0.5, 0.1, 0.1, 1.0 };
//float4 vec4_colour_1 = { 0.3, 0.6, 0.5, 1.0 };
//float4 vec4_colour_2 = { 0.5, 0.2, 0.1, 1.0 };

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
Texture2D t_specular_colour : register(t3);
Texture2D t_mask : register(t4);
TextureCube t_environment_map : register(t5);
TextureCube AmbiTexture : register(t6);

SamplerState s_default : register(s0)
{
	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = WRAP;
	AddressV = WRAP;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//	Colorimetry Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
//float _linear(in float fGamma)
//{
//	return pow(max(fGamma, 0.0001f), 2.2f);
//}

float3 _linear(in float3 vGamma)
{
	return pow(max(vGamma, 0.0001f), 2.2f);
}

float3 _gamma(in float3 vLinear)
{
	return pow(max(vLinear, 0.0001f), 1.0f / 2.2f);
}

//float _gamma(in float fLinear)
//{
//	return pow(max(fLinear, 0.0001f), 1.0f / 2.2f);
//}

float get_diffuse_scale_factor()
{
	return 0.004f;
}

float get_game_hdr_lighting_multiplier()
{
	return 5000.0f;
}

//float get_luminance(in float3 colour)
//{
//	float3 lumCoeff = float3(0.299, 0.587, 0.114);
//	float luminance = dot(colour, lumCoeff);
//	return saturate(luminance);
//}

//float3 get_adjusted_faction_colour(in float3 colour)
//{
//	float3 fc = colour;
//	float lum = get_luminance(fc);
//	float dark_scale = 1.5;
//	float light_scale = 0.5;
//
//	fc = fc * (lerp(dark_scale, light_scale, lum));
//
//	return fc;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
//	Conversion Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
float3 texcoordEnvSwizzle(in float3 ref)
{
	return -float3(ref.x, -ref.z, ref.y);
}

float3 normalSwizzle_UPDATED(in float3 ref)
{
	return float3(ref.x, ref.z, ref.y);
}

float cos2sin(float x)
{
	return sqrt(1 - x*x);
}

float cos2tan2(float x)
{
	return (1 - x*x) / (x*x);
}

//float contrast(float _val, float _contrast)
//{
//	_val = ((_val - 0.5f) * max(_contrast, 0)) + 0.5f;
//	return _val;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
//	Forward Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////
float3 tone_map_linear_hdr_pixel_value(in float3 linear_hdr_pixel_val);
float4 HDR_RGB_To_HDR_CIE_Log_Y_xy(in float3 linear_colour_val);
float4 tone_map_HDR_CIE_Log_Y_xy_To_LDR_CIE_Yxy(in float4 hdr_LogYxy);
float4 LDR_CIE_Yxy_To_Linear_LDR_RGB(in float4 ldr_cie_Yxy);
float  get_scurve_y_pos(const float x_coord);

////////////////////////////////////////////////////////////////////////////////////////////////////
//	Lighting Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
float3 get_environment_colour_UPDATED(in float3 direction, in float lod)
{
	return t_environment_map.SampleLevel(s_default, texcoordEnvSwizzle(direction), lod).rgb;
}

float3 cube_ambient(in float3 N)
{
	return AmbiTexture.Sample(s_default, texcoordEnvSwizzle(N)).rgb;
}

//float2 phong_diffuse(in float3 N, in float3 L)
//{
//	const float factor = max(0.0f, dot(N, -L));
//	return float2(factor, (factor > 0.0f));
//}
//
//float phong_specular(in float3 I, in float3 N, in float shininess, in float3 L)
//{
//	const float3 R = reflect(L, N);
//	return saturate(pow(max(0.0f, dot(R, -I)), shininess));
//}
//
//float aniso_specular(in float3 I, float3 N, in float3 T, in float shininess, in float3 L)
//{
//	float3 nH = normalize(I + L);
//	float3 nT = normalize(T);
//	nT = normalize(nT - N * dot(N, nT));
//	float spec = pow(sqrt(1 - (pow(dot(nT, nH), 2))), shininess);
//
//	return spec;
//}
//
//float blinn_specular(in float3 I, in float3 N, in float shininess, in float3 L)
//{
//	shininess = shininess*4.0;
//	float3 H = normalize(I + L);
//	const float3 R = reflect(L, N);
//	return saturate(pow(max(0.0f, dot(N, -H)), shininess));
//}
//
//float blinn_phong_specular(in float dotNH, in float SpecularExponent)
//{
//	float D = pow(dotNH, SpecularExponent) * (SpecularExponent + 1.0f) / 2.0f;
//	return D;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
//	Cook Torrance Model 
////////////////////////////////////////////////////////////////////////////////////////////////////
//float beckmann_distribution(in float dotNH, in float SpecularExponent)
//{
//	float invm2 = SpecularExponent / 2.0f;
//	float D = exp(-cos2tan2(dotNH) * invm2) / pow(dotNH, 4.0f) * invm2;
//	return D;
//}
//
//float3 fresnel_optimized(in float3 R, in float c)
//{
//	float3 F = lerp(R, saturate(60.0f * R), pow(1.0f - c, 4.0f));
//	return F;
//}
//
//float3 fresnel_full(in float3 R, in float c)
//{
//	float3 n = (1 + sqrt(R)) / (1 - sqrt(R));
//	float3 FS = (c - n*sqrt(1 - pow(cos2sin(c) / n, 2))) / (c + n*sqrt(1 - pow(cos2sin(c) / n, 2)));
//	float3 FP = (sqrt(1 - pow(cos2sin(c) / n, 2)) - n*c) / (sqrt(1 - pow(cos2sin(c) / n, 2)) + n*c);
//	return (FS*FS + FP*FP) / 2;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
//	STANDARD_LIGHTING_MODEL
////////////////////////////////////////////////////////////////////////////////////////////////////
struct StandardLightingModelMaterial_UPDATED
{
	float3  Diffuse_Colour;
	float3  Specular_Colour;
	float3  Normal;
	float   Smoothness;
	float   Reflectivity;

	float   Depth;
	float   Shadow;
	float   SSAO;
};

StandardLightingModelMaterial_UPDATED create_standard_lighting_material_UPDATED(in float3 _DiffuseColor, in float3 _Specular_Colour, in float3 _Normal, in float _Smoothness, in float _Reflectivity)
{
	StandardLightingModelMaterial_UPDATED material;

	material.Diffuse_Colour = _DiffuseColor;
	material.Specular_Colour = _Specular_Colour;
	material.Normal = _Normal;
	material.Smoothness = _Smoothness;
	material.Reflectivity = _Reflectivity;
	material.Depth = 1.0f;
	material.Shadow = 1.0f;
	material.SSAO = 1.0f;

	return material;
}

float get_sun_angular_diameter()
{
	const float suns_angular_diameter = 1.0f;
	return radians(suns_angular_diameter);
}

float get_sun_angular_radius()
{
	return 0.5f * get_sun_angular_diameter();
}

float get_error_func_a_value()
{
	return (8.0f * (pi - 3.0f)) / (3 * pi * (4.0f - pi));
}

float erf(float x)
{
	float x_squared = x * x;
	float a = get_error_func_a_value();
	float a_times_x_squared = a * x_squared;
	float numerator = (4.0f * one_over_pi) + (a_times_x_squared);
	float denominator = 1.0f + a_times_x_squared;
	float main_term = -1.0f * x_squared * (numerator / denominator);

	return sign(x) * sqrt(1 - exp(main_term));
}

float erfinv(float x)
{
	float one_over_a = 1.0f / get_error_func_a_value();
	float log_1_minus_x_squared = log(1.0f - (x * x));
	float root_of_first_term = (2.0f * one_over_pi * one_over_a) + (log_1_minus_x_squared * 0.5f);
	float first_term = root_of_first_term * root_of_first_term;
	float second_term = log_1_minus_x_squared * one_over_a;
	float third_term = (2.0f * one_over_pi * one_over_a) + (log_1_minus_x_squared * 0.5f);
	float all_terms = first_term - second_term - third_term;

	return sign(x) * sqrt(sqrt(first_term - second_term) - third_term);
}

float norm_cdf(float x, float sigma)
{
	float one_over_root_two = 0.70710678118654752440084436210485f;
	return 0.5f * (1.0f + erf(x * (1.0f / sigma) * one_over_root_two));
}

float norm_cdf(float x_min, float x_max, float sigma)
{
	float min_summed_area = norm_cdf(x_min, sigma);
	float max_summed_area = norm_cdf(x_max, sigma);
	
	return max_summed_area - min_summed_area;
}

float norminv_sigma(float x, float area_under_the_graph)
{
	float error_func_x = erfinv((2.0f * area_under_the_graph) - 1);
	float sigma = x / (error_func_x * 1.4142135623730950488016887242097f);

	return sigma;
}

float get_normal_distribution_sigma_about_origin(float area_under_graph_centred_around_origin, float x_half_distance_from_origin)
{
	float area_from_x_neg_infinity = 0.5f + (0.5f * area_under_graph_centred_around_origin);
	return norminv_sigma(x_half_distance_from_origin, area_from_x_neg_infinity);
}

float determine_fraction_of_facets_at_reflection_angle(in float smoothness, in float light_vec_reflected_view_vec_angle)
{
	float sun_angular_radius = get_sun_angular_radius();
	float max_fraction_of_facets = 0.9999f;
	float min_fraction_of_facets = get_diffuse_scale_factor();
	float fraction_of_facets = lerp(min_fraction_of_facets, max_fraction_of_facets, smoothness * smoothness);
	float fraction_of_facets_to_look_for = 0.5f + (fraction_of_facets * 0.5f);
	float sigma = max(norminv_sigma(sun_angular_radius, fraction_of_facets_to_look_for), 0.0001f);
	float proportion_of_facets = norm_cdf(light_vec_reflected_view_vec_angle - sun_angular_radius, light_vec_reflected_view_vec_angle + sun_angular_radius, sigma);

	return proportion_of_facets;
}

float determine_facet_visibility(in float roughness, in float3 normal_vec, in float3 light_vec)
{
	const float n_dot_l = saturate(dot(normal_vec, light_vec));
	const float towards_diffuse_surface = sin(roughness * pi * 0.5f); //	( 0 - 1 ) output...
	const float facet_visibility = lerp(1.0f, n_dot_l, towards_diffuse_surface);

	return facet_visibility;
}

float3 determine_surface_reflectivity(in float3 material_reflectivity, in float roughness, in float3 light_vec, in float3 view_vec)
{
	float fresnel_curve = 10;
	float val1 = max(0, dot(light_vec, -view_vec));
	float val2 = pow(val1, fresnel_curve);
	float fresnel_bias = 0.5f;
	float roughness_bias = 0.98f;
	float smoothness_val = pow(cos(roughness * roughness_bias * pi * 0.5f), fresnel_bias);

	return lerp(material_reflectivity, saturate(60.0f * material_reflectivity), val2 * smoothness_val);
}

float4 plot_standard_lighting_model_test_func(in float4 vpos)
{
	float4 g_vpos_texel_offset = float4(0, 0, 0, 0);
	float4 g_screen_size_minus_one = float4(0, 0, 0, 0);
	vpos -= g_vpos_texel_offset.xxxx;
	float xpos = vpos.x / g_screen_size_minus_one.x * 5.0f;
	float ypos = ((g_screen_size_minus_one.y - vpos.y) / g_screen_size_minus_one.y) * 10.0f;
	float y_value = norminv_sigma(lerp(0.01f, 5.0f, xpos), 0.7);

	return saturate((y_value * g_screen_size_minus_one.y) - (ypos * g_screen_size_minus_one.y)).xxxx;
}

float3 get_reflectivity_base(in float3 light_vec, in float3 normal_vec, in float3 view_vec, in float3 material_reflectivity, in float smoothness, in float roughness, in float light_vec_reflected_view_vec_angle)
{
	float n_dot_l = dot(light_vec, normal_vec);
	if (n_dot_l >= 0.0f)
	{
		float fraction_of_facets = determine_fraction_of_facets_at_reflection_angle(smoothness, light_vec_reflected_view_vec_angle);
		float facet_visibility = determine_facet_visibility(roughness, normal_vec, light_vec);  // Looks ok
		float3 surface_reflectivity = determine_surface_reflectivity(material_reflectivity, roughness, light_vec, view_vec);

		return fraction_of_facets * facet_visibility * surface_reflectivity;
	}
	else
		return float3(0.0f, 0.0f, 0.0f);
}

float ensure_correct_trig_value(in float value)
{
	return clamp(value, -1.0f, +1.0f);
}

float3 get_reflectivity_dir_light(in float3 light_vec, in float3 normal_vec, in float3 view_vec, in float3 reflected_view_vec, in float standard_material_reflectivity, in float smoothness, in float roughness)
{
	float light_vec_reflected_view_vec_angle = acos(ensure_correct_trig_value(dot(light_vec, reflected_view_vec)));
	return get_reflectivity_base(light_vec, normal_vec, view_vec, standard_material_reflectivity, smoothness, roughness, light_vec_reflected_view_vec_angle);
}

float3 get_reflectivity_env_light(in float3 light_vec, in float3 normal_vec, in float3 view_vec, in float standard_material_reflectivity, in float smoothness, in float roughness)
{
	return determine_surface_reflectivity(standard_material_reflectivity, roughness, light_vec, view_vec);
}

float3 standard_lighting_model_directional_light_UPDATED(in float3 LightColor, in float3 normalised_light_dir, in float3 normalised_view_dir, in StandardLightingModelMaterial_UPDATED material)
{
	LightColor *= get_game_hdr_lighting_multiplier();
	float3 diffuse_scale_factor = get_diffuse_scale_factor();
	float roughness = 1.0f - material.Smoothness;

	normalised_light_dir = normalised_light_dir;
	normalised_view_dir = -normalised_view_dir;

	float   normal_dot_light_vec = max(0.0f, dot(material.Normal, normalised_light_dir));
	float3  reflected_view_vec = reflect(-normalised_view_dir, material.Normal);
	float   texture_num_lods = 10.0f;
	float   env_map_lod = roughness * (texture_num_lods - 1);
	float3  environment_colour = get_environment_colour_UPDATED(reflected_view_vec, env_map_lod);
	float3  dlight_pixel_reflectivity = get_reflectivity_dir_light(normalised_light_dir, material.Normal, normalised_view_dir, reflected_view_vec, material.Reflectivity, material.Smoothness, roughness);
	float3  dlight_specular_colour = dlight_pixel_reflectivity * material.Specular_Colour * LightColor;
	float3  dlight_material_scattering = 1.0f.xxx - max(dlight_pixel_reflectivity, material.Reflectivity.xxx);     //  All photons not accounted for by reflectivity are accounted by scattering. From the energy difference between in-coming light and emitted light we could calculate the amount of energy turned into heat. This energy would not be enough to make a viewable difference at standard illumination levels.
	float3  env_light_pixel_reflectivity = max(material.Reflectivity, get_reflectivity_env_light(reflected_view_vec, material.Normal, normalised_view_dir, material.Reflectivity, material.Smoothness, roughness));
	float3  env_light_specular_colour = environment_colour * env_light_pixel_reflectivity * material.Specular_Colour;
	float3  dlight_diffuse = material.Diffuse_Colour * normal_dot_light_vec * LightColor * dlight_material_scattering;
	float3  ambient_colour = cube_ambient(material.Normal);
	float3  env_light_diffuse = ambient_colour * material.Diffuse_Colour * (1.0f - material.Reflectivity);

	dlight_diffuse *= diffuse_scale_factor;
	//material.Shadow = material.Shadow;

	float shadow_attenuation = material.Shadow;
	float reflection_shadow_attenuation = (1 - ((1 - (material.Shadow))*0.75));

	return (material.SSAO * (env_light_diffuse + (reflection_shadow_attenuation * env_light_specular_colour))) + (shadow_attenuation * (dlight_specular_colour + dlight_diffuse));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//	Inputs/Outputs
////////////////////////////////////////////////////////////////////////////////////////////////////
struct VS_OUTPUT
{
	float4 Position	: SV_POSITION;
	float4 TexCoord	: TEXCOORD;
	float3 Wpos		: POSITION;
	float3 Nml		: NORMAL;
	float3 Btgt		: BINORMAL;
	float3 Tgt		: TANGENT;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//	Pixel Shader
////////////////////////////////////////////////////////////////////////////////////////////////////
void alpha_test(in const float pixel_alpha)
{
	if (i_alpha_mode == 1)
	{
		clip(pixel_alpha - 0.5f);
	}
}

float4 ps_main( in VS_OUTPUT input ) : SV_Target
{
	float3 eye_vector = normalize(vMatrixI[3].xyz - input.Wpos);
	float3 light_vector = -lightVec.xyz;

	float4 diffuse_colour = t_albedo.Sample(s_default, input.TexCoord.xy);
	//alpha_test(diffuse_colour.a);
	float4 specular_colour = t_specular_colour.Sample(s_default, input.TexCoord.xy);
	float smoothness = t_gloss_map.Sample(s_default, input.TexCoord.xy).x;
	float reflectivity = t_gloss_map.Sample(s_default, input.TexCoord.xy).y;

	//float mask_p1 = t_mask.Sample(s_default, input.TexCoord.xy).r;
	//float mask_p2 = t_mask.Sample(s_default, input.TexCoord.xy).g;
	//float mask_p3 = t_mask.Sample(s_default, input.TexCoord.xy).b;

	//diffuse_colour.rgb = lerp(diffuse_colour.rgb, diffuse_colour.rgb*_linear(vec4_colour_0.rgb), mask_p1);
	//diffuse_colour.rgb = lerp(diffuse_colour.rgb, diffuse_colour.rgb*_linear(vec4_colour_1.rgb), mask_p2);
	//diffuse_colour.rgb = lerp(diffuse_colour.rgb, diffuse_colour.rgb*_linear(vec4_colour_2.rgb), mask_p3);

	float3x3 basis = MAXTBN
	float4 norm = t_normal.Sample(s_default, input.TexCoord.xy).rgba;
	norm.r = norm.a;
	norm.g = 1.0f - norm.g;
	norm.b = 1.0f;
	float3 N = (norm.rgb * 2.0f - 1.0f).xzy;
	float3 pixel_normal = normalize(mul(normalize(N), basis));

	StandardLightingModelMaterial_UPDATED standard_mat = create_standard_lighting_material_UPDATED(diffuse_colour.xyz, specular_colour.xyz, pixel_normal, smoothness, reflectivity);

	float3 hdr_linear_col = standard_lighting_model_directional_light_UPDATED(float3(1.0f, 1.0f, 1.0f), light_vector, eye_vector, standard_mat);
	//float3 ldr_linear_col = saturate(tone_map_linear_hdr_pixel_value(hdr_linear_col));

	return float4(hdr_linear_col, 1.0f);
}

/////////////////////////////////////////////////////////////
//  TONE MAPPER     
/////////////////////////////////////////////////////////////
float3 tone_map_linear_hdr_pixel_value(in float3 linear_hdr_pixel_val)
{
	float4 hdr_CIE_LogYxy_pixel = HDR_RGB_To_HDR_CIE_Log_Y_xy(linear_hdr_pixel_val);
	float4 tone_mapped_ldr_CIE_Yxy_pixel = tone_map_HDR_CIE_Log_Y_xy_To_LDR_CIE_Yxy(hdr_CIE_LogYxy_pixel);
	float4 tone_mapped_ldr_linear_rgb = LDR_CIE_Yxy_To_Linear_LDR_RGB(tone_mapped_ldr_CIE_Yxy_pixel);

	return tone_mapped_ldr_linear_rgb.rgb;
}

float4 HDR_RGB_To_HDR_CIE_Log_Y_xy(in float3 linear_colour_val)
{
	float3x3 cie_transform_mat = {
		0.4124f, 0.3576f, 0.1805f,
		0.2126f, 0.7152f, 0.0722f,
		0.0193f, 0.1192f, 0.9505f };

	float3 cie_XYZ = mul(cie_transform_mat, linear_colour_val);
	float  denominator = cie_XYZ.x + cie_XYZ.y + cie_XYZ.z;
	float  x = cie_XYZ.x / max(denominator, real_approx_zero);
	float  y = cie_XYZ.y / max(denominator, real_approx_zero);

	return float4(log10(max(cie_XYZ.y, real_approx_zero)), x, y, cie_XYZ.y);
}

float4 tone_map_HDR_CIE_Log_Y_xy_To_LDR_CIE_Yxy(in float4 hdr_LogYxy)
{
	float black_point = Tone_Map_Black;
	float white_point = Tone_Map_White;
	float log_Y_black_point = log10(Tone_Map_Black);
	float log_Y_white_point = log10(Tone_Map_White);

	hdr_LogYxy.x = max(hdr_LogYxy.x, log_Y_black_point);

	float log_y_display_range = log_Y_white_point - log_Y_black_point;
	float log_y_in_white_black = (hdr_LogYxy.x - log_Y_black_point) / log_y_display_range;
	float log_y_in_white_black_scurve_biased = get_scurve_y_pos(log_y_in_white_black);
	float biased_log_y = log_Y_black_point + (log_y_in_white_black_scurve_biased * log_y_display_range);
	float biased_y = pow(10.0f, biased_log_y);
	float ldr_y = (biased_y - black_point) / (white_point - black_point);

	return float4(ldr_y, hdr_LogYxy.yzw);
}

float4 LDR_CIE_Yxy_To_Linear_LDR_RGB(in float4 ldr_cie_Yxy)
{
	float Y = ldr_cie_Yxy[0];
	float x = ldr_cie_Yxy[1];
	float y = ldr_cie_Yxy[2];

	float safe_denominator = max(y, real_approx_zero);
	float cie_Y = Y;
	float3 cie_XYZ = { x * cie_Y / safe_denominator , cie_Y , (1.0f - x - y) * cie_Y / safe_denominator };

	float3x3 cie_XYZ_toRGB_transform_mat = {
		+3.2405f, -1.5372f, -0.4985f,
		-0.9693f, +1.8760f, +0.0416f,
		+0.0556f, -0.2040f, +1.0572f };

	float3 rgb = mul(cie_XYZ_toRGB_transform_mat, cie_XYZ);
	rgb.xyz = max(float3(0.0f, 0.0f, 0.0f), rgb);

	return float4(rgb.xyz, 1.0f);
}

float get_scurve_y_pos(const float x_coord)
{
	float point0_y = 0.0f;
	float point1_y = low_tones_scurve_bias;
	float point2_y = high_tones_scurve_bias;
	float point3_y = 1.0f;

	float4  t = { x_coord * x_coord * x_coord, x_coord * x_coord, x_coord, 1.0f };

	float4x4 BASIS = {
		-1.0f,	+3.0f,	-3.0f,	+1.0f,
		+3.0f,	-6.0f,	+3.0f,	+0.0f,
		-3.0f,	+3.0f,	+0.0f,	+0.0f,
		+1.0f,	+0.0f,	+0.0f,	+0.0f };

	float4  g = mul(t, BASIS);

	return (point0_y * g.x) + (point1_y * g.y) + (point2_y * g.z) + (point3_y * g.w);
}
