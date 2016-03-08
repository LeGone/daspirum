/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

// API-KEY dcc69a0dc835ca7e789c81a216ae862b7e75e2d9cd4216a0
#include "helper.hpp"
#include "dlib/sockets.h"
#include "dlib/sockstreambuf.h"
#include <iostream>

namespace notifymyandroid
{
	bool notificate(std::string sAPIKey, std::string sMessage)
	{
		try
		{
			char sBuffer[256];
			dlib::scoped_ptr<dlib::connection> con(dlib::connect("www.notifymyandroid.com", 80));

			// Replace _ (Spaces) with %20
			unsigned int size = sMessage.length();
			for (unsigned int i=0; i<size; i++)
				if (sMessage[i] == ' ')
					sMessage.replace(i, 1, "%20");

			// The GET-Request
			sMessage = "GET /publicapi/notify?apikey=" + sAPIKey + "&application=daspirum&event=Battlefield-3-Event&description=" + sMessage + "\r\n\r\n";
			if (con->write(sMessage.c_str(), sMessage.size()) != (int)sMessage.size())
				return (false);
			
			// Just to empty the buffer...
			con->read(sBuffer, sizeof(sBuffer), 200);

			if (std::string(sBuffer).find("rror"))
				return (false);

			dlib::close_gracefully(con);
		}
		catch (std::exception& e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}

		return (true);
	}
}
