#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "LCU.h"

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
			static int iLastSort = -1;
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

				std::string getSkins = LCU::Request("GET", "https://127.0.0.1/lol-inventory/v2/inventory/CHAMPION_SKIN");

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
							if (purchase.find("-") == std::string::npos && purchase.find(":") == std::string::npos)
							{
								sscanf(purchase.c_str(), "%04d%02d%02dT%02d%02d%02d",
									&skin.purchaseDate.tm_year, &skin.purchaseDate.tm_mon, &skin.purchaseDate.tm_mday,
									&skin.purchaseDate.tm_hour, &skin.purchaseDate.tm_min, &skin.purchaseDate.tm_sec);
							}
							else
							{
								sscanf(purchase.c_str(), "%04d-%02d-%02dT%02d:%02d:%02d",
									&skin.purchaseDate.tm_year, &skin.purchaseDate.tm_mon, &skin.purchaseDate.tm_mday,
									&skin.purchaseDate.tm_hour, &skin.purchaseDate.tm_min, &skin.purchaseDate.tm_sec);
							}
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

			ImGui::SameLine();
			static char allNamesSeparator[64] = ",";
			if (ImGui::Button("Copy names to clipboard##skinsTab"))
			{
				std::string allNames = "";
				for (const Skin& skin : ownedSkins)
				{
					if (skin.name == "")
						continue;
					allNames += skin.name + allNamesSeparator;
				}
				Utils::CopyToClipboard(allNames);
			}
			ImGui::SameLine();

			const ImVec2 label_size = ImGui::CalcTextSize("W", NULL, true);
			ImGui::InputTextMultiline("##separatorSkinsTab", allNamesSeparator, IM_ARRAYSIZE(allNamesSeparator),
				ImVec2(0, label_size.y + ImGui::GetStyle().FramePadding.y * 2.0f), ImGuiInputTextFlags_AllowTabInput);

			ImGui::Separator();
			ImGui::Text("Skins owned: %d", ownedSkins.size());

			for (const Skin& skin : ownedSkins)
			{
				char timeBuff[50];
				strftime(timeBuff, sizeof(timeBuff), "%G-%m-%d %H:%M:%S", &skin.purchaseDate);

				std::string inputId = "skinInput";
				inputId.append(std::to_string(skin.itemId));
				char input[512];
				strcpy(input, std::format(R"(name: {}
inventoryType: {}
itemId: {}
ownershipType: {}
isVintage: {}
purchaseDate: {}
quantity: {}
uuid: {})", skin.name, skin.inventoryType, skin.itemId, skin.ownershipType, skin.isVintage, timeBuff, skin.qunatity, skin.uuid).c_str());
				ImGui::PushID(inputId.c_str());
				ImGui::InputTextMultiline("", input, IM_ARRAYSIZE(input), ImVec2(ImGui::GetWindowSize().x, 0), ImGuiInputTextFlags_ReadOnly);
				ImGui::PopID();
			}

			ImGui::EndTabItem();
		}
		else
			bOnOpen = true;
	}
};