#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "HTTP.h"
#include "Utils.h"
#include "Auth.h"
#include "Misc.h"
#include "Settings.h"

class SettingsTab
{
public:

	static void ShowFontSelector(const char* label)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImFont* font_current = ImGui::GetFont();
		if (ImGui::BeginCombo(label, font_current->GetDebugName()))
		{
			for (int n = 0; n < io.Fonts->Fonts.Size; n++)
			{
				ImFont* font = io.Fonts->Fonts[n];
				ImGui::PushID((void*)font);
				if (ImGui::Selectable(font->GetDebugName(), font == font_current))
				{
					io.FontDefault = font;
					S.selectedFont = n;
					CSettings::Save();
				}
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}
		//ImGui::SameLine();
	/*	Misc::HelpMarker(
			"- Load additional fonts with io.Fonts->AddFontFromFileTTF().\n"
			"- The font atlas is built when calling io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
			"- Read FAQ and docs/FONTS.md for more details.\n"
			"- If you need to add/remove fonts at runtime (e.g. for DPI change), do it before calling NewFrame().");*/
	}

	static void Render()
	{
		if (ImGui::BeginTabItem("Settings"))
		{
			static std::string result;

			ImGui::Checkbox("Auto-rename", &S.autoRename);

			ImGui::Checkbox("Stream Proof", &S.streamProof);

			ImGui::Checkbox("Register debugger IFEO", &S.debugger);

			// Terminate all league related processes,
			// remove read only and hidden property from files
			// and delete them
			if (ImGui::Button("Clear logs"))
			{
				result = "";
				Misc::TerminateProcessByName("RiotClientServices.exe");
				Misc::TerminateProcessByName("RiotClientCrashHandler.exe");
				Misc::TerminateProcessByName("RiotClientUx.exe");
				Misc::TerminateProcessByName("RiotClientUxRender.exe");

				Misc::TerminateProcessByName("LeagueClient.exe");
				Misc::TerminateProcessByName("LeagueCrashHandler.exe");
				Misc::TerminateProcessByName("LeagueClientUx.exe");
				Misc::TerminateProcessByName("LeagueClientUxRender.exe");

				std::this_thread::sleep_for(std::chrono::seconds(2));

				std::error_code errorCode;

				std::string logsFolder = S.leaguePath + "Logs";
				if (std::filesystem::exists(logsFolder))
				{
					SetFileAttributesA(logsFolder.c_str(), GetFileAttributesA(logsFolder.c_str()) & ~FILE_ATTRIBUTE_READONLY & ~FILE_ATTRIBUTE_HIDDEN);
					std::filesystem::remove_all(logsFolder, errorCode);
					result += logsFolder + " - " + errorCode.message() + "\n";
				}

				std::string configFolder = S.leaguePath + "Config";
				if (std::filesystem::exists(configFolder))
				{
					SetFileAttributesA(configFolder.c_str(), GetFileAttributesA(configFolder.c_str()) & ~FILE_ATTRIBUTE_READONLY & ~FILE_ATTRIBUTE_HIDDEN);
					std::filesystem::remove_all(configFolder, errorCode);
					result += configFolder + " - " + errorCode.message() + "\n";
				}

				std::string programData = "C:/ProgramData/Riot Games";
				if (std::filesystem::exists(programData))
				{
					SetFileAttributesA(programData.c_str(), GetFileAttributesA(programData.c_str()) & ~FILE_ATTRIBUTE_READONLY & ~FILE_ATTRIBUTE_HIDDEN);
					std::filesystem::remove_all(programData, errorCode);
					result += programData + " - " + errorCode.message() + "\n";
				}

				char* pLocal;
				size_t localLen;
				_dupenv_s(&pLocal, &localLen, "LOCALAPPDATA");
				std::string local = pLocal;
				local += "\\Riot Games";
				if (std::filesystem::exists(local))
				{
					SetFileAttributesA(local.c_str(), GetFileAttributesA(local.c_str()) & ~FILE_ATTRIBUTE_READONLY & ~FILE_ATTRIBUTE_HIDDEN);
					std::filesystem::remove_all(local, errorCode);
					result += local + " - " + errorCode.message() + "\n";
				}

				int k = 0;
				for (const auto& file : std::filesystem::directory_iterator(std::filesystem::temp_directory_path()))
				{
					std::filesystem::remove_all(file, errorCode);
					k++;
				}
				result += "Deleted " + std::to_string(k) + " files in temp directory\n";
			}

			static char bufLeaguePath[MAX_PATH];
			std::copy(S.leaguePath.begin(), S.leaguePath.end(), bufLeaguePath);
			ImGui::Text("League path:");
			ImGui::InputText("##leaguePath", bufLeaguePath, MAX_PATH);
			S.leaguePath = bufLeaguePath;

			/*	if (ImGui::Button("Save Settings"))
				{
					CSettings::Save();
					result = "Saved";
				}*/

			ImGui::Separator();

			ImGui::Text("Font:");
			ShowFontSelector("##fontSelector");

			static char buffAddFot[MAX_PATH];
			std::string tempFont = "C:/Windows/Fonts/";
			std::copy(tempFont.begin(), tempFont.end(), buffAddFot);
			ImGui::Text("Font path:");

			ImGui::InputText("##inputFont", buffAddFot, MAX_PATH);
			ImGui::SameLine();
			if (ImGui::Button("Add font"))
			{
				std::string temp = buffAddFot;
				if (std::filesystem::exists(temp) && temp.find(".") != std::string::npos)
				{
					if (std::find(S.vFonts.begin(), S.vFonts.end(), temp) == S.vFonts.end())
					{
						S.vFonts.emplace_back(temp);
						S.bAddFont = true;
						CSettings::Save();
						result = temp + " font added, restart program to select it";
					}
					else
						result = "Font already added";
				}
				else
					result = "Font not found";
			}

			if (ImGui::Button("Reset window size"))
			{
				S.Window.width = 700;
				S.Window.height = 500;
				S.Window.resize = true;
				CSettings::Save();
			}
			ImGui::SameLine();
			ImGui::Text(std::format("{0}x{1}", S.Window.width, S.Window.height).c_str());

			ImGui::TextWrapped(result.c_str());

			ImGui::EndTabItem();
		}
	}
};