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

			static char bufLeaguePath[MAX_PATH];
			std::copy(S.leaguePath.begin(), S.leaguePath.end(), bufLeaguePath);
			ImGui::Text("League path:");
			ImGui::InputText("##leaguePath", bufLeaguePath, MAX_PATH);
			S.leaguePath = bufLeaguePath;

			if (ImGui::Button("Save Settings"))
			{
				CSettings::Save();
				result = "Saved";
			}

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