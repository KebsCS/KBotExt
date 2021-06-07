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

	static void Render()
	{
		if (ImGui::BeginTabItem("Settings"))
		{
			//if (ImGui::Button("Save"))
			CSettings::Save();

			ImGui::Checkbox("Auto-rename", &S.autoRename);

			static char bufLeaguePath[MAX_PATH];
			std::copy(S.leaguePath.begin(), S.leaguePath.end(), bufLeaguePath);
			ImGui::Text("League path:");
			ImGui::InputText("##leaguePath", bufLeaguePath, MAX_PATH);
			S.leaguePath = bufLeaguePath;

			static char bufRiotPath[MAX_PATH];
			std::copy(S.riotPath.begin(), S.riotPath.end(), bufRiotPath);
			ImGui::Text("Riot path:");
			ImGui::InputText("##riotPath", bufRiotPath, MAX_PATH);
			S.riotPath = bufRiotPath;

			ImGui::EndTabItem();
		}
	}
};