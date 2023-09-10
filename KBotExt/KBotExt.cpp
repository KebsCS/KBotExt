#pragma comment (lib, "cpr.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "Crypt32.lib")

#include <chrono>
#include <string>
#include <thread>

#include "Definitions.h"
#include "Includes.h"
#include "DirectX.h"
#include "LCU.h"
#include "Utils.h"
#include "Config.h"

settings s;

direct_3d11_render direct_3d11;

LRESULT WINAPI wnd_proc(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param);

int WINAPI wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int /*nCmdShow*/ )
{
	LPWSTR* sz_arg_list;
	int arg_count;
	sz_arg_list = CommandLineToArgvW(GetCommandLineW(), &arg_count);
	std::wstring program_path = sz_arg_list[0];
	std::wstring program_name = program_path.substr(program_path.find_last_of(L"/\\") + 1);
	if (arg_count > 1)
	{
		std::string application_name = utils::wstring_to_string(sz_arg_list[1]); // league
		std::string cmd_line;
		for (int i = 2; i < arg_count; i++)
		{
			cmd_line += "\"" + utils::wstring_to_string(sz_arg_list[i]) + "\" ";
		}

		cmd_line.replace(cmd_line.find("\"--no-proxy-server\""), strlen("\"--no-proxy-server\""), "");

		AllocConsole();
		FILE* f;
		errno_t _;
		_ = freopen_s(&f, "CONOUT$", "w", stdout);

		STARTUPINFOA startup_info = {};
		startup_info.cb = sizeof(startup_info);
		PROCESS_INFORMATION process_information = {};

		if (!CreateProcessA(application_name.c_str(), const_cast<char*>(cmd_line.c_str()), nullptr, nullptr, false, 2U,
		                    nullptr, nullptr, &startup_info, &process_information))
			return 0;

		std::cout << "App: " << application_name << std::endl;
		std::cout << "PID: " << process_information.dwProcessId << std::endl;
		std::cout << "Args: " << cmd_line << std::endl;

		if (!DebugActiveProcessStop(process_information.dwProcessId))
		{
			CloseHandle(process_information.hProcess);
			CloseHandle(process_information.hThread);
		    if (f && fclose(f) != 0) {
		        perror("Error closing file");
		    }
			FreeConsole();
			return 0;
		}

		WaitForSingleObject(process_information.hProcess, INFINITE);

		std::cout << "Exited" << std::endl;

		CloseHandle(process_information.hProcess);
		CloseHandle(process_information.hThread);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if (f && fclose(f) != 0) {
        perror("Error closing file");
    }
		FreeConsole();
	}
	else
	{
#ifndef NDEBUG
		AllocConsole();
		FILE* f;
		errno_t _;
		_ = freopen_s(&f, "CONOUT$", "w", stdout);
#endif

		config::load();
		std::wstring s_class_name = utils::random_w_string(utils::random_int(5, 10), {0x2e80, 0xfffff});
		LPCWSTR lpsz_overlay_class_name = s_class_name.c_str();
		//Register window class information
		WNDCLASSEXW wc = {
			sizeof(WNDCLASSEXW), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr,
			nullptr, s_class_name.c_str(), nullptr
		};

		if (s.auto_rename)
			utils::rename_exe();

		RegisterClassExW(&wc);

		// Create application window
		s.hwnd = ::CreateWindowW(s_class_name.c_str(), lpsz_overlay_class_name,
		                         WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX, 100, 100, s.window.width,
		                         s.window.height, NULL, NULL, wc.hInstance, NULL);

		if (s.hwnd == nullptr)
		{
			UnregisterClassW(wc.lpszClassName, wc.hInstance);
			MessageBoxA(nullptr, "Couldn't create window", nullptr, 0);
			return 0;
		}

		if (s.stream_proof)
			SetWindowDisplayAffinity(s.hwnd, WDA_EXCLUDEFROMCAPTURE);
		else
			SetWindowDisplayAffinity(s.hwnd, WDA_NONE);

		//Initialize Direct3D
		if (!direct_3d11.direct_x_init(s.hwnd))
		{
			direct_3d11_render::shutdown();
			UnregisterClassW(wc.lpszClassName, wc.hInstance);
			MessageBoxA(nullptr, "Couldn't initialize DirectX", nullptr, 0);
			return 0;
		}

		// Show the window
		ShowWindow(s.hwnd, SW_SHOWDEFAULT);
		UpdateWindow(s.hwnd);

		lcu::get_league_processes();

		if (lcu::set_league_client_info())
		{
			//league client is running
		}
		else
		{
			//riot client with login screen is up
			lcu::set_riot_client_info();
		}

		bool closed_now = false;
		bool done = false;
		// Main loop
		MSG msg;
		ZeroMemory(&msg, sizeof(msg));
		while (msg.message != WM_QUIT)
		{
			if (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				::DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
					done = true;
				continue;
			}
			if (done)
				break;

			//Start rendering
			direct_3d11_render::start_frame();

			//Render UI
			direct_3d11.render();

			//End rendering
			direct_3d11_render::end_frame();

			// idle if client closed and reconnect to it
			if (!FindWindowA("RCLIENT", "League of Legends"))
			{
				direct_3d11.closed_client = true;
				closed_now = true;
				if (!lcu::league_processes.empty())
					lcu::league_processes.clear();
				if (FindWindowA("RCLIENT", "Riot Client"))
				{
					if (lcu::riot.port == 0)
					{
						lcu::set_riot_client_info();
					}
				}
				else
				{
					lcu::riot.port = 0;
				}
			}
			else if (closed_now)
			{
				lcu::get_league_processes();
				lcu::set_league_client_info();

				direct_3d11.closed_client = false;
				closed_now = false;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		config::save();

		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

#ifndef NDEBUG
		if (fclose(f) != 0) {
		    perror("Error closing file");
		}
		FreeConsole();
#endif

		//Exit
		direct_3d11_render::shutdown();
		DestroyWindow(s.hwnd);
		UnregisterClassW(wc.lpszClassName, wc.hInstance);
	}
	return 0;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI wnd_proc(const HWND h_wnd, const UINT msg, const WPARAM w_param, const LPARAM l_param)
{
	if (ImGui_ImplWin32_WndProcHandler(h_wnd, msg, w_param, l_param))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3d_device != nullptr && w_param != SIZE_MINIMIZED)
		{
			direct_3d11_render::cleanup_render_target();
			g_p_swap_chain->ResizeBuffers(0, LOWORD(l_param), HIWORD(l_param), DXGI_FORMAT_UNKNOWN, 0);
			direct_3d11_render::create_render_target();

			RECT rect;
			if (GetWindowRect(h_wnd, &rect))
			{
				s.window.height = rect.bottom - rect.top;
				s.window.width = rect.right - rect.left;
				config::save();
			}
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((w_param & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default: ;
	}
	return DefWindowProcW(h_wnd, msg, w_param, l_param);
}
