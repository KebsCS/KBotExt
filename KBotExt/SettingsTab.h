#pragma once

#include "Includes.h"
#include "Misc.h"
#include "Config.h"

class SettingsTab
{
public:
	static void Render()
	{
		using tRegCreateKeyExA = LSTATUS(WINAPI*)(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass,
		                                          DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult,
		                                          LPDWORD lpdwDisposition);
		static auto RegCreateKeyExA = reinterpret_cast<tRegCreateKeyExA>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegCreateKeyExA"));

		using tRegOpenKeyExA = LSTATUS(WINAPI*)(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions,
		                                        REGSAM samDesired, PHKEY phkResult);
		static auto RegOpenKeyExA = reinterpret_cast<tRegOpenKeyExA>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegOpenKeyExA"));

		using tRegQueryValueExA = LSTATUS(WINAPI*)(HKEY hKey, LPCSTR lpValueName,
		                                           LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbDatan);
		static auto RegQueryValueExA = reinterpret_cast<tRegQueryValueExA>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegQueryValueExA"));

		using tRegSetValueExA = LSTATUS(WINAPI*)(HKEY hKey, LPCSTR lpValueName, DWORD Reserved,
		                                         DWORD dwType, const BYTE* lpData, DWORD cbData);
		static auto RegSetValueExA = reinterpret_cast<tRegSetValueExA>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegSetValueExA"));

		using tRegDeleteValueA = LSTATUS(WINAPI*)(HKEY hKey, LPCSTR lpValueName);
		static auto RegDeleteValueA = reinterpret_cast<tRegDeleteValueA>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegDeleteValueA"));

		using tRegCloseKey = LSTATUS(WINAPI*)(HKEY hKe);
		static auto RegCloseKey = reinterpret_cast<tRegCloseKey>(GetProcAddress(LoadLibraryW(L"advapi32.dll"), "RegCloseKey"));

		static bool once = true;
		if (ImGui::BeginTabItem("Settings"))
		{
			if (once)
			{
				once = false;
				HKEY hkResult;
				if (RegOpenKeyExA(
					HKEY_LOCAL_MACHINE, R"(Software\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\LeagueClientUx.exe)", 0,
					KEY_READ, &hkResult) == ERROR_SUCCESS)
				{
					char buffer[MAX_PATH];
					DWORD dwLen = sizeof(buffer);
					if (const LSTATUS regQuery = RegQueryValueExA(hkResult, "debugger", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &dwLen);
						regQuery == ERROR_SUCCESS)
					{
						S.currentDebugger = std::string(buffer, dwLen);
					}
					else if (regQuery == ERROR_FILE_NOT_FOUND)
					{
						S.currentDebugger = "Nothing";
						S.debugger = false;
					}
					else
					{
						S.currentDebugger = "Failed, error code " + regQuery;
					}
					RegCloseKey(hkResult);
				}
				else
					S.currentDebugger = "Error";
			}
			static std::string result;

			ImGui::Checkbox("Auto-rename", &S.autoRename);
			ImGui::SameLine();
			ImGui::HelpMarker("Automatically renames the program on launch");

			if (ImGui::Checkbox("Stream proof", &S.streamProof))
			{
				if (S.streamProof)
					SetWindowDisplayAffinity(S.hwnd, WDA_EXCLUDEFROMCAPTURE);
				else
					SetWindowDisplayAffinity(S.hwnd, WDA_NONE);
			}
			ImGui::SameLine();
			ImGui::HelpMarker("Hides the program in recordings and screenshots");

			ImGui::Checkbox("Launch client without admin", &S.noAdmin);

			if (ImGui::Checkbox("Register debugger IFEO", &S.debugger))
			{
				HKEY hkResult;
				if (const LSTATUS regCreate = RegCreateKeyExA(HKEY_LOCAL_MACHINE,
				                                              R"(Software\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\LeagueClientUx.exe)",
				                                              0, nullptr, 0, KEY_SET_VALUE | KEY_QUERY_VALUE | KEY_CREATE_SUB_KEY, nullptr, &hkResult,
				                                              nullptr); regCreate == ERROR_SUCCESS)
				{
					char* buffer[MAX_PATH];
					DWORD bufferLen = sizeof(buffer);

					char filePath[MAX_PATH + 1];
					GetModuleFileNameA(nullptr, filePath, MAX_PATH);
					const auto len = static_cast<DWORD>(strlen(filePath) + 1); // bugprone-misplaced-widening-cast?

					if (const LSTATUS regQuery = RegQueryValueExA(hkResult, "debugger", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer),
					                                              &bufferLen); regQuery == ERROR_SUCCESS || regQuery == ERROR_FILE_NOT_FOUND)
					{
						if (S.debugger)
						{
							auto messageBoxStatus = IDYES;
							if (S.autoRename || S.noAdmin)
								messageBoxStatus = MessageBoxA(nullptr, "Having \"Auto-rename\" or \"Launch client without admin\" "
								                               "enabled with \"debugger IFEO\" will prevent League client from starting\n\n"
								                               "Do you wish to continue?", "Warning", MB_YESNO | MB_SETFOREGROUND);

							if (messageBoxStatus == IDYES)
							{
								if (RegSetValueExA(hkResult, "debugger", 0, REG_SZ, reinterpret_cast<const BYTE*>(filePath), len) == ERROR_SUCCESS)
								{
									S.currentDebugger = filePath;
								}
							}
							else
							{
								S.debugger = false;
							}
						}
						else if (regQuery == ERROR_SUCCESS)
						{
							RegDeleteValueA(hkResult, "debugger");
							S.currentDebugger = "Nothing";
						}
					}
					RegCloseKey(hkResult);
				}
			}
			ImGui::SameLine();
			ImGui::HelpMarker("Allows for client traffic analysis via a web debugging proxy such as Fiddler."
				"Disable before deleting the program. Doesn't work when \"Auto-rename\" or \"Launch client without admin\" are enabled.");
			ImGui::SameLine();
			ImGui::Text("| Hooked to: %s", S.currentDebugger.c_str());

			if (ImGui::Button("Clean logs"))
			{
				if (MessageBoxA(nullptr, "Are you sure?", "Cleaning logs", MB_OKCANCEL) == IDOK)
				{
					result = Misc::ClearLogs();
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Force close client"))
				Misc::TaskKillLeague();

			static char bufLeaguePath[MAX_PATH];
			std::ranges::copy(S.leaguePath, bufLeaguePath);
			ImGui::Text("League path:");
			ImGui::InputText("##leaguePath", bufLeaguePath, MAX_PATH);
			S.leaguePath = bufLeaguePath;

			/*	if (ImGui::Button("Save Settings"))
				{
					CSettings::Save();
					result = "Saved";
				}*/

			ImGui::Separator();

			ImGui::Text("Font Scale:");
			if (ImGui::SliderFloat("##fontScaleSlider", &S.fontScale, 0.4f, 4.f, "%0.1f"))
			{
				ImGuiIO& io = ImGui::GetIO();
				io.FontGlobalScale = S.fontScale;
			}

			if (ImGui::Button("Reset window size"))
			{
				S.Window.width = 730;
				S.Window.height = 530;
				SetWindowPos(S.hwnd, nullptr, 0, 0, S.Window.width, S.Window.height, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

				S.fontScale = 1.f;
				ImGuiIO& io = ImGui::GetIO();
				io.FontGlobalScale = S.fontScale;

				Config::Save();
			}
			ImGui::SameLine();
			ImGui::Text(std::format("{0}x{1}", S.Window.width, S.Window.height).c_str());

			ImGui::Separator();
			ImGui::Text("Program's version: %s | Latest version: %s",
			            Misc::programVersion.c_str(), Misc::latestVersion.c_str());
			ImGui::Text("GitHub repository:");
			ImGui::TextURL("Click me!", "https://github.com/KebsCS/KBotExt", 1, 0);

			if (!result.empty())
				ImGui::Separator();
			ImGui::TextWrapped(result.c_str());

			ImGui::EndTabItem();
		}
		else
			once = true;
	}
};
