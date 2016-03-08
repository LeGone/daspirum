/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include "battlefield3.hpp"
#include "helper.hpp"
#include "SQLite.hpp"
#include "http.hpp"
#include "main.hpp"
#include "Config.hpp"
#include "mail.hpp"
#include "translator.hpp"
#include "LGN.hpp"

// Dlib
#include <dlib/config_reader.h>
#include <dlib/threads.h>
#include <dlib/misc_api.h>  // for dlib::sleep
#include <dlib/dir_nav.h>

using namespace std;

void main_command_loop();
bool bAlive = true;

// List of gameservers
std::vector<struct_gamestatus*> gGameServers;

int main()
{
#if defined _WIN32 || defined _WIN64
	SetConsoleTitle(L"eXp-Administration-Tool");
#endif
	
	helper::log::init();
	helper::log::message("eXp-Administration-Tool", LOG_COLOR_ORANGE);
	helper::log::message("Version " + dlib::cast_to_string(BUILD), LOG_COLOR_ORANGE);

#if defined _WIN32 || defined _WIN64
	helper::log::message("Running on Windows", LOG_COLOR_GREEN);
#else
	helper::log::message("Running on Linux", LOG_COLOR_GREEN);
#endif

	// Init Config-File
	helper::log::message("Reading config", LOG_COLOR_GREEN);
	if (!Config::Load())
		return (EXIT_FAILURE);

	// Check version
	int build = LGN::GetVersion();
	if (build != -1 && build != BUILD)
		helper::log::message("Your daspirum-version is outdated. Please consider updating.", LOG_COLOR_ORANGE);

	// +Webserver
	helper::log::message("Starting Webserver...", LOG_COLOR_GREEN);
	short webserver_port;
	try
	{
		webserver_port = Config::Webserver::port;
		helper::log::message(std::string("Webserver started on port ") + dlib::cast_to_string(webserver_port), LOG_COLOR_GREEN);
	}
	catch (...)
	{
		webserver_port = 80;
		helper::log::message("Webserver started on default port 80. error in config file", LOG_COLOR_ORANGE);
	}
	http::start(webserver_port);
	// -Webserver

	// Reading all individuell Server-Databases
	helper::log::message("Reading Server-Databases...", LOG_COLOR_GREEN);
	dlib::directory dir("db");
	std::vector<dlib::file> files;
	dir.get_files(files);

	// Start all game-connections
	for (unsigned char i = 0; i < files.size(); ++i)
	{
		struct_gamestatus *newGameConnection = new struct_gamestatus;
		newGameConnection->name = files[i].name();
		newGameConnection->name.erase(newGameConnection->name.find('.'));
		gGameServers.push_back(newGameConnection);

		// Start the individuell Game-Thread
		dlib::create_new_thread(thread_game, newGameConnection);
	}

	// Init finished
	helper::log::message("Init finished. Starting main-function-loop...", LOG_COLOR_BLUE);

	// Now the command-thread
	main_command_loop();
}

void main_command_loop()
{
	std::string sCommand;
	try
	{
		do
		{
			char sTmp[256];
			std::cin.getline(sTmp, sizeof(sTmp));
			sCommand = sTmp;

			helper::log::message('>' + sCommand, LOG_COLOR_ORANGE);
			if (sCommand == "quit")
			{
				bAlive = false;
				helper::log::message("Stopping Server...", LOG_COLOR_ORANGE);
				helper::log::message("This may take up to 30 seconds.", LOG_COLOR_ORANGE);
				helper::log::message("Stopping Webserver...", LOG_COLOR_GREEN);
				http::stop();

				helper::log::message("Bye Bye! Copyright (c) Raffael 'LeGone' Holz | <a href=\"http://www.legone.nae\">legone.name</a>", LOG_COLOR_GREEN);

				exit(EXIT_SUCCESS);
			}
			else if (sCommand == "bug")
			{
				for (std::vector<struct_gamestatus*>::const_iterator it = gGameServers.begin(); it != gGameServers.end(); ++it) 
				{
					try
					{
						if ((*it) != NULL && (*it)->state != Battlefield::gamestate::INACTIVE)
						{
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say("eXcellent performance!");
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
							(*it)->g->say(sCommand.erase(0, 3));
						}
					}
					catch (...)
					{
						helper::log::message("Error while sending manual-command.");
					}
				}
			}
			else if (sCommand.find("say") == 0 && sCommand.length() > 4)
			{
				for (std::vector<struct_gamestatus*>::const_iterator it = gGameServers.begin(); it != gGameServers.end(); ++it) 
				{
					try
					{
						if ((*it) != NULL && (*it)->state != Battlefield::gamestate::INACTIVE)
						{
							(*it)->g->say(sCommand.substr(4));
							//(*it)->g->say(sCommand.erase(0, 4));
						}
					}
					catch (...)
					{
						helper::log::message("Error while sending manual-command.");
					}
				}
			}
			else if (sCommand.find("status") == 0 && sCommand.length() > 4)
			{
				short count_of_active_servers = 0, count_of_inactive_servers = 0;
				for (std::vector<struct_gamestatus*>::const_iterator it = gGameServers.begin(); it != gGameServers.end(); ++it) 
				{
					try
					{
						if ((*it) != NULL && (*it)->state == Battlefield::gamestate::ACTIVE)
						{
							++count_of_active_servers;
							helper::log::message((*it)->name + " is currently active", LOG_COLOR_GREEN);
						}
						else
						{
							++count_of_inactive_servers;
							helper::log::message((*it)->name + " is not active. " + (*it)->sError, LOG_COLOR_RED);
						}
					}
					catch (...)
					{
						helper::log::message("Error while sending manual-command.");
					}
				}

				helper::log::message(dlib::cast_to_string(count_of_active_servers) + '/' + dlib::cast_to_string(count_of_active_servers+count_of_inactive_servers) + " servers are currently active");
			}
			else
			{
				helper::log::message("Unknown Command: " + sCommand, LOG_COLOR_ORANGE);
			}
		} while(bAlive);
	}
	catch (exception& e)
	{
		helper::log::error(__FILE__, __FUNCTION__, __LINE__, string("Error while starting server: ") + e.what());
	}
	catch (...)
	{
		helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Unknown Error!");
	}

	// There was an error?
	if (bAlive)
		main_command_loop(); // Call the function again
}

void thread_game(void *container_ptr)
{
	clock_t t;
	double nextPlayerUpdateTime = 0.0, nextVoteThinkTime = 0.0, nextAdvThinkTime = 0.0;
	Battlefield::game *g = NULL;
	Database::SQLite *sql;
	Database::struct_settings *settings = NULL;
	struct_gamestatus *gamecontainer = NULL;

	try
	{
		gamecontainer = static_cast<struct_gamestatus*>(container_ptr);

		do
		{
			try
			{
				g = new Battlefield::game(gamecontainer->name);
			}
			catch (const std::exception& e)
			{
				helper::log::error(__FILE__, __FUNCTION__, __LINE__, "[" + gamecontainer->name + "] Connection to SQLite-DB failed:" + e.what());
				gamecontainer->sError = e.what();
				gamecontainer->state = Battlefield::gamestate::INACTIVE;
				break;
			}
			catch (const std::string e)
			{
				helper::log::error(__FILE__, __FUNCTION__, __LINE__, "[" + gamecontainer->name + "] Connection to SQLite-DB failed:" + e);
				gamecontainer->sError = e;
				gamecontainer->state = Battlefield::gamestate::INACTIVE;
				break;
			}

			gamecontainer->g = g;
			settings = g->settings;
			
			sql = g->getSQLiteConnection();
			sql->updateAdvMessages();
			g->restrictedWeapons = sql->restrictedWeaponsList();

			while (!g->connect() && gamecontainer->state != Battlefield::gamestate::INACTIVE)
			{
				helper::log::error(__FILE__, __FUNCTION__, __LINE__, "[" + gamecontainer->name + "] Connection to Server failed (Reconnecting in 10 seconds)");
				gamecontainer->sError = "Unable to connect";
				gamecontainer->state = Battlefield::gamestate::UNCONNECTED;

				unsigned char reconnectTime = 10;
				while (bAlive && --reconnectTime && 0)
				{
#if defined _WIN32 || defined _WIN64
					Sleep(1000);
#else
					sleep(1);
#endif
				}
			}

			g->login();
			g->updateBanlist();
			g->updateWhitelist();
			g->events(true);

			gamecontainer->state = Battlefield::gamestate::ACTIVE;

			do
			{
				t = clock();

				try
				{
					g->refresh();

					if (t > nextPlayerUpdateTime)
					{
						nextPlayerUpdateTime = t + 2000;
						g->listplayers("all");
					}

					if (t > nextVoteThinkTime)
					{
						nextVoteThinkTime = t + 1000;
						g->vote.think();
					}

					if (t > nextAdvThinkTime)
					{
						nextAdvThinkTime = t + settings->ucAdvInterval * 1000;

						Database::message msg = sql->getNextMessage();

						if (g->isBusy())
							continue;

						if (msg.bYell)
							g->yell(msg.sMsg);
						else
							g->say(msg.sMsg);
					}
				}
				catch (const std::exception& e)
				{
					helper::log::error(__FILE__, __FUNCTION__, __LINE__, "[" + gamecontainer->name + "] " + e.what());
					gamecontainer->sError = e.what();
					break;
				}
				catch (const std::string e)
				{
					helper::log::error(__FILE__, __FUNCTION__, __LINE__, "[" + gamecontainer->name + "] " + e);
					gamecontainer->sError = e;
					break;
				}
				catch(...)
				{
					helper::log::error(__FILE__, __FUNCTION__, __LINE__, "[" + gamecontainer->name + "] Unknown Error!");
					gamecontainer->sError = "Unknown Error in inner loop!";
					break;
				}
			} while (bAlive && gamecontainer->state != Battlefield::gamestate::INACTIVE);

			g->events(false);
			g->logout();

			delete g;
			g = NULL;
#if defined _WIN32 || defined _WIN64
			Sleep(3000);
#else
			sleep(3);
#endif
		} while (bAlive && gamecontainer->state != Battlefield::gamestate::INACTIVE);
	}
	catch (exception& e)
	{
		helper::log::error(__FILE__, __FUNCTION__, __LINE__, "[" + gamecontainer->name + "] Error while starting server: " + e.what());
		gamecontainer->sError = string("Error while starting server: ") + e.what();
	}
	catch (const std::string e)
	{
		helper::log::error(__FILE__, __FUNCTION__, __LINE__, "[" + gamecontainer->name + "] " + e);
		gamecontainer->sError = e;
	}
	catch (...)
	{
		helper::log::error(__FILE__, __FUNCTION__, __LINE__, "[" + gamecontainer->name + "] Unknown Error!");
		gamecontainer->sError = "Unknown Error in outter loop!";
	}

	if (g)
		delete g;

#if defined _WIN32 || defined _WIN64
	Sleep(5000);
#else
	sleep(5);
#endif

	if (bAlive && gamecontainer->state != Battlefield::gamestate::INACTIVE)
		dlib::create_new_thread(thread_game, gamecontainer);
}
