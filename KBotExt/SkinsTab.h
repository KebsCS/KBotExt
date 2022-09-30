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
		static bool bOnOpen = true;
		static bool bSortOnOpen = false;

		if (ImGui::BeginTabItem("Skins"))
		{
			ImGui::Text("Sort by: ");
			ImGui::SameLine();

			static int iSort = 0;
			static short iLastSort = -1;
			ImGui::RadioButton("Alphabetically", &iSort, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Purchase date", &iSort, 1);
			ImGui::SameLine();
			ImGui::RadioButton("ID", &iSort, 2);

			if (bOnOpen)
			{
				bOnOpen = false;
				bSortOnOpen = true;
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

							std::string purchase = root[i]["purchaseDate"].asString().c_str();
							sscanf(purchase.c_str(), "%04d%02d%02dT%02d%02d%02d",
								&skin.purchaseDate.tm_year, &skin.purchaseDate.tm_mon, &skin.purchaseDate.tm_mday,
								&skin.purchaseDate.tm_hour, &skin.purchaseDate.tm_min, &skin.purchaseDate.tm_sec);
							skin.purchaseDate.tm_year -= 1901;
							skin.purchaseDate.tm_mon -= 1;

							skin.qunatity = root[i]["quantity"].asInt();
							skin.uuid = root[i]["uuid"].asString().c_str();

							if (!champSkins.empty())
							{
								bool found = false;
								for (const auto& champ : champSkins)
								{
									for (const auto& s : champ.skins)
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

			if ((iLastSort != iSort) || bSortOnOpen)
			{
				bSortOnOpen = false;
				iLastSort = iSort;
				switch (iSort)
				{
					// alphabetically
				case 0:
					std::sort(ownedSkins.begin(), ownedSkins.end(), [](const Skin& lhs, const Skin& rhs) {
						return lhs.name < rhs.name;
						});
					break;
					// purchase date
				case 1:
					std::sort(ownedSkins.begin(), ownedSkins.end(), [](Skin lhs, Skin rhs) {
						return mktime(&lhs.purchaseDate) < mktime(&rhs.purchaseDate);
						});
					break;
					// id
				case 2:
					std::sort(ownedSkins.begin(), ownedSkins.end(), [](const Skin& lhs, const Skin& rhs) {
						return lhs.itemId < rhs.itemId;
						});
					break;
				}
			}

			ImGui::Separator();
			ImGui::Text("Skins owned: %d", ownedSkins.size());

			for (const Skin& skin : ownedSkins)
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

				char timeBuff[50];
				strftime(timeBuff, sizeof(timeBuff), "%G-%m-%d %H:%M:%S", &skin.purchaseDate);
				ImGui::Text("purchaseDate: %s", timeBuff);

				ImGui::Text("quantity: %d", skin.qunatity);
				ImGui::Text("uuid: %s", skin.uuid.c_str());
			}

			ImGui::EndTabItem();
		}
		else
			bOnOpen = true;
	}
};