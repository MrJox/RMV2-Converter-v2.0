#include "pch.h"
#include "rmv2parser.h"
#include "Half.h"

using namespace std;

void read_default(ifstream&, const FXMVECTOR&, Vertex*);
void read_weighted(ifstream&, const FXMVECTOR&, Vertex*);
void read_cinematic(ifstream&, const FXMVECTOR&, Vertex*);
//void read_tree(ifstream&, const FXMVECTOR&, Vertex*);
void replace_texture_path(char*, wstring*);

Mesh::Mesh()
{
	m_header = { };
	m_lodHeader.resize(0);
}

Mesh::~Mesh()
{
}

bool Mesh::read_file(const std::wstring& filename)
{
	this->m_header.lodsCount = 0;
	this->m_header.version = 0;
	strcpy_s(this->m_header.skeleton, "");
	this->m_lodHeader.resize(0);
	this->m_model.resize(0);

	ifstream file(filename.c_str(), ios::in | ios::binary);
	if (file.is_open())
	{
		uint32_t temp;
		LodHeader lh;
		Group group;
		Texture texture;
		Vertex vertex;
		Triangle triangle;
		std::vector<uint16_t> vertexFormat;
		std::vector<Group> lod;
		uint32_t attp_count;

		file.read(reinterpret_cast<char *>(&temp), 4);
		if (temp != SIGNATURE)
		{
			MessageBoxA(nullptr, "The file you're attempting to read is not a rmv2 file!", "Error: Incorrect file", MB_OK);
			return false;
		}
		
		file.read(reinterpret_cast<char *>(&m_header.version), sizeof(m_header.version));
		if (m_header.version == 5)
		{
			MessageBoxA(nullptr, "SHOGUN2 file format is not supported yet!", "Error: Unsupported version", MB_OK);
			return false;
		}
		else if (m_header.version < 5)
		{
			MessageBoxA(nullptr, "Too old file formats are not supported!", "Error: Unsupported version", MB_OK);
			return false;
		}

		file.read(reinterpret_cast<char *>(&m_header.lodsCount), sizeof(m_header.lodsCount));
		file.read(reinterpret_cast<char *>(&m_header.skeleton), sizeof(m_header.skeleton));

		for (size_t i = 0; i < m_header.lodsCount; ++i)
		{
			file.read(reinterpret_cast<char *>(&lh.groupsCount), sizeof(lh.groupsCount));
			file.seekg(8, ios_base::cur);
			file.read(reinterpret_cast<char *>(&lh.lodOffset), sizeof(lh.lodOffset));
			file.seekg(4, ios_base::cur);
			if (m_header.version == 7)
				file.seekg(8, ios_base::cur);
			m_lodHeader.push_back(lh);
		}

		for (size_t i = 0; i < m_header.lodsCount; ++i)
		{
			for (size_t j = 0; j < m_lodHeader[i].groupsCount; ++j)
			{
				file.read(reinterpret_cast<char *>(&group.materialID), sizeof(group.materialID));
				
				if (   group.materialID == RigidMaterial::bow_wave
					|| group.materialID == RigidMaterial::projected_decal
					|| group.materialID == RigidMaterial::projected_decal_v2
					|| group.materialID == RigidMaterial::projected_decal_v3
					|| group.materialID == RigidMaterial::projected_decal_v4
					|| group.materialID == RigidMaterial::alpha_blend
					|| group.materialID == RigidMaterial::static_point_light
					|| group.materialID == RigidMaterial::cloth	)
				{
					MessageBoxA(nullptr, "Warning: One of the specified rigid materials is not supported! This may cause errors during the runtime.", "Error: Rigid material support", MB_OK);
					return m_model.size() == 0 ? false : true;
				}
				else
				{
					file.seekg(8, ios_base::cur);
					file.read(reinterpret_cast<char *>(&group.verticesCount), sizeof(group.verticesCount));
					file.seekg(4, ios_base::cur);
					file.read(reinterpret_cast<char *>(&group.indicesCount), sizeof(group.indicesCount));
					file.seekg(56, ios_base::cur);
					file.read(reinterpret_cast<char *>(&temp), 2);

					vertexFormat.push_back(static_cast<uint16_t>(temp));
					if (   vertexFormat[j]	== 6
						|| vertexFormat[j]	== 11
						|| vertexFormat[j]	== 12)
					{
						MessageBoxA(nullptr, "Trees, ropes and campaign vegetation are not yet supported!", "Error: Unsupported vertex format", MB_OK);
						return false;
					}

					file.read(reinterpret_cast<char *>(&group.groupName), sizeof(group.groupName));
					file.seekg(514, ios_base::cur);
					file.read(reinterpret_cast<char *>(&group.pivot.x), sizeof(group.pivot.x));
					file.read(reinterpret_cast<char *>(&group.pivot.y), sizeof(group.pivot.y));
					file.read(reinterpret_cast<char *>(&group.pivot.z), sizeof(group.pivot.z));
					file.seekg(152, ios_base::cur);
					file.read(reinterpret_cast<char *>(&attp_count), sizeof(attp_count));
					file.read(reinterpret_cast<char *>(&group.texturesCount), sizeof(group.texturesCount));
					file.seekg(140, ios_base::cur);
					file.seekg(attp_count * 84, ios_base::cur);

					for (size_t k = 0; k < group.texturesCount; ++k)
					{
						char tex[256];
						file.read(reinterpret_cast<char *>(&texture.texID), sizeof(texture.texID));
						file.read(reinterpret_cast<char *>(&tex), sizeof(tex));
						replace_texture_path(tex, &texture.texPath);
						texture.texPath = filename.substr(0, filename.find_last_of(L"/\\")) + texture.texPath;
						group.textures.push_back(texture);
					}

					file.seekg(4, ios_base::cur);
					if (group.materialID == RigidMaterial::tiled_dirtmap)
						file.seekg(16, ios_base::cur);
					else if (group.materialID == RigidMaterial::decal
						  || group.materialID == RigidMaterial::weighted_decal
						  || group.materialID == RigidMaterial::weighted_skin_decal)
						file.seekg(20, ios_base::cur);
					else if (group.materialID == RigidMaterial::dirtmap
						  || group.materialID == RigidMaterial::weighted_dirtmap
						  || group.materialID == RigidMaterial::weighted_skin_dirtmap)
						file.seekg(32, ios_base::cur);
					else if (group.materialID == RigidMaterial::decal_dirtmap
						  || group.materialID == RigidMaterial::weighted_decal_dirtmap
						  || group.materialID == RigidMaterial::weighted_skin_decal_dirtmap)
						file.seekg(52, ios_base::cur);
					else if (group.materialID == RigidMaterial::tree)
						file.seekg(56, ios_base::cur);

					file.read(reinterpret_cast<char *>(&group.alphaMode), sizeof(group.alphaMode));

					switch (vertexFormat[j])
					{
						case 0:
						{
							for (size_t k = 0; k < group.verticesCount; ++k)
							{
								read_default(file, XMLoadFloat3(&group.pivot), &vertex);
								group.vertices.push_back(vertex);
							}
							break;
						}
						case 3:
						{
							for (size_t k = 0; k < group.verticesCount; ++k)
							{
								read_weighted(file, XMLoadFloat3(&group.pivot), &vertex);
								group.vertices.push_back(vertex);
							}
							break;
						}
						case 4:
						{
							for (size_t k = 0; k < group.verticesCount; ++k)
							{
								read_cinematic(file, XMLoadFloat3(&group.pivot), &vertex);
								group.vertices.push_back(vertex);
							}
							break;
						}
						case 6:
						{
							//read_tree(file, XMLoadFloat3(&group.pivot), &vertex);
							return false;
						}
						case 11:
						{
							return false;
						}
						case 12:
						{
							return false;
						}

						default:
						{
							MessageBoxA(nullptr, "The specified vertex format is not yet supported!", "Error: Unsupported vertex format", MB_OK);
							return false;
						}
					}

					for (size_t k = 0; k < group.indicesCount / 3; ++k)
					{
						file.read(reinterpret_cast<char *>(&triangle.index1), sizeof(triangle.index1));
						file.read(reinterpret_cast<char *>(&triangle.index2), sizeof(triangle.index2));
						file.read(reinterpret_cast<char *>(&triangle.index3), sizeof(triangle.index3));

						group.triangles.push_back(triangle);
					}

					lod.push_back(group);
					group.textures.resize(0);
					group.vertices.resize(0);
					group.triangles.resize(0);
				}
			}
			m_model.push_back(lod);
			lod.resize(0);
			vertexFormat.resize(0);
		}
	}

	file.close();
	return true;
}

void read_default(ifstream& file, const FXMVECTOR& pivot, Vertex* vertex)
{
	HalfFloat t_pos;
	byte t_norm;
	XMFLOAT3 piv;
	XMStoreFloat3(&piv, pivot);

	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).position.x = t_pos + piv.x;
	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).position.y = t_pos + piv.y;
	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).position.z = t_pos + piv.z;
	file.seekg(2, ios_base::cur);

	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).texCoord.x = t_pos;
	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).texCoord.y = 1.0f - t_pos;
	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).texCoord2.x = t_pos;
	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).texCoord2.y = 1.0f - t_pos;

	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).normal.x = -(t_norm / 127.5f - 1.0f);
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).normal.y = t_norm / 127.5f - 1.0f;
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).normal.z = t_norm / 127.5f - 1.0f;
	XMStoreFloat3(&(*vertex).normal, XMVector3Normalize(XMLoadFloat3(&(*vertex).normal)));
	file.seekg(1, ios_base::cur);

	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).bitangent.x = -(t_norm / 127.5f - 1.0f);
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).bitangent.y = t_norm / 127.5f - 1.0f;
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).bitangent.z = t_norm / 127.5f - 1.0f;
	XMStoreFloat3(&(*vertex).bitangent, XMVector3Normalize(XMLoadFloat3(&(*vertex).bitangent)));
	file.seekg(1, ios_base::cur);

	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).tangent.x = -(t_norm / 127.5f - 1.0f);
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).tangent.y = t_norm / 127.5f - 1.0f;
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).tangent.z = t_norm / 127.5f - 1.0f;
	XMStoreFloat3(&(*vertex).tangent, XMVector3Normalize(XMLoadFloat3(&(*vertex).tangent)));
	file.seekg(5, ios_base::cur);
}

void read_weighted(ifstream& file, const FXMVECTOR& pivot, Vertex* vertex)
{
	HalfFloat t_pos;
	byte t_norm;
	XMFLOAT3 piv;
	XMStoreFloat3(&piv, pivot);

	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).position.x = t_pos + piv.x;
	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).position.y = t_pos + piv.y;
	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).position.z = t_pos + piv.z;
	file.seekg(2, ios_base::cur);

	file.read(reinterpret_cast<char *>(&(*vertex).boneID0), sizeof((*vertex).boneID0));
	file.read(reinterpret_cast<char *>(&(*vertex).boneID1), sizeof((*vertex).boneID1));
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).weight = t_norm / 255.0f;
	(*vertex).weight1 = 1.0f - (*vertex).weight;
	file.seekg(1, ios_base::cur);

	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).normal.x = -(t_norm / 127.5f - 1.0f);
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).normal.y = t_norm / 127.5f - 1.0f;
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).normal.z = t_norm / 127.5f - 1.0f;
	XMStoreFloat3(&(*vertex).normal, XMVector3Normalize(XMLoadFloat3(&(*vertex).normal)));
	file.seekg(1, ios_base::cur);

	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).texCoord.x = t_pos;
	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).texCoord.y = 1.0f - t_pos;

	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).bitangent.x = -(t_norm / 127.5f - 1.0f);
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).bitangent.y = t_norm / 127.5f - 1.0f;
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).bitangent.z = t_norm / 127.5f - 1.0f;
	XMStoreFloat3(&(*vertex).bitangent, XMVector3Normalize(XMLoadFloat3(&(*vertex).bitangent)));
	file.seekg(1, ios_base::cur);

	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).tangent.x = -(t_norm / 127.5f - 1.0f);
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).tangent.y = t_norm / 127.5f - 1.0f;
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).tangent.z = t_norm / 127.5f - 1.0f;
	XMStoreFloat3(&(*vertex).tangent, XMVector3Normalize(XMLoadFloat3(&(*vertex).tangent)));
	file.seekg(1, ios_base::cur);
}

void read_cinematic(ifstream& file, const FXMVECTOR& pivot, Vertex* vertex)
{
	HalfFloat t_pos;
	byte t_norm;
	XMFLOAT3 piv;
	XMStoreFloat3(&piv, pivot);

	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).position.x = -(t_pos + piv.x);
	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).position.y = t_pos + piv.y;
	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).position.z = t_pos + piv.z;
	file.seekg(2, ios_base::cur);

	file.seekg(8, ios_base::cur);

	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).normal.x = -(t_norm / 127.5f - 1.0f);
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).normal.y = t_norm / 127.5f - 1.0f;
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).normal.z = t_norm / 127.5f - 1.0f;
	XMStoreFloat3(&(*vertex).normal, XMVector3Normalize(XMLoadFloat3(&(*vertex).normal)));
	file.seekg(1, ios_base::cur);

	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).texCoord.x = t_pos;
	file.read(reinterpret_cast<char *>(&t_pos), sizeof(t_pos));
	(*vertex).texCoord.y = 1.0f - t_pos;

	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).bitangent.x = -(t_norm / 127.5f - 1.0f);
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).bitangent.y = t_norm / 127.5f - 1.0f;
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).bitangent.z = t_norm / 127.5f - 1.0f;
	XMStoreFloat3(&(*vertex).bitangent, XMVector3Normalize(XMLoadFloat3(&(*vertex).bitangent)));
	file.seekg(1, ios_base::cur);

	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).tangent.x = -(t_norm / 127.5f - 1.0f);
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).tangent.y = t_norm / 127.5f - 1.0f;
	file.read(reinterpret_cast<char *>(&t_norm), sizeof(t_norm));
	(*vertex).tangent.z = t_norm / 127.5f - 1.0f;
	XMStoreFloat3(&(*vertex).tangent, XMVector3Normalize(XMLoadFloat3(&(*vertex).tangent)));
	file.seekg(1, ios_base::cur);
}

void replace_texture_path(char* temp, wstring* texture)
{
	wstring ws(&temp[0], &temp[256]);
	*texture = ws.substr(ws.find_last_of(L"/\\") + 1);
	*texture = L"\\tex\\" + *texture;
}