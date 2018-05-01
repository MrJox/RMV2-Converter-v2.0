#include "pch.h"
#include "AnimParser.h"
#include "InvMatsParser.h"

using namespace std;
using namespace DirectX;

bool Skeleton::Read(const string& startupPath, const string& skeletonName)
{
	string path = startupPath + "\\data\\skeletons\\" + skeletonName + ".anim";
	ifstream file(path.c_str(), ios::in | ios::binary);
	if (file.is_open())
	{
		uint16_t nameLength;

		file.seekg(12, ios::beg);
		file.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
		file.seekg(nameLength + 4, ios::cur);
		file.read(reinterpret_cast<char *>(&mBonesCount), sizeof(mBonesCount));

		for (size_t boneIndex = 0; boneIndex < mBonesCount; ++boneIndex)
		{
			vector<char> boneName;
			int32_t parentID;

			file.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
			boneName.resize(nameLength);
			file.read(reinterpret_cast<char *>(&boneName[0]), nameLength);
			file.read(reinterpret_cast<char *>(&parentID), sizeof(parentID));

			mBoneName.push_back(string(boneName.begin(), boneName.end()));
			mBoneParentID.push_back(parentID);
		}

		file.seekg(mBonesCount * 8, ios::cur);
		file.read(reinterpret_cast<char *>(&mPosCount), sizeof(mPosCount));
		file.read(reinterpret_cast<char *>(&mRotCount), sizeof(mRotCount));
		file.read(reinterpret_cast<char *>(&mFramesCount), sizeof(mFramesCount));

		XMFLOAT3 pos;
		XMFLOAT4 rot;
		for (size_t position = 0; position < mPosCount; ++position)
		{
			file.read(reinterpret_cast<char *>(&pos.x), sizeof(pos.x));
			file.read(reinterpret_cast<char *>(&pos.y), sizeof(pos.y));
			file.read(reinterpret_cast<char *>(&pos.z), sizeof(pos.z));

			mFrame.position.push_back(move(pos));
		}

		for (size_t rotation = 0; rotation < mRotCount; ++rotation)
		{
			uint16_t rotTemp;

			file.read(reinterpret_cast<char *>(&rotTemp), sizeof(rotTemp));
			rot.x = rotTemp / factor - 1.0f;
			file.read(reinterpret_cast<char *>(&rotTemp), sizeof(rotTemp));
			rot.y = rotTemp / factor - 1.0f;
			file.read(reinterpret_cast<char *>(&rotTemp), sizeof(rotTemp));
			rot.z = rotTemp / factor - 1.0f;
			file.read(reinterpret_cast<char *>(&rotTemp), sizeof(rotTemp));
			rot.w = rotTemp / factor - 1.0f;

			XMStoreFloat4(&rot, XMQuaternionNormalize(XMLoadFloat4(&rot)));
			mFrame.rotation.push_back(move(rot));
		}
	}
	else
		return false;

	file.close();

	InvMatsParser imp;
	string imp_path = startupPath + "\\data\\skeletons\\" + skeletonName + ".bone_inv_trans_mats";
	if (!imp.Read(imp_path))
		return false;
		
	return true;
}

bool Animation::Read(const wstring& path)
{
	ifstream file(path.c_str(), ios::in | ios::binary);
	if (file.is_open())
	{
		uint16_t nameLength;

		file.seekg(8, ios::beg);
		file.read(reinterpret_cast<char *>(&mFPS), sizeof(mFPS));
		file.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
		file.seekg(nameLength, ios::cur);
		file.read(reinterpret_cast<char *>(&mDuration), sizeof(mDuration));
		file.read(reinterpret_cast<char *>(&mBonesCount), sizeof(mBonesCount));

		for (size_t boneIndex = 0; boneIndex < mBonesCount; ++boneIndex)
		{
			vector<char> boneName;
			int32_t parentID;

			file.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
			boneName.resize(nameLength);
			file.read(reinterpret_cast<char *>(&boneName[0]), nameLength);
			file.read(reinterpret_cast<char *>(&parentID), sizeof(parentID));

			mBoneName.push_back(string(boneName.begin(), boneName.end()));
			mBoneParentID.push_back(parentID);
		}

		for (size_t bonePosID = 0; bonePosID < mBonesCount; ++bonePosID)
		{
			uint32_t remmapedID;
			file.read(reinterpret_cast<char *>(&remmapedID), sizeof(remmapedID));
			mBonePosRemmapedID.push_back(remmapedID);
		}

		for (size_t boneRotID = 0; boneRotID < mBonesCount; ++boneRotID)
		{
			uint32_t remmapedID;
			file.read(reinterpret_cast<char *>(&remmapedID), sizeof(remmapedID));
			mBoneRotRemmapedID.push_back(remmapedID);
		}

		file.read(reinterpret_cast<char *>(&mPosCount), sizeof(mPosCount));
		file.read(reinterpret_cast<char *>(&mRotCount), sizeof(mRotCount));
		file.read(reinterpret_cast<char *>(&mFramesCount), sizeof(mFramesCount));

		Frame _frame;
		XMFLOAT3 pos;
		XMFLOAT4 rot;
		for (size_t frame = 0; frame < mFramesCount; ++frame)
		{
			_frame.position.resize(0);
			_frame.rotation.resize(0);
			for (size_t position = 0; position < mPosCount; ++position)
			{
				file.read(reinterpret_cast<char *>(&pos.x), sizeof(pos.x));
				file.read(reinterpret_cast<char *>(&pos.y), sizeof(pos.y));
				file.read(reinterpret_cast<char *>(&pos.z), sizeof(pos.z));

				_frame.position.push_back(move(pos));
			}

			for (size_t rotation = 0; rotation < mRotCount; ++rotation)
			{
				uint16_t rotTemp;

				file.read(reinterpret_cast<char *>(&rotTemp), sizeof(rotTemp));
				rot.x = rotTemp / factor - 1.0f;
				file.read(reinterpret_cast<char *>(&rotTemp), sizeof(rotTemp));
				rot.y = rotTemp / factor - 1.0f;
				file.read(reinterpret_cast<char *>(&rotTemp), sizeof(rotTemp));
				rot.z = rotTemp / factor - 1.0f;
				file.read(reinterpret_cast<char *>(&rotTemp), sizeof(rotTemp));
				rot.w = rotTemp / factor - 1.0f;

				XMStoreFloat4(&rot, XMQuaternionNormalize(XMLoadFloat4(&rot)));
				_frame.rotation.push_back(move(rot));
			}
			mFrame.push_back(move(_frame));
		}
	}
	else
		return false;

	file.close();
	return true;
}