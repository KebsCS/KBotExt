#pragma once

#include <string>

#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "d3d11.lib")

// Data
inline ID3D11Device* g_pd3dDevice = nullptr;
inline ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
inline ID3D11RenderTargetView* g_pd3dRenderTargetView = nullptr;
inline IDXGISwapChain* g_pSwapChain = nullptr;

class Direct3D11Render
{
	std::string gamePatch;

public:
	bool closedClient = false;

	Direct3D11Render() = default;

	~Direct3D11Render() = default;

	static void StartFrame();

	static void EndFrame();

	// initializes directx, fonts, imgui and objects
	bool DirectXInit(HWND hWnd);

	static bool CreateRenderTarget();

	static void CleanupRenderTarget();

	// main rendering loop
	int Render() const;

	//releases directx and clears imgui
	static void Shutdown();

	//initializes imgui
	void Renderimgui(HWND hWnd);

	static void InitializeFonts();

	//initializes imgui styles
	static void MenuInit();
};
