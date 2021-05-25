#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "HTTP.h"
#include "Auth.h"

class SkinsTab
{
public:
	static void Render()
	{
		static bool skinsOpen = true;
		if (ImGui::BeginTabItem("Skins"))
		{
			static std::string result;
			static Json::Value root;

			if (skinsOpen)
			{
				skinsOpen = false;
				result = http->Request("GET", "https://127.0.0.1/lol-inventory/v2/inventory/CHAMPION_SKIN", "", auth->leagueHeader, "", "", auth->leaguePort);

				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				JSONCPP_STRING err;

				if (!reader->parse(result.c_str(), result.c_str() + static_cast<int>(result.length()), &root, &err))
					result = "Failed to parse JSON";
			}

			if (root.isArray())
			{
				ImGui::Text("Skins owned: %d", root.size());

				for (Json::Value::ArrayIndex i = 0; i < root.size(); ++i)
				{
					ImGui::Separator();
					int itemId = root[i]["itemId"].asInt();
					if (!champSkins.empty())
					{
						bool found = false;
						for (auto champ : champSkins)
						{
							if (std::to_string(itemId).substr(0, std::to_string(champ.key).size()) == std::to_string(champ.key))
							{
								for (auto skin : champ.skins)
								{
									if (std::to_string(itemId) == skin.first)
									{
										ImGui::Text("Name: %s", skin.second.c_str());
										found = true;
										break;
									}
								}
								if (found)
									break;
							}
						}
					}

					ImGui::Text("inventoryType: %s", root[i]["inventoryType"].asString().c_str());
					ImGui::Text("itemId: %d", itemId);
					ImGui::Text("ownershipType: %s", root[i]["ownershipType"].asString().c_str());
					auto payloadObj = root[i]["payload"];
					ImGui::Text("isVintage: %d", payloadObj["isVintage"].asInt());
					std::string purchaseDateFormatted = root[i]["purchaseDate"].asString();
					purchaseDateFormatted.insert(4, ".");
					purchaseDateFormatted.insert(7, ".");
					purchaseDateFormatted.insert(10, " ");
					ImGui::Text("purchaseDate: %s", purchaseDateFormatted.c_str());
					ImGui::Text("quantity: %d", root[i]["quantity"].asInt());
					ImGui::Text("uuid: %s", root[i]["uuid"].asString().c_str());
				}
			}
			ImGui::EndTabItem();
		}
		else skinsOpen = true;
	}
};