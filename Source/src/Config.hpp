/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#include <string>

namespace Config
{
	namespace Webserver
	{
		extern unsigned short port;
		extern std::string rsp_password;
	}

	namespace Mail
	{
		extern std::string smtp_address;
		extern unsigned short port;
	}

	namespace GeoIP
	{
		extern std::string server;
		extern std::string path;
	}

	namespace LGN
	{
		extern std::string server;
	}

	bool Load();
}