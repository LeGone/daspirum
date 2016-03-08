/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#include "Config.hpp"
#include "helper.hpp"

// Dlib
#include <dlib/config_reader.h>

namespace Config
{
	namespace Webserver
	{
		unsigned short port = 80;
		std::string rsp_password;
	}

	namespace Mail
	{
		std::string smtp_address;
		unsigned short port = 25;
	}

	namespace GeoIP
	{
		std::string server;
		std::string path;
	}

	namespace LGN
	{
		std::string server;
	}

	bool Load()
	{
		try
		{
			dlib::config_reader config("config.cfg");
			
			// Webserver
			Webserver::port = dlib::string_cast<unsigned short>(config.block("webserver")["port"]);
			Webserver::rsp_password = config.block("webserver")["rsp_password"];
			
			// Mail
			Mail::smtp_address = config.block("mail")["smtp_address"];
			Mail::port = dlib::string_cast<unsigned short>(config.block("mail")["smtp_port"]);
			
			// GeoIP
			GeoIP::server = config.block("geoip")["server"];
			GeoIP::path = config.block("geoip")["path"];

			// LGN - LeGone Global Network
			LGN::server = config.block("lgn")["server"];
		}
		catch (std::exception &e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Did you just modified the configfile?");
			return (false);
		}

		return (true);
	}
}