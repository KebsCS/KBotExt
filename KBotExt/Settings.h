#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

#include "Includes.h"

#pragma warning (disable: 4996)

struct Settings
{
	Settings()
	{};
	std::string settingsFile = "config.JSON";

	bool autoRename = false;
	std::string leaguePath = "C:/Riot Games/League of Legends/";
	std::vector<std::string>vFonts;
	int selectedFont = 0;
	bool bAddFont = false;
	bool streamProof = false;
	bool debugger = false;

	struct
	{
		int width = 700;
		int height = 500;
		bool resize = false;
	}Window;

	struct
	{
		std::string playerName;
	}infoTab;

	struct
	{
		std::string method;
		std::string urlText;
		std::string requestText;
	}customTab;

	struct
	{
		std::string destination;
		std::string method;
		std::string args;
	}invokeTab;

	struct
	{
		std::string language = "en_US";
		std::string leagueArgs = "--locale=en_US";
	}loginTab;

	//struct
	//{
	//
	//}profileTab;
};
extern Settings S;

class CSettings
{
public:

	static void Save()
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
				root["autoRename"] = S.autoRename;
				root["leaguePath"] = S.leaguePath;
				root["debugger"] = S.debugger;
				root["window"]["width"] = S.Window.width;
				root["window"]["height"] = S.Window.height;
				root["selectedFont"] = S.selectedFont;
				root["loginTab"]["language"] = S.loginTab.language;
				root["loginTab"]["leagueArgs"] = S.loginTab.leagueArgs;
				root["streamProof"] = S.streamProof;
				root["infoTab"]["playerName"] = S.infoTab.playerName;
				root["customTab"]["method"] = S.customTab.method;
				root["customTab"]["urlText"] = S.customTab.urlText;
				root["customTab"]["requestText"] = S.customTab.requestText;
				root["invokeTab"]["destination"] = S.invokeTab.destination;
				root["invokeTab"]["method"] = S.invokeTab.method;
				root["invokeTab"]["args"] = S.invokeTab.args;

				if (S.bAddFont)
				{
					S.bAddFont = false;
					if (!root["fonts"].isArray())
						root["fonts"] = Json::Value(Json::arrayValue);
					Json::Value fontsArray = root["fonts"];

					// clear so we dont append same fonts again
					fontsArray.clear();
					for (std::string font : S.vFonts)
						fontsArray.append(font);
					root["fonts"] = fontsArray;
				}

				if (!root.toStyledString().empty())
				{
					std::ofstream oFile(S.settingsFile);
					oFile << root.toStyledString() << std::endl;
					oFile.close();
				}
			}
		}
		iFile.close();
	}

	static void Load()
	{
		std::fstream file(S.settingsFile, std::ios_base::in);
		if (file.good())
		{
			std::string config;
			std::string temp;
			while (std::getline(file, temp))
			{
				config += temp + "\n";
			}

			Json::Value root;
			Json::CharReaderBuilder builder;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			JSONCPP_STRING err;

			if (reader->parse(config.c_str(), config.c_str() + static_cast<int>(config.length()), &root, &err))
			{
				if (auto t = root["autoRename"]; !t.empty()) S.autoRename = t.asBool();
				if (auto t = root["leaguePath"]; !t.empty()) S.leaguePath = t.asString();
				if (auto t = root["debugger"]; !t.empty()) S.debugger = t.asBool();
				if (auto t = root["window"]["width"]; !t.empty()) S.Window.width = t.asInt();
				if (auto t = root["window"]["height"]; !t.empty()) S.Window.height = t.asInt();
				if (auto t = root["selectedFont"]; !t.empty()) S.selectedFont = t.asInt();
				if (auto t = root["loginTab"]["language"]; !t.empty()) S.loginTab.language = t.asString();
				if (auto t = root["loginTab"]["leagueArgs"]; !t.empty()) S.loginTab.leagueArgs = t.asString();
				if (auto t = root["streamProof"]; !t.empty()) S.streamProof = t.asBool();
				if (auto t = root["infoTab"]["playerName"]; !t.empty()) S.infoTab.playerName = t.asString();
				if (auto t = root["customTab"]["method"]; !t.empty()) S.customTab.method = t.asString();
				if (auto t = root["customTab"]["urlText"]; !t.empty()) S.customTab.urlText = t.asString();
				if (auto t = root["customTab"]["requestText"]; !t.empty()) S.customTab.requestText = t.asString();
				if (auto t = root["invokeTab"]["destination"]; !t.empty()) S.invokeTab.destination = t.asString();
				if (auto t = root["invokeTab"]["method"]; !t.empty()) S.invokeTab.method = t.asString();
				if (auto t = root["invokeTab"]["args"]; !t.empty()) S.invokeTab.args = t.asString();

				if (root["fonts"].isArray() && !root["fonts"].empty())
				{
					for (Json::Value::ArrayIndex i = 0; i < root["fonts"].size(); i++)
					{
						S.vFonts.emplace_back(root["fonts"][i].asString());
					}
				}
			}
		}
		file.close();
	}
};
