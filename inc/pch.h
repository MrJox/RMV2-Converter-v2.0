#pragma once

//#define WIN32_LEAN_AND_MEAN
//#define NOMINMAX
#define NODRAWTEXT
//#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11_2.h>
#include <DirectXMath.h>
#include <D3Dcompiler.h>
#include <Windows.h>
#include <wrl/client.h>
#include <assert.h>
#include <stdint.h>

#include <algorithm>
#include <memory>
#include <exception>
#include <vector>
#include <fstream>
#include <string>

inline void ThrowIfFailed(HRESULT hr)
{
#ifdef _DEBUG
	if (FAILED(hr))
	{
		// Set a breakpoint on this line to catch DirectX API errors
		throw std::exception();
	}
#else
	UNREFERENCED_PARAMETER(hr);
#endif
}
