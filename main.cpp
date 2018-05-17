#include "inc\pch.h"
#include "inc\Core.h"
#include "inc\Camera.h"
#include "inc\DDSTextureLoader.h"
#include "inc\DaeExporter.h"

using namespace DirectX;
using namespace std;

struct Verts
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	XMFLOAT3 Bitangent;
	XMFLOAT2 TexCoord0;
	XMFLOAT2 TexCoord1;
};

struct buf_per_obj
{
	XMFLOAT4X4 worldViewProj;
	XMFLOAT4X4 world;
	XMFLOAT4X4 worldViewInverse;
	XMFLOAT4X4 viewInverse;
	XMFLOAT4X4 view;
	XMFLOAT4 lightVec;
};

struct Material
{
	ComPtr<ID3D11ShaderResourceView> diffuse;
	ComPtr<ID3D11ShaderResourceView> normal;
	ComPtr<ID3D11ShaderResourceView> specular;
	ComPtr<ID3D11ShaderResourceView> gloss_map;
};

HRESULT CreateTexture(ID3D11Device1*, const Mesh&, const uint8_t& groupNum, const TextureID&, ID3D11ShaderResourceView**);
void DisplayFPS(const Timer&, const wstring&, const HWND&);

class Converter : public Core
{
public:
	Converter(HINSTANCE hInstance);
	Converter(const Converter&) = delete;
	Converter(Converter&&) = delete;
	Converter& operator=(const Converter&) = delete;
	Converter& operator=(Converter&&) = delete;
	~Converter();

	virtual bool Initialize()override;

private:

	virtual void OnResize(uint32_t width, uint32_t height)final;
	virtual void Update(const Timer& timer)final;
	virtual void Render()final;
	virtual void ReadModel(const wstring& filename)final;
	virtual void ReadAnim(const string& filename)final;
	virtual void ExportMesh()final;
	virtual void ExportMeshAnim()final;
	virtual void ShowStats()final;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)final;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)final;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)final;
	virtual void OnMouseScroll(WPARAM delta)final;

	bool LoadTextures();
	void InitializeInputLayout();
	void InitializeBuffers();
	void RenderModels(const size_t&);

	ComPtr<ID3D11Buffer>		m_vertexBuffer;
	ComPtr<ID3D11Buffer>		m_indexBuffer;
	ComPtr<ID3D11Buffer>		m_cbPerObj;
	ComPtr<ID3D11Buffer>		m_cbPerFrame;

	ComPtr<ID3D11InputLayout>	m_inputLayout;
	ComPtr<ID3D11VertexShader>	m_vertexShader;
	ComPtr<ID3D11PixelShader>	m_pixelShader;
	ComPtr<ID3D11SamplerState>	m_samplerState;

	Camera						m_camera;
	Mesh						m_mesh;
	Animation					m_animation;
	vector<Material>			m_material;

	wstring						m_execPath;
	string						m_exportName;
	vector<UINT>				m_offset;
	vector<UINT>				m_indSize;
	UINT						m_modelsCount;
	POINT						m_lastMousePos;
};

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	Converter Converter(hInstance);
	if (!Converter.Initialize())
		return 0;

	return Converter.Run();
}

Converter::Converter(HINSTANCE hInstance) :
	Core(hInstance),

	m_vertexBuffer(0),
	m_indexBuffer(0),
	m_cbPerObj(0),
	m_cbPerFrame(0),

	m_inputLayout(nullptr),
	m_vertexShader(nullptr),
	m_pixelShader(nullptr),
	m_samplerState(nullptr),

	m_modelsCount(1)
{
	m_wndCaption = L"RMV2 Converter v2.0";
	m_material.resize(0);
	m_offset.resize(0);
	m_indSize.resize(0);

	m_lastMousePos.x = 0;
	m_lastMousePos.y = 0;
}

Converter::~Converter()
{
}

bool Converter::Initialize()
{
	if (!Core::Initialize())
		return false;

	wchar_t buf[MAX_PATH];
	GetModuleFileNameW(nullptr, buf, MAX_PATH);
	m_execPath = wstring(buf);
	m_execPath = m_execPath.substr(0, m_execPath.find_last_of(L"/\\"));

	return true;
}

void Converter::OnResize(uint32_t width, uint32_t height)
{
	Core::OnResize(width, height);
	m_camera.SetProj(XM_PIDIV4, width, height, 0.1f, 1000.0f);
}

void Converter::ReadModel(const wstring& filename)
{
#ifndef _DEBUG
	wstring wfilename(filename.length(), L' ');
	copy(filename.begin(), filename.end(), wfilename.begin());
	wfilename = wfilename.substr(wfilename.find_last_of(L"/\\") + 1);

	SetWindowTextW(m_hWnd, (m_wndCaption + L"    " + wfilename).c_str());
#endif

	if (m_material.size() > 0)
		m_material.resize(0);
	if (m_indSize.size() > 0)
		m_indSize.resize(0);
	if (m_offset.size() > 0)
		m_offset.resize(0);

	if (!m_mesh.read_file(filename))
	{
		m_cbPerObj.Reset();
		return;
	}
	m_modelsCount = m_mesh.GetGroupsCount(0);

	if (!LoadTextures())
	{
		MessageBoxA(m_hWnd, "Error: At least one of the textures was not found. Check textures or open another file.", "Texture not found", MB_OK);
		m_cbPerObj.Reset();
		return;
	}

	InitializeInputLayout();
	InitializeBuffers();

	m_exportName = string(filename.begin(), filename.end());
	m_exportName = m_exportName.substr(m_exportName.find_last_of("/\\") + 1);
	m_exportName = string(m_execPath.begin(), m_execPath.end()) + "\\data\\exported\\" + m_exportName.erase(m_exportName.size() - 15) + ".dae";
	m_camera.ResetPosition();
}

void Converter::ReadAnim(const string& filename)
{
	if (!m_animation.Read(filename))
	{
		MessageBoxA(m_hWnd, "Error: Animation file wasn't found or was corrupted", "Animation not found", MB_OK);
		return;
	}
}

void Converter::ExportMesh()
{
	if (!m_exportName.empty())
	{
		DaeExporter exporter(m_exportName, m_mesh);
		if (!exporter.ExportMesh())
		{
			MessageBoxA(m_hWnd, "Error. Couldn't write the file to /data/exported/", "Export Error", MB_OK);
			return;
		}

		MessageBoxA(m_hWnd, "Done! Your file is in /data/exported/", "Export Successful", MB_OK);
	}
	else
	{
		MessageBoxA(m_hWnd, "Error. Open RMV2 file first!", "Export Error", MB_OK);
		return;
	}
}

void Converter::ExportMeshAnim()
{
	if (!m_exportName.empty())
	{
		DaeExporter exporter(m_exportName);
		if (!exporter.ExportMeshAnim())
		{
			MessageBoxA(m_hWnd, "Error. Couldn't write the file to /data/exported/", "Export Error", MB_OK);
			return;
		}

		MessageBoxA(m_hWnd, "Done! Your file is in /data/exported/", "Export Successful", MB_OK);
	}
	else
	{
		MessageBoxA(m_hWnd, "Error. Open RMV2 file first!", "No file to write", MB_OK);
		return;
	}
}

void Converter::ShowStats()
{
	if (m_cbPerObj)
	{
		string output = "skeleton: " + m_mesh.GetSkeletonName() + "\n\n";
		if (m_mesh.GetSkeletonName().empty())
			output = "skeleton: none\n\n";

		size_t lodsCount = m_mesh.GetLodsCount();
		for (size_t lodsNum = 0; lodsNum < lodsCount; ++lodsNum)
		{
			output += "LOD" + to_string(lodsNum + 1) + ":\n";

			for (size_t groupsNum = 0; groupsNum < m_mesh.GetGroupsCount(static_cast<uint8_t>(lodsNum)); ++groupsNum)
			{
				output += string(m_mesh.GetGroupName(static_cast<uint8_t>(lodsNum), static_cast<uint8_t>(groupsNum))) + ":\n";
				output += "rigid_material: " + m_mesh.GetRigidMaterial(static_cast<uint8_t>(lodsNum), static_cast<uint8_t>(groupsNum)) + "\n";
				output += "vertices count: " + to_string(m_mesh.GetVerticesCountPerGroup(static_cast<uint8_t>(lodsNum), static_cast<uint8_t>(groupsNum))) + "\n";
				output += "indices count: " + to_string(m_mesh.GetIndicesCountPerGroup(static_cast<uint8_t>(lodsNum), static_cast<uint8_t>(groupsNum))) + "\n";
				output += "alpha mode: " + m_mesh.GetAlphaMode(static_cast<uint8_t>(lodsNum), static_cast<uint8_t>(groupsNum)) + "\n";
				
				output += "\n";
			}
		}

		MessageBoxA(m_hWnd, output.c_str(), "Model statistics", MB_OK);
	}
	else
		MessageBoxA(m_hWnd, "Select a file first!", "Model statistics", MB_OK);
}

void Converter::Update(const Timer& t)
{
#ifdef _DEBUG
	DisplayFPS(t, m_wndCaption, m_hWnd);
#else
	UNREFERENCED_PARAMETER(t);
#endif
	m_camera.Update();
}

void Converter::Render()
{
	D3D11_MAPPED_SUBRESOURCE m_subresource;

	static const FLOAT f[4] = { 0.533f, 0.745f, 0.882f, 0.0f };
	m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), f);
	m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
	
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (m_cbPerObj)
	{
		UINT stride = sizeof(Verts);
		UINT offset = 0;

		m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, offset);

		XMMATRIX view = m_camera.GetView();
		XMMATRIX proj = m_camera.GetProj();
		XMMATRIX viewProj = view * proj;
		XMMATRIX viewInv = XMMatrixInverse(nullptr, view);

		m_d3dContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
		m_d3dContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

		m_d3dContext->Map(m_cbPerObj.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m_subresource);
			buf_per_obj* data_perObj = (buf_per_obj*)m_subresource.pData;
			XMStoreFloat4x4(&data_perObj->worldViewProj, viewProj);
			XMStoreFloat4x4(&data_perObj->world, XMMatrixIdentity());
			XMStoreFloat4x4(&data_perObj->worldViewInverse, viewInv);
			XMStoreFloat4x4(&data_perObj->viewInverse, viewInv);
			XMStoreFloat4x4(&data_perObj->view, view);
			XMStoreFloat4(&data_perObj->lightVec, m_camera.GetPositionV());
		m_d3dContext->Unmap(m_cbPerObj.Get(), 0);

		m_d3dContext->VSSetConstantBuffers(0, 1, m_cbPerObj.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers(0, 1, m_cbPerObj.GetAddressOf());

		for (size_t modelNum = 0; modelNum < m_modelsCount; ++modelNum)
			RenderModels(modelNum);
	}
	m_swapChain->Present(1, 0);
}

void Converter::RenderModels(const size_t& modelNum)
{
	m_d3dContext->PSSetShaderResources(0, 1, m_material[modelNum].diffuse.GetAddressOf());
	m_d3dContext->PSSetShaderResources(1, 1, m_material[modelNum].normal.GetAddressOf());
	m_d3dContext->PSSetShaderResources(2, 1, m_material[modelNum].gloss_map.GetAddressOf());
	m_d3dContext->PSSetShaderResources(3, 1, m_material[modelNum].specular.GetAddressOf());
	m_d3dContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	m_d3dContext->DrawIndexed(m_indSize[modelNum], m_offset[modelNum], 0);
}

void Converter::OnMouseDown(WPARAM btnState, int x, int y)
{
	UNREFERENCED_PARAMETER(btnState);

	m_lastMousePos.x = x;
	m_lastMousePos.y = y;

	SetCapture(m_hWnd);
}

void Converter::OnMouseUp(WPARAM btnState, int x, int y)
{
	UNREFERENCED_PARAMETER(btnState);
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);

	ReleaseCapture();
}

void Converter::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.5f*static_cast<float>(x - m_lastMousePos.x));
		float dy = XMConvertToRadians(0.5f*static_cast<float>(y - m_lastMousePos.y));

		m_camera.Rotate(-dx, -dy);
	}

	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dy = XMConvertToRadians(0.5f*static_cast<float>(y - m_lastMousePos.y));

		m_camera.Move(dy);
	}

	m_lastMousePos.x = x;
	m_lastMousePos.y = y;
}

void Converter::OnMouseScroll(WPARAM delta)
{
	m_camera.Zoom(-0.002f * GET_WHEEL_DELTA_WPARAM(delta));
}

bool Converter::LoadTextures()
{
	Material mat;

	for (uint8_t modelNum = 0; modelNum < m_modelsCount; ++modelNum)
	{
		HRESULT hr = CreateTexture(m_d3dDevice.Get(), m_mesh, modelNum, TextureID::t_albedo, &mat.diffuse);
		if (FAILED(hr))
		{
			hr = CreateDDSTextureFromFile(m_d3dDevice.Get(), (m_execPath + L"\\data\\resources\\textures\\test_gray.dds").c_str(), nullptr, &mat.diffuse);
			if (FAILED(hr))
				return false;
		}

		hr = CreateTexture(m_d3dDevice.Get(), m_mesh, modelNum, TextureID::t_normal, &mat.normal);
		if (FAILED(hr))
		{
			hr = CreateDDSTextureFromFile(m_d3dDevice.Get(), (m_execPath + L"\\data\\resources\\textures\\flatnormal.dds").c_str(), nullptr, &mat.normal);
			if (FAILED(hr))
				return false;
		}

		hr = CreateTexture(m_d3dDevice.Get(), m_mesh, modelNum, TextureID::t_specular, &mat.specular);
		if (FAILED(hr))
		{
			hr = CreateDDSTextureFromFile(m_d3dDevice.Get(), (m_execPath + L"\\data\\resources\\textures\\test_gray.dds").c_str(), nullptr, &mat.specular);
			if (FAILED(hr))
				return false;
		}

		hr = CreateTexture(m_d3dDevice.Get(), m_mesh, modelNum, TextureID::t_gloss_map, &mat.gloss_map);
		if (FAILED(hr))
		{
			hr = CreateDDSTextureFromFile(m_d3dDevice.Get(), (m_execPath + L"\\data\\resources\\textures\\test_gray.dds").c_str(), nullptr, &mat.gloss_map);
			if (FAILED(hr))
				return false;
		}

		m_material.push_back(mat);
	}

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;

	ThrowIfFailed(m_d3dDevice->CreateSamplerState(&samplerDesc, &m_samplerState));

	return true;
}

void Converter::InitializeInputLayout()
{
	ID3DBlob* vsbyteCode = nullptr;
	ID3DBlob* psbyteCode = nullptr;

	HRESULT result = D3DReadFileToBlob((m_execPath + L"\\data\\shaders\\standard_vs.cso").c_str(), &vsbyteCode);
	if (FAILED(result))
		return;

	if (m_mesh.GetVersion() == 7)
	{
		result = D3DReadFileToBlob((m_execPath + L"\\data\\shaders\\wh_ps.cso").c_str(), &psbyteCode);
		if (FAILED(result))
			return;
	}
	else
	{
		result = D3DReadFileToBlob((m_execPath + L"\\data\\shaders\\standard_ps.cso").c_str(), &psbyteCode);
		if (FAILED(result))
			return;
	}

	ThrowIfFailed(m_d3dDevice->CreateVertexShader(vsbyteCode->GetBufferPointer(), vsbyteCode->GetBufferSize(), nullptr, &m_vertexShader));
	ThrowIfFailed(result = m_d3dDevice->CreatePixelShader(psbyteCode->GetBufferPointer(), psbyteCode->GetBufferSize(), nullptr, &m_pixelShader));

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	m_d3dDevice->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), vsbyteCode->GetBufferPointer(),
		vsbyteCode->GetBufferSize(), m_inputLayout.GetAddressOf());
}

void Converter::InitializeBuffers()
{
	uint8_t lodNum = 0;
	uint8_t groupsCount = static_cast<uint8_t>(m_mesh.GetGroupsCount(lodNum));
	UINT vertSize = 0;
	UINT indSize = 0;

	for (uint8_t i = 0; i < groupsCount; ++i)
	{
		m_offset.push_back(indSize);
		m_indSize.push_back(m_mesh.GetIndicesCountPerGroup(lodNum, i));
		vertSize += m_mesh.GetVerticesCountPerGroup(lodNum, i);
		indSize += m_mesh.GetIndicesCountPerGroup(lodNum, i);
	}

	vector<Verts> v(vertSize);
	vector<uint32_t> ind(indSize);
	uint32_t voffset = 0;
	uint32_t ioffset = 0;
	vector<Vertex> verticesArray(0);
	vector<Triangle> indicesArray(0);

	for (uint8_t groupNum = 0; groupNum < groupsCount; ++groupNum)
	{
		m_mesh.GetVerticesArray(lodNum, groupNum, &verticesArray);
		for (uint32_t i = voffset; i < m_mesh.GetVerticesCountPerGroup(lodNum, groupNum) + voffset; ++i)
		{
			swap(v[i].Position, verticesArray[i - voffset].position);
			swap(v[i].Normal, verticesArray[i - voffset].normal);
			swap(v[i].Tangent, verticesArray[i - voffset].tangent);
			swap(v[i].Bitangent, verticesArray[i - voffset].bitangent);
			swap(v[i].TexCoord0, verticesArray[i - voffset].texCoord);
			swap(v[i].TexCoord1, verticesArray[i - voffset].texCoord2);
		}
		verticesArray.resize(0);

		m_mesh.GetIndicesArray(lodNum, groupNum, &indicesArray);
		for (uint32_t j = ioffset; j < m_mesh.GetIndicesCountPerGroup(lodNum, groupNum) / 3 + ioffset; ++j)
		{
			ind[j * 3 + 0] = indicesArray[j - ioffset].index1 + voffset;
			ind[j * 3 + 1] = indicesArray[j - ioffset].index2 + voffset;
			ind[j * 3 + 2] = indicesArray[j - ioffset].index3 + voffset;
		}
		voffset += m_mesh.GetVerticesCountPerGroup(lodNum, groupNum);
		ioffset += m_mesh.GetIndicesCountPerGroup(lodNum, groupNum) / 3;
		indicesArray.resize(0);
	}

	D3D11_BUFFER_DESC vertexBuffer_desc = {};
	vertexBuffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBuffer_desc.ByteWidth = sizeof(Verts) * vertSize;
	vertexBuffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBuffer_desc.CPUAccessFlags = 0;
	vertexBuffer_desc.MiscFlags = 0;
	vertexBuffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = &v[0];
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	HRESULT hr = m_d3dDevice->CreateBuffer(&vertexBuffer_desc, &vertexData, &m_vertexBuffer);
	if (FAILED(hr))
		return;

	D3D11_BUFFER_DESC indexBuffer_desc = {};
	indexBuffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBuffer_desc.ByteWidth = sizeof(uint32_t) * indSize;
	indexBuffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBuffer_desc.CPUAccessFlags = 0;
	indexBuffer_desc.MiscFlags = 0;
	indexBuffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = &ind[0];
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	hr = m_d3dDevice->CreateBuffer(&indexBuffer_desc, &indexData, &m_indexBuffer);
	if (FAILED(hr))
		return;

	D3D11_BUFFER_DESC cb_perObj_desc = {};
	cb_perObj_desc.Usage = D3D11_USAGE_DYNAMIC;
	cb_perObj_desc.ByteWidth = sizeof(buf_per_obj);
	cb_perObj_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_perObj_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb_perObj_desc.MiscFlags = 0;
	cb_perObj_desc.StructureByteStride = 0;

	hr = m_d3dDevice->CreateBuffer(&cb_perObj_desc, nullptr, &m_cbPerObj);
	if (FAILED(hr))
		return;
}

HRESULT CreateTexture(ID3D11Device1* device, const Mesh& mesh, const uint8_t& groupNum, const TextureID& tID, ID3D11ShaderResourceView** texture)
{
	return CreateDDSTextureFromFile(device, mesh.GetTexturePath(0, groupNum, tID).c_str(), nullptr, texture);
}

void DisplayFPS(const Timer& timer, const wstring& wndCaption, const HWND& hWnd)
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((timer.GetTotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;

		wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);

		wstring windowText = wndCaption +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr;

		SetWindowTextW(hWnd, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}