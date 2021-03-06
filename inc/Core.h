#ifndef CORE_H
#define CORE_H
using Microsoft::WRL::ComPtr;
#include "StepTimer.h"

class Core
{
protected:

	Core(HINSTANCE);
	Core(const Core&) = delete;
	Core(Core&&) = delete;
	Core& operator=(const Core&) = delete;
	Core& operator=(Core&&) = delete;
	virtual ~Core();

	virtual bool Initialize();
	virtual void OnResize(uint32_t, uint32_t);
	virtual void Update(const Timer&) = 0;
	virtual void Render() = 0;
	virtual void ReadModel(const std::wstring&) = 0;
	virtual void ReadAnim(const std::string&) = 0;
	virtual void ExportMesh() = 0;
	virtual void ExportMeshAnim() = 0;
	virtual void ShowStats() = 0;

	virtual void OnMouseDown(WPARAM, int, int) = 0;
	virtual void OnMouseUp(WPARAM, int, int) = 0;
	virtual void OnMouseMove(WPARAM, int, int) = 0;
	virtual void OnMouseScroll(WPARAM) = 0;

public:

	static Core* GetPtr();
	int Run();
	virtual LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);

protected:

	bool InitializeWindow();
	bool CreateDevice();
	bool CreateResources();
	bool OnDeviceLost();

	static Core*					m_pCore;
	HINSTANCE						m_hInst;
	HWND							m_hWnd;
	uint32_t						m_width;
	uint32_t						m_height;
	std::wstring					m_wndCaption;
	D3D_DRIVER_TYPE					m_d3dDriverType;

	bool							m_isInit;
	bool							m_isPaused;
	bool							m_isMinimized;
	bool							m_isMaximized;
	bool							m_isResizing;
	bool							m_isFullScreen;
	bool							m_allowFullScreen;

	D3D_FEATURE_LEVEL				m_featureLevel;
	UINT							m_msaa4xQuality;
	UINT							m_buffersCount;
	bool							m_enable4xMSAA;

	ComPtr<ID3D11Device1>			m_d3dDevice;
	ComPtr<ID3D11DeviceContext1>	m_d3dContext;
	ComPtr<IDXGISwapChain1>			m_swapChain;
	ComPtr<ID3D11RenderTargetView>	m_renderTargetView;
	ComPtr<ID3D11DepthStencilView>	m_depthStencilView;
	CD3D11_VIEWPORT					m_viewport;

	Timer							m_timer;

};


#endif // !CORE_H