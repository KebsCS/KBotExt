#pragma once

#include <vector>
#include <string>

struct champ_minimal
{
	bool active;
	std::string alias;
	std::string ban_vo_path;
	std::string base_load_screen_path;
	bool bot_enabled;
	std::string choose_vo_path;
	//disabledQueues
	bool free_to_play;
	int id;
	std::string name;

	//ownership
	bool free_to_play_reward;
	int owned;

	std::string purchased;
	bool ranked_play_enabled;
	std::pair<std::string, std::string> roles;
	std::string square_portrait_path;
	std::string stinger_sfx_path;
	std::string title;
};

struct champ_mastery
{
	int champion_id;
	int champion_level;
	int champion_points = 0;
	int champion_points_since_last_level;
	int champion_points_until_next_level;
	bool chest_granted;
	std::string formatted_champion_points;
	std::string formatted_mastery_goal;
	std::string highest_grade;
	std::string last_play_time;
	std::string player_id;
	int tokens_earned;
};

struct champ_all
{
	champ_minimal min;
	champ_mastery mas;
};

inline std::vector<champ_minimal> champs_minimal;
inline std::vector<champ_mastery> champs_mastery;
inline std::vector<champ_all> champs_all;

struct skin
{
	std::string name;
	std::string inventory_type;
	int item_id;
	std::string ownership_type;
	bool is_vintage;
	tm purchase_date;
	int quantity;
	std::string uuid;
};

inline std::vector<skin> owned_skins;

enum queue_id
{
	draft_pick = 400,
	solo_duo = 420,
	blind_pick = 430,
	flex = 440,
	aram = 450,
	clash = 700,
	intro_bots = 830,
	beginner_bots = 840,
	intermediate_bots = 850,
	arurf = 900,
	tft_normal = 1090,
	tft_ranked = 1100,
	tft_tutorial = 1110,
	tft_hyper_roll = 1130,
	tft_double_up = 1160,
	nexus_blitz = 1300,
	tutorial1 = 2000,
	tutorial2 = 2010,
	tutorial3 = 2020,
};

struct champ
{
	int key;
	std::string name;
	std::vector<std::pair<std::string, std::string>> skins;
};

inline std::vector<champ> champ_skins;
