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
		//todo add sorting

		static bool bOnOpen = true;
		if (ImGui::BeginTabItem("Skins"))
		{
			if (bOnOpen)
			{
				bOnOpen = false;
				ownedSkins.clear();

				static Json::Value root;
				static Json::CharReaderBuilder builder;
				static const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				static JSONCPP_STRING err;

				std::string getSkins = http->Request("GET", "https://127.0.0.1/lol-inventory/v2/inventory/CHAMPION_SKIN", "", auth->leagueHeader, "", "", auth->leaguePort);

				if (reader->parse(getSkins.c_str(), getSkins.c_str() + static_cast<int>(getSkins.length()), &root, &err))
				{
					if (root.isArray())
					{
						for (Json::Value::ArrayIndex i = 0; i < root.size(); ++i)
						{
							Skin skin;

							skin.inventoryType = root[i]["inventoryType"].asString().c_str();;
							skin.itemId = root[i]["itemId"].asInt();
							skin.ownershipType = root[i]["ownershipType"].asString().c_str();
							skin.isVintage = root[i]["payload"]["isVintage"].asBool();
							skin.purchaseDate = root[i]["purchaseDate"].asString().c_str();
							skin.qunatity = root[i]["quantity"].asInt();
							skin.uuid = root[i]["uuid"].asString().c_str();

							if (!champSkins.empty())
							{
								bool found = false;
								for (auto champ : champSkins)
								{
									for (auto s : champ.skins)
									{
										if (skin.itemId == std::stoi(s.first))
										{
											skin.name = s.second.c_str();
											found = true;
											break;
										}
									}
									if (found)
										break;
								}
							}
							ownedSkins.emplace_back(skin);
						}
					}
				}
			}

			ImGui::Text("Skins owned: %d", ownedSkins.size());

			for (Skin skin : ownedSkins)
			{
				ImGui::Separator();
				if (!skin.name.empty())
					ImGui::Text("name: %s", skin.name.c_str());
				//else if (!champSkins.empty())
				//	ImGui::Text("name: Chroma"); // todo find a way to get chroma's name
				ImGui::Text("inventoryType: %s", skin.inventoryType.c_str());
				ImGui::Text("itemId: %d", skin.itemId);
				ImGui::Text("ownershipType: %s", skin.ownershipType.c_str());
				ImGui::Text("isVintage: %d", skin.isVintage);
				ImGui::Text("purchaseDate: %s", skin.purchaseDate.c_str());

				/*std::string purchaseDateFormatted = root[i]["purchaseDate"].asString();
				size_t fintTPos = purchaseDateFormatted.find("T");
				size_t finddDotPos = purchaseDateFormatted.find(".");
				size_t findMinusPos = purchaseDateFormatted.find("-");
				if (findMinusPos == std::string::npos)
				{
					purchaseDateFormatted = purchaseDateFormatted.substr(0, finddDotPos);
					purchaseDateFormatted.insert(4, "-");
					purchaseDateFormatted.insert(7, "-");
					purchaseDateFormatted.replace(fintTPos + 2, 1, " ");
					purchaseDateFormatted.insert(fintTPos + 2 + 3, ":");
					purchaseDateFormatted.insert(fintTPos + 2 + 3 + 3, ":");
				}
				else
				{
					purchaseDateFormatted = purchaseDateFormatted.substr(0, finddDotPos);
					purchaseDateFormatted.replace(fintTPos, 1, " ");
				}
				ImGui::Text("purchaseDate: %s", purchaseDateFormatted.c_str());*/

				ImGui::Text("quantity: %d", skin.qunatity);
				ImGui::Text("uuid: %s", skin.uuid.c_str());
			}

			ImGui::EndTabItem();
		}
		else
			bOnOpen = true;
	}
};