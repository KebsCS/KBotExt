#pragma once

#ifndef _DIRECTX_H_
#define _DIRECTX_H_

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <tchar.h>

#include <dinput.h>
#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <dcomp.h>
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "d3d11.lib")

#include "Definitions.h"

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static ID3D11RenderTargetView* g_pd3dRenderTargetView = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;

inline int clientPort = 0;
inline int loginPort = 0;
inline std::string authToken;
inline std::string loginToken;

class Direct3D9Render
{
private:
	void ArrowButtonDisabled(const char* id, ImGuiDir dir);
	void HotkeyButton(int& key, bool mouse = false);
	void HelpMarker(const char* desc);
	std::string LolHeader;

	bool sessionOpen = true;
	std::string session;
	bool champsOpen = true;
	bool skinsOpen = true;

	std::string gamePatch;
	std::string GetCurrentPatch();

	bool bAutoAccept = false;
	int instalockID;
	bool bInstalock = false;
	char instantMessage[50];

	std::vector<ChampMinimal>champsMinimal;
	std::vector<ChampMastery>champsMastery;

	std::vector<std::string>lolProcs =
	{
		XorStr("RiotClientServices.exe"),
	XorStr("LeagueClient.exe"),
	XorStr("RiotClientCrashHandler.exe"),
	XorStr("LeagueClientUx.exe"),
	XorStr("LeagueClientUxRender.exe"),
	};

public:
	bool closedClient = false;

	Direct3D9Render()
	{
	}

	~Direct3D9Render() = default;

	void GameTab();
	void ProfileTab();
	void SessionTab();
	void InfoTab();
	void ChampsTab();
	void SkinsTab();
	void LootTab();
	void MiscTab();
	void CustomTab();

	void AutoAccept();

	void LaunchOldClient();

	void StartFrame();

	void EndFrame();

	void MakeHeader();

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
