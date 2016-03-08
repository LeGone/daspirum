/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#pragma warning(disable:4996)
#include <dlib/string.h>
#include <dlib/md5.h>
#pragma warning(default:4996)
#include "helper.hpp"
#include "SQLite.hpp"
#include <iostream>

namespace Database
{
	SQLite::SQLite()
	{
		database = NULL;
	}
 
	SQLite::~SQLite()
	{
	}

	bool SQLite::open(std::string filename)
	{
		if (sqlite3_open(filename.c_str(), &database) == SQLITE_OK)
			return (true);

		return (false);
	}

	SQLResult SQLite::query(std::string query)
	{
		sqlite3_stmt *statement;
		SQLResult results;

		if (sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK)
		{
			int cols = sqlite3_column_count(statement);
			int result = 0;
			while (true)
			{
				result = sqlite3_step(statement);
			
				if (result == SQLITE_ROW)
				{
					std::vector<std::string> values;
					for(int col = 0; col < cols; col++)
					{
						values.push_back((char*)sqlite3_column_text(statement, col));
					}
					results.push_back(values);
				}
				else
				{
					break;
				}
			}

			sqlite3_finalize(statement);
		}

		std::string error = sqlite3_errmsg(database);
		if (error != "not an error")
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, error + " On Query " + query);
	
		return (results);
	}

	void SQLite::close()
	{
		sqlite3_close(database);
	}

	void SQLite::updateAdminName(const std::string guid, const std::string name)
	{
		try 
		{
			query("UPDATE admin SET n='" + dlib::tolower(name) + "' WHERE eaID='" + guid + "'");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::updateAdminPassword(const std::string guid, const std::string pwd)
	{
		try 
		{
			query("UPDATE admin SET pwd='" + dlib::md5(pwd) + "' WHERE eaID='" + guid + "'");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::updateAdminPayday(const std::string adminID, const std::string date)
	{
		try 
		{
			query("UPDATE admin SET payday='" + date + "' WHERE adminID=" + adminID);
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::updateAdminLevel(const std::string adminID, const std::string level)
	{
		try 
		{
			query("UPDATE admin SET l=" + level + " WHERE adminID=" + adminID);
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	std::string SQLite::getAdminPassword(const std::string adminName)
	{
		try
		{
			SQLResult result = query("SELECT pwd FROM admin WHERE n='" + adminName + "'");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;
				return (row.at(0));
			}
		}
		catch (std::exception &e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return ("");
	}

	std::string SQLite::getChatResponse(const std::string cmd)
	{
		try
		{
			SQLResult result = query("SELECT response FROM chatresponse WHERE command='" + cmd + "'");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;
				return (row.at(0));
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return ("Unknown Command");
	}

	unsigned int SQLite::getAdminLevel(const std::string sToken)
	{
		try
		{
			std::string sQuery = "SELECT l FROM admin WHERE eaID='";
			sQuery += sToken + "'";
			sQuery += " OR n='";
			sQuery += sToken + "'";

			SQLResult result = query(sQuery);
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;
				return (dlib::string_cast<unsigned int>(row.at(0)));
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
			throw e.what();
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
			throw "Unknown Error in getAdminLevel.";
		}

		return (0);
	}

	unsigned int SQLite::getAdminID(const std::string sToken)
	{
		try
		{
			std::string sQuery = "SELECT adminID FROM admin WHERE eaID='";
			sQuery += sToken + '\'';
			sQuery += " OR n='";
			sQuery += sToken + '\'';

			SQLResult result = query(sQuery);
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;
				return (dlib::string_cast<unsigned int>(row.at(0)));
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
			throw e.what();
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
			throw "Unknown Error in getAdminID.";
		}

		return (0);
	}

	std::string SQLite::getAdminName(const unsigned int adminID)
	{
		try
		{
			SQLResult result = query("SELECT n FROM admin WHERE adminID=" + dlib::cast_to_string(adminID));
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;
				return (row.at(0));
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
			throw e.what();
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
			throw "Unknown Error in getAdminName.";
		}

		return ("Removed");
	}

	std::string SQLite::getAdminPayday(const unsigned int adminID)
	{
		try
		{
			SQLResult result = query("SELECT payday FROM admin WHERE adminID=" + dlib::cast_to_string(adminID));
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;

				if (row.at(0) == "NOW")
					return ("");

				return (row.at(0));
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
			throw e.what();
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
			throw "Unknown Error in getAdminName.";
		}

		return ("");
	}

	std::vector<admin> SQLite::adminlist()
	{
		std::vector<admin> vAdminlist;
		try 
		{
			SQLResult result = query("SELECT * FROM admin");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;

				admin a;
				a.uiAdminID = dlib::string_cast<unsigned int>(row.at(0));
				a.sGuID = row.at(1);
				a.sName = dlib::tolower(row.at(2));
				a.ucLevel = dlib::string_cast<unsigned short>(row.at(3));
				a.sNextPaymentDate = row.at(5);
				if (a.sNextPaymentDate == "NOW")
					a.sNextPaymentDate = "";

				vAdminlist.push_back(a);
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (vAdminlist);
	}

	void SQLite::deleteAdmin(const std::string sAdminID)
	{
		try 
		{
			query("DELETE FROM admin WHERE adminID=" + sAdminID);
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::addAdmin(const std::string sEaID, const std::string sName, const unsigned char uiLevel)
	{
		try 
		{
			query("INSERT INTO admin (eaID,n,l) VALUES ('" + sEaID + "','" + dlib::tolower(sName) + "'," + dlib::cast_to_string(uiLevel) + ")");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	std::string SQLite::getMemberString()
	{
		try 
		{
			std::string sList;
			SQLResult result = query("SELECT n FROM admin ORDER BY l DESC");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;
				sList += row.at(0);
				sList += ',';
			}
			return (sList);
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return ("");
	}

	bool SQLite::isBanned(const std::string sToken)
	{
		try
		{
			SQLResult result = query("SELECT banID FROM ban WHERE eaID='" + sToken + "\'");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				return (true);
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (false);
	}

	void SQLite::addBan(const std::string sToken)
	{
		try
		{
			query("INSERT INTO ban (eaID) VALUES ('"+ sToken +"')");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::addBan(const std::string sGuID, const std::string sName, const unsigned int adminID, const std::string sReason)
	{
		try
		{
			query("INSERT INTO ban (eaID,date,n,adminID,reason) VALUES ('"+ sGuID +"',CURRENT_TIMESTAMP,'"+ dlib::tolower(sName) +"'," + dlib::cast_to_string(adminID) + ",'"+ sReason +"')");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::unban(const std::string sBanID)
	{
		try
		{
			query("DELETE FROM ban WHERE banID="+ sBanID);
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	std::vector<ban> SQLite::banlist()
	{
		std::vector<ban> vBanlist;
		try 
		{
			SQLResult result = query("SELECT * FROM ban");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;

				ban b;
				b.uiBanID = dlib::string_cast<unsigned int>(row.at(0));
				b.sGuID = row.at(1);
				b.sName = row.at(2);
				b.sReason = row.at(3);
				b.sAdmin = getAdminName(dlib::string_cast<unsigned int>(row.at(4)));
				b.sDate = row.at(5);
				vBanlist.push_back(b);
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (vBanlist);
	}

	std::vector<std::string> SQLite::whitelist()
	{
		std::vector<std::string> vWhitelist;
		try 
		{
			SQLResult result = query("SELECT name FROM whitelist UNION SELECT n FROM admin");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;
				vWhitelist.push_back(dlib::tolower(row.at(0)));
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (vWhitelist);
	}

	void SQLite::addwhite(const std::string sName)
	{
		try
		{
			query("INSERT INTO whitelist (name) VALUES ('"+ sName +"')");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::unwhite(const std::string sName)
	{
		try
		{
			query("DELETE FROM whitelist WHERE name='"+ sName + "'");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::sendRequest(const std::string sName, const std::string sRequest)
	{
		try 
		{
			query("INSERT INTO request (n,t) VALUES ('"+ sName +"','"+ sRequest +"')");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::log(const std::string sUser, const std::string sText, const unsigned int ccType, const unsigned int ccSubType) 
	{
		std::string sQuery, text = sText;
		try 
		{
			if (ccType == LOG_CHAT)
			{
				helper::security::getCleanString(text);
			}

			query("INSERT INTO log (n,date,txt,type,stype) VALUES ('"+ sUser +"',CURRENT_TIMESTAMP,'"+ text +"','"+ dlib::cast_to_string(ccType) +"','"+ dlib::cast_to_string(ccSubType) +"')");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	std::vector<struct_log> SQLite::logList(unsigned short usLimit)
	{
		std::vector<struct_log> vLogList;
		std::string sQuery;
		try 
		{
			SQLResult result = query("SELECT logID,date,n,txt,type,stype FROM log ORDER BY logID DESC LIMIT " + dlib::cast_to_string(usLimit));
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;

				struct_log newLog;
				newLog.uiID = dlib::string_cast<unsigned int>(row.at(0));
				newLog.sTime = row.at(1);
				newLog.sName = row.at(2);
				newLog.sMessage = row.at(3);
				newLog.ucType = dlib::string_cast<unsigned short>(row.at(4));
				newLog.ucSubType = dlib::string_cast<unsigned short>(row.at(5));

				vLogList.push_back(newLog);
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (vLogList);
	}
	
	void SQLite::updateAdvMessages()
	{
		helper::log::message("Updating Adv. Messages");
		try 
		{
			vAdvMessages.clear();

			SQLResult result = query("SELECT m,y FROM messages");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;

				message msg;
				msg.sMsg = row.at(0);
				msg.bYell = dlib::string_cast<bool>(row.at(1));
				vAdvMessages.push_back(msg);
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::deleteAdvMessage(const std::string sAdvID)
	{
		try 
		{
			query("DELETE FROM messages WHERE advID=" + sAdvID);
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::addAdvMessage(const std::string sText, const std::string sYell)
	{
		try 
		{
			query("INSERT INTO messages (m,y) VALUES ('" + sText + "'," + sYell + ")");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	std::vector<message> SQLite::advlist()
	{
		std::vector<message> vMessagelist;
		try 
		{
			SQLResult result = query("SELECT * FROM messages");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;

				message msg;
				msg.uiMsgID = dlib::string_cast<unsigned int>(row.at(0));
				msg.sMsg = row.at(1);
				msg.bYell = dlib::string_cast<bool>(row.at(2));
				vMessagelist.push_back(msg);
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (vMessagelist);
	}

	message SQLite::getNextMessage()
	{
		unsigned char ucElementCount = vAdvMessages.size();
		if (ucElementCount)
		{
			++ucAdvMessageIndex;
			if (ucAdvMessageIndex < ucElementCount)
			{
				return (vAdvMessages[ucAdvMessageIndex]);
			}
			else
			{
				ucAdvMessageIndex = 0;
				return (vAdvMessages[0]);
			}
		}

		message msg;
		msg.sMsg = "www.legone.name";
		msg.bYell = true;
		return (msg);
	}

	std::vector<std::string> SQLite::nmalist()
	{
		std::vector<std::string> lNma;
		try 
		{
			SQLResult result = query("SELECT apikey FROM nma");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;

				lNma.push_back(row.at(0));
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (lNma);
	}

	void SQLite::deleteNmaKey(const std::string sKey)
	{
		try 
		{
			query("DELETE FROM nma WHERE apikey='" + sKey + "'");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::addNmaKey(const std::string sKey)
	{
		try 
		{
			query("INSERT INTO mail (apikey) VALUES ('" + sKey + "')");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}
	
	std::vector<std::string> SQLite::mailReceiverList()
	{
		std::vector<std::string> lMailReceiver;
		try 
		{
			SQLResult result = query("SELECT addy FROM mail");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;
				lMailReceiver.push_back(row.at(0));
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (lMailReceiver);
	}

	void SQLite::deleteMailReceiver(const std::string sKey)
	{
		try 
		{
			query("DELETE FROM mail WHERE addy='" + sKey + "'");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::addMailReceiver(const std::string sKey)
	{
		try 
		{
			query("INSERT INTO mail (addy) VALUES ('" + sKey + "')");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	std::vector<struct_chatresponse> SQLite::chatResponseList()
	{
		std::vector<struct_chatresponse> list;
		try 
		{
			SQLResult result = query("SELECT chatresponseID,command,response FROM chatresponse");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;

				struct_chatresponse chatResponse;
				chatResponse.uiChatResponseID = dlib::string_cast<unsigned int>(row.at(0));
				chatResponse.sCommand = row.at(1);
				chatResponse.sResponse = row.at(2);
				list.push_back(chatResponse);
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (list);
	}

	std::string SQLite::chatResponseListAsString()
	{
		std::string list;
		try 
		{
			SQLResult result = query("SELECT command FROM chatresponse");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;

				list += ',';
				list += row.at(0);
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (list);
	}

	void SQLite::deleteChatResponse(const unsigned int chatresponseID)
	{
		try 
		{
			query("DELETE FROM chatresponse WHERE chatresponseID=" + dlib::cast_to_string(chatresponseID));
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::addChatResponse(const std::string command, const std::string response)
	{
		try 
		{
			query("INSERT INTO chatresponse (command,response) VALUES ('" + command + "','" + response + "')");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::updateServerSetting(const std::string propertyName, const std::string value)
	{
		try 
		{
			query("UPDATE settings SET " + propertyName + "='" + value + "'"); 
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}


	std::vector<std::string> SQLite::restrictedWeaponsList()
	{
		std::vector<std::string> lRestrictedWeapons;
		try 
		{
			SQLResult result = query("SELECT name FROM restrictedweapons");
			for (SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;
				lRestrictedWeapons.push_back(row.at(0));
			}
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (lRestrictedWeapons);
	}

	void SQLite::deleteRestrictedWeapon(const std::string sWeapon)
	{
		try 
		{
			query("DELETE FROM restrictedweapons WHERE name='" + sWeapon + "'");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	void SQLite::addRestrictedWeapon(const std::string sWeapon)
	{
		try 
		{
			query("INSERT INTO restrictedweapons (name) VALUES ('" + sWeapon + "')");
		}
		catch (std::exception &e) 
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}
	}

	struct_settings *SQLite::getServerSettings()
	{
		try 
		{
			SQLResult result = query("SELECT * FROM settings");
			for(SQLResult::iterator it = result.begin(); it < result.end(); ++it)
			{
				std::vector<std::string> row = *it;
				struct_settings *settings = new struct_settings();
				settings->sDefaultpunishedmsg = row.at(0);
				settings->sWelcomeMsg = row.at(1);
				settings->sBF3_Server_Host = row.at(2);
				settings->usBF3_Server_Port = dlib::string_cast<unsigned short>(row.at(3));
				settings->sBF3_Server_Pwd = row.at(4);
				settings->sOwner = row.at(5);
				settings->bShowHeadshots = dlib::string_cast<bool>(row.at(6));
				settings->bWelcomeMsg = dlib::string_cast<bool>(row.at(7));
				settings->bAdminJoinMsg = dlib::string_cast<bool>(row.at(8));
				settings->bAutoTeamBalance = dlib::string_cast<bool>(row.at(9));
				settings->ucAdvInterval = dlib::string_cast<unsigned short>(row.at(10));
				settings->ucAdvTimer = 0;
				settings->bVoteKick = dlib::string_cast<bool>(row.at(11));
				settings->bVoteMap = dlib::string_cast<bool>(row.at(12));
				settings->cMinPlayerCountForRestrictedWeaponPunish = dlib::string_cast<unsigned short>(row.at(13));

				return (settings);
			}
		}
		catch (std::exception& e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
		}

		return (NULL);
	}
}