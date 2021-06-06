#include <chrono>
#include <string>
#include <iostream>
#include <thread>

#include "Definitions.h"
#include "Includes.h"
#include "DirectX.h"
#include "Auth.h"
#include "Utils.h"

#pragma warning(disable : 4996)

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hwnd;

Direct3D9Render Direct3D9;

float processTimeMs = 0;

// Main code
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Randomize using current time, todo swap with recent c++ random
	srand(time(0));

	std::string sClassName = utils->RandomString(RandomInt(5, 10));
	LPCSTR lpszOverlayClassName = sClassName.c_str();
	//Register window class information
	WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, sClassName.c_str(), NULL };

	utils->RenameExe();

	::RegisterClassExA(&wc);

	// Create application window
	hwnd = ::CreateWindowA(sClassName.c_str(), lpszOverlayClassName, WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, 100, 100, 700, 500, NULL, NULL, wc.hInstance, NULL);

	if (hwnd == NULL)
	{
		::UnregisterClassA(wc.lpszClassName, wc.hInstance);
		MessageBoxA(0, "Couldn't create window", 0, 0);
		return 0;
	}

	//Initialize Direct3D
	if (!Direct3D9.DirectXInit(hwnd))
	{
		Direct3D9.Shutdown();
		::UnregisterClassA(wc.lpszClassName, wc.hInstance);
		MessageBoxA(0, "Couldn't initalize DirectX", 0, 0);
		return 0;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

#ifndef NDEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
#endif

	if (auth->GetLeagueClientInfo())
	{
		//league client is running
	}
	else
	{
		//riot client with login screen is up
		auth->GetRiotClientInfo();
	}

	bool closedNow = false;
	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		//auto timeBefore = std::chrono::high_resolution_clock::now();

		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		//Start rendering
		Direct3D9.StartFrame();

		//Render UI
		Direct3D9.Render();

		//End rendering
		Direct3D9.EndFrame();

		// idle if client closed and reconnect to it
		if (!::FindWindowA(0, "League of Legends"))
		{
			Direct3D9.closedClient = true;
			closedNow = true;
			if (::FindWindowA(0, "Riot Client"))
			{
				if (auth->riotPort == 0)
					auth->GetRiotClientInfo();
			}
			else
				auth->riotPort = 0;
		}
		else if (closedNow)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			auth->GetLeagueClientInfo();
			Direct3D9.closedClient = false;
			closedNow = false;
		}

		//std::chrono::duration<float, std::milli> timeDuration = std::chrono::high_resolution_clock::now() - timeBefore;
		//processTimeMs = timeDuration.count();

		//std::cout << processTimeMs << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//Exit
	Direct3D9.Shutdown();
	::DestroyWindow(hwnd);
	::UnregisterClassA(wc.lpszClassName, wc.hInstance);

	return 0;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			Direct3D9.CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			Direct3D9.CreateRenderTarget();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}