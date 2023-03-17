#include <chrono>
#include <string>
#include <thread>

#include "HTTP.h"
#include "Definitions.h"
#include "Includes.h"
#include "DirectX.h"
#include "LCU.h"
#include "Utils.h"
#include "Config.h"

Settings S;

Direct3D11Render Direct3D11;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	LPWSTR* szArgList;
	int argCount;
	szArgList = CommandLineToArgvW(GetCommandLineW(), &argCount);
	std::wstring programPath = szArgList[0];
	std::wstring programName = programPath.substr(programPath.find_last_of(L"/\\") + 1);
	if (argCount > 1)
	{
		std::string applicationName = Utils::WstringToString(szArgList[1]); // league
		std::string cmdLine;
		for (int i = 2; i < argCount; i++)
		{
			cmdLine += "\"" + Utils::WstringToString(szArgList[i]) + "\" ";
		}

		cmdLine.replace(cmdLine.find("\"--no-proxy-server\""), strlen("\"--no-proxy-server\""), "");

		AllocConsole();
		FILE* f;
		freopen_s(&f, "CONOUT$", "w", stdout);

		STARTUPINFOA startupInfo;
		memset(&startupInfo, 0, sizeof(STARTUPINFOA));
		startupInfo.cb = sizeof(startupInfo);
		PROCESS_INFORMATION processInformation;
		memset(&processInformation, 0, sizeof(PROCESS_INFORMATION));

		if (!CreateProcessA(applicationName.c_str(), const_cast<char*>(cmdLine.c_str()), 0, 0, false, 2U, 0, 0, &startupInfo, &processInformation))
			return 0;

		std::cout << "App: " << applicationName << std::endl;
		std::cout << "PID: " << processInformation.dwProcessId << std::endl;
		std::cout << "Args: " << cmdLine << std::endl;

		if (!DebugActiveProcessStop(processInformation.dwProcessId))
		{
			CloseHandle(processInformation.hProcess);
			CloseHandle(processInformation.hThread);
			fclose(f);
			FreeConsole();
			return 0;
		}

		WaitForSingleObject(processInformation.hProcess, INFINITE);

		std::cout << "Exited" << std::endl;

		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		fclose(f);
		FreeConsole();
	}
	else
	{
#ifndef NDEBUG
		AllocConsole();
		FILE* f;
		freopen_s(&f, "CONOUT$", "w", stdout);
#endif

		//Randomize using current time, todo swap with recent c++ random
		//srand(static_cast<unsigned>(time(0)));

		Config::Load();
		std::wstring sClassName = Utils::RandomWString(Utils::RandomInt(5, 10), { 0x2e80, 0xfffff });
		LPCWSTR lpszOverlayClassName = sClassName.c_str();
		//Register window class information
		WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, sClassName.c_str(), NULL };

		if (S.autoRename)
			Utils::RenameExe();

		::RegisterClassExW(&wc);

		// Create application window
		S.hwnd = ::CreateWindowW(sClassName.c_str(), lpszOverlayClassName, WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX, 100, 100, S.Window.width, S.Window.height, NULL, NULL, wc.hInstance, NULL);

		if (S.hwnd == NULL)
		{
			::UnregisterClassW(wc.lpszClassName, wc.hInstance);
			MessageBoxA(0, "Couldn't create window", 0, 0);
			return 0;
		}

		if (S.streamProof)
			SetWindowDisplayAffinity(S.hwnd, WDA_EXCLUDEFROMCAPTURE);
		else
			SetWindowDisplayAffinity(S.hwnd, WDA_NONE);

		//Initialize Direct3D
		if (!Direct3D11.DirectXInit(S.hwnd))
		{
			Direct3D11.Shutdown();
			::UnregisterClassW(wc.lpszClassName, wc.hInstance);
			MessageBoxA(0, "Couldn't initalize DirectX", 0, 0);
			return 0;
		}

		// Show the window
		::ShowWindow(S.hwnd, SW_SHOWDEFAULT);
		::UpdateWindow(S.hwnd);

		LCU::GetLeagueProcesses();

		if (LCU::SetLeagueClientInfo())
		{
			//league client is running
		}
		else
		{
			//riot client with login screen is up
			LCU::SetRiotClientInfo();
		}

		bool closedNow = false;
		bool done = false;
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
				if (msg.message == WM_QUIT)
					done = true;
				continue;
			}
			if (done)
				break;

			//Start rendering
			Direct3D11.StartFrame();

			//Render UI
			Direct3D11.Render();

			//End rendering
			Direct3D11.EndFrame();

			// idle if client closed and reconnect to it
			if (!::FindWindowA("RCLIENT", "League of Legends"))
			{
				Direct3D11.closedClient = true;
				closedNow = true;
				if (!LCU::leagueProcesses.empty())
					LCU::leagueProcesses.clear();
				if (::FindWindowA("RCLIENT", "Riot Client"))
				{
					if (LCU::riot.port == 0)
					{
						LCU::SetRiotClientInfo();
					}
				}
				else
				{
					LCU::riot.port = 0;
				}
			}
			else if (closedNow)
			{
				LCU::GetLeagueProcesses();
				LCU::SetLeagueClientInfo();

				Direct3D11.closedClient = false;
				closedNow = false;
			}

			//std::chrono::duration<float, std::milli> timeDuration = std::chrono::high_resolution_clock::now() - timeBefore;
			//processTimeMs = timeDuration.count();

			//std::cout << processTimeMs << std::endl;

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		Config::Save();

		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

#ifndef NDEBUG
		fclose(f);
		FreeConsole();
#endif

		//Exit
		Direct3D11.Shutdown();
		::DestroyWindow(S.hwnd);
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
	}
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
			Direct3D11.CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			Direct3D11.CreateRenderTarget();

			RECT rect;
			if (GetWindowRect(hWnd, &rect))
			{
				S.Window.height = rect.bottom - rect.top;
				S.Window.width = rect.right - rect.left;
				Config::Save();
			}
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
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}