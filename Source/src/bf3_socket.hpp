/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#ifndef BF3_SOCKET_HPP
#define BF3_SOCKET_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <dlib/sockets.h>
#include <dlib/sockstreambuf.h>

struct struct_response
{
	unsigned char ucBuffer[5000];
	unsigned short usPointer;
	unsigned int uiSequence;
	unsigned int uiPacketSize;
	unsigned int uiWordCount;
	bool bOriginatedOnServer;
	bool bIsResponse;
	std::vector<std::string> words;
};

class bf3_socket
{
	private:

		bool bConnected;
		char sBuffer[16384];
		unsigned int uiSequence;

		std::string make_packet(const std::vector<std::string>&);

	protected:

		dlib::scoped_ptr<dlib::connection> socket;
		struct_response response;

		bf3_socket();
		~bf3_socket();

		void _connect(const char* sAddress, unsigned int uiPort);
		void decodeWords(const char *cucBuffer);
		void sendCMD(std::vector<std::string> cmd);
		void _refresh();

		// Inlines
		inline unsigned int getPacketSize() const
		{
			return (sBuffer[4] & 0xff | ((sBuffer[5] & 0xff) << 8) | ((sBuffer[6] & 0xff) << 16) | ((sBuffer[7] & 0xff) << 24));
		}

		inline unsigned int getUnsignedInt32(unsigned short offset) const
		{
			return ((sBuffer[offset]) | (sBuffer[offset + 1] << 8) | (sBuffer[offset + 2] << 16) | (sBuffer[offset + 3] << 24));
		}
};
#endif
