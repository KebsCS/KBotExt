#include <chrono>
#include <string>
#include <iostream>
#include <thread>

#include "NtQueryInfoProc.h"
#include "Utils.h"
#include "DirectX.h"
#include "base64.h"

#pragma warning(disable : 4996)

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hwnd;

Direct3D9Render Direct3D9;

bool RenameExe()
{
	char szExeFileName[MAX_PATH];
	GetModuleFileNameA(NULL, szExeFileName, MAX_PATH);
	std::string path = std::string(szExeFileName);
	std::string exe = path.substr(path.find_last_of("\\") + 1, path.size());
	std::string newname;
	newname = utils->RandomString(RandomInt(5, 10));
	newname += XorStr(".exe");
	if (!rename(exe.c_str(), newname.c_str()))
		return true;
	else return false;
}

bool GetLoginPort()
{
	std::string auth = utils->WstringToString(GetAuth(XorStr("RiotClientUx.exe")));
	if (auth.empty())
	{
		//MessageBoxA(0, XorStr("Client not found"), 0, 0);
		return 0;
	}

	std::string appPort = XorStr(R"(--app-port=)");
	size_t nPos = auth.find(appPort);
	if (nPos != std::string::npos)
		loginPort = std::stoi(auth.substr(nPos + appPort.size(), 5));

	std::string remotingAuth = XorStr("--remoting-auth-token=");
	nPos = auth.find(remotingAuth) + strlen(remotingAuth.c_str());
	if (nPos != std::string::npos)
	{
		std::string token = XorStr("riot:") + auth.substr(nPos, 22);
		unsigned char m_Test[50];
		strncpy((char*)m_Test, token.c_str(), sizeof(m_Test));
		loginToken = base64_encode(m_Test, token.size()).c_str();
	}
	else
	{
		MessageBoxA(0, XorStr("Couldn't connect to client"), 0, 0);

		return 0;
	}
	return 1;
}

bool MakeAuth()
{
	// Get client port and auth code from it's command line
	std::string auth = utils->WstringToString(GetAuth(XorStr("LeagueClientUx.exe")));
	if (auth.empty())
	{
		//MessageBoxA(0, XorStr("Client not found"), 0, 0);
		return 0;
	}

	std::string appPort = XorStr("\"--app-port=");
	size_t nPos = auth.find(appPort);
	if (nPos != std::string::npos)
		clientPort = std::stoi(auth.substr(nPos + appPort.size(), 5));

	std::string remotingAuth = XorStr("--remoting-auth-token=");
	nPos = auth.find(remotingAuth) + strlen(remotingAuth.c_str());
	if (nPos != std::string::npos)
	{
		std::string token = XorStr("riot:") + auth.substr(nPos, 22);
		unsigned char m_Test[50];
		strncpy((char*)m_Test, token.c_str(), sizeof(m_Test));
		authToken = base64_encode(m_Test, token.size()).c_str();
	}
	else
	{
		MessageBoxA(0, XorStr("Couldn't connect to client"), 0, 0);

		return 0;
	}
	return 1;
}

// Main code
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Randomize using current time, todo swap with recent c++ random
	srand(time(0));

	std::string sClassName = utils->RandomString(RandomInt(5, 10));
	LPCSTR lpszOverlayClassName = sClassName.c_str();
	//Register window class information
	WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, sClassName.c_str(), NULL };

	// Rename exe to random string
	RenameExe();

	::RegisterClassExA(&wc);

	// Create application window
	hwnd = ::CreateWindowA(sClassName.c_str(), lpszOverlayClassName, WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, 100, 100, 700, 500, NULL, NULL, wc.hInstance, NULL);

	if (hwnd == NULL)
	{
		::UnregisterClassA(wc.lpszClassName, wc.hInstance);
		return 0;
	}

	if (MakeAuth())
	{
		//client is running
	}
	else
	{
		//client with login screen is up
		GetLoginPort();
	}

	//Initialize Direct3D
	if (!Direct3D9.DirectXInit(hwnd))
	{
		Direct3D9.Shutdown();
		::UnregisterClassA(wc.lpszClassName, wc.hInstance);
		return 0;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

#ifndef NDEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
#endif

	bool closedNow = false;
	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
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
		if (!::FindWindowA(0, XorStr("League of Legends")))
		{
			Direct3D9.closedClient = true;
			closedNow = true;
			if (::FindWindowA(0, XorStr("Riot Client")))
			{
				if (loginPort == 0)
					GetLoginPort();
			}
			else
				loginPort = 0;
		}
		else if (closedNow)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			MakeAuth();
			Direct3D9.MakeHeader();
			Direct3D9.closedClient = false;
			closedNow = false;
		}

#ifdef NDEBUG
		if (IsDebuggerPresent())
			break;
#endif

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