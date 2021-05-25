#pragma once

#include <filesystem>

#include "Definitions.h"
#include "Includes.h"
#include "HTTP.h"
#include "Utils.h"
#include "Auth.h"
#include "Misc.h"

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
		static std::string result;
		ImGui::Columns(2, 0, false);

		if (ImGui::Button("Launch client"))
		{
			ShellExecute(NULL, NULL, L"C:\\Riot Games\\Riot Client\\RiotClientServices.exe", L"--launch-product=league_of_legends --launch-patchline=live", NULL, SW_SHOWNORMAL);
		}

		ImGui::SameLine();
		Misc::HelpMarker("Typing in custom league path in future update");

		ImGui::NextColumn();

		if (ImGui::Button("Launch legacy client"))
		{
			if (!std::filesystem::exists("C:/Riot Games/League of Legends/"))
			{
				//todo typing in lol path
				result = "League isnt installed in default path";
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
			result = Login(username, password);
		}

		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			std::ofstream accFile;
			accFile.open("accounts.txt", std::ios_base::app);
			accFile << username << ":" << password << std::endl;

			accFile.close();
		}

		ImGui::SameLine();
		Misc::HelpMarker("This part is only, if you want to save your login and pass to .txt file and login with 1 click. You don't have to do that, you can just log in the usual way in client and launch the tool anytime you want");

		ImGui::Separator();

		std::fstream accFile("accounts.txt");
		std::vector<std::string> vAccounts;
		std::string tempAcc;
		while (accFile >> tempAcc)
		{
			vAccounts.emplace_back(tempAcc);
		}
		accFile.close();
		for (std::string& acc : vAccounts)
		{
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
				acc = "";
				std::ofstream accFile1;
				accFile1.open("accounts.txt");
				for (std::string acc1 : vAccounts)
				{
					std::string username1 = acc1.substr(0, acc1.find(":"));
					std::string password1 = acc1.substr(acc1.find(":") + 1);
					if (acc1 != "")
						accFile1 << username1 << ":" << password1 << std::endl;
				}
				accFile1.close();
			}
		}

		ImGui::TextWrapped(result.c_str());
	}
};