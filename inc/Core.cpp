#include "pch.h"
#include "Core.h"
#include "../resource.h"
#include <windowsx.h>

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Core::GetPtr()->WndProc(hwnd, msg, wParam, lParam);
}

Core* Core::m_pCore = nullptr;
Core* Core::GetPtr()
{
	return m_pCore;
}

Core::Core(HINSTANCE hInstance) :
	m_hInst(hInstance),
	m_hWnd(0),
	m_width(800),
	m_height(600),
	m_wndCaption(L"LightBlue Engine"),
	m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),

	m_isInit(false),
	m_isPaused(false),
	m_isMinimized(false),
	m_isMaximized(false),
	m_isResizing(false),
	m_isFullScreen(false),
	m_allowFullScreen(false),

	m_featureLevel(D3D_FEATURE_LEVEL_11_0),
	m_msaa4xQuality(1),
	m_buffersCount(1),
	m_enable4xMSAA(false),

	m_d3dDevice(nullptr),
	m_d3dContext(nullptr),
	m_swapChain(nullptr),
	m_renderTargetView(nullptr),
	m_depthStencilView(nullptr)
{
#ifdef _DEBUG
	assert(m_pCore == nullptr);
#endif
	m_pCore = this;
}

Core::~Core()
{
}

bool Core::Initialize()
{
	if (!m_isInit)
	{
		if (!InitializeWindow())
			return false;
		if (!CreateDevice())
			return false;

		OnResize(m_width, m_height - 20);
		m_isInit = true;
		return true;
	}
	else
		return false;
}

bool Core::InitializeWindow()
{
	WNDCLASSEX wcex = {};
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInst;
	wcex.hIcon			= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName	= MAKEINTRESOURCEW(IDR_MENU1);
	wcex.lpszClassName	= L"MainWnd";
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_hWnd = CreateWindowEx(0, L"MainWnd", m_wndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hInst, 0);
	if (!m_hWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return true;
}

bool Core::CreateDevice()
{
	UINT creationFlags = 0;
#ifdef _DEBUG 
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

	static const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	ComPtr<ID3D11Device>			d3dDevice;
	ComPtr<ID3D11DeviceContext>		d3dContext;
	HRESULT hr = D3D11CreateDevice(nullptr, m_d3dDriverType, nullptr, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &d3dDevice, &m_featureLevel, &d3dContext	);

	if (hr == E_INVALIDARG)
		// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it.
		hr = D3D11CreateDevice(nullptr,	m_d3dDriverType, nullptr, creationFlags, &featureLevels[1],	ARRAYSIZE(featureLevels) - 1, D3D11_SDK_VERSION, &d3dDevice, &m_featureLevel, &d3dContext);

	ThrowIfFailed(hr);

#ifndef NDEBUG
	ComPtr<ID3D11Debug> d3dDebug;
	if (SUCCEEDED(d3dDevice.As(&d3dDebug)))
	{
		ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
		{
#ifdef _DEBUG
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif	//	_DEBUG
			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				// TODO: Add more message IDs here as needed.
			};
			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
	}
#endif	//	!NDEBUG

	ThrowIfFailed(d3dDevice.As(&m_d3dDevice));
	ThrowIfFailed(d3dContext.As(&m_d3dContext));

	return true;
}

bool Core::CreateResources()
{
#ifdef _DEBUG
	assert(m_d3dDevice);
	assert(m_d3dContext);
#endif

	ThrowIfFailed(m_d3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_msaa4xQuality));
	if (m_msaa4xQuality >= 4)
		m_enable4xMSAA = true;

	if (m_swapChain)
	{
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
		m_renderTargetView.Reset();
		m_depthStencilView.Reset();
		m_d3dContext->Flush();

		HRESULT hr = m_swapChain->ResizeBuffers(m_buffersCount, m_width, m_height, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			if (!OnDeviceLost())
				return false;
		}
		else
			ThrowIfFailed(hr);
	}
	else
	{
		ComPtr<IDXGISwapChain> swapChain;

		ComPtr<IDXGIDevice2> dxgiDevice;
		ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

		ComPtr<IDXGIAdapter> dxgiAdapter;
		ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));

		ComPtr<IDXGIFactory1> dxgiFactory;
		ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

		ComPtr<IDXGIFactory2> dxgiFactory2;
		if (dxgiFactory.As(&dxgiFactory2))
		{
			// DirectX 11.1 or later
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			swapChainDesc.Width = m_width;
			swapChainDesc.Height = m_height;
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.SampleDesc.Count = m_enable4xMSAA ? 4 : 1;
			swapChainDesc.SampleDesc.Quality = m_enable4xMSAA ? (m_msaa4xQuality - 1) : 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = m_buffersCount;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.Flags = 0;
			swapChainDesc.Stereo = false;
			swapChainDesc.Scaling = DXGI_SCALING_NONE;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC sd_fullscreen = {};
			sd_fullscreen.RefreshRate.Numerator = 60;
			sd_fullscreen.RefreshRate.Denominator = 1;
			sd_fullscreen.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sd_fullscreen.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sd_fullscreen.Windowed = !m_isFullScreen;

			ThrowIfFailed(dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), m_hWnd, &swapChainDesc, &sd_fullscreen, nullptr, &m_swapChain));
		}
		else
		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			swapChainDesc.BufferDesc.Width = m_width;
			swapChainDesc.BufferDesc.Height = m_height;
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			swapChainDesc.SampleDesc.Count = m_enable4xMSAA ? 4 : 1;
			swapChainDesc.SampleDesc.Quality = m_enable4xMSAA ? (m_msaa4xQuality - 1) : 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = m_buffersCount;
			swapChainDesc.OutputWindow = m_hWnd;
			swapChainDesc.Windowed = !m_isFullScreen;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			ThrowIfFailed(dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &swapChainDesc, &swapChain));
			ThrowIfFailed(swapChain.As(&m_swapChain));
		}
		ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_WINDOW_CHANGES));
	}

	ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
	ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView));

	D3D11_TEXTURE2D_DESC backBufferDesc = {};
	backBuffer->GetDesc(&backBufferDesc);

	CD3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Width = m_width;
	depthStencilDesc.Height = m_height;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.SampleDesc.Count = m_enable4xMSAA ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_enable4xMSAA ? (m_msaa4xQuality - 1) : 0;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> depthStencil;
	ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil));
	ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), nullptr, &m_depthStencilView));

	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width = static_cast<float>(backBufferDesc.Width);
	m_viewport.Height = static_cast<float>(backBufferDesc.Height);
	m_viewport.MinDepth = D3D11_MIN_DEPTH;
	m_viewport.MaxDepth = D3D11_MAX_DEPTH;
	m_d3dContext->RSSetViewports(1, &m_viewport);

	return true;
}

bool Core::OnDeviceLost()
{
	m_depthStencilView.Reset();
	m_renderTargetView.Reset();
	m_swapChain.Reset();
	m_d3dContext.Reset();
	m_d3dDevice.Reset();

	CreateDevice();
	CreateResources();

	return true;
}

int Core::Run()
{
	MSG msg = {};
	
	m_timer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_timer.Tick();

			if (!m_isPaused)
			{
				Update(m_timer);
				Render();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

void Core::OnResize(uint32_t width, uint32_t height)
{
	m_width = max(width, 1);
	m_height = max(height, 1);

	//if (m_isFullScreen)
	//	m_swapChain->SetFullscreenState(true, nullptr);
	CreateResources();
}

LRESULT Core::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				m_isPaused = true;
				m_timer.Stop();
			}
			else
			{
				m_isPaused = false;
				m_timer.Start();
			}
		break;

		case WM_SIZE:
			if (m_d3dDevice)
			{
				if (wParam == SIZE_MINIMIZED)
				{
					m_isPaused = true;
					m_isMinimized = true;
					m_isMaximized = false;
				}
				else if (wParam == SIZE_MAXIMIZED)
				{
					m_isPaused = false;
					m_isMinimized = false;
					m_isMaximized = true;
					OnResize(LOWORD(lParam), HIWORD(lParam));
				}
				else if (wParam == SIZE_RESTORED)
				{
					if (m_isMinimized)
					{
						m_isPaused = false;
						m_isMinimized = false;
						OnResize(LOWORD(lParam), HIWORD(lParam));
					}
					else if (m_isMaximized)
					{
						m_isPaused = false;
						m_isMaximized = false;
						OnResize(LOWORD(lParam), HIWORD(lParam));
					}
					else if (m_isResizing)
					{
					}
					else
					{
						OnResize(LOWORD(lParam), HIWORD(lParam));
					}
				}
			}
		break;

		case WM_ENTERSIZEMOVE:
			m_isPaused = true;
			m_isResizing = true;
			m_timer.Stop();
		break;

		case WM_EXITSIZEMOVE:
			m_isPaused = false;
			m_isResizing = false;
			m_timer.Start();

			RECT rc;
			GetClientRect(hWnd, &rc);
			OnResize(rc.right - rc.left, rc.bottom - rc.top);
		break;

		case WM_GETMINMAXINFO:
		{
			auto info = reinterpret_cast<MINMAXINFO*>(lParam);
			info->ptMinTrackSize.x = 320;
			info->ptMinTrackSize.y = 200;
		}
		break;

		case WM_MENUCHAR:
			// A menu is active and the user presses a key that does not correspond
			// to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
		return MAKELRESULT(0, MNC_CLOSE);

		case WM_SYSKEYDOWN:
			if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
			{
				if (m_allowFullScreen)
				{
					// Implements the classic ALT+ENTER fullscreen toggle
					if (m_isFullScreen)
					{
						SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
						SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

						int width = 800;
						int height = 600;

						ShowWindow(hWnd, SW_SHOWNORMAL);

						SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
					}
					else
					{
						SetWindowLongPtr(hWnd, GWL_STYLE, 0);
						SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

						SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

						ShowWindow(hWnd, SW_SHOWMAXIMIZED);
					}
					m_isFullScreen = !m_isFullScreen;
				}
			}
		break;

		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case ID_OPEN_OPENRMV2:
				{
					wchar_t File[256];
					OPENFILENAMEW ofn = {};
					ofn.lStructSize = sizeof(OPENFILENAMEW);
					ofn.hwndOwner = hWnd;
					ofn.lpstrFilter = L".rigid_model_v2 Files\0*.rigid_model_v2\0";
					ofn.lpstrFile = File;
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrTitle = L"Select a File";
					ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

					BOOL res = GetOpenFileNameW(&ofn);
					if (res < 0)
					{
						MessageBoxA(hWnd, "An error occured while opening this file!", "File open error!!!", MB_OK);
						return false;
					}
					else if (res == 0)
						return false;

					ReadFile(std::wstring(File));
				}
					break;
				case ID_OPEN_OPENANIM:

					break;
				case ID_FILE_EXIT:
					DestroyWindow(hWnd);
					break;
				case ID_TOOL_SHOWSTATS:
					ShowStats();
					break;
				case ID_HELP_ABOUT:
					MessageBox(hWnd, L"Creator: Hayali Salimov aka Mr.Jox\n\nCredits: acgessler for parts of HalfFloat type\n\nContact: salimovhayali@gmail.com\n\nRMV2 Converter v2.0 © 2018", L"About", MB_OK);
					break;
				default:
					return DefWindowProc(hWnd, msg, wParam, lParam);
			}
		}
		break;

		case WM_RBUTTONDOWN:
			OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

		case WM_RBUTTONUP:
			OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

		case WM_MOUSEMOVE:
			OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

		case WM_MOUSEWHEEL:
			OnMouseScroll(wParam);
			break;

		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
				DestroyWindow(hWnd);
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
