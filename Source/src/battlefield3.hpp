/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#ifndef BATTLEFIELD3_HPP
#define BATTLEFIELD3_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include "bf3_socket.hpp"
#include "SQLite.hpp"
#include "vote.hpp"

namespace Battlefield
{
	namespace Weapons
	{
		const unsigned char maxWeapons = 148;
		const char names[maxWeapons][23] =
		{
			"DamageArea"
			"Death",
			"Melee",
			"RoadKill",
			"SoldierCollision",
			"Suicide",
			"U_870",
			"U_A91",
			"U_ACR",
			"U_AEK971",
			"U_AEK971_M320_HE",
			"U_AEK971_M320_LVG",
			"U_AK12",
			"U_AK12_M320_HE",
			"U_AK5C",
			"U_AKU12",
			"U_AMR2",
			"U_AMR2_MED",
			"U_AT4",
			"U_C4",
			"U_C4_Support",
			"U_CBJ-MS",
			"U_Claymore",
			"U_Claymore_Recon",
			"U_CS-LR4",
			"U_CZ805",
			"U_CZ75",
			"U_DBV12",
			"U_Defib",
			"U_FAMAS",
			"U_FGM148",
			"U_FIM92",
			"U_Flashbang",
			"U_FN57",
			"U_FY-JS",
			"U_G36C",
			"U_GalilACE",
			"U_GalilACE23",
			"U_GalilACE52",
			"U_GalilACE53",
			"U_Glock18",
			"U_Grenade_RGO",
			"U_HAWK",
			"U_HK45C",
			"U_JNG90",
			"U_JS2",
			"U_L85A2",
			"U_L96A1",
			"U_LSAT",
			"U_M1014",
			"U_M15",
			"U_M16A4",
			"U_M16A4_M320_HE",
			"U_M1911",
			"U_M200",
			"U_M240",
			"U_M249",
			"U_M26Mass",
			"U_M26Mass_Flechette",
			"U_M26Mass_Frag",
			"U_M26Mass_Slug",
			"U_M320_HE",
			"U_M320_LVG",
			"U_M320_SHG",
			"U_M320_SMK",
			"U_M34",
			"U_M39EBR",
			"U_M40A5",
			"U_M416",
			"U_M416_M26_Buck",
			"U_M416_M26_Flechette",
			"U_M416_M26_Slug",
			"U_M416_M320_FLASH",
			"U_M416_M320_HE",
			"U_M416_M320_SMK",
			"U_M4A1",
			"U_M67",
			"U_M82A3",
			"U_M82A3_CQB",
			"U_M82A3_MED",
			"U_M9",
			"U_M93R",
			"U_M98B",
			"U_MagpulPDR",
			"U_Medkit",
			"U_MG4",
			"U_MGL",
			"U_MK11",
			"U_MP7",
			"U_MP412Rex",
			"U_MP443",
			"U_MTAR21",
			"U_MX4",
			"U_NLAW",
			"U_P226",
			"U_P90",
			"U_Pecheneg",
			"U_PortableMedicpack",
			"U_PP2000",
			"U_QBB95",
			"U_QBS09",
			"U_QBU88",
			"U_QBZ951",
			"U_QBZ951_M320_HE", 
			"U_QSZ92",
			"U_Repairtool",
			"U_RFB",
			"U_RPG7",
			"U_RPK12",
			"U_RPK-74",
			"U_Sa18IGLA",
			"U_SAIGA_20K",
			"U_SAR21",
			"U_SAR21_M320_HE",
			"U_SAR21_M320_SMK",
			"U_SCAR-H",
			"U_SCAR-H_M26_Buck",
			"U_SCAR-H_M26_Flechette",
			"U_SCAR-H_M320_HE",
			"U_SCAR-HSV",
			"U_Scorpion",
			"U_Scout",
			"U_SerbuShorty",
			"U_SG553LB",
			"U_SKS",
			"U_SLAM",
			"U_SMAW",
			"U_SPAS12",
			"U_SRAW",
			"U_SRS",
			"U_Starstreak",
			"U_SteyrAug",
			"U_SteyrAug_M320_HE",
			"U_SteyrAug_M320_LVG",
			"U_SV98",
			"U_SVD12",
			"U_Taurus44",
			"U_Tomahawk",
			"U_Type88",
			"U_Type95B",
			"U_Ultimax",
			"U_UMP45",
			"U_USAS-12",
			"U_USAS-12_Nightvision",
			"U_UTAS",
			"U_V40",
			"U_XM25",
			"U_XM25_Flechette",
			"U_XM25_Smoke"
		};
	}

	struct struct_player
	{
		std::string name;
		std::string guid;
		std::string ip;
		unsigned char team;
		unsigned char squad;
		unsigned short kills;
		unsigned short deaths;
		unsigned int score;
		unsigned int rank;
		unsigned int ping;
		unsigned short hs;
		bool spawned;
	};

	struct struct_count
	{
		unsigned char ucPlayer;
		unsigned char ucTeamUS;
		unsigned char ucTeamRU;
	};

	struct struct_chat
	{
		bool bMessageToTeam;
		char cIsOnTeam;
		std::string sTime;
		std::string sName;
		std::string sMessage;
	};

	namespace gamestate
	{
		enum state
		{
			ACTIVE,
			EMPTY,
			INACTIVE,
			UNCONNECTED
		};
	}

	class game : public bf3_socket
	{
		private:
			
			// MySQL Connection
			Database::SQLite *sql;

			// Whitelist
			std::vector<std::string> whitelist;

			// Chatlist
			std::list<struct_chat> chatlist; // For AJAX

			bool bBusy;
			std::vector<std::string> cmd;
			std::string sOldPlayerNames[64];
			std::string sLastJoiner;
			bool bJustJoined;

			unsigned char getPlayerIndexByName(std::string &name);
			void ingameCmd(std::string, std::string, std::string = "");

		public:

			// Vote
			Vote vote;

			// Restricted Weapons
			std::vector<std::string> restrictedWeapons;

			// Constructor
			game(std::string name);
			~game();

			struct_count count;
			Database::struct_settings *settings;
			struct_player pPlayerlist[64];

			bool connect();
			void refresh();

			void sendSimpleCmd(const std::string);
			void sendSimpleCmdWithValue(const std::string sCmd, const std::string sValue);
			bool login();
			void say(std::string sText, std::string sTarget = "all", std::string sPlayer = "");
			void yell(std::string sText, std::string sTarget = "all", std::string sPlayer = "");
			void player_kill(std::string);
			void player_kick(std::string, std::string);
			void player_move(const std::string, const unsigned char, const unsigned char, const bool = false);
			void player_ban(const std::string, const std::string, const std::string, const std::string, const bool = true);
			void player_unban(const std::string, const bool = true);
			void player_white(const std::string);
			void listplayers(std::string);
			void events(bool active=true);
			void logout();
			bool isBusy();
			void updateBanlist();
			void updateWhitelist();
			void addChatMessage(const struct_chat);

			// Get SQL Connection
			inline Database::SQLite *getSQLiteConnection() const
			{
				return (sql);
			}

			// Get Chatlist
			std::list<struct_chat> getChatlist();

			void nma_NotificateAll(std::string sMessage);
			void mail_SendToAll(std::string sMessage);
			void restrictWeapon(std::string sName);
			void unRestrictWeapon(std::string sName);
	};
}
#endif
