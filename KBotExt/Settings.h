#pragma once

#include <string>
#include <fstream>
#include <filesystem>

#include "Includes.h"

#pragma warning (disable: 4996)

struct Settings
{
	Settings()
	{};
	std::string settingsFile = "config.JSON";

	bool autoRename = false;
	std::string leaguePath = "C:/Riot Games/League of Legends/";
	std::string riotPath = "C:/Riot Games/Riot Client/";
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
				root["riotPath"] = S.riotPath;

				std::ofstream oFile(S.settingsFile);
				oFile << root.toStyledString() << std::endl;
				oFile.close();
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
				if (auto t = root["riotPath"]; !t.empty()) S.riotPath = t.asString();
			}
		}
		file.close();
	}
};
