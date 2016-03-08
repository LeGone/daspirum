/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#include "helper.hpp"
#include <algorithm> // std::min
#include <fstream>
#include <iostream>
#include <iomanip>
#include <dlib/string.h>
#include <ctime>
#if defined _WIN32 || defined _WIN64
	#include <windows.h>
#endif

std::string gTime;

namespace helper
{
	namespace log
	{
		void init()
		{
			time_t rawtime;
			time(&rawtime);

#if defined _WIN32 || defined _WIN64
			struct tm timeinfo;
			localtime_s(&timeinfo, &rawtime);
			gTime = "www/log/" + dlib::cast_to_string(timeinfo.tm_year + 1900) + "-" + dlib::cast_to_string(timeinfo.tm_mon) + "-" + 
				dlib::cast_to_string(timeinfo.tm_mday) + "-" + dlib::cast_to_string(timeinfo.tm_hour) + "-" + 
				dlib::cast_to_string(timeinfo.tm_min) + ".html";
#else
			struct tm *timeinfo;
			timeinfo = localtime(&rawtime);
			gTime = "www/log/" + dlib::cast_to_string(timeinfo->tm_year + 1900) + "-" + dlib::cast_to_string(timeinfo->tm_mon) + "-" + 
				dlib::cast_to_string(timeinfo->tm_mday) + "-" + dlib::cast_to_string(timeinfo->tm_hour) + "-" + 
				dlib::cast_to_string(timeinfo->tm_min) + ".html";
#endif
		}

		void error(const char *file, const char *function, long line, std::string sError)
		{
			std::ofstream logfile;
			std::string sOut(file);
			
#if defined _WIN32 || defined _WIN64
			logfile.open(gTime, std::ios::app);
			sOut.erase(0, sOut.rfind('\\')+1);
#else
			logfile.open(gTime.c_str(), std::ios::app);
			sOut.erase(0, sOut.rfind('/')+1);
#endif
			sOut += "::";
			sOut += function;
			sOut += " Line ";
			sOut += dlib::cast_to_string(line);
			sOut += " -> ";
			sOut += sError;
			logfile << "<font color=\"" << LOG_COLOR_RED << "\">" << sOut << "</font><br/>";
			logfile.close();

#if defined _WIN32 || defined _WIN64
			std::cout << sOut << std::endl;
#else
			std::cout << LOG_LINUX_COLOR_RED << sOut << LOG_LINUX_COLOR_WHITE << std::endl;
#endif
		}

		void message(std::string sMessage, const char *csColor)
		{
			std::ofstream logfile;
#if defined _WIN32 || defined _WIN64
			logfile.open(gTime, std::ios::app);
#else
			logfile.open(gTime.c_str(), std::ios::app);
#endif
			logfile << "<font color=\"" << csColor << "\">" << sMessage << "</font><br/>";
			logfile.close();

#if defined _WIN32 || defined _WIN64
			std::cout << sMessage << std::endl;
#else
			std::string sColor(csColor);
			if (sColor == LOG_COLOR_ORANGE)
				std::cout << LOG_LINUX_COLOR_ORANGE;
			else if (sColor == LOG_COLOR_GREEN)
				std::cout << LOG_LINUX_COLOR_GREEN;
			else if (sColor == LOG_COLOR_BLUE)
				std::cout << LOG_LINUX_COLOR_BLUE;
			else if (sColor == LOG_COLOR_RED)
				std::cout << LOG_LINUX_COLOR_RED;
			else if (sColor == LOG_COLOR_WHITE)
				std::cout << LOG_LINUX_COLOR_WHITE;
			
			std::cout << sMessage << LOG_LINUX_COLOR_WHITE << std::endl;
#endif
		}
	}

	unsigned int getIntFromBuffer(const char *buffer, unsigned short &cusFrom, unsigned short cucLength)
	{
		unsigned short cusOldFrom = cusFrom;
		cusFrom += cucLength;
		return (atoi(&buffer[cusOldFrom]));
	}

	std::string getDateTimeAsString()
	{
			time_t rawtime;
			time(&rawtime);

#if defined _WIN32 || defined _WIN64
			struct tm timeinfo;
			localtime_s(&timeinfo, &rawtime);
			return (std::string(dlib::cast_to_string(timeinfo.tm_year + 1900) + "-" + dlib::cast_to_string(timeinfo.tm_mon) + "-" + 
				dlib::cast_to_string(timeinfo.tm_mday) + "-" + dlib::cast_to_string(timeinfo.tm_hour) + "-" + 
				dlib::cast_to_string(timeinfo.tm_min)));
#else
			struct tm *timeinfo;
			timeinfo = localtime(&rawtime);
			return (std::string(dlib::cast_to_string(timeinfo->tm_year + 1900) + "-" + dlib::cast_to_string(timeinfo->tm_mon) + "-" + 
				dlib::cast_to_string(timeinfo->tm_mday) + "-" + dlib::cast_to_string(timeinfo->tm_hour) + "-" + 
				dlib::cast_to_string(timeinfo->tm_min)));
#endif
	}

	// http://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C.2B.2B
	unsigned char levenshtein_distance(const std::string &s1, const std::string &s2)
	{
		const size_t len1 = s1.size(), len2 = s2.size();
		std::vector<unsigned char> col(len2+1), prevCol(len2+1);
 
		for (unsigned int i = 0; i < prevCol.size(); i++)
			prevCol[i] = i;

		for (unsigned char i = 0; i < len1; i++)
		{
			col[0] = i+1;
			for (unsigned char j = 0; j < len2; j++)
				col[j+1] = std::min(std::min(1 + col[j], 1 + prevCol[1 + j]), prevCol[j] + (s1[i]==s2[j]? 0 : 1));

			col.swap(prevCol);
		}
		return (prevCol[len2]);
	}
}
