/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#include "bf3_socket.hpp"
#include "helper.hpp"

#define MAXPACKETSIZE			16384
#define SEQUENCEMASK			0x3fffffff
#define ORIGINATEDONSERVERFLAG	0x80000000U
#define RESPONSEFLAG			0x40000000

bf3_socket::bf3_socket()
{
}

bf3_socket::~bf3_socket()
{
	if (socket)
	{
		socket->shutdown();
		socket.reset();
	}
}

void bf3_socket::decodeWords(const char *cucBuffer)
{
#ifdef DEBUG_FUNCTIONS
	helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
#endif
	unsigned short usPointer = 0;

#ifdef DEBUG
	std::cout << "============== DECODEWORDS ==============" << std::endl;
	std::cout << "usPointer=" << static_cast<int>(usPointer) << std::endl;
	std::cout << "cucWords=" << static_cast<int>(response.uiWordCount) << std::endl;
	std::cout << "cusPacketSize=" << static_cast<int>(response.uiPacketSize) << std::endl;
#endif
	do
	{
		unsigned char ucLength = cucBuffer[++usPointer]; 
		usPointer += 4;
		response.words.push_back(&cucBuffer[usPointer]);
		usPointer += ucLength;
	} while (usPointer < response.uiPacketSize);
#ifdef DEBUG
	std::cout << "============ /DECODEWORDS ==============" << std::endl;
#endif
#ifdef DEBUG_FUNCTIONS
	helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
#endif
}

void bf3_socket::_connect(const char* sAddress, unsigned int uiPort)
{
#ifdef DEBUG_FUNCTIONS
	helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
#endif

	socket.reset();

	dlib::create_connection(socket, uiPort, sAddress);

	uiSequence = 0;
#ifdef DEBUG_FUNCTIONS
	helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
#endif
}

// THX to JogDiveZero
std::string bf3_socket::make_packet(const std::vector<std::string>& words) 
{ 
#ifdef DEBUG_FUNCTIONS
	helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
#endif
	// 1. Schritt: Calculating packet length 
	unsigned int TotalSize = 4;    // 4 Byte für Paketheader 
	for( std::vector<std::string>::const_iterator it = words.begin(); it != words.end(); ++it ) 
	{ 
		TotalSize    += 4;                // 4 Byte for word length 
		TotalSize    += it->size(); // count bytes in word
		TotalSize    += 1;                // 1 Byte for terminating 0 
	}
	TotalSize += 8; // 4 Byte count the words + 4 Byte Total size of the whole packet
 
	std::ostringstream oss; 
 
	// Write Sequence ID
	// But first inkrement it
	++uiSequence;
	oss << static_cast<char>(uiSequence & 0xff) 
			<< static_cast<char>((uiSequence >> 8) & 0xff) 
			<< static_cast<char>((uiSequence >> 16) & 0xff) 
			<< static_cast<char>((uiSequence >> 24) & 0xff); 
 
	//Write total packet size
	oss << static_cast<char>(TotalSize & 0xff) 
			<< static_cast<char>((TotalSize >> 8) & 0xff) 
			<< static_cast<char>((TotalSize >> 16) & 0xff) 
			<< static_cast<char>((TotalSize >> 24) & 0xff); 
 
	// Write word count
	oss << static_cast<char>(words.size() & 0xff) 
			<< static_cast<char>((words.size() >> 8) & 0xff) 
			<< static_cast<char>((words.size() >> 16) & 0xff) 
			<< static_cast<char>((words.size() >> 24) & 0xff); 
 
	// Write words 
	for(std::vector<std::string>::const_iterator it = words.begin(); it != words.end(); ++it) 
	{
		// Summary of chars in the word 
		oss << static_cast<char>(it->size() & 0xff) 
				<< static_cast<char>((it->size() >> 8) & 0xff) 
				<< static_cast<char>((it->size() >> 16) & 0xff) 
				<< static_cast<char>((it->size() >> 24) & 0xff); 
 
		// Write word itself 
		oss << *it; 
 
		// Write terminating 0 
		oss << static_cast<char>( 0 );
	} 
#ifdef DEBUG_FUNCTIONS
	helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
#endif
	return oss.str();
}

void bf3_socket::sendCMD(std::vector<std::string> cmd)
{
#ifdef DEBUG_FUNCTIONS
	helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
#endif
	std::string temp = make_packet(cmd);
#ifdef DEBUG
	// Send an initial buffer
	for(std::vector<std::string>::const_iterator it = cmd.begin(); it != cmd.end(); ++it) 
		helper::log::message(*it);
#endif

	if (socket->write(temp.c_str(), temp.size()) != (int)temp.size())
		throw std::string("Error while sending command!");

#ifdef DEBUG_FUNCTIONS
	helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
#endif
}

void bf3_socket::_refresh()
{
#ifdef DEBUG_FUNCTIONS
	helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
#endif
	unsigned short usBytesReceived = 0;

	// Clear the last received words
	response.words.clear();

	// Get only the Header
	usBytesReceived = socket->read(sBuffer, 11, 1000);
	if (usBytesReceived == 11)
	{
		// Sequence
		unsigned short rawSeq = getUnsignedInt32(0);
		response.uiSequence = rawSeq & SEQUENCEMASK;
		response.bOriginatedOnServer = ((rawSeq & ORIGINATEDONSERVERFLAG) ? true : false);
		response.bIsResponse = ((rawSeq & RESPONSEFLAG) ? true : false);

		response.uiPacketSize = getPacketSize();
		response.uiWordCount = sBuffer[8];

		//std::cout << "===================================================" << std::endl << "Sequence=" << response.uiSequence << " PacketSize=" << response.uiPacketSize << " Wordcount=" << response.uiWordCount << std::endl;
		if (response.uiPacketSize > MAXPACKETSIZE)
		{
			for (int i=0; i<11; i++)
				std::cout << '(' << std::hex << (int)sBuffer[i] << ')' << std::dec;
			std::cout << std::endl;
			helper::log::message("Incoming Packetsize to huge!");

			// Empty the buffer
			socket->read(sBuffer, sizeof(sBuffer), 100);
			return;
		}
		
#if defined _WIN32 || defined _WIN64
		Sleep(10);
#else
		usleep(10000);
#endif
		
		response.uiPacketSize -= 11;
		usBytesReceived = socket->read(sBuffer, response.uiPacketSize, 1000);
		if (usBytesReceived == response.uiPacketSize)
		{
			/*
			for (unsigned int i=0; i<response.uiPacketSize; i++)
				if ((sBuffer[i] >= 'A' && sBuffer[i] <= 'Z') || (sBuffer[i] >= 'a' && sBuffer[i] <= 'z') || (sBuffer[i] >= '0' && sBuffer[i] <= '9'))
					std::cout << sBuffer[i];
				else
					std::cout << '(' << std::hex << (int)sBuffer[i] << ')' << std::dec;
			
			std::cout << std::endl;*/
			decodeWords(sBuffer);
		}
		else
		{
			std::cout << "Error: Not received full packet " << usBytesReceived << "!=" << response.uiPacketSize << std::endl;
		}
	}
	else
	{
		// Empty the buffer
		//socket->read(sBuffer, sizeof(sBuffer), 100);
	}
}
