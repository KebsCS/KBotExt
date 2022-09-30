#include <chrono>
#include <string>
#include <thread>

#include "Definitions.h"
#include "Includes.h"
#include "DirectX.h"
#include "Auth.h"
#include "Utils.h"
#include "Config.h"

Settings S;

Direct3D9Render Direct3D9;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hwnd;

//float processTimeMs = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LPWSTR* szArgList;
	int argCount;
	szArgList = CommandLineToArgvW(GetCommandLineW(), &argCount);
	if (argCount > 1)
	{
		std::string applicationName = Utils::WstringToString(szArgList[1]);
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
		//Randomize using current time, todo swap with recent c++ random
		srand(static_cast<unsigned>(time(0)));

		bool oldStreamProof = S.streamProof;

		Config::Load();

		bool oldDebugger = S.debugger;

		std::string sClassName = Utils::RandomString(RandomInt(5, 10));
		LPCSTR lpszOverlayClassName = sClassName.c_str();
		//Register window class information
		WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, sClassName.c_str(), NULL };

		if (S.autoRename)
			Utils::RenameExe();

		::RegisterClassExA(&wc);

		// Create application window
		hwnd = ::CreateWindowA(sClassName.c_str(), lpszOverlayClassName, WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX, 100, 100, S.Window.width, S.Window.height, NULL, NULL, wc.hInstance, NULL);

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
		FILE* f;
		freopen_s(&f, "CONOUT$", "w", stdout);
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

		std::cout << lpCmdLine << std::endl;

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

			// pressed button to reset to original size
			if (S.Window.resize)
			{
				S.Window.resize = false;
				::SetWindowPos(hwnd, 0, 0, 0, S.Window.width, S.Window.height, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
			}

			if (oldStreamProof != S.streamProof)
			{
				oldStreamProof = S.streamProof;
				if (oldStreamProof)
					SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
				else
					SetWindowDisplayAffinity(hwnd, WDA_NONE);
			}

			if (oldDebugger != S.debugger)
			{
				oldDebugger = S.debugger;
				HKEY hkResult;
				LSTATUS regCreate = RegCreateKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\LeagueClientUx.exe", 0, 0, 0, KEY_SET_VALUE | KEY_QUERY_VALUE | KEY_CREATE_SUB_KEY, 0, &hkResult, 0);
				if (regCreate == ERROR_SUCCESS)
				{
					char* buffer[MAX_PATH];
					DWORD bufferLen;

					char filePath[MAX_PATH + 1];
					GetModuleFileNameA(NULL, filePath, MAX_PATH);
					DWORD len = (DWORD)(strlen(filePath) + 1);

					LSTATUS regQuery = RegQueryValueExA(hkResult, "debugger", 0, 0, (LPBYTE)buffer, &bufferLen);
					if (regQuery == ERROR_SUCCESS)
					{
						if (oldDebugger)
						{
							if (RegSetValueExA(hkResult, "debugger", 0, REG_SZ, (const BYTE*)filePath, len) == ERROR_SUCCESS)
							{
								std::cout << "changed key " << std::endl;
								S.currentDebugger = filePath;
							}
							else
								std::cout << "failed change" << std::endl;
						}
						else
						{
							std::cout << "delete key" << RegDeleteValueA(hkResult, "debugger") << std::endl;
							S.currentDebugger = "Nothing";
						}
					}
					else if (regQuery == ERROR_FILE_NOT_FOUND && oldDebugger) // if key doesnt exist, create it
					{
						if (RegSetValueExA(hkResult, "debugger", 0, REG_SZ, (const BYTE*)filePath, len) == ERROR_SUCCESS)
						{
							std::cout << "created key " << std::endl;
							S.currentDebugger = filePath;
						}
					}
					RegCloseKey(hkResult);
				}
			}

			//Start rendering
			Direct3D9.StartFrame();

			//Render UI
			Direct3D9.Render();

			//End rendering
			Direct3D9.EndFrame();

			// idle if client closed and reconnect to it
			if (!::FindWindowA("RCLIENT", "League of Legends"))
			{
				Direct3D9.closedClient = true;
				closedNow = true;
				if (::FindWindowA("RCLIENT", "Riot Client"))
				{
					if (auth->riotPort == 0)
						auth->GetRiotClientInfo();
				}
				else
					auth->riotPort = 0;
			}
			else if (closedNow)
			{
				auth->GetLeagueClientInfo();
				Direct3D9.closedClient = false;
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
		Direct3D9.Shutdown();
		::DestroyWindow(hwnd);
		::UnregisterClassA(wc.lpszClassName, wc.hInstance);
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
			Direct3D9.CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			Direct3D9.CreateRenderTarget();

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
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}