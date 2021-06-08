#pragma once

#include <filesystem>

#include "Definitions.h"
#include "Includes.h"
#include "HTTP.h"
#include "Utils.h"
#include "Auth.h"
#include "Misc.h"
#include "Settings.h"

#pragma warning(disable : 4996)

class LoginTab
{
private:

	static std::string Login(std::string username, std::string password)
	{
		std::string loginBody = R"({"username":")" + username + R"(","password":")" + password + R"(","persistLogin":false})";
		return http->Request("PUT", "https://127.0.0.1/rso-auth/v1/session/credentials", loginBody, auth->riotHeader, "", "", auth->riotPort);
	}

public:
	static void Render()
	{
		if (ImGui::BeginTabItem("Login"))
		{
			static std::string result;
			ImGui::Columns(2, 0, false);

			static std::vector<std::pair<std::string, std::string>>langs = {
				{"English (US)", "en_US"},{"Japanese","ja_JP"},{"Korean","ko_KR"},{"Chinese (China)","zh_CN"},
				{"German","de_DE"},{"Spanish (Spain)","es_ES"},{"Polish","pl_PL"},{"Russian","ru_RU"},
				{"French","fr_FR"},{"Turkish","tr_TR"},{"Portuguese","pt_BR"},{"Czech","cs_CZ"},{"Greek","el_GR"},
				{"Romanian","ro_RO"},{"Hungarian","hu_HU"},{"English (UK)","en_GB"},{"Italian","it_IT"},
				{"Spanish (Mexico)","es_MX"},{"Spanish (Argentina)","es_AR"},{"English (Australia)","en_AU"},
				{"Malay","ms_MY"},{"English (Philippines)","en_PH"},{"English (Singapore)","en_SG"},{"Thai","th_TH"},
				{"Vietnamese","vn_VN"},{"Indonesian","id_ID"},{"Chinese (Malaysia)","zh_MY"},{"Chinese (Taiwan)","zh_TW"}
			};
			// find saved lang from cfg file
			auto findLang = std::find_if(langs.begin(), langs.end(), [](std::pair<std::string, std::string>k) { return k.second == S.language; });

			static std::pair<std::string, std::string>selectedLang = { findLang[0].first,findLang[0].second };

			if (ImGui::Button("Launch client"))
			{
				if (!std::filesystem::exists(S.leaguePath))
				{
					result = "Invadlid path, change it in Settings tab";
				}
				else
				{
					ShellExecuteA(NULL, NULL, std::format("{}LeagueClient.exe", S.leaguePath).c_str(), std::format("--locale={}", selectedLang.second).c_str(), NULL, SW_SHOWNORMAL);
					result = S.leaguePath + "LeagueClient.exe --locale=" + selectedLang.second; // todo custom arguments
				}
			}
			ImGui::SameLine();

			if (ImGui::BeginCombo("##language", selectedLang.first.c_str()))
			{
				for (auto lang : langs)
				{
					if (ImGui::Selectable(lang.first.c_str(), lang.first == selectedLang.first))
					{
						selectedLang = { lang.first,lang.second };
						S.language = lang.second;
						CSettings::Save();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::NextColumn();

			if (ImGui::Button("Launch legacy client"))
			{
				if (!std::filesystem::exists(S.leaguePath))
				{
					result = "Invadlid path, change it in Settings tab";
				}
				else
				{
					Misc::LaunchLegacyClient();
				}
			}
			ImGui::Columns(1);

			ImGui::Separator();

			static char username[50];
			ImGui::Text("Username:");
			ImGui::InputText("##inputUsername", username, IM_ARRAYSIZE(username));

			static char password[50];
			ImGui::Text("Password:");
			ImGui::InputText("##inputPassword", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

			if (ImGui::Button("Login") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter), false))
			{
				if (!std::string(username).empty() && !std::string(password).empty())
					result = Login(username, password);
			}

			ImGui::SameLine();
			if (ImGui::Button("Save") && !std::string(username).empty() && !std::string(password).empty())
			{
				// if file doesn't exist, create new one with {} so it can be parsed
				if (!std::filesystem::exists(S.settingsFile))
				{
					std::ofstream file(S.settingsFile);
					file << "{}";
					file.close();
				}

				Json::Reader reader;
				Json::Value root;

				std::ifstream iFile(S.settingsFile);

				if (iFile.good())
				{
					if (reader.parse(iFile, root, false))
					{
						if (!root["accounts"].isArray())
							root["accounts"] = Json::Value(Json::arrayValue);
						Json::Value accArray = root["accounts"];

						accArray.append(std::format("{0}:{1}", username, password));
						root["accounts"] = accArray;

						std::ofstream oFile(S.settingsFile);
						oFile << root.toStyledString() << std::endl;
						oFile.close();
					}
				}
				iFile.close();
			}

			ImGui::SameLine();
			Misc::HelpMarker("This part is only, if you want to save your login and pass to .txt file and login with 1 click. You don't have to do that, you can just log in the usual way in client and launch the tool anytime you want");

			ImGui::Separator();

			Json::Reader reader;
			Json::Value root;

			std::ifstream iFile(S.settingsFile);

			if (iFile.good())
			{
				if (reader.parse(iFile, root, false))
				{
					auto accArray = root["accounts"];
					if (accArray.isArray())
					{
						for (Json::Value::ArrayIndex i = 0; i < accArray.size(); ++i)
						{
							std::string acc = accArray[i].asString();
							std::string username = acc.substr(0, acc.find(":"));
							std::string password = acc.substr(acc.find(":") + 1);
							if (ImGui::Button(username.c_str()))
							{
								result = Login(username, password);
							}

							ImGui::SameLine();
							std::string deleteButton = "Delete##" + acc;
							if (ImGui::Button(deleteButton.c_str()))
							{
								std::ofstream oFile(S.settingsFile);
								accArray.removeIndex(i, 0);
								root["accounts"] = accArray;
								oFile << root.toStyledString() << std::endl;
								oFile.close();
							}
						}
					}
				}
			}
			iFile.close();

			ImGui::TextWrapped(result.c_str());

			ImGui::EndTabItem();
		}
	}
};