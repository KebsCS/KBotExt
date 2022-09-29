#pragma once

#include <vector>
#include <string>

#define RandomInt(min, max) (rand() % (max - min + 1) + min)

struct ChampMinimal
{
	bool active;
	std::string alias;
	std::string banVoPath;
	std::string baseLoadScreenPath;
	bool botEnabled;
	std::string chooseVoPath;
	//disabledQueues
	bool freeToPlay;
	int id;
	std::string name;

	//ownership
	bool freeToPlayReward;
	int owned;

	std::string purchased;
	bool rankedPlayEnabled;
	std::pair<std::string, std::string>roles;
	std::string squarePortraitPath;
	std::string stingerSfxPath;
	std::string title;
};

struct ChampMastery
{
	int championId;
	int championLevel;
	int championPoints = 0;
	int championPointsSinceLastLevel;
	int championPointsUntilNextLevel;
	bool chestGranted;
	std::string formattedChampionPoints;
	std::string formattedMasteryGoal;
	std::string highestGrade;
	std::string lastPlayTime;
	std::string playerId;
	int tokensEarned;
};

struct ChampAll
{
	ChampMinimal min;
	ChampMastery mas;
};

inline std::vector<ChampMinimal>champsMinimal;
inline std::vector<ChampMastery>champsMastery;
inline std::vector<ChampAll>champsAll;

struct Skin
{
	std::string name;
	std::string inventoryType;
	int itemId;
	std::string ownershipType;
	bool isVintage;
	tm purchaseDate;
	int qunatity;
	std::string uuid;
};

inline std::vector<Skin>ownedSkins;

enum QueueID : const int
{
	DraftPick = 400,
	SoloDuo = 420,
	BlindPick = 430,
	Flex = 440,
	ARAM = 450,
	Clash = 700,
	IntroBots = 830,
	BeginnerBots = 840,
	IntermediateBots = 850,
	ARURF = 900,
	TFTNormal = 1090,
	TFTRanked = 1100,
	TFTTutorial = 1110,
	TFTHyperRoll = 1130,
	NexusBlitz = 1300,
	Tutorial1 = 2000,
	Tutorial2 = 2010,
	Tutorial3 = 2020,
};

struct Champ
{
	int key;
	std::string name;
	std::vector < std::pair<std::string, std::string>>skins;
};

inline std::vector<Champ>champSkins;
