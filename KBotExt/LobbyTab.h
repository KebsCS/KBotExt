#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "HTTP.h"
#include "Utils.h"
#include "Config.h"
#include "Auth.h"
#include "LCU.h"
#include "Misc.h"
#include <string>

class LobbyTab
{
public:
	static void Render()
	{
		if (ImGui::BeginTabItem("Lobby"))
		{
			static bool once = true;
			static std::string result;
			static bool bPressed = false;

			// static std::string accID;
			// static std::string summID;
			// static std::string summName;

			static char lobbyMessage[2056];
			static char messageName[50];
			if (once)
			{
				once = false;
				std::copy(S.lobbyTab.lobbyMessage.begin(), S.lobbyTab.lobbyMessage.end(), lobbyMessage);
			}

			ImGui::Text("Message name:");
			ImGui::InputText("##inputmessageName", messageName, IM_ARRAYSIZE(messageName));
			S.lobbyTab.messageName = messageName;

			ImGui::Text("Message content:");
			ImGui::InputText("##inputlobbyMessage", lobbyMessage, IM_ARRAYSIZE(lobbyMessage));
			S.lobbyTab.lobbyMessage = lobbyMessage;
			ImGui::SameLine();

			if (ImGui::Button("Send##lobbyMessage") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter), false))
			{
				Json::Value root;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;
				std::string getChat = LCU::Request("GET", "https://127.0.0.1/lol-chat/v1/conversations");
				if (reader->parse(getChat.c_str(), getChat.c_str() + static_cast<int>(getChat.length()), &root, &err))
				{
					if (root.isArray())
					{
						int j = 0;
						for (Json::Value::ArrayIndex i = 0; (i < root.size()) && (j == 0); i++)
						{
							if (root[i]["type"].asString() == "championSelect" || root[i]["type"].asString() == "customGame" || root[i]["type"].asString() == "postGame") {
								j++;
								// printf("root[i]['type'].asString() == 'championSelect': %d\n", root[i]["type"].asString() == "championSelect");
								// printf("root[i]['type'].asString() == 'customGame': %d\n", root[i]["type"].asString() == "customGame");
								std::string lobbyID = root[i]["id"].asString();
								std::string request = "https://127.0.0.1/lol-chat/v1/conversations/" + lobbyID + "/messages";
								std::string error = "errorCode";
								// ImGui::InputTextMultiline("##result", &error[0], 11, ImVec2(600, 200));
								error = LCU::Request("POST", request, R"({"type":"chat", "body":")" + std::string(S.lobbyTab.lobbyMessage) + R"("})");
								printf("%s", error.c_str());

							}
						}
					}
				}


			}

			ImGui::SameLine();
			if (ImGui::Button("Save") && !std::string(lobbyMessage).empty() && !std::string(messageName).empty())
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
						if (!root["messages"].isArray())
							root["messages"] = Json::Value(Json::arrayValue);
						Json::Value accArray = root["messages"];
						Json::Value nameArray = root["names"];

						accArray.append(std::format("{0}", lobbyMessage));
						root["messages"] = accArray;

						nameArray.append(std::format("{0}", messageName));
						root["names"] = nameArray;

						std::ofstream oFile(S.settingsFile);
						oFile << root.toStyledString() << std::endl;
						oFile.close();
					}
				}
				iFile.close();
			}

			ImGui::SameLine();
			ImGui::HelpMarker("Save a message to send later");

			ImGui::Separator();

			Json::Reader reader;
			Json::Value root;

			std::ifstream iFile(S.settingsFile);

			if (iFile.good())
			{
				if (reader.parse(iFile, root, false))
				{
					auto accArray = root["messages"];
					auto nameArray = root["names"];
					if (accArray.isArray())
					{
						for (Json::Value::ArrayIndex i = 0; i < accArray.size(); ++i)
						{
							std::string msg = accArray[i].asString();
							std::string msgDisp = accArray[i].asString();
							std::string msgName = nameArray[i].asString();
							msgDisp.resize(60);
							if (ImGui::Button(msgName.c_str()))
							{
								Json::Value root;
								Json::CharReaderBuilder builder;
								const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
								JSONCPP_STRING err;
								std::string getChat = LCU::Request("GET", "https://127.0.0.1/lol-chat/v1/conversations");
								if (reader->parse(getChat.c_str(), getChat.c_str() + static_cast<int>(getChat.length()), &root, &err))
								{
									if (root.isArray())
									{
										int j = 0;
										for (Json::Value::ArrayIndex i = 0; (i < root.size()) && (j == 0); i++)
										{
											if (root[i]["type"].asString() == "championSelect" || root[i]["type"].asString() == "customGame" || root[i]["type"].asString() == "postGame") {
												j++;
												// printf("root[i]['type'].asString() == 'championSelect': %d\n", root[i]["type"].asString() == "championSelect");
												// printf("root[i]['type'].asString() == 'customGame': %d\n", root[i]["type"].asString() == "customGame");
												std::string lobbyID = root[i]["id"].asString();
												std::string request = "https://127.0.0.1/lol-chat/v1/conversations/" + lobbyID + "/messages";
												std::string error = "errorCode";
												// ImGui::InputTextMultiline("##result", &error[0], 11, ImVec2(600, 200));
												error = LCU::Request("POST", request, R"({"type":"chat", "body":")" + std::string(msg) + R"("})");
											}
										}
									}
								}
							}

							ImGui::SameLine();
							std::string deleteButton = "Delete##" + msg;
							if (ImGui::Button(deleteButton.c_str()))
							{
								std::ofstream oFile(S.settingsFile);
								accArray.removeIndex(i, 0);
								nameArray.removeIndex(i, 0);
								root["messages"] = accArray;
								root["names"] = nameArray;
								oFile << root.toStyledString() << std::endl;
								oFile.close();
							}
						}
					}
				}
			}
			iFile.close();


			ImGui::EndTabItem();
		}
	}
};