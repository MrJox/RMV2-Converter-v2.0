#include "pch.h"
#include "InvMatsParser.h"
using namespace std;

InvMatsParser::InvMatsParser()
{
}

InvMatsParser::~InvMatsParser()
{
}

bool InvMatsParser::Read(const string& path)
{
	ifstream file(path.c_str(), ios::in | ios::binary);
	if (file.is_open())
	{
		DirectX::XMFLOAT4X4 matrix;
		uint32_t bonesCount;

		file.seekg(4, ios::beg);
		file.read(reinterpret_cast<char *>(&bonesCount), sizeof(bonesCount));

		for (size_t mat = 0; mat < bonesCount; ++mat)
		{
			file.read(reinterpret_cast<char *>(&matrix._11), sizeof(matrix._11));
			file.read(reinterpret_cast<char *>(&matrix._31), sizeof(matrix._31));
			file.read(reinterpret_cast<char *>(&matrix._21), sizeof(matrix._21));
			matrix._41 = 0.0f;

			file.read(reinterpret_cast<char *>(&matrix._12), sizeof(matrix._12));
			file.read(reinterpret_cast<char *>(&matrix._32), sizeof(matrix._32));
			file.read(reinterpret_cast<char *>(&matrix._22), sizeof(matrix._22));
			matrix._42 = 0.0f;

			file.read(reinterpret_cast<char *>(&matrix._13), sizeof(matrix._13));
			file.read(reinterpret_cast<char *>(&matrix._33), sizeof(matrix._33));
			file.read(reinterpret_cast<char *>(&matrix._23), sizeof(matrix._23));
			matrix._43 = 0.0f;

			file.read(reinterpret_cast<char *>(&matrix._14), sizeof(matrix._14));
			file.read(reinterpret_cast<char *>(&matrix._34), sizeof(matrix._34));
			file.read(reinterpret_cast<char *>(&matrix._24), sizeof(matrix._24));
			matrix._44 = 1.0f;
			
			mMatricesArray.push_back(move(matrix));
		}
	}
	else
		return false;

	file.close();
	return true;
}