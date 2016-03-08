/* 
 * Copyright (C) 2012 Raffael Holz LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#include "battlefield3.hpp"
#include "helper.hpp"
#include "notifymyandroid.hpp"
#include "mail.hpp"
#include "translator.hpp"
#include <dlib/string.h>
#include <dlib/md5.h>

namespace Battlefield
{
	game::game(std::string name)
	{
		sql = new Database::SQLite();
		if (!sql->open("db/" + name + ".sqlite"))
			throw (std::string("Could load database for server " + name));

		settings = sql->getServerSettings();

		count.ucPlayer = 0;
		count.ucTeamRU = 0;
		count.ucTeamUS = 0;
		bBusy = true;

		helper::log::message(settings->sBF3_Server_Host + ":" + dlib::cast_to_string(settings->usBF3_Server_Port) + " successfully added");

		vote.g = this;
	}

	game::~game()
	{
		delete sql;
		delete settings;
		// We cannot delete "settings" here
		// delete settings;
	}

	bool game::connect()
	{
		_connect(settings->sBF3_Server_Host.c_str(), settings->usBF3_Server_Port);
		bBusy = false;

		if (socket == NULL)
			return (false);

		return (true);
	}

	void game::refresh()
	{
		_refresh();

		unsigned short usRealWordCount = response.words.size();
		if (usRealWordCount)
		{
			std::string function = response.words[0];
#ifdef BF4
			if (usRealWordCount > 2 && response.words[1] == "9" && response.words[2] == "name")
#else
			if (usRealWordCount > 2 && response.words[1] == "8" && response.words[2] == "name")
#endif
			{
				// Playercount
#ifdef BF4
				unsigned short usPointer = 11;
#else
				unsigned short usPointer = 10;
#endif
				count.ucPlayer = atoi(response.words[usPointer].c_str());
				count.ucTeamUS = 0;
				count.ucTeamRU = 0;

				for (char cPlayer=0; cPlayer < count.ucPlayer; cPlayer++)
				{
					// Name
					// std::cout << static_cast<int>(cPlayer) << ". Name:" << response.words[usPointer+1] << std::endl;
					pPlayerlist[cPlayer].name = dlib::tolower(response.words[++usPointer]);

					if (bJustJoined)
					{
						for (unsigned int i=0; i<count.ucPlayer; i++)
						{
							if (pPlayerlist[i].name == sLastJoiner)
							{
								pPlayerlist[i].hs = 0;

								break;
							}
						}
					}

					// EaID
					// std::cout << "EaID=" << response.words[usPointer+1] << std::endl;
					pPlayerlist[cPlayer].guid = response.words[++usPointer];

					// Team
					// std::cout << "Team=" << response.words[usPointer+1] << std::endl;
					pPlayerlist[cPlayer].team = dlib::string_cast<unsigned short>(response.words[++usPointer]);

					// Squad
					// std::cout << "Squad=" << response.words[usPointer+1] << std::endl;
					pPlayerlist[cPlayer].squad = dlib::string_cast<unsigned short>(response.words[++usPointer]);

					// Kills
					// std::cout << "Kills=" << response.words[usPointer+1] << std::endl;
					pPlayerlist[cPlayer].kills = dlib::string_cast<unsigned short>(response.words[++usPointer]);

					// Deaths
					// std::cout << "Deaths=" << response.words[usPointer+1] << std::endl;
					pPlayerlist[cPlayer].deaths = dlib::string_cast<unsigned short>(response.words[++usPointer]);
					
					// Score
					// std::cout << "Score=" << response.words[usPointer+1] << std::endl;
					pPlayerlist[cPlayer].score = dlib::string_cast<unsigned int>(response.words[++usPointer]);
					
					// Rank
					// std::cout << "Rank=" << response.words[usPointer+1] << std::endl;
					pPlayerlist[cPlayer].rank = dlib::string_cast<unsigned int>(response.words[++usPointer]);
#ifdef BF4
					// Ping
					// std::cout << "Ping=" << response.words[usPointer+1] << std::endl;
					pPlayerlist[cPlayer].ping = dlib::string_cast<unsigned int>(response.words[++usPointer]);
#endif
					if (pPlayerlist[cPlayer].team == 1)
						count.ucTeamUS++;
					else if (pPlayerlist[cPlayer].team == 2)
						count.ucTeamRU++;
				}
			}
			else if (function.length() >= 10 && function[0] == 'p')
			{
				if (function[9] == 'C' && usRealWordCount >= 5 && !response.words[2].empty()) // player.onChat
				{
					if (response.words[2][0] != '/' && response.words[2][0] != '!')
					{
						if (response.words.size() >= 4 && response.words[1] != "Server")
						{
							if (response.words[3] == "all")
								sql->log(response.words[1], response.words[2], LOG_CHAT);
							else if (response.words[3] == "team")
								sql->log(response.words[1], response.words[2], LOG_CHAT, response.words[4] == "1" ? LOG_CHAT_US : LOG_CHAT_RU);
							else if (response.words[3] == "squad")
								sql->log(response.words[1], response.words[2], LOG_CHAT, LOG_CHAT_SQUAD);

							struct_chat chat;
							chat.sName = dlib::tolower(response.words[1]);
							for (unsigned int i=0; i<count.ucPlayer; i++)
							{
								if (pPlayerlist[i].name == chat.sName)
								{
									if (response.words[3] == "team")
										chat.bMessageToTeam = true;
									else
										chat.bMessageToTeam = false;

									chat.cIsOnTeam = pPlayerlist[i].team;
									chat.sTime = helper::getDateTimeAsString();
									chat.sMessage = response.words[2];

									break;
								}
							}

							addChatMessage(chat);
						}
					}
					else
					{
						if (response.words.size() >= 3)
						{
							int iPosition = response.words[2].find(" ");
							if (iPosition)
							{
								std::string asd = response.words[2].substr(0, iPosition);
								ingameCmd(response.words[1], asd, response.words[2].substr(iPosition+1, response.words[2].length()));
							}
						}
					}
				}
				else if (function[9] == 'S' && function[10] == 'p') // player.onSpawn
				{
					if (bJustJoined)
					{
						if (dlib::tolower(response.words[1]) == sLastJoiner)
						{
							yell("Welcome to eXp", "player", sLastJoiner);
							bJustJoined = false;
						}
					}

					for (unsigned int i=0; i<count.ucPlayer; i++)
					{
						if (pPlayerlist[i].name == dlib::tolower(response.words[1]))
						{
							pPlayerlist[i].spawned = true;
						}
					}
				}
				else if (function[9] == 'J' && usRealWordCount >= 3) // player.onJoin
				{
					//if (sql->isBanned(response.words[1], response.words[2]))
						//cmd::player_kick(response.words[1], "HP&TS3:xlnt.cc");

					sLastJoiner = dlib::tolower(response.words[1]);
					bJustJoined = true;

					if (sql->getAdminLevel(response.words[2]))
					{
						if (settings->bAdminJoinMsg)
							say("eXp-Member " + sLastJoiner + " joined the game!");

						sql->updateAdminName(response.words[2], response.words[1]);
						updateWhitelist();
					}
				}
				else if (function[9] == 'K') // player.onKill
				{
					std::string sAttacker = dlib::tolower(response.words[1]);
					std::string sVictim = dlib::tolower(response.words[2]);
					std::string sWeapon = response.words[3];
					if (response.words[4] == "true")
					{
						for (unsigned int i=0; i<count.ucPlayer; i++)
						{
							if (pPlayerlist[i].name == sAttacker)
							{
								yell("Headshot Nr. " + dlib::cast_to_string(++pPlayerlist[i].hs), "player", sAttacker);
							}
							else if (pPlayerlist[i].name == sVictim)
							{
								pPlayerlist[i].spawned = false;
							}
						}
					}
					else 
					{
						if (sWeapon == "U_Melee")
							yell(sAttacker + " knifed " + sVictim);

						for (unsigned int i=0; i<count.ucPlayer; i++)
						{
							if (pPlayerlist[i].name == sVictim)
							{
								pPlayerlist[i].spawned = false;
							}
						}
					}

					if (settings->cMinPlayerCountForRestrictedWeaponPunish <= count.ucPlayer && std::find(restrictedWeapons.begin(), restrictedWeapons.end(), sWeapon) != restrictedWeapons.end())
					{
						say(sWeapon + " is currently restricted!", "player", sAttacker);
						yell(sWeapon + " is currently restricted!", "player", sAttacker);
						player_kill(sAttacker);
					}
					
					if (settings->bAutoTeamBalance)
					{
						static unsigned char whitelistIncrement = 0;

						unsigned char ucDifference = std::abs(count.ucTeamUS - count.ucTeamRU);
						if (ucDifference > 1)
						{
							unsigned short usTeam;
							if (count.ucTeamUS > count.ucTeamRU)
								usTeam = 1;
							else
								usTeam = 2;
						
							char cMoveList = count.ucPlayer - ++whitelistIncrement - static_cast<int>(count.ucPlayer*0.30);

							if (cMoveList < 0)
							{
								say("ATB: Unable to switch dead players. Phase 2 began.");
								say("ATB: Switching the last one connected.");

								unsigned char playercount = count.ucPlayer;
								while (--playercount)
								{
									if (pPlayerlist[playercount].team == usTeam)
									{
										player_move(pPlayerlist[playercount].name, pPlayerlist[playercount].team, 0, true);
										yell("Sorry! Autoteambalance =(", "player", pPlayerlist[playercount].name);
										say("Due to teambalance '" + pPlayerlist[playercount].name + "' had to be switched");
									}
								}

								return;
							}

							bool bOnWhitelist = false;
							for (unsigned char i=cMoveList > 1?cMoveList:count.ucPlayer-1; i<count.ucPlayer; i++)
							{
								if (pPlayerlist[i].team == usTeam && pPlayerlist[i].name == sVictim)
								{
									for (std::vector<std::string>::const_iterator it = whitelist.begin(); it != whitelist.end(); ++it)
									{
										if ((*it) == pPlayerlist[i].name)
										{
											bOnWhitelist = true;
											break;
										}
									}

									if (bOnWhitelist)
									{
										yell("You are on the Whitelist!", "player", pPlayerlist[i].name);
										say("ATB: " + pPlayerlist[i].name + " is on the whitelist. Not switching.");
									}
									else
									{
										player_move(pPlayerlist[i].name, pPlayerlist[i].team, 0, true);
										yell("Sorry! Autoteambalance =(", "player", pPlayerlist[i].name);
										say("Due to teambalance '" + pPlayerlist[i].name + "' had to be switched");
										break;
									}
								}
							}
						}
						else
						{
							// Teams are balanced
							whitelistIncrement = 0;
						}
					}
				}
				else if (function == "punkBuster.onMessage")
				{
					std::string message = response.words[1];

					size_t slotPos = message.find("slot #");
					if (slotPos != std::string::npos)
					{
						message = &message[slotPos+6];
						size_t slashPos = message.find(')');
						if (slashPos != std::string::npos)
						{
							const unsigned char slot = dlib::string_cast<unsigned short>(message.substr(0, slashPos));

							message = &message[slashPos+2];
							slotPos = message.find(':');
							std::string ip = message.substr(0, slotPos);

							slotPos = message.find("[?]");
							if (slotPos != std::string::npos)
							{
								message = message.substr(slotPos+5);
								// We need to do this twice. We have to find the second "
								message = message.substr(0, message.find('"')); // message is now the name of the player
								message = dlib::tolower(message);

								for (unsigned int i=0; i<count.ucPlayer; i++)
								{
									if (pPlayerlist[i].name == message)
									{
										pPlayerlist[i].ip = ip;
									}
								}
							}
						}
					}
				}
				else
				{
					//helper::log::message(function);
				}
			}
		}
	}

	void game::addChatMessage(const struct_chat chat)
	{
		if (chatlist.size() > 15)
			chatlist.pop_front();

		chatlist.push_back(chat);
	}

	unsigned char game::getPlayerIndexByName(std::string &name)
	{
		unsigned char bestMatchingIndex = 0;
		unsigned int bestDistance = std::string::npos;

		for (unsigned char i=0; i<count.ucPlayer; ++i)
		{
			unsigned char currentDistance = helper::levenshtein_distance(pPlayerlist[i].name, name);
			if (currentDistance < bestDistance)
			{
				bestDistance = currentDistance;
				bestMatchingIndex = i;
			}
		}

		return (bestMatchingIndex);
	}

	void game::ingameCmd(std::string sPlayerName, std::string sCmd, std::string sArg)
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		sPlayerName = dlib::tolower(sPlayerName);
		sCmd = dlib::tolower(sCmd);
		sArg = dlib::tolower(sArg);

		unsigned char ucUserID = 0;
		for (int i=0; i<count.ucPlayer; i++)
		{
			if (pPlayerlist[i].name == sPlayerName)
			{
				ucUserID = i;
			}
		}
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + "2 ===========");
	#endif
		unsigned int uiCMDLength = sCmd.length();

		if (uiCMDLength && sCmd[0] == '/')
		{
			unsigned int adminID = sql->getAdminID(pPlayerlist[ucUserID].guid);
			if (adminID)
			{
				double diff = 0;
				std::string date = sql->getAdminPayday(adminID);
				if (!date.empty())
				{
					std::vector<std::string> time = dlib::split(date, "/");
					diff = helper::timeFromNow(dlib::string_cast<unsigned short>(time[1]), dlib::string_cast<unsigned short>(time[0]), dlib::string_cast<unsigned short>(time[2]));
				}
				else
				{
					helper::log::error(__FILE__, __FUNCTION__, __LINE__, "Error while receiving payday. AdminID=" + dlib::cast_to_string(adminID));
				}

				if (diff > 0)
				{
					say("Unpaid for " + dlib::cast_to_string(diff) + " days! No permission to continue!", "player", sPlayerName);
					return;
				}
				else if (diff > -4 && diff != -0)
				{
					say("You have only " + dlib::cast_to_string(diff*(-1)) + " paid days left.", "player", sPlayerName);
				}

				if (sCmd == "/kill")
				{
					if (sArg == "/kill")
					{
						player_kill(sPlayerName);
						yell("You killed yourself!", "player", sPlayerName);
						sql->log(sPlayerName, "D->SELFKILL", LOG_ADMIN, LOG_ADMIN_KILL);
					}
					else
					{
						for (unsigned int i=0; i<count.ucPlayer; ++i)
						{
							if (pPlayerlist[i].name == sArg)
							{
								sql->log(sPlayerName, "D->Victim: " + sArg, LOG_ADMIN, LOG_ADMIN_KICK);
								yell(sPlayerName + " killed you!", "player", sArg);
								player_kill(sArg);
								
								say("You killed " + sArg, "player", sPlayerName);
								return;
							}
						}
						
						unsigned char index = getPlayerIndexByName(sArg);
						say("Player not found! Did you mean " + pPlayerlist[index].name + "|" + dlib::toupper(pPlayerlist[index].name) + "?", "player", sPlayerName);
					}
				}
				else if (sCmd == "/kick")
				{
					unsigned char whitespacePos = sArg.find(' ');
					if (whitespacePos != std::string::npos)
					{
						std::string victim = sArg.substr(0, whitespacePos);
						std::string reason = &sArg[whitespacePos];

						if (reason.length() < 3)
						{
							say("You need to write a reason", "player", sPlayerName);
							return;
						}

						for (unsigned int i=0; i<count.ucPlayer; ++i)
						{
							if (pPlayerlist[i].name == victim)
							{
								sql->log(sPlayerName, "K->Victim: " + victim + " Reason: " + &sArg[whitespacePos], LOG_ADMIN, LOG_ADMIN_KICK);
								player_kick(victim, &sArg[whitespacePos]);
								say("You kicked " + sArg, "player", sPlayerName);
								return;
							}
						}
						
						unsigned char index = getPlayerIndexByName(victim);
						say("Player not found! Did you mean " + pPlayerlist[index].name + "|" + dlib::toupper(pPlayerlist[index].name) + "?", "player", sPlayerName);
					}
					else
						say("/kick NAME REASON", "player", sPlayerName);
				}
				else if (sCmd == "/kicks")
				{
					unsigned char whitespacePos = sArg.find(' ');
					if (whitespacePos != std::string::npos)
					{
						std::string victim = sArg.substr(0, whitespacePos);
						for (unsigned int i=0; i<count.ucPlayer; ++i)
						{
							std::string victim = sArg.substr(0, whitespacePos);
							std::string reason = &sArg[whitespacePos];

							for (unsigned int i=0; i<count.ucPlayer; ++i)
							{
								if (pPlayerlist[i].name == victim)
								{
									sql->log(sPlayerName, "KS->Victim: " + victim, LOG_ADMIN, LOG_ADMIN_KICK);
									player_kick(victim, &sArg[whitespacePos]);
									say("You kicked " + sArg, "player", sPlayerName);
									return;
								}
							}
						
							unsigned char index = getPlayerIndexByName(victim);
							say("Player not found! Did you mean " + pPlayerlist[index].name + "|" + dlib::toupper(pPlayerlist[index].name) + "?", "player", sPlayerName);
						}
						
						unsigned char index = getPlayerIndexByName(victim);
						say("Player not found! Did you mean " + pPlayerlist[index].name + "|" + dlib::toupper(pPlayerlist[index].name) + "?", "player", sPlayerName);
					}
					else
						say("/kicks NAME", "player", sPlayerName);
				}
				else if (sCmd == "/say")
				{
					sql->log(sPlayerName, "S->" + sArg, LOG_ADMIN, LOG_ADMIN_SAY);
					say(sArg);
				}
				else if (sCmd == "/sayto")
				{
					std::string victim = sArg.substr(0, sArg.find(" "));
					std::string msg = sArg.erase(0, sArg.find(" "));
					sql->log(sPlayerName, "S->Victim: " + victim + "Msg:" + msg, LOG_ADMIN, LOG_ADMIN_SAY);
					say(sPlayerName + ":" + msg, "player", victim);
				}
				else if (sCmd == "/yell")
				{
					sql->log(sPlayerName, "Y->" + sArg, LOG_ADMIN, LOG_ADMIN_YELL);
					yell(sArg);
				}
				else if (sCmd == "/yellto")
				{
					std::string victim = sArg.substr(0, sArg.find(" "));
					std::string msg = sArg.erase(0, sArg.find(" "));
					sql->log(sPlayerName, "Y->Victim: " + victim + "Msg:" + msg, LOG_ADMIN, LOG_ADMIN_YELL);
					yell(sPlayerName + ":" + msg, "player", victim);
				}
				else if (sCmd == "/move")
				{
					for (unsigned int i=0; i<count.ucPlayer; ++i)
					{
						if (pPlayerlist[i].name == sArg)
						{
							sql->log(sPlayerName, "MF->Victim: " + sArg, LOG_ADMIN, LOG_ADMIN_MOVE);
							player_move(sArg, pPlayerlist[i].team, 0);
							return;
						}
					}

					unsigned char index = getPlayerIndexByName(sArg);
					say("Player not found! Did you mean " + pPlayerlist[index].name + "|" + dlib::toupper(pPlayerlist[index].name) + "?", "player", sPlayerName);
				}
				else if (sCmd == "/movef")
				{
					for (unsigned int i=0; i<count.ucPlayer; ++i)
					{
						if (pPlayerlist[i].name == sArg)
						{
							sql->log(sPlayerName, "MF->Victim: " + sArg, LOG_ADMIN, LOG_ADMIN_FMOVE);
							player_move(sArg, pPlayerlist[i].team, 0, true);
							return;
						}
					}
					
					unsigned char index = getPlayerIndexByName(sArg);
					say("Player not found! Did you mean " + pPlayerlist[index].name + "|" + dlib::toupper(pPlayerlist[index].name) + "?", "player", sPlayerName);
				}
				else if (sCmd == "/ban")
				{
					unsigned char whitespacePos = sArg.find(' ');
					if (whitespacePos != std::string::npos)
					{
						std::string victim = sArg.substr(0, whitespacePos);
						std::string reason = &sArg[whitespacePos];

						if (reason.length() < 3)
						{
							say("You need to write a reason", "player", sPlayerName);
							return;
						}

						for (unsigned int i=0; i<count.ucPlayer; ++i)
						{
							if (pPlayerlist[i].name == victim)
							{
								sql->log(sPlayerName, "B->Victim: " + victim + " Reason: " + &sArg[whitespacePos], LOG_ADMIN, LOG_ADMIN_BAN);
								player_ban(pPlayerlist[i].guid, victim, sPlayerName, &sArg[whitespacePos], true);
								say("You banned " + sArg, "player", sPlayerName);
								return;
							}
						}
						
						unsigned char index = getPlayerIndexByName(victim);
						say("Player not found! Did you mean " + pPlayerlist[index].name + "|" + dlib::toupper(pPlayerlist[index].name) + "?", "player", sPlayerName);
					}
					else
						say("/ban NAME REASON", "player", sPlayerName);
				}
				else if (sCmd == "/mute")
				{
					unsigned char whitespacePos = sArg.find(' ');
					if (whitespacePos != std::string::npos)
					{
						for (unsigned int i=0; i<count.ucPlayer; ++i)
						{
							if (pPlayerlist[i].name == sArg)
							{
								sql->log(sPlayerName, "M->Victim: " + sArg, LOG_ADMIN, LOG_ADMIN_BAN);
								say("Disabled chat of " + sArg, "player", sPlayerName);
								for (unsigned char i=0; i<50; i++)
								{
									say("YOUR CHAT HAS BEEN DISABLED", "player", sArg);
									say("", "player", sArg);
									say("", "player", sArg);
									say("", "player", sArg);
									say("", "player", sArg);
									say("", "player", sArg);
								}
								say("YOUR ARE FREE TO CHAT", "player", sArg);
								return;
							}
						}
						
						unsigned char index = getPlayerIndexByName(sArg);
						say("Player not found! Did you mean " + pPlayerlist[index].name + "|" + dlib::toupper(pPlayerlist[index].name) + "?", "player", sPlayerName);
					}
					else
						say("/mute NAME", "player", sPlayerName);
				}
				else if (sCmd == "/hack")
				{
					unsigned char whitespacePos = sArg.find(' ');
					if (whitespacePos != std::string::npos)
					{
						for (unsigned int i=0; i<count.ucPlayer; ++i)
						{
							if (pPlayerlist[i].name == sArg)
							{
								sql->log(sPlayerName, "H->Victim: " + sArg, LOG_ADMIN, LOG_ADMIN_BAN);
								say("Hacked chat of " + sArg, "player", sPlayerName);

								for (unsigned char i=0; i<4; i++)
									say("", "player", sArg);

								say("YOU HAVE BEEN HACKED!", "player", sArg);
								for (unsigned char i=0; i<4; i++)
									say("", "player", sArg);

								say("BRUTEFORCING EA-ACCOUNT PASSWORD...", "player", sArg);
								for (unsigned char i2=0; i2<40; i2++)
									say("", "player", sArg);

								say("| 10%", "player", sArg);
								say("|| 21%", "player", sArg);
								say("||| 33%", "player", sArg);
								say("|||| 47%", "player", sArg);
								say("||||| 52%", "player", sArg);
								say("||||||| 69%", "player", sArg);
								say("||||||||| 99%", "player", sArg);

								say("FOUND!", "player", sArg);
								say("CHANGING EA-ACCOUNT-E-MAIL-ADDRESS...", "player", sArg);
								for (unsigned char i2=0; i2<40; i2++)
									say("", "player", sArg);
								say("SUCCESSFULLY CHANGED!", "player", sArg);
								say("SHUTTING DOWN REMOTE-COMPUTER IN 10 SEC!", "player", sArg);

								int bla = 10;
								while (bla--)
								{
									say("", "player", sArg);
									say("", "player", sArg);
									say(dlib::cast_to_string(bla), "player", sArg);
									say("", "player", sArg);
									say("", "player", sArg);
								}

								say("JUST KIDDING :D", "player", sArg);
								return;
							}
						}
						
						unsigned char index = getPlayerIndexByName(sArg);
						say("Player not found! Did you mean " + pPlayerlist[index].name + "|" + dlib::toupper(pPlayerlist[index].name) + "?", "player", sPlayerName);
					}
					else
						say("/hack NAME", "player", sPlayerName);
				}
				else if (sCmd == "/hackall")
				{
					for (unsigned char i=0; i<4; i++)
						say("");

					say("YOU HAVE BEEN HACKED!");
					for (unsigned char i=0; i<4; i++)
						say("");

					say("BRUTEFORCING EA-ACCOUNT PASSWORD...");
					for (unsigned char i2=0; i2<40; i2++)
						say("");

					say("| 10%");
					say("|| 21%");
					say("||| 33%");
					say("|||| 47%");
					say("||||| 52%");
					say("||||||| 69%");
					say("||||||||| 99%");

					say("FOUND!");
					say("CHANGING EA-ACCOUNT-E-MAIL-ADDRESS...");
					for (unsigned char i2=0; i2<40; i2++)
						say("");
					say("SUCCESSFULLY CHANGED!");
					say("SHUTTING DOWN REMOTE-COMPUTER IN 10 SEC!");

					int bla = 10;
					while (bla--)
					{
						say("");
						say("");
						say(dlib::cast_to_string(bla));
						say("");
						say("");
					}

					say("JUST KIDDING :D");
				}
				else if (sCmd == "/help")
				{
					say("Available commands are /kill/kick/kicks/ban/move/movef/mute/say/sayto/yell/yellto/guid/pwd", "player", sPlayerName);
				}
				else if (sCmd == "/guid")
				{
					for (unsigned int i=0; i<count.ucPlayer; ++i)
					{
						if (pPlayerlist[i].name == sArg)
						{
							say(pPlayerlist[i].guid, "player", sPlayerName);
							return;
						}
					}

					say("Did you mean " + pPlayerlist[getPlayerIndexByName(sArg)].name + "?", "player", sPlayerName);
				}
				else if (sCmd == "/updateadv")
				{
					sql->updateAdvMessages();
					say("Updated!", "player", sPlayerName);
				}
				else if (sCmd == "/pwd")
				{
					for (unsigned int i=0; i<count.ucPlayer; ++i)
					{
						if (pPlayerlist[i].name == sPlayerName)
						{
							sql->updateAdminPassword(pPlayerlist[i].guid, sArg);
							break;
						}
					}
				}
				else if (sCmd == "/payment")
				{
					if (diff == 0)
						say("You need to pay today!", "player", sPlayerName);
					else
						say("You have paid for " + dlib::cast_to_string(diff*(-1)) + " days.", "player", sPlayerName);
				}
				else
				{
					say("Unknown Command!", "player", sPlayerName);
				}
			}
			else
			{
				say("Only eXp-Members can use such commands!", "player", sPlayerName);
			}
		}
		else
		{
			if (sCmd == "!yes")
			{
				vote.addVoter(sPlayerName, 1);
			}
			else if (sCmd == "!no")
			{
				vote.addVoter(sPlayerName, 0);
			}
			else if (sCmd == "!votekick")
			{
				bool playerFound = false;
				for (unsigned char i=0; i<count.ucPlayer; i++)
				{
					if (pPlayerlist[i].name == sArg)
					{
						playerFound = true;
						break;
					}
				}

				if (playerFound && !sArg.empty())
					vote.startVoteKick(sArg);
			}
			else if (sCmd == "!member")
			{
				std::string sTemp;
				std::vector<Database::admin> vAdminlist = sql->adminlist();
				say("eXp-Members are:", "player", sPlayerName);
				char i = 0;
				for (std::vector<Database::admin>::const_iterator it = vAdminlist.begin(); it != vAdminlist.end(); ++it)
				{
					sTemp += (*it).sName + ",";
					if (++i > 4)
					{
						i = 0;
						say(sTemp, "player", sPlayerName);
						sTemp.clear();
					}
				}
			}
			else if (sCmd == "!tool")
			{
				say("eXp-Administration-Tool(daspirum.de) | Crossplatform(Windows&Linux) - C++");
			}
			else if (sCmd == "!contact")
			{
				say("Send your request to: hostmaster@exp-clan.com or use the ingame command !request.", "player", sPlayerName);
			}
			else if (sCmd == "!restricted")
			{
				std::string str;
				char i = 0;
				for (std::vector<std::string>::const_iterator it = restrictedWeapons.begin(); it != restrictedWeapons.end(); ++it)
				{
					std::string weaponName = (*it);
					if (weaponName.at(0) == 'U')
						str += weaponName.erase(0, 2) + ",";
					else
						str += weaponName + ",";

					if (++i > 4)
					{
						i = 0;
						say(str);
						str.clear();
					}
				}
			}
			else if (sCmd == "!admin" || sCmd == "!report" || sCmd == "!request")
			{
				if (sArg.empty())
				{
					say("We need to know what you want to report: !admin TEXT", "player", sPlayerName);
				}
				else
				{
					sql->sendRequest(sPlayerName, sArg);
					say("Request send!", "player", sPlayerName);
					nma_NotificateAll("The Player " + sPlayerName + " reported the following on one of your Gameservers(" + settings->sBF3_Server_Host + ":" + dlib::cast_to_string(settings->usBF3_Server_Port) + "): " + sArg);
					mail_SendToAll("The Player " + sPlayerName + " reported the following on one of your Gameservers(" + settings->sBF3_Server_Host + ":" + dlib::cast_to_string(settings->usBF3_Server_Port) + "): " + sArg);
				}
			}
			else if (sCmd == "!guid")
			{
				say("Your EA guid is " + pPlayerlist[ucUserID].guid, "player", sPlayerName);
			}
			else if (sCmd == "!help")
			{
				say("Available commands are !member,!tool,!restricted,!contact,!request,!guid,!trans,!translast" + sql->chatResponseListAsString(), "player", sPlayerName);
			}
			else if (sCmd == "!trans")
			{
				int pos = sArg.find(' ');

				if (pos != -1)
				{
					std::string language = sArg.substr(0, pos);
					say(translator::translate(&sArg[pos+1], language));
				}

				/*
				int pos = sArg.find('-');
				int pos2 = sArg.find(' ');

				if (pos != -1 && pos2 != -1)
				{
					std::string languageFrom = sArg.substr(0, pos);
					std::string languageTo = sArg.substr(pos+1, pos2-2);
					say(translator::translate(&sArg[pos2+1], languageFrom, languageTo));
				}
				*/
			}
			else if (sCmd == "!translast")
			{
				if (chatlist.size())
					say(translator::translate(chatlist.back().sMessage, sArg));
			}
			else
			{
				say(sql->getChatResponse(sCmd));
			}
		}
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::sendSimpleCmd(const std::string sCmd)
	{
		std::vector<std::string> cmd;
		cmd.push_back(sCmd);
		sendCMD(cmd);
		cmd.clear();
	}

	void game::sendSimpleCmdWithValue(const std::string sCmd, const std::string sValue)
	{
		std::vector<std::string> cmd;
		cmd.push_back(sCmd);
		cmd.push_back(sValue);
		sendCMD(cmd);
		cmd.clear();
	}

	const unsigned char HEXTOINT[16] = {0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240};

	bool game::login()
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		sendSimpleCmd("login.hashed");
		_refresh();
		if (response.words[0] == "OK")
		{
			std::string hash;

			// Fast way of converting Hex-String to Int-String
			const char *pwd = response.words[1].c_str();
			for (int i=0; i<32; i+=2)
			{
				unsigned char ch1 = pwd[i];
				unsigned char ch2 = pwd[i+1];

				if (ch1 < 65)
					ch1 -= 48;
				else
					ch1 -= 55;

				if (ch2 < 65)
					ch2 -= 48;
				else
					ch2 -= 55;

				hash += HEXTOINT[ch1] + ch2;
			}

			// Send the hashed-password back to the server
			std::vector<std::string> cmd;
			cmd.push_back("login.hashed");
			// Append the password, then md5 it. Make the result to upper-chars only :D
			cmd.push_back(dlib::toupper(dlib::md5(hash + settings->sBF3_Server_Pwd)));
			sendCMD(cmd);
			cmd.clear();

			// Now check the result
			_refresh();
			if (response.words[0] == "OK")
				return (true);
		}
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
		return (false);
	}

	void game::say(std::string sText, std::string sTarget, std::string sPlayer)
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		std::vector<std::string> cmd;
		cmd.push_back("admin.say");
		cmd.push_back(sText);
		cmd.push_back(sTarget);
		if (sPlayer != "")
			cmd.push_back(sPlayer);
		sendCMD(cmd);
		cmd.clear();
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::yell(std::string sText, std::string sTarget, std::string sPlayer)
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		std::vector<std::string> cmd;
		cmd.push_back("admin.yell");
		cmd.push_back(sText);
		cmd.push_back("4");
		cmd.push_back(sTarget);
		if (sPlayer != "")
			cmd.push_back(sPlayer);
		sendCMD(cmd);
		cmd.clear();
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::player_kill(std::string sPlayerName)
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		std::vector<std::string> cmd;
		cmd.push_back("admin.killplayer");
		cmd.push_back(sPlayerName);
		sendCMD(cmd);
		cmd.clear();
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::player_kick(std::string sPlayerName, std::string sReason)
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		std::vector<std::string> cmd;
		cmd.push_back("admin.kickplayer");
		cmd.push_back(sPlayerName);
		cmd.push_back(sReason);
		sendCMD(cmd);
		cmd.clear();
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::player_move(const std::string sPlayers, const unsigned char usTeam, const unsigned char usSquad, const bool cbForce)
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		std::vector<std::string> cmd;
		cmd.push_back("admin.movePlayer");
		cmd.push_back(sPlayers);

		if (usTeam == 1)
			cmd.push_back("2");
		else
			cmd.push_back("1");

		if (usSquad)
			cmd.push_back(dlib::cast_to_string(usSquad));
		else
			cmd.push_back("0");

		cmd.push_back(cbForce ? "true" : "false");

		sendCMD(cmd);
		cmd.clear();
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::player_ban(const std::string sGuID, const std::string name, const std::string admin, const std::string reason, const bool bAddToDB)
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		std::vector<std::string> cmd;
		cmd.push_back("banList.add");
		cmd.push_back("guid");
		cmd.push_back(sGuID);
		cmd.push_back("perm");
		cmd.push_back(reason);
		sendCMD(cmd);
		cmd.clear();

		if (bAddToDB)
			sql->addBan(sGuID, name, sql->getAdminID(admin), reason);
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::player_unban(const std::string sGuID, const bool bAddToDB)
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		std::vector<std::string> cmd;
		cmd.push_back("banList.remove");
		cmd.push_back("guid");
		cmd.push_back(sGuID);
		sendCMD(cmd);
		cmd.clear();

		if (bAddToDB)
			sql->unban(sGuID);
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::player_white(const std::string sName)
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		std::vector<std::string> cmd;
		cmd.push_back("reservedSlotsList.add");
		cmd.push_back(sName);
		sendCMD(cmd);
		cmd.clear();
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::listplayers(std::string sPlayers)
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		std::vector<std::string> cmd;
		cmd.push_back("admin.listPlayers");
		cmd.push_back(sPlayers);
		sendCMD(cmd);
		cmd.clear();
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::events(bool active)
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		std::vector<std::string> cmd;
		cmd.push_back("admin.eventsEnabled");
		if (active)
			cmd.push_back("1");
		else
			cmd.push_back("0");
		sendCMD(cmd);
		cmd.clear();
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::logout()
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif
		std::vector<std::string> cmd;
		cmd.push_back("logout");
		sendCMD(cmd);
		cmd.clear();
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	bool game::isBusy()
	{
		return (bBusy);
	}

	void game::updateBanlist()
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif

		// ToDo : Unban everyone
		std::vector<std::string> cmd;
		cmd.push_back("banList.clear");
		sendCMD(cmd);
		cmd.clear();

		std::vector<Database::ban> banlist = sql->banlist();
		for(std::vector<Database::ban>::const_iterator it = banlist.begin(); it != banlist.end(); ++it)
			player_ban((*it).sGuID, (*it).sName, "", (*it).sReason, false);

	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	void game::updateWhitelist()
	{
	#ifdef DEBUG_FUNCTIONS
		helper::log::message("=========== " + std::string(__FUNCTION__) + " ===========");
	#endif

		std::vector<std::string> cmd;
		cmd.push_back("reservedSlotsList.clear");
		sendCMD(cmd);
		cmd.clear();

		whitelist.clear();
		whitelist = sql->whitelist();
		for(std::vector<std::string>::const_iterator it = whitelist.begin(); it != whitelist.end(); ++it)
			player_white((*it));

		cmd.push_back("reservedSlotsList.save");
		sendCMD(cmd);
		cmd.clear();

	#ifdef DEBUG_FUNCTIONS
		helper::log::message("========== /" + std::string(__FUNCTION__) + " ===========");
	#endif
	}

	std::list<struct_chat> game::getChatlist()
	{
		return (chatlist);
	}

	void game::nma_NotificateAll(std::string sMessage)
	{
		std::vector<std::string> nmalist = sql->nmalist();

		for (std::vector<std::string>::const_iterator it = nmalist.begin(); it != nmalist.end(); ++it) 
		{
			if (notifymyandroid::notificate((*it), sMessage) == false)
			{
				helper::log::message("API-KEY Wrong?!");
			}
		}
	}

	void game::mail_SendToAll(std::string sMessage)
	{
		std::vector<std::string> mailReceiverList = sql->mailReceiverList();

		for (std::vector<std::string>::const_iterator it = mailReceiverList.begin(); it != mailReceiverList.end(); ++it) 
		{
			if (mail::sendMail((*it), "daspirum report", sMessage) == false)
			{
				helper::log::message("Wrong Mail-Addy?!");
			}
		}
	}

	void game::restrictWeapon(std::string sName)
	{
		if (!(std::find(restrictedWeapons.begin(), restrictedWeapons.end(), sName) != restrictedWeapons.end()))
		{
			restrictedWeapons.push_back(sName);
			sql->addRestrictedWeapon(sName);
		}
	}

	void game::unRestrictWeapon(std::string sName)
	{
		restrictedWeapons.erase(std::remove(restrictedWeapons.begin(), restrictedWeapons.end(), sName), restrictedWeapons.end());
		sql->deleteRestrictedWeapon(sName);
	}
}
