#ifndef ANIMPARSER_H
#define ANIMPARSER_H

const static float factor = 32767.0f;

struct Frame
{
	std::vector<DirectX::XMFLOAT4> rotation;
	std::vector<DirectX::XMFLOAT3> position;
	uint32_t pad;
};

class AnimParser
{
public:
	AnimParser() :
		mBonesCount(0),
		mPosCount(0),
		mRotCount(0),
		mFramesCount(0)
	{ };
	~AnimParser() { };

protected:

	std::vector<std::string>	mBoneName;
	std::vector<int32_t>		mBoneParentID;
	uint32_t					mBonesCount;
	uint32_t					mPosCount;
	uint32_t					mRotCount;
	uint32_t					mFramesCount;
};

class Skeleton : AnimParser
{
public:
	Skeleton() { mFrame.position.resize(0); mFrame.rotation.resize(0); mFrame.pad = 0; };
	~Skeleton() { };

	bool Read(const std::string& startupPath, const std::string& skeletonName);

private:
	
	Frame	mFrame;
};

class Animation : AnimParser
{
public:
	Animation() :
		mDuration(0.0f),
		mFPS(0.0f)
	{ };
	~Animation() { };

	bool Read(const std::wstring& path);

private:

	std::vector<Frame>		mFrame;
	std::vector<int32_t>	mBonePosRemmapedID;
	std::vector<int32_t>	mBoneRotRemmapedID;
	float					mDuration;
	float					mFPS;
};

#endif // !ANIMPARSER_H