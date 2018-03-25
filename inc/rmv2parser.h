#pragma once
#ifndef RMV2PARSER_H
#define RMV2PARSER_H
using namespace DirectX;

const uint32_t SIGNATURE = 0x32564D52; // "RMV2"

enum class RigidMaterial : uint32_t
{
	bow_wave					= 22,
	non_renderable				= 26,
	texture_combo_vertex_wind	= 29,
	texture_combo				= 30,
	decal_waterfall				= 31,
	standard_simple				= 32,
	campaign_trees				= 34,
	point_light					= 38,
	static_point_light			= 45,
	debug_geometry				= 46,
	custom_terrain				= 49,
	weighted_cloth				= 58,
	cloth						= 60,
	collision					= 61,
	collision_shape				= 62,
	tiled_dirtmap				= 63,
	ship_ambientmap				= 64,
	weighted					= 65,
	projected_decal				= 67,
	default						= 68,
	grass						= 69,
	weighted_skin				= 70,
	decal						= 71,
	decal_dirtmap				= 72,
	dirtmap						= 73,
	tree						= 74,
	tree_leaf					= 75,
	weighted_decal				= 77,
	weighted_decal_dirtmap		= 78,
	weighted_dirtmap			= 79,
	weighted_skin_decal			= 80,
	weighted_skin_decal_dirtmap	= 81,
	weighted_skin_dirtmap		= 82,
	water						= 83,
	unlit						= 84,
	weighted_unlit				= 85,
	terrain_blend				= 86,
	projected_decal_v2			= 87,
	ignore						= 88,
	tree_billboard_material		= 89,
	water_displace_volume		= 91,
	rope						= 93,
	campaign_vegetation			= 94,
	projected_decal_v3			= 95,
	weighted_texture_blend		= 96,
	projected_decal_v4			= 97,
	global_terrain				= 98,
	decal_overlay				= 99,
	alpha_blend					= 100
};

enum class TextureID : uint32_t
{
	t_albedo			= 0,
	t_normal			= 1,
	t_mask				= 3,
	t_ambient_occlusion	= 5,
	t_tiling_dirt_uv2	= 7,
	t_dirt_alpha_mask	= 8,
	t_skin_mask			= 10,
	t_specular			= 11,
	t_gloss_map			= 12,
	t_decal_dirtmap		= 13,
	t_decal_dirtmask	= 14,
	t_decal_mask		= 15,
	t_diffuse_damage	= 17
};

enum class AlphaMode : uint32_t
{
	Opaque		= 0,
	Alpha_Test	= 1,
	Alpha_Blend	= 2
};

struct Header
{
	char		skeleton[128];
	uint32_t	version;
	uint32_t	lodsCount;
};

struct LodHeader
{
	uint32_t	groupsCount;
	uint32_t	lodOffset;
};

struct Vertex
{
	XMFLOAT3	position;
	float		weight;
	XMFLOAT3	normal;
	float		weight1;
	XMFLOAT3	tangent;
	float		pad;
	XMFLOAT3	bitangent;
	float		pad1;
	XMFLOAT2	texCoord;
	XMFLOAT2	texCoord2;
	byte		boneID0;
	byte		boneID1;
	byte		pad3[12];
};

struct Triangle
{
	uint16_t	index1;
	uint16_t	index2;
	uint16_t	index3;
};

struct Texture
{
	std::wstring texPath;
	TextureID	texID;
};

struct Group
{
	std::vector<Vertex>		vertices;
	std::vector<Triangle>	triangles;
	std::vector<Texture>	textures;

	XMFLOAT3		pivot;
	float			pad;
	RigidMaterial	materialID;
	uint32_t		verticesCount;
	uint32_t		indicesCount;
	uint32_t		texturesCount;
	char			groupName[32];
	AlphaMode		alphaMode;
};

class Mesh
{
public:
	Mesh();
	~Mesh();

	bool read_file(std::wstring filename);

	inline std::string GetGroupName(uint8_t lodNum, uint8_t groupNum)const
	{
		return std::string(m_model[lodNum][groupNum].groupName);
	}
	inline std::string GetSkeletonName()const
	{
		return std::string(m_header.skeleton);
	}
	inline std::string GetRigidMaterial(uint8_t lodNum, uint8_t groupNum)const
	{
		std::string rma;
		switch (m_model[lodNum][groupNum].materialID)
		{
			case RigidMaterial::alpha_blend:
				rma = "alpha_blend";
				break;
			case RigidMaterial::bow_wave:
				rma = "bow_wave";
				break;
			case RigidMaterial::campaign_trees:
				rma = "campaign_trees";
				break;
			case RigidMaterial::campaign_vegetation:
				rma = "campaign_vegetation";
				break;
			case RigidMaterial::cloth:
				rma = "cloth";
				break;
			case RigidMaterial::collision:
				rma = "collision";
				break;
			case RigidMaterial::collision_shape:
				rma = "collision_shape";
				break;
			case RigidMaterial::custom_terrain:
				rma = "custom_terrain";
				break;
			case RigidMaterial::debug_geometry:
				rma = "debug_geometry";
				break;
			case RigidMaterial::decal:
				rma = "decal";
				break;
			case RigidMaterial::decal_dirtmap:
				rma = "decal_dirtmap";
				break;
			case RigidMaterial::decal_overlay:
				rma = "decal_overlay";
				break;
			case RigidMaterial::decal_waterfall:
				rma = "decal_waterfall";
				break;
			case RigidMaterial::default:
				rma = "default";
				break;
			case RigidMaterial::dirtmap:
				rma = "dirtmap";
				break;
			case RigidMaterial::global_terrain:
				rma = "global_terrain";
				break;
			case RigidMaterial::grass:
				rma = "grass";
				break;
			case RigidMaterial::ignore:
				rma = "ignore";
				break;
			case RigidMaterial::non_renderable:
				rma = "non_renderable";
				break;
			case RigidMaterial::point_light:
				rma = "point_light";
				break;
			case RigidMaterial::projected_decal:
				rma = "projected_decal";
				break;
			case RigidMaterial::projected_decal_v2:
				rma = "projected_decal_v2";
				break;
			case RigidMaterial::projected_decal_v3:
				rma = "projected_decal_v3";
				break;
			case RigidMaterial::projected_decal_v4:
				rma = "projected_decal_v4";
				break;
			case RigidMaterial::rope:
				rma = "rope";
				break;
			case RigidMaterial::ship_ambientmap:
				rma = "ship_ambientmap";
				break;
			case RigidMaterial::standard_simple:
				rma = "standard_simple";
				break;
			case RigidMaterial::static_point_light:
				rma = "static_point_light";
				break;
			case RigidMaterial::terrain_blend:
				rma = "terrain_blend";
				break;
			case RigidMaterial::texture_combo:
				rma = "texture_combo";
				break;
			case RigidMaterial::texture_combo_vertex_wind:
				rma = "texture_combo_vertex_wind";
				break;
			case RigidMaterial::tiled_dirtmap:
				rma = "tiled_dirtmap";
				break;
			case RigidMaterial::tree:
				rma = "tree";
				break;
			case RigidMaterial::tree_billboard_material:
				rma = "tree_billboard_material";
				break;
			case RigidMaterial::tree_leaf:
				rma = "tree_leaf";
				break;
			case RigidMaterial::unlit:
				rma = "unlit";
				break;
			case RigidMaterial::water:
				rma = "water";
				break;
			case RigidMaterial::water_displace_volume:
				rma = "water_displace_volume";
				break;
			case RigidMaterial::weighted:
				rma = "weighted";
				break;
			case RigidMaterial::weighted_cloth:
				rma = "weighted_cloth";
				break;
			case RigidMaterial::weighted_decal:
				rma = "weighted_decal";
				break;
			case RigidMaterial::weighted_decal_dirtmap:
				rma = "weighted_decal_dirtmap";
				break;
			case RigidMaterial::weighted_dirtmap:
				rma = "weighted_dirtmap";
				break;
			case RigidMaterial::weighted_skin:
				rma = "weighted_skin";
				break;
			case RigidMaterial::weighted_skin_decal:
				rma = "weighted_skin_decal";
				break;
			case RigidMaterial::weighted_skin_decal_dirtmap:
				rma = "weighted_skin_decal_dirtmap";
				break;
			case RigidMaterial::weighted_skin_dirtmap:
				rma = "weighted_skin_dirtmap";
				break;
			case RigidMaterial::weighted_texture_blend:
				rma = "weighted_texture_blend";
				break;
			case RigidMaterial::weighted_unlit:
				rma = "weighted_unlit";
				break;
			default:
				rma = "undefined";
		}
		return rma;
	}
	inline std::string GetAlphaMode(uint8_t lodNum, uint8_t groupNum)const
	{
		std::string amode;
		switch (m_model[lodNum][groupNum].alphaMode)
		{
			case AlphaMode::Alpha_Blend:
				amode = "Alpha Blend (2)";
				break;
			case AlphaMode::Alpha_Test:
				amode = "Alpha Test (1)";
				break;
			case AlphaMode::Opaque:
				amode = "Opaque (0)";
				break;
			default:
				amode = "unknown";
		}
		return amode;
	}
	inline size_t GetTexturesCount(uint8_t lodNum, uint8_t groupNum)const
	{
		return m_model[lodNum][groupNum].texturesCount;
	}
	inline std::wstring GetTexturePath(uint8_t lodNum, uint8_t groupNum, TextureID tId)const
	{
		std::wstring path = L"";
		for (size_t i = 0; i < m_model[lodNum][groupNum].texturesCount; ++i)
			if (tId == m_model[lodNum][groupNum].textures[i].texID)
				path = m_model[lodNum][groupNum].textures[i].texPath;
		return path;
	}
	inline size_t GetLodsCount()const
	{
		return m_model.size();
	}
	inline size_t GetGroupsCount(uint8_t lodNum)const
	{
		return m_model[lodNum].size();
	}
	inline size_t GetVerticesCountPerGroup(uint8_t lodNum, uint8_t groupNum)const
	{
		return m_model[lodNum][groupNum].verticesCount;
	}
	inline size_t GetIndicesCountPerGroup(uint8_t lodNum, uint8_t groupNum)const
	{
		return m_model[lodNum][groupNum].indicesCount;
	}
	inline void GetVerticesArray(uint8_t lodNum, uint8_t groupNum, std::vector<Vertex>* verticesArray)const
	{
		*verticesArray = m_model[lodNum][groupNum].vertices;
	}
	inline void GetIndicesArray(uint8_t lodNum, uint8_t groupNum, std::vector<Triangle>* indicesArray)const
	{
		*indicesArray = m_model[lodNum][groupNum].triangles;
	}

private:
	Header							m_header;
	std::vector<LodHeader>			m_lodHeader;
	std::vector<std::vector<Group>>	m_model;

};

#endif	//	!RMV2PARSER_H