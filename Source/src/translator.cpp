/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#include "helper.hpp"
#include "dlib/sockets.h"
#include "dlib/sockstreambuf.h"
#include <iostream>

namespace translator
{
	std::string translate(std::string textToTranslate, std::string language)
	{
		try
		{
			char sBuffer[256];
			dlib::scoped_ptr<dlib::connection> con(dlib::connect("bypass.xlnt.cc", 80));

			// Replace _ (Spaces) with %20
			unsigned int size = textToTranslate.length();
			for (unsigned int i=0; i<size; i++)
				if (textToTranslate[i] == ' ')
					textToTranslate.replace(i, 1, "%20");

			std::string request;
			request = "GET /translator/index.php?q=" + textToTranslate + "&l=" + language + " HTTP/1.1\r\n";
			request += "Host: bypass.xlnt.cc\r\n";
			request += "\r\n";

			// The GET-Request
			if (con->write(request.c_str(), request.size()) != (int)request.size())
				return (false);

			con->read(sBuffer, sizeof(sBuffer), 10000);

			request = sBuffer;

			request = request.erase(0, request.find("\r\n\r\n")+4);
			request = request.erase(0, request.find("\r\n")+2);
			request = request.substr(0, request.find("\r\n"));

			helper::log::message(sBuffer);

			dlib::close_gracefully(con);

			return (request);
		}
		catch (std::exception& e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}

		return ("");
	}

	/*
	std::string translate(std::string textToTranslate, std::string translateFromLanguage, std::string translateToLanguage)
	{
		try
		{
			char sBuffer[256];
			dlib::scoped_ptr<dlib::connection> con(dlib::connect("api.apertium.org", 80));

			// Replace _ (Spaces) with %20
			boost::replace_all(textToTranslate, " ", "%20");
			std::string request;
			request = "GET /json/translate?q=" + textToTranslate + "&langpair=" + translateFromLanguage + "%7C" + translateToLanguage + " HTTP/1.1\r\n";
			request += "Host: api.apertium.org\r\n";
			request += "\r\n";

			// The GET-Request
			if (con->write(request.c_str(), request.size()) != (int)request.size())
				return (false);

			con->read(sBuffer, sizeof(sBuffer), 10000);

			helper::log::message(sBuffer);

			request = sBuffer;
			int pos = request.find("translatedText");
			if (pos != -1)
			{
				pos += 17;
				request = request.erase(0, pos);
				request = request.substr(0, request.find("},")-1);
			}

			dlib::close_gracefully(con);

			return (request);
		}
		catch (std::exception& e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}

		return ("");
	}
	*/
}

/*

0x02d3e114 "HTTP/1.1 200 OK
Date: Fri, 14 Jun 2013 16:28:10 GMT
Server: Apache/2.2.22 (Ubuntu)
X-Powered-By: PHP/5.3.10-1ubuntu3.6
Vary: Accept-Encoding
Transfer-Encoding: chunked
Content-Type: text/html

11
Dies ist ein Test
0

ллллллллллллллллллллллллллллллл▌З╙ТчстЛс|"

"HTTP/1.1 200 OK
Date: Fri, 14 Jun 2013 12:57:12 GMT
Server: Apache/2.2.16 (Debian)
Content-Type: application/json
Transfer-Encoding: chunked

64
{"responseData":{"translatedText":null},"responseDetails":"Not supported pair","responseStatus":451}
0
ллллЦ≥УyХАiДПiЮ/"

0x02cddf60 "HTTP/1.1 200 OK
Date: Fri, 14 Jun 2013 12:26:09 GMT
Server: Apache/2.2.16 (Debian)
Content-Type: application/json
Transfer-Encoding: chunked

5d
{"responseData":{"translatedText":" hola Mundo"},"responseDetails":null,"responseStatus":200}
0

лллллллл"%Б┬Юэм─КмЙ"
*/