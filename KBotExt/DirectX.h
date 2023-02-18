#pragma once

#include <string>

#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <tchar.h>
#include <dinput.h>
#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <dcomp.h>
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "d3d11.lib")

// Data
inline ID3D11Device* g_pd3dDevice = NULL;
inline ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
inline ID3D11RenderTargetView* g_pd3dRenderTargetView = NULL;
inline IDXGISwapChain* g_pSwapChain = NULL;

class Direct3D11Render
{
private:

	std::string gamePatch;
public:
	bool closedClient = false;

	Direct3D11Render() = default;

	~Direct3D11Render() = default;

	void StartFrame();

	void EndFrame();

	// initializes directx, fonts, imgui and objects
	bool DirectXInit(HWND hWnd);

	bool CreateRenderTarget();

	void CleanupRenderTarget();

	// main rendering loop
	int Render();

	//releases directx and clears imgui
	void Shutdown();

	//initializes imgui
	void Renderimgui(HWND hWnd);

	void InitializeFonts();

	//initializes imgui styles
	void MenuInit();
};