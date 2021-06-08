#pragma once

#ifndef _DIRECTX_H_
#define _DIRECTX_H_

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

class Direct3D9Render
{
private:

	std::string gamePatch;
	void AutoAccept();
public:
	bool closedClient = false;

	Direct3D9Render()
	{
	}

	~Direct3D9Render() = default;

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

	//initializes imgui styles
	void MenuInit();
};
extern Direct3D9Render Direct3D9;

#endif //_DIRECTX_H_
