/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

// LGN - LeGone Global Network
// Version 1
#include <iostream>
#include "helper.hpp"
#include "config.hpp"

// Dlib
#include <dlib/iosockstream.h>
#include <dlib/string.h>

namespace LGN
{
	// -1 on error
	int GetVersion()
	{
		try
		{
			dlib::iosockstream stream(Config::LGN::server);

			stream << "get|daspirum|version\r\n";

			std::string version;
			// Here we print each character we get back one at a time. 
			while (stream.peek() != EOF)
			{
				version += (char)stream.get();
			}

			return (dlib::string_cast<int>(version));
		}
		catch (std::exception& e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
			return (-1);
		}

		return (1);
	}
}