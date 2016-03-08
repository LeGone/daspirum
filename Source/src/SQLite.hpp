/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#ifndef __SQLITE_HPP__
#define __SQLITE_HPP__

#include <string>
#include <vector>
#include <sqlite3.h>

// Space between > > needed!
typedef std::vector<std::vector<std::string> > SQLResult;

namespace Database
{
	struct message
	{
		unsigned int uiMsgID;
		std::string sMsg;
		bool bYell;
	};

	struct ban
	{
		unsigned int uiBanID;
		std::string sGuID;
		std::string sName;
		std::string sReason;
		std::string sAdmin;
		std::string sDate;
	};

	struct admin
	{
		unsigned int uiAdminID;
		std::string sGuID;
		std::string sName;
		unsigned char ucLevel;
		std::string sNextPaymentDate;
	};

	struct struct_chatresponse
	{
		unsigned int uiChatResponseID;
		std::string sCommand;
		std::string sResponse;
	};

	struct struct_log
	{
		unsigned int uiID;
		std::string sTime;
		std::string sName;
		std::string sMessage;
		unsigned char ucType;
		unsigned char ucSubType;
	};

	struct struct_settings
	{
		std::string		sBF3_Server_Host;
		unsigned short	usBF3_Server_Port;
		std::string		sBF3_Server_Pwd;	
		std::string		sOwner;
		unsigned short	usMySQLPingTimer;
		unsigned short	usMySQLPingInterval;
		bool			bShowHeadshots;
		bool			bWelcomeMsg;
		std::string		sWelcomeMsg;
		std::string		sDefaultpunishedmsg;
		bool			bAdminJoinMsg;
		bool			bAutoTeamBalance;
		unsigned char	ucAdvInterval;
		unsigned char	ucAdvTimer;
		bool			bVoteKick;
		bool			bVoteMap;
		unsigned char	cMinPlayerCountForRestrictedWeaponPunish;
	};

	class SQLite
	{
	private:
		sqlite3 *database;

		unsigned char ucAdvMessageIndex;
		std::vector<message> vAdvMessages;

	public:
		SQLite();
		~SQLite();
	
		bool open(std::string filename);
		SQLResult query(std::string query);
		void close();

		void updateAdminName(const std::string guid, const std::string name);
		void updateAdminPassword(const std::string guid, const std::string pwd);
		void updateAdminPayday(const std::string adminID, const std::string date);
		void updateAdminLevel(const std::string adminID, const std::string level);
		std::string getChatResponse(const std::string cmd);
		std::string getAdminPassword(const std::string adminName);
		unsigned int getAdminLevel(const std::string);
		unsigned int getAdminID(const std::string sToken);
		std::string getAdminName(const unsigned int adminID);
		std::string getAdminPayday(const unsigned int adminID);
		std::vector<admin> adminlist();
		void deleteAdmin(const std::string);
		void addAdmin(const std::string, const std::string, const unsigned char);
		std::string getMemberString();
		bool isBanned(const std::string);
		void addBan(const std::string);
		void addBan(const std::string, const std::string, const unsigned int, const std::string);
		void unban(const std::string);
		std::vector<ban> banlist();
		std::vector<std::string> whitelist();
		void addwhite(const std::string sName);
		void unwhite(const std::string sName);
		void sendRequest(const std::string, const std::string);
		void log(const std::string sUser, const std::string sText, const unsigned int ccType, const unsigned int ccSubType = 0);
		std::vector<struct_log> logList(unsigned short usLimit = 100);

		void ping();

		void updateAdvMessages();
		void deleteAdvMessage(const std::string);
		void addAdvMessage(const std::string, const std::string);
		std::vector<message> advlist();
		message getNextMessage();

		std::vector<std::string> nmalist();
		void deleteNmaKey(const std::string sKey);
		void addNmaKey(const std::string sKey);

		std::vector<std::string> mailReceiverList();
		void deleteMailReceiver(const std::string sKey);
		void addMailReceiver(const std::string sKey);

		std::vector<struct_chatresponse> chatResponseList();
		std::string chatResponseListAsString();
		void deleteChatResponse(const unsigned int chatresponseID);
		void addChatResponse(const std::string command, const std::string response);

		// Restricted Weapons
		std::vector<std::string> restrictedWeaponsList();
		void deleteRestrictedWeapon(const std::string sWeapon);
		void addRestrictedWeapon(const std::string sWeapon);

		void updateServerSetting(const std::string propertyName, const std::string value);
		struct_settings *getServerSettings();

		// Inlines
		inline sqlite3 *getDatabase() const
		{
			return (database);
		}
	};
}

#endif