/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#ifndef HELPER_HPP
#define HELPER_HPP
/*
#define DEBUG
#define DEBUG_FUNCTIONS
#define DEBUG_EVENTS
#define DEBUG_PLAYER
*/

#define DB_MYSQL		0
#define DB_SQLITE		1

#define ADMIN_LEVEL_BADASS 1
#define ADMIN_LEVEL_SUPERBADASS 2

#define LOG_ADMIN		0
#define LOG_ADMIN_KILL	0
#define LOG_ADMIN_KICK	1
#define LOG_ADMIN_SAY	2
#define LOG_ADMIN_YELL	3
#define LOG_ADMIN_MOVE	4
#define LOG_ADMIN_FMOVE	5
#define LOG_ADMIN_BAN	6
#define LOG_CHAT		1
#define LOG_CHAT_US		0
#define LOG_CHAT_RU		1
#define LOG_CHAT_SQUAD	2

#define LOG_COLOR_BLACK		"#000000"
#define LOG_COLOR_WHITE		"#FFFFFF"
#define LOG_COLOR_RED		"#DD0000"
#define LOG_COLOR_GREEN		"#00DD00"
#define LOG_COLOR_BLUE		"#0000DD"
#define LOG_COLOR_ORANGE	"#FFAA00"

#define LOG_LINUX_COLOR_BLACK	"\e[30m"
#define LOG_LINUX_COLOR_WHITE	"\e[37m"
#define LOG_LINUX_COLOR_RED		"\e[31m"
#define LOG_LINUX_COLOR_GREEN	"\e[32m"
#define LOG_LINUX_COLOR_BLUE	"\e[34m"
#define LOG_LINUX_COLOR_ORANGE	"\e[33m"

const unsigned char SQUADNAME[14][8] =
{
	"UNKNOWN",
	"Alpha",
	"Bravo",
	"Charlie",
	"Delta",
	"Echo",
	"Foxtrot",
	"Golf",
	"Hotel",
	"India",
	"Juliet",
	"Kilo",
	"Lima",
	"Mike"
};

const unsigned char SQUADCOLORASHEX[14][8] =
{
	"#ddffdd",
	"#dde1dd",
	"#dde3dd",
	"#dde6dd",
	"#dde8dd",
	"#ddeadd",
	"#ddebdd",
	"#ddecdd",
	"#ddeddd",
	"#ddeedd",
	"#ddefdd",
	"#ddf1dd",
	"#ddf3dd",
	"#ddf5dd"
};

#include <iostream>
#include <string>
#include <algorithm>
#include <ctime>

namespace helper
{
	namespace log
	{
		void init();
		void error(const char *, const char *, long, std::string);
		void message(std::string sMessage, const char *csColor = LOG_COLOR_BLACK);
	}

	namespace security
	{
		inline bool getCleanString_internal(char c)
		{
			return !(isalpha(c) || isdigit(c) || c=='_' || c==' ' || c=='-');
		}

		inline void getCleanString(std::string &stringToBeCleaned)
		{
			stringToBeCleaned.erase(std::remove_if(stringToBeCleaned.begin(), stringToBeCleaned.end(), getCleanString_internal), stringToBeCleaned.end());
		}

		inline bool getCleanWord_internal(char c)
		{
			return !(isalpha(c) || isdigit(c) || c=='_' || c=='-');
		}

		inline void getCleanWord(std::string &stringToBeCleaned)
		{
			stringToBeCleaned.erase(std::remove_if(stringToBeCleaned.begin(), stringToBeCleaned.end(), getCleanWord_internal), stringToBeCleaned.end());
		}
	}

	inline const char *getStringFromBuffer(const char *buffer, const unsigned short cusFrom)
	{
		return (&buffer[cusFrom]);
	}

#pragma warning(disable:4996)
	inline double timeFromNow(unsigned char day, unsigned char month, unsigned short year)
	{
		time_t rawtime,rawtime2;
		struct tm * timeinfo;
		struct tm * timeinfo2;
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		timeinfo->tm_year = year - 1900;
		timeinfo->tm_mon = month - 1;
		timeinfo->tm_mday = day;
		timeinfo->tm_hour = 0;
		timeinfo->tm_min = 0;
		timeinfo->tm_sec = 0;
		rawtime = mktime ( timeinfo );
		time ( &rawtime2 );
		timeinfo2 = localtime ( &rawtime2 );
		timeinfo2->tm_min = 0;
		timeinfo2->tm_hour = 0;
		timeinfo2->tm_sec = 0;
		rawtime2 = mktime( timeinfo2 );

		return (difftime(rawtime2, rawtime) / 86400); // only days
	}
#pragma warning(default:4996)

	unsigned int getIntFromBuffer(const char *, unsigned short &, unsigned short);
	std::string getDateTimeAsString();

	unsigned char levenshtein_distance(const std::string &s1, const std::string &s2);
}
#endif
