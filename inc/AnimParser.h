#ifndef ANIMPARSER_H
#define ANIMPARSER_H

const static float factor = 32767.0f;

struct Frame
{
	std::vector<DirectX::XMFLOAT4> rotation;
	std::vector<DirectX::XMFLOAT3> position;
};

class AnimParser
{
public:
	AnimParser() :
		mBonesCount(0),
		mPosCount(0),
		mRotCount(0),
		mFramesCount(0),
		mRootBonesCount(0)
	{ };
	virtual ~AnimParser() { };

	virtual bool Read(const std::string& path) = 0;
	inline uint32_t GetRootBonesCount()const
	{
		return mRootBonesCount;
	}
	inline uint32_t GetBonesCount()const
	{
		return mBonesCount;
	}
	inline uint32_t GetChildBonesCount(const uint32_t& index)const
	{
		return mChildBonesCount[index];
	}
	inline int32_t GetBoneParentID(const uint32_t& index)const
	{
		return mBoneParentID[index];
	}
	inline std::string GetBoneName(const uint32_t& index)const
	{
		return mBoneName[index];
	}
	void GetChildIDs(const int32_t& parentID, std::vector<uint32_t>* vector)const
	{
		for (uint32_t childID = 0; childID < mBonesCount; ++childID)
			if (mBoneParentID[childID] == parentID)
				vector->push_back(childID);
	}

protected:

	void QuaternionToEuler(const DirectX::XMFLOAT4&, DirectX::XMFLOAT3&)const;

	std::vector<std::string>	mBoneName;
	std::vector<int32_t>		mBoneParentID;
	std::vector<uint32_t>		mChildBonesCount;
	uint32_t					mBonesCount;
	uint32_t					mPosCount;
	uint32_t					mRotCount;
	uint32_t					mFramesCount;
	uint32_t					mRootBonesCount;
};

class Skeleton : public AnimParser
{
public:
	Skeleton() { mFrame.position.resize(0); mFrame.rotation.resize(0); };
	virtual ~Skeleton() { };

	virtual bool Read(const std::string& path)final;
	inline DirectX::XMFLOAT3 GetPosByID(const uint32_t& index)const
	{
		return mFrame.position[index];
	}
	DirectX::XMVECTOR GetRotationVectorByID(const uint32_t& index)const
	{
		DirectX::XMFLOAT3 eulerAngles;
		QuaternionToEuler(mFrame.rotation[index], eulerAngles);

		return XMLoadFloat3(&eulerAngles);
	}

private:
	
	Frame mFrame;
};

class Animation : public AnimParser
{
public:
	Animation() :
		mDuration(0.0f),
		mFPS(0.0f)
	{ };
	virtual ~Animation() { };

	virtual bool Read(const std::string& path)final;
	inline DirectX::XMFLOAT3 GetPosByFrameAndID(const uint32_t& frame, const uint32_t& index)const
	{
		return mFrame[frame].position[index];
	}

private:

	std::vector<Frame>		mFrame;
	std::vector<int32_t>	mBonePosRemmapedID;
	std::vector<int32_t>	mBoneRotRemmapedID;
	float					mDuration;
	float					mFPS;
};

#endif // !ANIMPARSER_H