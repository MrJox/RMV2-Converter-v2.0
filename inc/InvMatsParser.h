#ifndef INVMATSPARSER_H
#define INVMATSPARSER_H

class InvMatsParser
{
public:
	InvMatsParser();
	InvMatsParser(const InvMatsParser&) = delete;
	InvMatsParser(InvMatsParser&&) = delete;
	InvMatsParser& operator=(const InvMatsParser&) = delete;
	InvMatsParser& operator=(InvMatsParser&&) = delete;
	~InvMatsParser();

	bool Read(const std::string& path);
	inline void GetMatricesArray(std::vector<DirectX::XMFLOAT4X4>* matricesArray)const
	{
		*matricesArray = mMatricesArray;
	}

private:

	std::vector<DirectX::XMFLOAT4X4> mMatricesArray;
};

#endif	//	!INVMATSPARSER_H