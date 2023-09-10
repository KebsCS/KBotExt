#pragma once

#include "Definitions.h"
#include "Includes.h"
#include "LCU.h"

class skins_tab
{
public:
	static void render()
	{
		static bool b_on_open = true;
		static bool b_sort_on_open = false;

		if (ImGui::BeginTabItem("Skins"))
		{
			ImGui::Text("Sort by: ");
			ImGui::SameLine();

			static int i_sort = 0;
			static int i_last_sort = -1;
			ImGui::RadioButton("Alphabetically", &i_sort, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Purchase date", &i_sort, 1);
			ImGui::SameLine();
			ImGui::RadioButton("ID", &i_sort, 2);

			if (b_on_open)
			{
				b_on_open = false;
				b_sort_on_open = true;
				owned_skins.clear();

				static Json::Value root;
				static Json::CharReaderBuilder builder;
				static const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				static JSONCPP_STRING err;

				std::string get_skins =
					lcu::request("GET", "https://127.0.0.1/lol-inventory/v2/inventory/CHAMPION_SKIN");

				if (reader->parse(get_skins.c_str(), get_skins.c_str() + static_cast<int>(get_skins.length()), &root,
				                  &err))
				{
					if (root.isArray())
					{
						for (auto& i : root)
						{
							skin skin;

							skin.inventory_type = i["inventoryType"].asString();
							skin.item_id = i["itemId"].asInt();
							skin.ownership_type = i["ownershipType"].asString();
							skin.is_vintage = i["payload"]["isVintage"].asBool();

							std::string purchase = i["purchaseDate"].asString();
							if (purchase.find('-') == std::string::npos && purchase.find(':') == std::string::npos)
							{
								sscanf(purchase.c_str(), "%04d%02d%02dT%02d%02d%02d",
								       &skin.purchase_date.tm_year, &skin.purchase_date.tm_mon,
								       &skin.purchase_date.tm_mday,
								       &skin.purchase_date.tm_hour, &skin.purchase_date.tm_min,
								       &skin.purchase_date.tm_sec);
							}
							else
							{
								sscanf(purchase.c_str(), "%04d-%02d-%02dT%02d:%02d:%02d",
								       &skin.purchase_date.tm_year, &skin.purchase_date.tm_mon,
								       &skin.purchase_date.tm_mday,
								       &skin.purchase_date.tm_hour, &skin.purchase_date.tm_min,
								       &skin.purchase_date.tm_sec);
							}
							skin.purchase_date.tm_year -= 1901;
							skin.purchase_date.tm_mon -= 1;

							skin.quantity = i["quantity"].asInt();
							skin.uuid = i["uuid"].asString();

							if (!champ_skins.empty())
							{
								bool found = false;
								for (const auto& [key, name, skins] : champ_skins)
								{
									for (const auto& [fst, snd] : skins)
									{
										if (skin.item_id == std::stoi(fst))
										{
											skin.name = snd.c_str();
											found = true;
											break;
										}
									}
									if (found)
										break;
								}
							}
							owned_skins.emplace_back(skin);
						}
					}
				}
			}

			if (i_last_sort != i_sort || b_sort_on_open)
			{
				b_sort_on_open = false;
				i_last_sort = i_sort;
				switch (i_sort)
				{
				case 0:
					std::ranges::sort(owned_skins, [](const skin& lhs, const skin& rhs) {
						return lhs.name < rhs.name;
					});
					break;
				case 1:
					std::ranges::sort(owned_skins, [](skin lhs, skin rhs) {
						return mktime(&lhs.purchase_date) < mktime(&rhs.purchase_date);
					});
					break;
				case 2:
					std::ranges::sort(owned_skins, [](const skin& lhs, const skin& rhs) {
						return lhs.item_id < rhs.item_id;
					});
					break;
				default: ;
				}
			}

			ImGui::SameLine();
			static char all_names_separator[64] = ",";
			if (ImGui::Button("Copy names to clipboard##skinsTab"))
			{
				std::string all_names;
				for (const auto& [name, inventoryType, itemId, ownershipType, isVintage, purchaseDate, quantity, uuid] :
				     owned_skins)
				{
					if (name.empty())
						continue;
					all_names += name + all_names_separator;
				}
				utils::copy_to_clipboard(all_names);
			}
			ImGui::SameLine();

			const ImVec2 label_size = ImGui::CalcTextSize("W", nullptr, true);
			ImGui::InputTextMultiline("##separatorSkinsTab", all_names_separator, IM_ARRAYSIZE(all_names_separator),
			                          ImVec2(0, label_size.y + ImGui::GetStyle().FramePadding.y * 2.0f),
			                          ImGuiInputTextFlags_AllowTabInput);

			ImGui::Separator();
			ImGui::Text("Skins owned: %d", owned_skins.size());

			for (const auto& [name, inventoryType, itemId, ownershipType, isVintage, purchaseDate, quantity, uuid] :
			     owned_skins)
			{
				char time_buff[50];
				strftime(time_buff, sizeof(time_buff), "%G-%m-%d %H:%M:%S", &purchaseDate);

				std::string input_id = "skinInput";
				input_id.append(std::to_string(itemId));
				char input[512];
				strcpy(input, std::format(R"(name: {}
inventoryType: {}
itemId: {}
ownershipType: {}
isVintage: {}
purchaseDate: {}
quantity: {}
uuid: {})", name, inventoryType, itemId, ownershipType, isVintage, time_buff, quantity, uuid).c_str());
				ImGui::PushID(input_id.c_str());
				ImGui::InputTextMultiline("", input, IM_ARRAYSIZE(input), ImVec2(ImGui::GetWindowSize().x, 0),
				                          ImGuiInputTextFlags_ReadOnly);
				ImGui::PopID();
			}

			ImGui::EndTabItem();
		}
		else
			b_on_open = true;
	}
};
