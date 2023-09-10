#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "Utils.h"
#include "LCU.h"
#include "Misc.h"
#include "Config.h"

class settings_tab
{
public:
	static void render()
	{
		using t_reg_create_key_ex_a = LSTATUS(WINAPI*)(HKEY h_key, LPCSTR lp_sub_key, DWORD reserved, LPSTR lp_class,
		                                               DWORD dw_options, REGSAM sam_desired,
		                                               LPSECURITY_ATTRIBUTES lp_security_attributes, PHKEY phkResult,
		                                               LPDWORD lpdw_disposition);
		static auto reg_create_key_ex_a = reinterpret_cast<t_reg_create_key_ex_a>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegCreateKeyExA"));

		using t_reg_open_key_ex_a = LSTATUS(WINAPI*)(HKEY h_key, LPCSTR lp_sub_key, DWORD ul_options,
		                                             REGSAM sam_desired, PHKEY phk_result);
		static auto reg_open_key_ex_a = reinterpret_cast<t_reg_open_key_ex_a>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegOpenKeyExA"));

		using t_reg_query_value_ex_a = LSTATUS(WINAPI*)(HKEY h_key, LPCSTR lp_value_name,
		                                                LPDWORD lp_reserved, LPDWORD lp_type, LPBYTE lp_data,
		                                                LPDWORD lpcb_datan);
		static auto reg_query_value_ex_a = reinterpret_cast<t_reg_query_value_ex_a>(GetProcAddress(
			LoadLibraryW(L"advapi32.dll"), "RegQueryValueExA"));

		using t_reg_set_value_ex_a = LSTATUS(WINAPI*)(HKEY h_key, LPCSTR lp_value_name, DWORD reserved,
		                                              DWORD dw_type, const BYTE* lp_data, DWORD cb_data);
		static auto reg_set_value_ex_a = reinterpret_cast<t_reg_set_value_ex_a>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegSetValueExA"));

		using t_reg_delete_value_a = LSTATUS(WINAPI*)(HKEY h_key, LPCSTR lp_value_name);
		static auto reg_delete_value_a = reinterpret_cast<t_reg_delete_value_a>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegDeleteValueA"));

		using t_reg_close_key = LSTATUS(WINAPI*)(HKEY h_ke);
		static auto reg_close_key = reinterpret_cast<t_reg_close_key>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegCloseKey"));

		static bool once = true;
		if (ImGui::BeginTabItem("Settings"))
		{
			if (once)
			{
				once = false;
				HKEY hk_result;
				if (reg_open_key_ex_a(
					HKEY_LOCAL_MACHINE,
					"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\LeagueClientUx.exe",
					0, KEY_READ, &hk_result) == ERROR_SUCCESS)
				{
					char buffer[MAX_PATH];
					DWORD dw_len = sizeof(buffer);
					if (const LSTATUS reg_query = reg_query_value_ex_a(hk_result, "debugger", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer),
					                                                   &dw_len); reg_query == ERROR_SUCCESS)
					{
						s.current_debugger = std::string(buffer, dw_len);
					}
					else if (reg_query == ERROR_FILE_NOT_FOUND)
					{
						s.current_debugger = "Nothing";
						s.debugger = false;
					}
					else
					{
						s.current_debugger = "Failed, error code " + reg_query;
					}
					reg_close_key(hk_result);
				}
				else
					s.current_debugger = "Error";
			}
			static std::string result;

			ImGui::Checkbox("Auto-rename", &s.auto_rename);
			ImGui::SameLine();
			ImGui::help_marker("Automatically renames the program on launch");

			if (ImGui::Checkbox("Stream proof", &s.stream_proof))
			{
				if (s.stream_proof)
					SetWindowDisplayAffinity(s.hwnd, WDA_EXCLUDEFROMCAPTURE);
				else
					SetWindowDisplayAffinity(s.hwnd, WDA_NONE);
			}
			ImGui::SameLine();
			ImGui::help_marker("Hides the program in recordings and screenshots");

			ImGui::Checkbox("Launch client without admin", &s.no_admin);

			if (ImGui::Checkbox("Register debugger IFEO", &s.debugger))
			{
				HKEY hk_result;
				if (const LSTATUS reg_create = reg_create_key_ex_a(HKEY_LOCAL_MACHINE,
				                                                   "Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\LeagueClientUx.exe",
				                                                   0, nullptr, 0, KEY_SET_VALUE | KEY_QUERY_VALUE | KEY_CREATE_SUB_KEY,
				                                                   nullptr, &hk_result, nullptr); reg_create == ERROR_SUCCESS)
				{
					char* buffer[MAX_PATH];
					DWORD buffer_len = sizeof(buffer);

					char file_path[MAX_PATH + 1];
					GetModuleFileNameA(nullptr, file_path, MAX_PATH);
					const DWORD len = strlen(file_path) + 1;

					if (const LSTATUS reg_query = reg_query_value_ex_a(hk_result, "debugger", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer),
					                                                   &buffer_len); reg_query == ERROR_SUCCESS || reg_query == ERROR_FILE_NOT_FOUND)
					{
						if (s.debugger)
						{
							auto message_box_status = IDYES;
							if (s.auto_rename || s.no_admin)
								message_box_status = MessageBoxA(
									nullptr, "Having \"Auto-rename\" or \"Launch client without admin\" "
									"enabled with \"debugger IFEO\" will prevent League client from starting\n\n"
									"Do you wish to continue?", "Warning", MB_YESNO | MB_SETFOREGROUND);

							if (message_box_status == IDYES)
							{
								if (reg_set_value_ex_a(hk_result, "debugger", 0, REG_SZ, reinterpret_cast<const BYTE*>(file_path), len) ==
									ERROR_SUCCESS)
								{
									s.current_debugger = file_path;
								}
							}
							else
							{
								s.debugger = false;
							}
						}
						else if (reg_query == ERROR_SUCCESS)
						{
							reg_delete_value_a(hk_result, "debugger");
							s.current_debugger = "Nothing";
						}
					}
					reg_close_key(hk_result);
				}
			}
			ImGui::SameLine();
			ImGui::help_marker("Allows for client traffic analysis via a web debugging proxy such as Fiddler."
				"Disable before deleting the program. Doesn't work when \"Auto-rename\" or \"Launch client without admin\" are enabled.");
			ImGui::SameLine();
			ImGui::Text("| Hooked to: %s", s.current_debugger.c_str());

			if (ImGui::Button("Clean logs"))
			{
				if (MessageBoxA(nullptr, "Are you sure?", "Cleaning logs", MB_OKCANCEL) == IDOK)
				{
					result = misc::clear_logs();
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Force close client"))
				misc::task_kill_league();

			static char buf_league_path[MAX_PATH];
			std::ranges::copy(s.league_path, buf_league_path);
			ImGui::Text("League path:");
			ImGui::InputText("##leaguePath", buf_league_path, MAX_PATH);
			s.league_path = buf_league_path;

			ImGui::Separator();

			ImGui::Text("Font Scale:");
			if (ImGui::SliderFloat("##fontScaleSlider", &s.font_scale, 0.4f, 4.f, "%0.1f"))
			{
				ImGuiIO& io = ImGui::GetIO();
				io.FontGlobalScale = s.font_scale;
			}

			if (ImGui::Button("Reset window size"))
			{
				s.window.width = 730;
				s.window.height = 530;
				SetWindowPos(s.hwnd, nullptr, 0, 0, s.window.width, s.window.height,
				             SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

				s.font_scale = 1.f;
				ImGuiIO& io = ImGui::GetIO();
				io.FontGlobalScale = s.font_scale;

				config::save();
			}
			ImGui::SameLine();
			ImGui::Text(std::format("{0}x{1}", s.window.width, s.window.height).c_str());

			ImGui::Separator();
			ImGui::Text("Program's version: %s | Latest version: %s",
			            misc::program_version.c_str(), misc::latest_version.c_str());
			ImGui::Text("GitHub repository:");
			ImGui::text_url("Click me!", "https://github.com/KebsCS/KBotExt", 1, 0);

			if (!result.empty())
				ImGui::Separator();
			ImGui::TextWrapped(result.c_str());

			ImGui::EndTabItem();
		}
		else
			once = true;
	}
};
