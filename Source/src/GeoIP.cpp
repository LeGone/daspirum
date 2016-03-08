/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#include "GeoIP.hpp"
#include "Config.hpp"

// Dlib
#include <dlib/iosockstream.h>

#include <iostream>

namespace GeoIP
{
	std::string GetCountryCodeUsingURL(std::string &ip)
	{
		try
		{
			dlib::iosockstream stream(Config::GeoIP::server);

			stream << "GET " << Config::GeoIP::path << "?ip=" << ip << " HTTP/1.0\r\nHost: " << Config::GeoIP::server << "\r\n\r\n";

			std::string completeFileAsString;
			while (stream.peek() != EOF)
			{
				completeFileAsString += (unsigned char)stream.get();
			}
		
			size_t posOfHeaderEnd = completeFileAsString.find("c:->");
			if (posOfHeaderEnd != std::string::npos)
			{
				// Erase the \r\n\r\n chars
				posOfHeaderEnd += 4;

				completeFileAsString = &completeFileAsString[posOfHeaderEnd];

				return (completeFileAsString);
			}
		}
		catch (...)
		{
			// Just ignore the exception
		}

		return ("");
	}
}