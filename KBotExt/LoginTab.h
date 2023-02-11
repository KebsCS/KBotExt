#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "Utils.h"
#include "LCU.h"
#include "Misc.h"
#include "Config.h"

#pragma warning(disable : 4996)

class LoginTab
{
private:

	static void LoginOnClientOpen(std::string username, std::string password)
	{
		while (true)
		{
			if (::FindWindowA("RCLIENT", "Riot Client") && LCU::riot.port != 0)
			{
				// waits to be sure that client is fully loaded
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				Login(username, password);
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	static std::string Login(std::string username, std::string password)
	{
		// If riot client not open
		if (LCU::riot.port == 0)
		{
			if (std::filesystem::exists(S.leaguePath))
			{
				Misc::LaunchClient("");
				std::thread t(LoginOnClientOpen, username, password);
				t.detach();
				return "Launching client...";
			}
			else
				return "Invalid client path, change it in Settings tab";
		}

		// refresh session
		HTTP::Request("POST", "https://127.0.0.1/rso-auth/v2/authorizations", R"({"clientId":"riot-client","trustLevels":["always_trusted"]})", LCU::riot.header, "", "", LCU::riot.port);

		std::string loginBody = R"({"username":")" + username + R"(","password":")" + password + R"(","persistLogin":false})";
		std::string result = HTTP::Request("PUT", "https://127.0.0.1/rso-auth/v1/session/credentials", loginBody, LCU::riot.header, "", "", LCU::riot.port);

		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		JSONCPP_STRING err;
		Json::Value root;
		if (reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
		{
			if (!root["type"].empty())
				return root["type"].asString();
			else if (!root["message"].empty())
				return root["message"].asString();
		}
		return result;
	}

public:
	static void Render()
	{
		if (ImGui::BeginTabItem("Login"))
		{
			static std::string result;
			static bool once = true;
			static char leagueArgs[1024 * 16];
			static std::string sArgs;

			if (once)
			{
				once = false;
				std::copy(S.loginTab.leagueArgs.begin(), S.loginTab.leagueArgs.end(), leagueArgs);
			}

			ImGui::Columns(2, 0, false);

			static std::vector<std::pair<std::string, std::string>>langs = {
				{"English (US)", "en_US"},{"Japanese","ja_JP"},{"Korean","ko_KR"},{"Chinese (China)","zh_CN"},
				{"German","de_DE"},{"Spanish (Spain)","es_ES"},{"Polish","pl_PL"},{"Russian","ru_RU"},
				{"French","fr_FR"},{"Turkish","tr_TR"},{"Portuguese","pt_BR"},{"Czech","cs_CZ"},{"Greek","el_GR"},
				{"Romanian","ro_RO"},{"Hungarian","hu_HU"},{"English (UK)","en_GB"},{"Italian","it_IT"},
				{"Spanish (Mexico)","es_MX"},{"Spanish (Argentina)","es_AR"},{"English (Australia)","en_AU"},
				{"Malay","ms_MY"},{"English (Philippines)","en_PH"},{"English (Singapore)","en_SG"},{"Thai","th_TH"},
				{"Vietnamese","vi_VN"},{"Indonesian","id_ID"},{"Tagalog","tl_PH"},{"Chinese (Malaysia)","zh_MY"},{"Chinese (Taiwan)","zh_TW"}
			};
			// find saved lang from cfg file
			auto findLang = std::find_if(langs.begin(), langs.end(), [](std::pair<std::string, std::string>k) { return k.second == S.loginTab.language; });

			static std::pair<std::string, std::string>selectedLang = { findLang[0].first,findLang[0].second };

			if (ImGui::Button("Launch client"))
			{
				if (!std::filesystem::exists(S.leaguePath))
				{
					result = "Invalid path, change it in Settings tab";
				}
				else
				{
					Misc::LaunchClient(sArgs);
					result = S.leaguePath + "LeagueClient.exe " + sArgs;
				}
			}
			ImGui::SameLine();

			if (ImGui::BeginCombo("##language", selectedLang.first.c_str()))
			{
				for (const auto& lang : langs)
				{
					if (ImGui::Selectable(lang.first.c_str(), lang.first == selectedLang.first))
					{
						selectedLang = { lang.first,lang.second };
						S.loginTab.language = lang.second;
						Config::Save();

						std::string localeArg = std::format("--locale={} ", selectedLang.second);
						size_t localePos = sArgs.find("--locale=");
						if (localePos != std::string::npos)
						{
							sArgs.replace(localePos, localeArg.size(), localeArg);
						}
						else
							sArgs += localeArg;
					}
				}
				ImGui::EndCombo();
			}

			ImGui::NextColumn();

			if (ImGui::Button("Launch legacy client"))
			{
				if (!std::filesystem::exists(S.leaguePath))
				{
					result = "Invalid path, change it in Settings tab";
				}
				else
				{
					Misc::LaunchLegacyClient();
				}
			}
			ImGui::Columns(1);

			std::copy(sArgs.begin(), sArgs.end(), leagueArgs);
			ImGui::Text(" Args: ");
			ImGui::SameLine();
			ImGui::InputText("##inputLeagueArgs", leagueArgs, IM_ARRAYSIZE(leagueArgs));

			sArgs = leagueArgs;
			S.loginTab.leagueArgs = sArgs;

			ImGui::Separator();

			static char username[128];
			ImGui::Text("Username:");
			ImGui::InputText("##inputUsername", username, IM_ARRAYSIZE(username));

			static char password[128];
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
			ImGui::HelpMarker("This part is only, if you want to save your login and pass to config file and login with 1 click. You don't have to do that, you can just log in the usual way in client and launch the tool anytime you want");

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
							std::string accUsername = acc.substr(0, acc.find(":"));
							std::string accPassword = acc.substr(acc.find(":") + 1);
							if (ImGui::Button(accUsername.c_str()))
							{
								result = Login(accUsername, accPassword);
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