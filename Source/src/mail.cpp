/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#include <string>
#include <dlib/config_reader.h>
#include <dlib/string.h>
#include "dlib/sockets.h"
#include "dlib/sockstreambuf.h"
#include "helper.hpp"
#include <ctime>
#if defined _WIN32 || defined _WIN64
	#include <windows.h>
#endif

// No STARTTLS, yet
namespace mail
{
	std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);

	bool sendMail(std::string to, std::string subject, std::string msg)
	{
		try
		{
			dlib::config_reader config("config.cfg");

			char sBuffer[256];
			std::string tmpStr, smtp_address = config.block("mail")["smtp_address"];

			// Connect
			dlib::scoped_ptr<dlib::connection> con(dlib::connect(smtp_address, dlib::string_cast<unsigned short>(config.block("mail")["smtp_port"])));

			// Read welcome-msg
			con->read(sBuffer, sizeof(sBuffer), 1000);
			helper::log::message(std::string("Mail-S: ") + sBuffer);

			// EHLO to server
			tmpStr = "EHLO xlnt.cc\r\n";
			helper::log::message("Mail-C: " + tmpStr);
			if (con->write(tmpStr.data(), tmpStr.size()) != (int)tmpStr.size())
				return (false);

			con->read(sBuffer, sizeof(sBuffer), 1000);
			helper::log::message(std::string("Mail-S: ") + sBuffer);

			/*
			// AUTH PLAIN
			tmpStr = "AUTH PLAIN\r\n";
			helper::log::message(std::string("Mail-C: " + tmpStr));
			if (con->write(tmpStr.c_str(), tmpStr.size()) != (int)tmpStr.size())
				return (false);

			con->read(sBuffer, sizeof(sBuffer), 1000);
			helper::log::message(std::string("Mail-S: ") + sBuffer);


			tmpStr = "legone@xlnt.cc";
			tmpStr = base64_encode(reinterpret_cast<const unsigned char*>(tmpStr.c_str()), tmpStr.length()) + "\r\n";
			helper::log::message(std::string("Mail-C: " + tmpStr));
			if (con->write(tmpStr.c_str(), tmpStr.size()) != (int)tmpStr.size())
				return (false);
				 
			con->read(sBuffer, sizeof(sBuffer), 1000);
			helper::log::message(std::string("Mail-S: ") + sBuffer);

			tmpStr = "asdf";
			tmpStr = base64_encode(reinterpret_cast<const unsigned char*>(tmpStr.c_str()), tmpStr.length()) + "\r\n";
			helper::log::message(std::string("Mail-C: " + tmpStr));
			if (con->write(tmpStr.c_str(), tmpStr.size()) != (int)tmpStr.size())
				return (false);

			con->read(sBuffer, sizeof(sBuffer), 1000);
			helper::log::message(std::string("Mail-S: ") + sBuffer);
			*/

			tmpStr = "MAIL FROM:<daspirum@" + smtp_address + ">\r\n";
			helper::log::message("Mail-C: " + tmpStr);
			if (con->write(tmpStr.data(), tmpStr.size()) != (int)tmpStr.size())
				return (false);
			
			con->read(sBuffer, sizeof(sBuffer), 200);
			helper::log::message(std::string("Mail: ") + sBuffer);
			
			tmpStr = "RCPT TO:<" + to + ">\r\n";
			if (con->write(tmpStr.data(), tmpStr.size()) != (int)tmpStr.size())
				return (false);
			
			con->read(sBuffer, sizeof(sBuffer), 200);
			helper::log::message(std::string("Mail: ") + sBuffer);

			
			tmpStr = "DATA\r\n";
			if (con->write(tmpStr.data(), tmpStr.size()) != (int)tmpStr.size())
				return (false);
			
			con->read(sBuffer, sizeof(sBuffer), 200);
			helper::log::message(std::string("Mail: ") + sBuffer);
			
			// Send msg
			tmpStr = "From: daspirum@" + smtp_address  + "\r\n";
			tmpStr+= "To: <" + to + ">\r\n";
			tmpStr+= "Subject: " + subject + "\r\n";
			//tmpStr+= "Date: Thu, 26 Oct 2013 13:10:50 +0200\r\n\r\n";
			tmpStr+= "\r\n";

			tmpStr+= msg + "\r\n";
			tmpStr+= ".\r\n";

			if (con->write(tmpStr.data(), tmpStr.size()) != (int)tmpStr.size())
				return (false);
			
			con->read(sBuffer, sizeof(sBuffer), 200);
			helper::log::message(std::string("Mail: ") + sBuffer);
			
			// QUIT
			tmpStr = "QUIT";
			if (con->write(tmpStr.data(), tmpStr.size()) != (int)tmpStr.size())
				return (false);
			
			con->read(sBuffer, sizeof(sBuffer), 200);
			helper::log::message(std::string("Mail: ") + sBuffer);

			dlib::close_gracefully(con);
		}
		catch (std::exception& e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
		catch (...)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown error!");
		}

		return (true);
	}
}