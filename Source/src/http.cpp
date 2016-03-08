/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#include <iostream>
#include <sstream>
#include <string>
#include "main.hpp"
#include "Config.hpp"
#include "helper.hpp"
#include "battlefield3.hpp"
#include "GeoIP.hpp"

// Dlib
#include "dlib/server.h"
#include "dlib/ref.h" // for ref()
#include <dlib/md5.h>

using namespace dlib;
using namespace std;

namespace http
{
	std::string s404, sLogin, sLoginRSP, sHeader_1, sHeader_2, sHeader_3, sHeader_RSP, sFooter, sUnknownServer;

	std::string read_file_html(std::string sFileName)
	{
#if defined _WIN32 || defined _WIN64
		ifstream file ("www/" + sFileName);
#else
		ifstream file (("www/" + sFileName).c_str());
#endif
		if (file.is_open())
		{
			std::string sOutput;
			std::string line;

			while (file.good())
			{
				getline (file, line);
				sOutput += line + "\r\n";
			}
			file.close();

			return (sOutput);
		}
		else 
		{
			return (s404); 
		}		
	}

	std::string load_critical_html_file(char *sFileName)
	{
		std::string sOutput;
		std::string line;

		ifstream file(sFileName);

		if (file.is_open())
		{
			while (file.good())
			{
				getline (file, line);
				sOutput += line;
			}
			file.close();
		}
		else
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, std::string(sFileName) + " not found?!");
		}

		return (sOutput);
	}

	std::string read_file_plain(std::string sFileName)
	{
#if defined _WIN32 || defined _WIN64
		ifstream file("www/" + sFileName);
#else
		ifstream file(("www/" + sFileName).c_str());
#endif
		if (file.is_open())
		{
			std::string sOutput;
			std::string line;

			while (file.good())
			{
				getline (file, line);
				sOutput += line + "<br/>";
			}
			file.close();

			return (sOutput);
		}
		else 
		{
			return (s404); 
		}
	}

	std::string jQuerySuccess(std::string sOutput)
	{
		sOutput = "<div class='ui-widget'><div class='ui-state-highlight ui-corner-all' style='margin-top: 20px; padding: 0 .7em;'><p><span class='ui-icon ui-icon-info' style='float: left; margin-right: .3em;'></span><strong>Success:</strong>"
				 + sOutput + "</p></div></div>";

		return (sOutput);
	}

	std::string jQueryAlert(std::string sOutput)
	{
		sOutput = "<div class='ui-widget'><div class='ui-state-error ui-corner-all' style='padding: 0 .7em;'><p><span class='ui-icon ui-icon-alert' style='float: left; margin-right: .3em;'></span><strong>Alert:</strong>"
				 + sOutput + "</p></div></div>";

		return (sOutput);
	}

	void generateStringVarCode(ostringstream &sOutput, std::string sVar)
	{
		sOutput << "<p><form action='!s_varset' method='get'><input name='var' type='hidden' value='vars." << sVar << "'>"
			<< sVar << ": <input id=\"" << sVar << "\" name='value' type='text' size='32' maxlength='32'>"
			<< "<input type=\"submit\" value=\"Save\"></form>"
			<< "<a id=\"get_db_" << sVar << "\">Load from Database</a> | <a id=\"get_gs_" << sVar << "\">Load from GameServer</a></p>"
			<< "<script>$('#get_db_" << sVar << "').click(function() { $.get(\"!s_vargetdb\", function(data){$(\"#" << sVar << "\").val(data)}); });"
			<< "$('#get_gs_" << sVar << "').click(function() { $.get(\"!s_vargetgs\", function(data){$(\"#" << sVar << "\").val(data)}); });</script>";
	}

	void generateBooleanVarCode(ostringstream &sOutput, std::string sVar)
	{
		sOutput << "<p><form action='!s_varset' method='get'><input name='var' type='hidden' value='vars." << sVar << "'><input name='var' type='hidden' value=''>"
			<< sVar << ": <input id=\"" << sVar << "\" name='value' type='text' size='32' maxlength='32'>"
			<< "<input type=\"submit\" value=\"Save\"></form>"
			<< "<a id=\"get_db_" << sVar << "\">Load from Database</a> | <a id=\"get_gs_" << sVar << "\">Load from GameServer</a></p>"
			<< "<script>$('#get_db_" << sVar << "').click(function() { $.get(\"!s_vargetdb\", function(data){$(\"#" << sVar << "\").val(data)}); });"
			<< "$('#get_gs_" << sVar << "').click(function() { $.get(\"!s_vargetgs\", function(data){$(\"#" << sVar << "\").val(data)}); });</script>";
	}
	
	void ajax_popup(ostringstream &sOutput)
	{
		sOutput << "<link rel=\"stylesheet\" href=\"css/jquery.dataTables_themeroller.css\" />";
		sOutput << "<script src=\"js/jquery.dataTables.min.js\"></script>";
		sOutput << "<script>$(document).ready(function()";
		sOutput << "{";
		sOutput << "$('#table').dataTable(";
		sOutput << "{";
		sOutput << "\"bJQueryUI\": true,";
		sOutput << "\"bPaginate\": true,";
		sOutput << "\"bFilter\": true,";
		sOutput << "\"bSort\": true,";
		sOutput << "\"bInfo\": true,";
		sOutput << "\"bAutoWidth\": true,";
		sOutput << "\"aLengthMenu\": [[25, 50, 100, 200, -1],[25, 50, 100, 200, \"All\"]],";
		sOutput << "\"iDisplayLength\": 100";
		sOutput << "} );";
		sOutput << "} );</script>";
	}

	std::string real_function(struct_gamestatus *gamecontainer, std::string sFunctionName, const incoming_things& incoming, outgoing_things& outgoing)
	{
		Battlefield::game *g = gamecontainer->g;
		ostringstream sOutput;

		if (sFunctionName.empty())
			return ("<head><meta http-equiv=\"refresh\" content=\"0; URL=p_list\"></head>");

		if (gamecontainer->state != Battlefield::gamestate::INACTIVE)
		{
			Database::SQLite *sql = g->getSQLiteConnection();

			if (sFunctionName[0] == '?')
			{
				if (sFunctionName == "?banlist")
				{
					std::vector<Database::ban> banlist = sql->banlist();

					sOutput << "{\"banlist\":[";

					for (std::vector<Database::ban>::const_iterator it = banlist.begin(); it != banlist.end(); ++it) 
					{
						sOutput << "{";
						sOutput << "\"banid\":\"" << (*it).uiBanID << "\",";
						sOutput << "\"guid\":\"" << (*it).sGuID << "\",";
						sOutput << "\"name\":\"" << (*it).sName << "\",";
						sOutput << "\"admin\":\"" << (*it).sAdmin << "\",";
						sOutput << "\"reason\":\"" << (*it).sReason << "\",";
						sOutput << "\"date\":\"" << (*it).sDate << "\"";
						sOutput << "},";
					}
					std::string str = sOutput.str();
					str.erase(str.length()-1);
					str += "]}";

					banlist.clear();
					return (str);
				}

				return (sOutput.str());
			}

			std::string sIncomingQueryPassword = incoming.queries["pass"], sIncomingCookiePassword = incoming.cookies["pass"];
			std::string sIncomingQueryUser = incoming.queries["user"], sIncomingCookieUser = incoming.cookies["user"];

//#if defined _WIN32 || defined _WIN64
			if (!sIncomingQueryPassword.empty())
				helper::security::getCleanWord(sIncomingQueryPassword);
			if (!sIncomingCookiePassword.empty())
				helper::security::getCleanWord(sIncomingCookiePassword);
			if (!sIncomingQueryUser.empty())
				helper::security::getCleanWord(sIncomingQueryUser);
			if (!sIncomingCookieUser.empty())
				helper::security::getCleanWord(sIncomingCookieUser);
//#endif
			std::string user_Name, user_Pwd;

			if (sIncomingCookieUser.empty() || !sIncomingQueryUser.empty())
			{
				user_Name = sIncomingQueryUser;
			}
			else
			{
				user_Name = sIncomingCookieUser;
			}

			user_Name = dlib::tolower(user_Name);
			user_Name = dlib::rtrim(user_Name);
			if (!sql->getAdminLevel(user_Name))
				return (sLogin);

			outgoing.cookies["user"] = user_Name;

			if (sIncomingCookiePassword.empty() || !sIncomingQueryPassword.empty())
			{
				user_Pwd = sIncomingQueryPassword;
			}
			else
			{
				user_Pwd = sIncomingCookiePassword;
			}
		
			user_Pwd = dlib::rtrim(user_Pwd);
			std::string adminPwd = sql->getAdminPassword(user_Name);
			if (adminPwd.empty() || adminPwd != dlib::md5(user_Pwd))
				return (sLogin);

			outgoing.cookies["pass"] = user_Pwd;

			unsigned int level = sql->getAdminLevel(user_Name);

			if (sFunctionName[0] != '!')
			{
				sOutput << sHeader_1;
				ajax_popup(sOutput);
				sOutput << sHeader_2;
				sOutput << "Welcome " << user_Name << " | <a href=\"serverlist\" class=\"serverlist\">Serverlist</a>";
				sOutput << sHeader_3;
			}
		
			int positionOfExtendedCharacter = sFunctionName.find('?');
			if (positionOfExtendedCharacter)
				sFunctionName = sFunctionName.substr(0, positionOfExtendedCharacter);

			if (sFunctionName.size() >= 2)
			{
				if (sFunctionName[0] == '!')
				{
					if (sFunctionName == "!livechat")
					{
						std::list<Battlefield::struct_chat> chatlist = g->getChatlist();
						sOutput << "<div id='ajax_livechat'>";
						for (std::list<Battlefield::struct_chat>::const_iterator it = chatlist.begin(); it != chatlist.end(); ++it)
							sOutput << "<dfn id='ajax_livechat_time'>" << (*it).sTime << "</dfn> &lt;<a OnClick=\"javascript:$('<div />').load('!p_administrate?name=" << (*it).sName << "').dialog({title:'" << (*it).sName << "',show:'clip',hide:'clip',width:500});\" href=\"#\">" << (*it).sName << "</a>" << ((*it).bMessageToTeam ? ((*it).cIsOnTeam == 2 ? "(<font color='#FFDDDD'>RU</font>)" : "(<font color='#DDDDFF'>US</font>)") : "") << "&gt; <i>" << (*it).sMessage << "</i><br/>";
						sOutput << "</div>";
						return (sOutput.str());
					}
					else if (sFunctionName == "!p_list")
					{
						sOutput << "<table class=\"border\" style=\"margin:15px;width:40%;float:left;background-color: #ffdddd;\" cellpadding=\"0\" cellspacing=\"0\">";
						sOutput << "<tr>";
						//sOutput << "<td class=\"thead\"></td>";
						sOutput << "<td class=\"thead\">Name</td>";
						sOutput << "<td class=\"thead\">Score</td>";
						sOutput << "<td class=\"thead\">Squad</td>";
						sOutput << "<td class=\"thead\">Ping</td>";
						sOutput << "<td class=\"thead\">Admin</td>";
						sOutput << "</tr>";

						for (int i=0; i<g->count.ucPlayer; i++)
						{ 
							if (g->pPlayerlist[i].team == 2)
							{
								sOutput << "<tr>";

								//std::string country = GeoIP::GetCountryCodeUsingURL(g->pPlayerlist[i].ip);
								//sOutput << "<td><img src=\"blank.png\" class=\"flag flag-" << country << "\" alt=\"" << country << "\"/></td>";
								sOutput << (g->pPlayerlist[i].spawned ? "<td>" : "<td style=\"text-decoration:line-through\">") << g->pPlayerlist[i].name << "</td>";
								sOutput << "<td>" << g->pPlayerlist[i].score << "</td>";
								sOutput << "<td style=\"background-color: " << SQUADCOLORASHEX[g->pPlayerlist[i].squad] << "\">" << SQUADNAME[g->pPlayerlist[i].squad] << "</td>";
								sOutput << "<td>" << g->pPlayerlist[i].ping << "</td>";
								sOutput << "<td><a OnClick=\"javascript:$('<div />').load('!p_administrate?name=" << g->pPlayerlist[i].name << "').dialog({title:'" << g->pPlayerlist[i].name << "|" << g->pPlayerlist[i].guid << "',show:'clip',hide:'clip',width:500});\" href=\"#\">Admin</a></td>";
								
								sOutput << "</tr>";
							}
						}
						sOutput << "</table>";
						
						sOutput << "<table class=\"border\" style=\"margin:15px;width:40%;float:right;background-color: #ddddff\" cellpadding=\"0\" cellspacing=\"0\">";
						sOutput << "<tr>";
						//sOutput << "<td class=\"thead\"></td>";
						sOutput << "<td class=\"thead\">Name</td>";
						sOutput << "<td class=\"thead\">Score</td>";
						sOutput << "<td class=\"thead\">Squad</td>";
						sOutput << "<td class=\"thead\">Ping</td>";
						sOutput << "<td class=\"thead\">Admin</td>";
						sOutput << "</tr>";

						for (int i=0; i<g->count.ucPlayer; i++)
						{ 
							if (g->pPlayerlist[i].team == 1)
							{
								sOutput << "<tr>";

								//std::string country = GeoIP::GetCountryCodeUsingURL(g->pPlayerlist[i].ip);
								//sOutput << "<td><img src=\"blank.png\" class=\"flag flag-" << country << "\" alt=\"" << country << "\"/></td>";
								sOutput << (g->pPlayerlist[i].spawned ? "<td>" : "<td style=\"text-decoration:line-through\">") << g->pPlayerlist[i].name << "</td>";
								sOutput << "<td>" << g->pPlayerlist[i].score << "</td>";
								sOutput << "<td style=\"background-color: " << SQUADCOLORASHEX[g->pPlayerlist[i].squad] << "\">" << SQUADNAME[g->pPlayerlist[i].squad] << "</td>";
								sOutput << "<td>" << g->pPlayerlist[i].ping << "</td>";
								sOutput << "<td><a OnClick=\"javascript:$('<div />').load('!p_administrate?name=" << g->pPlayerlist[i].name << "').dialog({title:'" << g->pPlayerlist[i].name << "|" << g->pPlayerlist[i].guid << "',show:'clip',hide:'clip',width:500});\" href=\"#\">Admin</a></td>";

								sOutput << "</tr>";
							}
						}
						sOutput << "</table>";
					}
					else if (sFunctionName == "!s_vargetdb")
					{
						sOutput << "Server #1 Bandar Desert!";
					}
					else if (sFunctionName == "!s_vargetgs")
					{
						sOutput << "Server #1 Bandar Desert!";
					}
					else if (sFunctionName == "!restrictweapon")
					{
						g->restrictWeapon(incoming.queries["name"]);
					}
					else if (sFunctionName == "!unrestrictweapon")
					{
						g->unRestrictWeapon(incoming.queries["name"]);
					}
					else if (sFunctionName == "!restrictweaponminplayers")
					{
						std::string value = incoming.queries["value"];
						if (!value.empty())
						{
							g->settings->cMinPlayerCountForRestrictedWeaponPunish = dlib::string_cast<unsigned short>(value);
							sql->updateServerSetting("minplayersforresweaponpunish", value);
						}
					}
					else if (sFunctionName == "!s_varset")
					{
						if (incoming.queries["value"] == "")
							g->sendSimpleCmd(incoming.queries["var"]);
						else
							g->sendSimpleCmdWithValue(incoming.queries["var"], incoming.queries["value"]);
					}
					else if (sFunctionName == "!p_administrate")
					{
						if (!incoming.queries["name"].empty())
						{
							if (!incoming.queries["func"].empty())
							{
								if (incoming.queries["func"] == "kick")
								{
									if (incoming.queries["reason"].empty())
										g->player_kick(incoming.queries["name"], g->settings->sDefaultpunishedmsg);
									else
										g->player_kick(incoming.queries["name"], incoming.queries["reason"]);
								}
								else if (incoming.queries["func"] == "kill")
								{
									g->player_kill(incoming.queries["name"]);
									if (!incoming.queries["reason"].empty())
										g->yell(incoming.queries["reason"], "player", incoming.queries["name"]);
								}
								else if (incoming.queries["func"] == "move")
								{
									g->player_move(incoming.queries["name"], incoming.queries["team"] == "1" ? 1 : 2, 0, incoming.queries["force"] == "1" ? true : false);
								}
								else if (incoming.queries["func"] == "msg" && !incoming.queries["msg"].empty())
								{
									if (incoming.queries["yell"] == "1")
										g->yell(incoming.queries["msg"], "player", incoming.queries["name"]);
									else
										g->say(incoming.queries["msg"], "player", incoming.queries["name"]);
								}
								else if (incoming.queries["func"] == "ban" && !incoming.queries["guid"].empty() && !incoming.queries["reason"].empty())
								{
									g->player_ban(incoming.queries["guid"], incoming.queries["name"], user_Name, incoming.queries["reason"]);
								}

								outgoing.headers["Refresh"] = "1; url=p_list\r\n";
								sOutput << "Redirecting...";
							}
							else
							{
								std::string sGuid = "";
								unsigned char ucTeam = 0;
								for (int i=0; i<g->count.ucPlayer; i++)
								{
									if (g->pPlayerlist[i].name == incoming.queries["name"])
									{
										sGuid = g->pPlayerlist[i].guid;
										ucTeam = g->pPlayerlist[i].team;

										break;
									}
								}

								if (ucTeam == 0)
								{
									sOutput << jQueryAlert("There is no player named <b>" + incoming.queries["name"] + "</b>");
								}
								else
								{
									// Kick Player
									sOutput << "<p><b>KICK PLAYER</b><br/><form class=\"niceform\" action='!p_administrate' method='get'><input type='hidden' name='func' value='kick'>"
											<< "<input type='hidden' name='name' value='" << incoming.queries["name"] << "'>Reason: <input name='reason' type='text'>"
											<< "<input type=\"submit\" value=\"Kick Player\" style='float:right'></form></p>";

									// Kill Player
									sOutput << "<p><b>KILL PLAYER</b><br/><form class=\"niceform\" action='!p_administrate' method='get'><input type='hidden' name='func' value='kill'>"
											<< "<input type='hidden' name='name' value='" << incoming.queries["name"] << "'>Reason: <input name='reason' type='text'>"
											<< "<input type=\"submit\" value=\"Kill Player\" style='float:right'></form></p>";

									// Move Player
									sOutput << "<p><b>MOVE PLAYER</b><br/><form class=\"niceform\" action='!p_administrate' method='get'><input type='hidden' name='func' value='move'>"
											<< "<input type='hidden' name='name' value='" << incoming.queries["name"] << "'>"
											<< "<input type='hidden' name='team' value='" << ucTeam << "'>"
											<< "Force<input type=\"checkbox\" name=\"force\" value=\"1\">"
											<< "<input type=\"submit\" value=\"Move Player\" style='float:right'></form></p>";

									// Ban Player
									sOutput << "<p><b>BAN PLAYER</b><br/><form class=\"niceform\" action='!p_administrate' method='get'><input type='hidden' name='func' value='ban'>"
											<< "<input type='hidden' name='guid' value='" << sGuid << "'>"
											<< "<input type='hidden' name='name' value='" << incoming.queries["name"] << "'>Reason: <input name='reason' type='text'>"
											// << "Bantime: <input name='time' type='text' value='0'>"
											<< "<input type=\"submit\" value=\"Ban Player\" style='float:right'></form></p>";

									// Contact Player
									sOutput << "<p><b>CONTACT PLAYER</b><br/><form class=\"niceform\" action='!p_administrate' method='get'><input type='hidden' name='func' value='msg'>"
											<< "<input type='hidden' name='name' value='" << incoming.queries["name"] << "'>Message: <input name='msg' type='text'><br/>"
											<< "Yell<input type=\"checkbox\" name=\"yell\" value=\"1\">"
											<< "<input type=\"submit\" value=\"Tell Player\" style='float:right'></form></p>";

									// Make Admin
									sOutput << "<p><b>MAKE ADMIN</b><br/><form class=\"niceform\" action='a_add' method='get'>"
											<< "<input type='hidden' name='name' value='" << incoming.queries["name"] << "'>"
											<< "<input type='hidden' name='eaID' value='" << sGuid << "'>"
											<< "Adminlevel:<select name=\"level\"><option value=\"1\">BadAss</option><option value=\"2\">Super BadAss</option></select>"
											<< "</br><input type=\"submit\" value=\"Add Admin\" style='float:right'></form></p>";

									// Add to Whitelist
									sOutput << "<p><b>ADD TO WHITELIST</b><br/><form class=\"niceform\" action='p_addwhite' method='get'>"
											<< "<input type='hidden' name='name' value='" << incoming.queries["name"] << "'>"
											<< "</br><input type=\"submit\" value=\"Add to Whitelist\" style='float:right'></form></p>";
								}
							}
						}
					}

					return (sOutput.str());
				}
				else if (sFunctionName[0] == 'p')
				{
					if (sFunctionName[2] == 'l')
					{
						sOutput << "<h2>PLAYERLIST</h2>";
						sOutput << "<script>var auto_refresh = setInterval(function(){$('#ajax_playerlist').fadeOut('fast').load('!p_list').fadeIn(\"fast\");}, 10000);$(document).ready(function() {$('#ajax_playerlist').hide().load('!p_list').fadeIn(\"slow\")});</script>"
								<< "<div id=\"ajax_playerlist\">Loading Playerlist...</div>";
					}
					else if (sFunctionName[2] == 'b')
					{
						std::vector<Database::ban> banlist = sql->banlist();

						sOutput << "<h2>BANLIST</h2>";

						sOutput << "<table id=\"table\" class=\"datatable\" style=\"width:100%;font-size:90%\" cellpadding=\"0\" cellspacing=\"0\">";
						sOutput << "<thead>";
						sOutput << "<tr>";
						sOutput << "<th>ID</th>";
						sOutput << "<th>EAID</th>";
						sOutput << "<th>Name</th>";
						sOutput << "<th>Admin</th>";
						sOutput << "<th>Reason</th>";
						sOutput << "<th>Date</th>";
						sOutput << "<th></th>";
						sOutput << "</tr>";
						sOutput << "</thead>";
						sOutput << "<tbody>";

						for (std::vector<Database::ban>::const_iterator it = banlist.begin(); it != banlist.end(); ++it) 
						{
							sOutput << "<tr>";
							sOutput << "<td>" << (*it).uiBanID << "</td>";
							sOutput << "<td style= \"font-size:75%\">" << (*it).sGuID << "</td>";
							sOutput << "<td>" << (*it).sName << "</td>";
							sOutput << "<td>" << (*it).sAdmin << "</td>";
							sOutput << "<td>" << (*it).sReason << "</td>";
							sOutput << "<td>" << (*it).sDate << "</td>";
							sOutput << "<td><a class=\"ask\" href=\"p_deleteban?guid=" << (*it).uiBanID << "\"><span class=\"ui-icon ui-icon-trash\"></span></a></td>";
							sOutput << "</tr>";
						}

						sOutput << "</tbody></table>";

						banlist.clear();

						sOutput << "<div style=\"margin-top: 20px;\"><h2>Add a Ban</h2><form class=\"niceform\" action=\"p_addban\" method=\"post\">"
								<< "<fieldset>"
								<< "<dl>"
								<< "<dt><label for=\"name\">Name of Player:</label></dt>"
								<< "<dd><input type=\"text\" name=\"name\" size=\"32\"/></dd>"
								<< "</dl>"
								<< "<dl>"
								<< "<dt><label for=\"guid\">Ea#:</label></dt>"
								<< "<dd><input type=\"text\" name=\"guid\" size=\"32\"/></dd>"
								<< "</dl>"
								<< "<dl>"
								<< "<dt><label for=\"reason\">Reason:</label></dt>"
								<< "<dd><input type=\"text\" name=\"reason\" size=\"32\"/></dd>"
								<< "</dl>"
								<< "<dl class=\"submit\">"
								<< "<input type=\"submit\" name=\"submit\" value=\"Add\"/>"
								<< "</dl>"
								<< "</fieldset>"
								<< "</form></div>"
								<< "You can also clear the gameserver-banlist. This will erase EVERY ban, that is not stored in the list above.<br/><a href=\"p_updatebanlist\">Click here to use this function and resync all bans.</a>";
					}
					else if (sFunctionName == "p_deleteban")
					{
						if (!incoming.queries["guid"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=p_banlist\r\n";
							g->player_unban(incoming.queries["guid"]);
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "p_addban")
					{
						if (!incoming.queries["name"].empty() && !incoming.queries["guid"].empty() && !incoming.queries["reason"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=p_banlist\r\n";
							g->player_ban(incoming.queries["guid"], incoming.queries["name"], user_Name, incoming.queries["reason"]);
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "p_updatebanlist")
					{
						outgoing.headers["Refresh"] = "0; url=p_banlist\r\n";
						g->updateBanlist();
						sOutput << "Redirecting...";
					}
					else if (sFunctionName == "p_whitelist")
					{
						std::vector<std::string> whitelist = sql->whitelist();

						sOutput << "<h2>WHITELIST</h2>";
						sOutput <<  jQueryAlert("Admins are also listed here, but cannot be deleted!");

						sOutput << "<table id=\"table\" class=\"datatable\" style=\"width:100%\" cellpadding=\"0\" cellspacing=\"0\">";
						sOutput << "<thead>";
						sOutput << "<tr>";
						sOutput << "<th>Name</th>";
						sOutput << "<th>Options</th>";
						sOutput << "</tr>";
						sOutput << "</thead>";
						sOutput << "<tbody>";

						for (std::vector<std::string>::const_iterator it = whitelist.begin(); it != whitelist.end(); ++it) 
						{
							sOutput << "<tr>";
							sOutput << "<td>" << (*it) << "</td>";
							sOutput << "<td><a class=\"ask\" href=\"p_deletewhite?name=" << (*it) << "\"><span class=\"ui-icon ui-icon-trash\"></a></td>";
							sOutput << "</tr>";
						}
						sOutput << "</tbody></table>";

						whitelist.clear();

						sOutput << "<div style=\"margin-top: 20px;\"><h2>Insert into Whitelist</h2><form class=\"niceform\" action=\"p_addwhite\" method=\"post\">"
								<< "<fieldset>"
								<< "<dl>"
								<< "<dt><label for=\"name\">Name of Player:</label></dt>"
								<< "<dd><input type=\"text\" name=\"name\" size=\"32\"/></dd>"
								<< "</dl>"
								<< "<dl class=\"submit\">"
								<< "<input type=\"submit\" name=\"submit\" value=\"Add\"/>"
								<< "</dl>"
								<< "</fieldset>"
								<< "</form></div>";
					}
					else if (sFunctionName == "p_deletewhite")
					{
						if (!incoming.queries["name"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=p_whitelist\r\n";
							sql->unwhite(incoming.queries["name"]);
							g->updateWhitelist();
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "p_addwhite")
					{
						if (!incoming.queries["name"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=p_whitelist\r\n";
							sql->addwhite(incoming.queries["name"]);
							g->updateWhitelist();
							sOutput << "Redirecting...";
						}
					}
				}
				else if (sFunctionName[0] == 'a')
				{
					if (sFunctionName == "a_list")
					{
						std::vector<Database::admin> adminlist = sql->adminlist();

						sOutput << "<h2>ADMINLIST</h2>";

						sOutput << "<table id=\"table\" class=\"datatable\" style=\"width:100%\" cellpadding=\"0\" cellspacing=\"0\">";
						sOutput << "<thead>";
						sOutput << "<tr>";
						sOutput << "<th>ID</th>";
						sOutput << "<th>Name</th>";
						sOutput << "<th>EaID</th>";
						sOutput << "<th>Level</th>";
						sOutput << "<th>Options</th>";
						sOutput << "</tr>";
						sOutput << "</thead>";
						sOutput << "<tbody>";

						for(std::vector<Database::admin>::const_iterator it = adminlist.begin(); it != adminlist.end(); ++it) 
						{
							if ((*it).ucLevel == 1)
								sOutput << "<tr style=\"background-color: #ddffdd;\">";
							else if ((*it).ucLevel == 2)
								sOutput << "<tr style=\"background-color: #ccffcc;\">";
							else
								sOutput << "<tr>";

							sOutput << "<td>" << (*it).uiAdminID << "</td>";
							sOutput << "<td>" << (*it).sName << "</td>";
							
							std::string guid = (*it).sGuID;
							guid.replace(7, 15, "...");
							sOutput << "<td>" << guid << "</td>";

							if (level == ADMIN_LEVEL_SUPERBADASS)
							{
								sOutput << "<td><form name=\"input\" action=\"a_edit\" method=\"get\">"
								<< "<input type=\"hidden\" name=\"adminID\" value=\"" << (*it).uiAdminID << "\">"
								<< "<select size=\"1\" name=\"level\">"
								<< "<option value=\"1\" " << ((*it).ucLevel == 1 ? "selected" : "") << ">Badass</option>"
								<< "<option value=\"2\" " << ((*it).ucLevel == 2 ? "selected" : "") << ">Super Badass</option>"
								<< "</select>"

								<< "<button class=\"ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only\" role=\"button\" aria-disabled=\"false\" title=\"Save changes\">"
								<< "<span class=\"ui-button-icon-primary ui-icon ui-icon-circle-check\"></span>"
								<< "<span class=\"ui-button-text\">Save changes</span></button>"
								<< "</form></td>";

								sOutput << "<td><a class=\"ask\" href=\"a_delete?adminID=" << (*it).uiAdminID << "\"><span class=\"ui-icon ui-icon-trash\"></a></td>";
							}
							else
							{
								if ((*it).ucLevel == ADMIN_LEVEL_SUPERBADASS)
									sOutput << "<td>SUPERBADASS</td>";
								else if ((*it).ucLevel == ADMIN_LEVEL_BADASS)
									sOutput << "<td>BADASS</td>";

								sOutput << "<td></td>";
							}
							sOutput << "</tr>";
						}
						sOutput << "</tbody></table>";

						adminlist.clear();

						sOutput << "<div style=\"margin-top: 20px;\"><h2>Add BadAss (Admin)</h2><form class=\"niceform\" action=\"a_add\" method=\"post\">"
								<< "<fieldset>"
								<< "<dl>"
								<< "<dt><label for=\"name\">Name of Admin:</label></dt>"
								<< "<dd><input type=\"text\" name=\"name\" size=\"32\"/></dd>"
								<< "</dl>"
								<< "<dl>"
								<< "<dt><label for=\"eaID\">EA#:</label></dt>"
								<< "<dd><input type=\"text\" name=\"eaID\" size=\"54\"/></dd>"
								<< "</dl>"
								<< "<dl>"
								<< "<dt><label for=\"name\">Adminlevel:</label></dt>"
								<< "<dd>"
								<< "<select size=\"1\" name=\"level\">"
								<< "<option value=\"1\">Badass</option>"
								<< "<option value=\"2\">Super Badass</option>"
								<< "</select>"
								<< "</dd>"
								<< "</dl>"
								<< "<dl class=\"submit\">"
								<< "<input type=\"submit\" name=\"submit\" value=\"Add\"/>"
								<< "</dl>"
								<< "</fieldset>"
								<< "</form></div>";
					}
					else if (sFunctionName == "a_delete")
					{
						if (!incoming.queries["adminID"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=a_list\r\n";
							sql->deleteAdmin(incoming.queries["adminID"]);
							g->updateWhitelist();
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "a_add")
					{
						if (!incoming.queries["name"].empty() && !incoming.queries["eaID"].empty() && !incoming.queries["level"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=a_list\r\n";
							sql->addAdmin(incoming.queries["eaID"], incoming.queries["name"], dlib::string_cast<unsigned short>(incoming.queries["level"]));
							g->updateWhitelist();
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "a_edit")
					{
						if (!incoming.queries["adminID"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=a_list\r\n";
							sql->updateAdminLevel(incoming.queries["adminID"], incoming.queries["level"]);
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "a_payments")
					{
						std::vector<Database::admin> adminlist = sql->adminlist();

						sOutput << "<h2>PAYMENTS</h2>";

						sOutput << "<table id=\"table\" class=\"datatable\" style=\"width:100%\" cellpadding=\"0\" cellspacing=\"0\">";
						sOutput << "<thead>";
						sOutput << "<tr>";
						sOutput << "<th>AdminID</th>";
						sOutput << "<th>Name</th>";
						sOutput << "<th>Next Payday</th>";
						sOutput << "<th>Days left</th>";
						sOutput << "</tr>";
						sOutput << "</thead>";
						sOutput << "<tbody>";

						for (std::vector<Database::admin>::const_iterator it = adminlist.begin(); it != adminlist.end(); ++it) 
						{
							sOutput << "<tr>";
							sOutput << "<td>" << (*it).uiAdminID << "</td>";
							sOutput << "<td>" << (*it).sName << "</td>";
							if (level == ADMIN_LEVEL_SUPERBADASS)
							{
								sOutput << "<td><form name=\"input\" action=\"a_payments_edit\" method=\"post\">"
										<< "<input type=\"hidden\" name=\"adminID\" value=\"" << (*it).uiAdminID << "\">"
										<< "<input type=\"text\" name=\"payday\" class=\"datepicker\" style=\"border: none;\" value=\"" << (*it).sNextPaymentDate << "\" placeholder=\"Click me!\">"

										<< "<button class=\"ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only\" role=\"button\" aria-disabled=\"false\" title=\"Save changes\">"
										<< "<span class=\"ui-button-icon-primary ui-icon ui-icon-circle-check\"></span>"
										<< "<span class=\"ui-button-text\">Save changes</span></button>"
										<< "<button name=\"remove\" class=\"ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only\" role=\"button\" aria-disabled=\"false\" title=\"Remove check completely\">"
										<< "<span class=\"ui-button-icon-primary ui-icon ui-icon-trash\"></span>"
										<< "<span class=\"ui-button-text\">Remove check completely</span></button>"
										<< "</form>"
										<< "</td>";
							}
							else
							{
								sOutput << "<td>" << (*it).sNextPaymentDate << "</td>";
							}

							double diff = 0;
							std::string date = (*it).sNextPaymentDate;
							if (!date.empty())
							{
								std::vector<std::string> time = dlib::split(date, "/");
								diff = helper::timeFromNow(dlib::string_cast<unsigned short>(time[1]), dlib::string_cast<unsigned short>(time[0]), dlib::string_cast<unsigned short>(time[2]));
							}

							if (diff > 0)
								sOutput << "<td style=\"background-color: #ffbbbb;\">Unpaid for " + dlib::cast_to_string(diff) + " days</td>";
							else if (diff < 0)
								sOutput << "<td style=\"background-color: #bbffbb;\">Paid for " + dlib::cast_to_string(diff*(-1)) + " days</td>";
							else
								sOutput << "<td style=\"background-color: #ffffbb;\">Today</td>";
							sOutput << "</tr>";
						}

						sOutput << "</tbody></table>";

						adminlist.clear();
					}
					else if (sFunctionName == "a_payments_edit")
					{
						if (!incoming.queries["adminID"].empty() || !incoming.queries["payday"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=a_payments\r\n";
							sql->updateAdminPayday(incoming.queries["adminID"], incoming.queries.count("remove") ? "" : incoming.queries["payday"]);
							sOutput << "Redirecting...";
						}
					}
				}
				else if (sFunctionName[0] == 's')
				{
					if (sFunctionName == "s_vars")
					{
						sOutput << "<h2>VARS CONFIGURATION</h2>";

						generateStringVarCode(sOutput, "serverName");
						generateStringVarCode(sOutput, "serverMessage");
						generateStringVarCode(sOutput, "serverDescription");
						generateStringVarCode(sOutput, "gamePassword");
						generateBooleanVarCode(sOutput, "ranked"); // Bool
						generateBooleanVarCode(sOutput, "autoBalance"); // Bool
						generateStringVarCode(sOutput, "roundStartPlayerCount"); // Int
						generateStringVarCode(sOutput, "roundRestartPlayerCount"); // Int
						generateStringVarCode(sOutput, "roundLockdownCountdown"); // Int
						generateBooleanVarCode(sOutput, "friendlyFire"); // Bool
						generateStringVarCode(sOutput, "maxPlayers"); // Int
						generateBooleanVarCode(sOutput, "killCam"); // Bool
						generateBooleanVarCode(sOutput, "miniMap"); // Bool
						generateBooleanVarCode(sOutput, "hud"); // Bool
						generateBooleanVarCode(sOutput, "crossHair"); // Bool
						generateBooleanVarCode(sOutput, "3dSpotting"); // Bool
						generateBooleanVarCode(sOutput, "miniMapSpotting"); // Bool
						generateBooleanVarCode(sOutput, "nameTag"); // Bool
						generateBooleanVarCode(sOutput, "3pCam"); // Bool
						generateBooleanVarCode(sOutput, "regenerateHealth"); // Bool
						generateStringVarCode(sOutput, "teamKillCountForKick"); // Int
						generateStringVarCode(sOutput, "teamKillValueForKick"); // Int
						generateStringVarCode(sOutput, "teamKillValueIncrease"); // Int
						generateStringVarCode(sOutput, "teamKillValueDecreasePerSecond"); // Int
						generateStringVarCode(sOutput, "teamKillKickForBan"); // Int
						generateStringVarCode(sOutput, "idleTimeout"); // Int
						generateBooleanVarCode(sOutput, "idleBanRounds"); // Bool
						generateBooleanVarCode(sOutput, "vehicleSpawnAllowed"); // Bool
						generateStringVarCode(sOutput, "vehicleSpawnDelay"); // Int
						generateStringVarCode(sOutput, "soldierHealth"); // Int
						generateStringVarCode(sOutput, "playerRespawnTime"); // Int
						generateStringVarCode(sOutput, "playerManDownTime"); // Int
						generateStringVarCode(sOutput, "bulletDamage"); // Int
						generateStringVarCode(sOutput, "gameModeCounter"); // Int
						generateBooleanVarCode(sOutput, "onlySquadLeaderSpawn"); // Bool
						generateStringVarCode(sOutput, "unlockMode"); // Int
						generateBooleanVarCode(sOutput, "premiumStatus"); // Bool
						generateStringVarCode(sOutput, "bannerUrl");
						generateStringVarCode(sOutput, "roundsPerMap");
						generateStringVarCode(sOutput, "gunMasterWeaponsPreset"); // Int
					}
					else if (sFunctionName == "serverlist")
					{
						Battlefield::gamestate::state state;
						sOutput << "<h2>SERVERLIST</h2>";
						sOutput << "<table id=\"rounded-corner\" summary=\"Serverlist\">"
								<< "<thead>"
								<< "<tr>"
								<< "<th scope=\"col\" class=\"rounded-company\">Address</th>"
								<< "<th scope=\"col\" class=\"rounded\">Port</th>"
								<< "<th scope=\"col\" class=\"rounded\">Playercount</th>"
								<< "<th scope=\"col\" class=\"rounded\">Status</th>"
								<< "<th scope=\"col\" class=\"rounded\">Administrate</th>"
								<< "</tr>"
								<< "</thead>"
								<< "<tfoot>"
								<< "<tr>"
								<< "<td colspan=\"6\" class=\"rounded-foot-left\"><em>This is still a work in progress!!!</em></td>"
								<< "<td class=\"rounded-foot-right\">&nbsp;</td>"
								<< "</tr>"
								<< "</tfoot>"
								<< "<tbody>";

						int i=0;
						for (std::vector<struct_gamestatus*>::const_iterator it = gGameServers.begin(); it != gGameServers.end(); ++it) 
						{
							if ((*it)->g->settings->sOwner == g->settings->sOwner)
							{
								state = (*it)->state;
								sOutput << "<tr><td>" << (*it)->g->settings->sBF3_Server_Host << "</td>";
								sOutput << "<td>" << (*it)->g->settings->usBF3_Server_Port << "</td>";
								sOutput << "<td>" << static_cast<int>((*it)->g->count.ucPlayer) << "</td>";
								if (state == Battlefield::gamestate::ACTIVE)
								{
									sOutput << "<td>Server is running</td>";
								}
								else if (state == Battlefield::gamestate::EMPTY)
								{
									sOutput << "<td>Server is running, but empty</td>";
								}
								else
								{
									sOutput << "<td><img src=\"images/user_logout.png\" alt=\"\" title=\"\" border=\"0\" />" << (*it)->sError << "</td>";
								}
								sOutput << "<td><a href=\"../-" << (*it)->name << "/\">Administrate</a></td>";
								sOutput << "</tr>";
							}

							++i;
						}
						sOutput << "</tbody></table>";
					}
				}
				else if (sFunctionName[0] == 'e')
				{
					if (sFunctionName == "e_weaponrestrict")
					{
						std::vector<Database::message> advlist = sql->advlist();

						sOutput << read_file_html("restrict.html");

						sOutput << "<p><label for=\"spinner\">Minimum playercount: </label><input id=\"spinner\" name=\"spinner\" value=\"" 
								<< dlib::cast_to_string(static_cast<unsigned short>(g->settings->cMinPlayerCountForRestrictedWeaponPunish)) << "\" /></p>";

						sOutput << "<div class=\"ui-widget ui-helper-clearfix\"><div id=\"locked\" class=\"ui-widget-content ui-state-default\">"
								<< "<h4 class=\"ui-widget-header\"><span class=\"ui-icon ui-icon-locked\">Restricted</span> Restricted</h4><ul class=\"gallery ui-helper-reset\"/>";

						for (std::vector<std::string>::const_iterator it = g->restrictedWeapons.begin(); it != g->restrictedWeapons.end(); ++it) 
						{
							sOutput << "<li class=\"ui-widget-content ui-corner-tr\"><h5 class=\"ui-widget-header\">" << (*it) << "</h5>";
							sOutput << "<a href=\"link/to/locked/script/when/we/have/js/off\" title=\"Unrestrict this weapon\" class=\"ui-icon ui-icon-unlocked\">Unrestrict weapon</a></li>";
						}
						
						sOutput << "</ul></div><ul id=\"gallery\" class=\"gallery ui-helper-reset ui-helper-clearfix\">";

						for (int i=0; i<Battlefield::Weapons::maxWeapons; i++)
						{
							/* With Image
							sOutput << "<li class=\"ui-widget-content ui-corner-tr\"><h5 class=\"ui-widget-header\">" << Battlefield::Weapons::names[i] << "</h5>";
							sOutput << "<div class=\"weapon medium m98b_lineart \"></div>";
							sOutput << "<a href=\"link/to/locked/script/when/we/have/js/off\" title=\"Restrict this weapon\" class=\"ui-icon ui-icon-locked\">Restrict weapon</a></li>";
							*/

							if (!(std::find(g->restrictedWeapons.begin(), g->restrictedWeapons.end(), Battlefield::Weapons::names[i]) != g->restrictedWeapons.end()))
							{
								sOutput << "<li class=\"ui-widget-content ui-corner-tr\"><h5 class=\"ui-widget-header\">" << Battlefield::Weapons::names[i] << "</h5>";
								sOutput << "<a href=\"link/to/locked/script/when/we/have/js/off\" title=\"Restrict this weapon\" class=\"ui-icon ui-icon-locked\">Restrict weapon</a></li>";
							}
						}

						sOutput << "</ul></div>";
					}
					else if (sFunctionName == "e_adv")
					{
						std::vector<Database::message> advlist = sql->advlist();

						sOutput << "<h2>ADV MESSAGES</h2>";

						sOutput << "<table id=\"table\" class=\"datatable\" style=\"width:100%\" cellpadding=\"0\" cellspacing=\"0\">";
						sOutput << "<thead>";
						sOutput << "<tr>";
						sOutput << "<th>AdvID</th>";
						sOutput << "<th>Message</th>";
						sOutput << "<th>Yell</th>";
						sOutput << "<th>Options</th>";
						sOutput << "</tr>";
						sOutput << "</thead>";
						sOutput << "<tbody>";

						for (std::vector<Database::message>::const_iterator it = advlist.begin(); it != advlist.end(); ++it) 
						{
							sOutput << "<tr>";
							sOutput << "<td>" << (*it).uiMsgID << "</td>";
							sOutput << "<td>" << (*it).sMsg << "</td>";

							if ((*it).bYell)
								sOutput << "<td><span class=\"ui-icon ui-icon-check\"></td>";
							else
								sOutput << "<td></td>";

							sOutput << "<td><a class=\"ask\" href=\"e_deleteadv?advID=" << (*it).uiMsgID << "\"><span class=\"ui-icon ui-icon-trash\"></a></td>";
							sOutput << "</tr>";
						}
						sOutput << "</tbody></table>";

						advlist.clear();

						sOutput << "<div style=\"margin-top: 20px;\"><h2>Add new Message</h2><form class=\"niceform\" action=\"e_addadv\" method=\"post\">"
								<< "<fieldset>"
								<< "<dl>"
								<< "<dt><label for=\"message\">Message:</label></dt>"
								<< "<dd><input type=\"text\" name=\"message\" size=\"54\"/></dd>"
								<< "</dl>"
								<< "<dl>"
								<< "<dt><label></label></dt>"
								<< "<dd>"
								<< "<input type=\"checkbox\" name=\"yell\" value=\"1\"/><label class=\"check_label\">Yell to players</label>"
								<< "</dd>"
								<< "</dl>"
								<< "<dl class=\"submit\">"
								<< "<input type=\"submit\" name=\"submit\" value=\"Add\"/>"
								<< "</dl>"
								<< "</fieldset>"
								<< "</form></div>";
					}
					else if (sFunctionName == "e_deleteadv")
					{
						if (!incoming.queries["advID"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=e_adv\r\n";
							sql->deleteAdvMessage(incoming.queries["advID"]);
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "e_addadv")
					{
						if (!incoming.queries["message"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=e_adv\r\n";
							if (incoming.queries["yell"].empty())
								sql->addAdvMessage(incoming.queries["message"], "0");
							else
								sql->addAdvMessage(incoming.queries["message"], "1");
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "e_nma")
					{
						std::vector<std::string> nmalist = sql->nmalist();

						sOutput << "<h2>NOTIFY MY ANDROID</h2>";

						sOutput << "<table id=\"table\" class=\"datatable\" style=\"width:100%\" cellpadding=\"0\" cellspacing=\"0\">";
						sOutput << "<thead>";
						sOutput << "<tr>";
						sOutput << "<th width=\"80%\">API-Keys</th>";
						sOutput << "<th>Delete</th>";
						sOutput << "</tr>";
						sOutput << "</thead>";
						sOutput << "<tbody>";

						for (std::vector<std::string>::const_iterator it = nmalist.begin(); it != nmalist.end(); ++it) 
						{
							sOutput << "<tr>";
							sOutput << "<td>" << (*it) << "</td>";

							sOutput << "<td><a class=\"ask\" href=\"e_deletenma?apikey=" << (*it) << "\"><span class=\"ui-icon ui-icon-trash\"></a></td>";
							sOutput << "</tr>";
						}
						sOutput << "</tbody></table>";

						nmalist.clear();

						sOutput << "<div style=\"margin-top: 20px;\"><h2>Add NMA API-Key</h2><form class=\"niceform\" action=\"e_addnma\" method=\"post\">"
								<< "<fieldset>"
								<< "<dl>"
								<< "<dt><label for=\"apikey\">API-Key:</label></dt>"
								<< "<dd><input type=\"text\" name=\"apikey\" size=\"54\"/></dd>"
								<< "</dl>"
								<< "<dl class=\"submit\">"
								<< "<input type=\"submit\" name=\"submit\" value=\"Add\"/>"
								<< "</dl>"
								<< "</fieldset>"
								<< "</form></div>";
					}
					else if (sFunctionName == "e_deletenma")
					{
						if (!incoming.queries["apikey"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=e_nma\r\n";
							sql->deleteNmaKey(incoming.queries["apikey"]);
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "e_addnma")
					{
						if (!incoming.queries["apikey"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=e_nma\r\n";
							sql->addNmaKey(incoming.queries["apikey"]);
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "e_nmatest")
					{
						g->nma_NotificateAll("TEST!");
					}
					else if (sFunctionName == "e_mail")
					{
						std::vector<std::string> maillist = sql->mailReceiverList();

						sOutput << "<h2>NOTIFY MAIL RECEIVER(S)</h2>";

						sOutput << "<table id=\"table\" class=\"datatable\" style=\"width:100%\" cellpadding=\"0\" cellspacing=\"0\">";
						sOutput << "<thead>";
						sOutput << "<tr>";
						sOutput << "<th width=\"80%\"></th>";
						sOutput << "<th>Delete</th>";
						sOutput << "</tr>";
						sOutput << "</thead>";
						sOutput << "<tbody>";

						for (std::vector<std::string>::const_iterator it = maillist.begin(); it != maillist.end(); ++it) 
						{
							sOutput << "<tr>";
							sOutput << "<td>" << (*it) << "</td>";

							sOutput << "<td><a class=\"ask\" href=\"e_deletemailreceiver?mailaddy=" << (*it) << "\"><span class=\"ui-icon ui-icon-trash\"></a></td>";
							sOutput << "</tr>";
						}
						sOutput << "</tbody></table>";

						maillist.clear();

						sOutput << "<div style=\"margin-top: 20px;\"><h2>Add Mailreceiver</h2><form class=\"niceform\" action=\"e_addmailreceiver\" method=\"post\">"
								<< "<fieldset>"
								<< "<dl>"
								<< "<dt><label for=\"mailaddy\">Mail-Addy:</label></dt>"
								<< "<dd><input type=\"text\" name=\"mailaddy\" size=\"54\"/></dd>"
								<< "</dl>"
								<< "<dl class=\"submit\">"
								<< "<input type=\"submit\" name=\"submit\" value=\"Add\"/>"
								<< "</dl>"
								<< "</fieldset>"
								<< "</form></div>";
					}
					else if (sFunctionName == "e_deletemailreceiver")
					{
						if (!incoming.queries["mailaddy"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=e_mail\r\n";
							sql->deleteMailReceiver(incoming.queries["mailaddy"]);
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "e_addmailreceiver")
					{
						if (!incoming.queries["mailaddy"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=e_mail\r\n";
							sql->addMailReceiver(incoming.queries["mailaddy"]);
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "e_chatresponse")
					{
						std::vector<Database::struct_chatresponse> list = sql->chatResponseList();

						sOutput << "<h2>AUTO CHATRESPONSE</h2>";

						sOutput << "<table id=\"table\" class=\"datatable\" style=\"width:100%\" cellpadding=\"0\" cellspacing=\"0\">";
						sOutput << "<thead>";
						sOutput << "<tr>";
						sOutput << "<th>Command</th>";
						sOutput << "<th>Response</th>";
						sOutput << "<th>Delete</th>";
						sOutput << "</tr>";
						sOutput << "</thead>";
						sOutput << "<tbody>";

						for (std::vector<Database::struct_chatresponse>::const_iterator it = list.begin(); it != list.end(); ++it) 
						{
							sOutput << "<tr>";
							sOutput << "<td>" << (*it).sCommand << "</td>";
							sOutput << "<td>" << (*it).sResponse << "</td>";

							sOutput << "<td><a class=\"ask\" href=\"e_deletechatresponse?chatresponseid=" << (*it).uiChatResponseID << "\"><span class=\"ui-icon ui-icon-trash\"></a></td>";
							sOutput << "</tr>";
						}
						sOutput << "</tbody></table>";

						list.clear();

						sOutput << "<div style=\"margin-top: 20px;\"><h2>Add Chatresponse</h2><form class=\"niceform\" action=\"e_addchatresponse\" method=\"post\">"
								<< "<fieldset>"
								<< "<dl>"
								<< "<dt><label for=\"command\">Command:</label></dt>"
								<< "<dd><input type=\"text\" name=\"command\" size=\"54\"/></dd>"
								<< "</dl>"
								<< "<dl>"
								<< "<dt><label for=\"response\">Response:</label></dt>"
								<< "<dd><input type=\"text\" name=\"response\" size=\"54\"/></dd>"
								<< "</dl>"
								<< "<dl class=\"submit\">"
								<< "<input type=\"submit\" name=\"submit\" value=\"Add\"/>"
								<< "</dl>"
								<< "</fieldset>"
								<< "</form></div>";
					}
					else if (sFunctionName == "e_deletechatresponse")
					{
						if (!incoming.queries["chatresponseid"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=e_chatresponse\r\n";
							sql->deleteChatResponse(dlib::string_cast<unsigned int>(incoming.queries["chatresponseid"]));
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "e_addchatresponse")
					{
						if (!incoming.queries["command"].empty() && !incoming.queries["response"].empty())
						{
							outgoing.headers["Refresh"] = "0; url=e_chatresponse\r\n";
							sql->addChatResponse(incoming.queries["command"], incoming.queries["response"]);
							sOutput << "Redirecting...";
						}
					}
					else if (sFunctionName == "e_atb") // Autoteambalance
					{
						sOutput << "<h2>AUTOTEAMBALANCE</h2>";

						if (!incoming.queries["autoteambalance"].empty())
						{
							if (incoming.queries["autoteambalance"] == "1")
								g->settings->bAutoTeamBalance = true;
							else
								g->settings->bAutoTeamBalance = false;

							sql->updateServerSetting("atb", g->settings->bAutoTeamBalance ? "1" : "0");
						}

						if (g->settings->bAutoTeamBalance)
							sOutput << "<font color=\"#00CC00\">Autoteambalance</font> | <a href=\"?autoteambalance=0\">DISABLE</a><br/>";
						else
							sOutput << "<font color=\"#CC0000\">Autoteambalance</font> | <a href=\"?autoteambalance=1\">ENABLE</a><br/>";
					}
					else if (sFunctionName == "e_shs") // Show Headshots
					{
						sOutput << "<h2>SHOW HEADSHOTS</h2>";

						if (!incoming.queries["showhs"].empty())
						{
							if (incoming.queries["showhs"] == "1")
								g->settings->bShowHeadshots = true;
							else
								g->settings->bShowHeadshots = false;

							sql->updateServerSetting("showheadshots", g->settings->bShowHeadshots ? "1" : "0");
						}

						if (g->settings->bShowHeadshots)
							sOutput << "<font color=\"#00CC00\">Show Headshots</font> | <a href=\"?showhs=0\">DISABLE</a><br/>";
						else
							sOutput << "<font color=\"#CC0000\">Show Headshots</font> | <a href=\"?showhs=1\">ENABLE</a><br/>";
					}
					else if (sFunctionName == "e_wm") // Welcome Message
					{
						sOutput << "<h2>SHOW WELCOME-MESSAGE</h2>";

						if (!incoming.queries["welcomemsg"].empty())
						{
							if (incoming.queries["welcomemsg"] == "1")
								g->settings->bWelcomeMsg = true;
							else
								g->settings->bWelcomeMsg = false;

							sql->updateServerSetting("showwelcomemsg", g->settings->bWelcomeMsg ? "1" : "0");
						}

						if (g->settings->bWelcomeMsg)
						{
							sOutput << "<font color=\"#00CC00\">Welcome Message</font> | <a href=\"?welcomemsg=0\">DISABLE</a><br/>";

							if (!incoming.queries["welcomemsgtext"].empty())
							{
								g->settings->sWelcomeMsg = incoming.queries["welcomemsgtext"];

								sql->updateServerSetting("welcomemsg", g->settings->sWelcomeMsg);
							}

							sOutput << "<form action='e_showwelcomemsg' method='post'>Welcome Message Text: <input name='welcomemsgtext' type='text' value=\"" << g->settings->sWelcomeMsg << "\"><input type=\"submit\" value=\"Store\"></form>";
						}
						else
						{
							sOutput << "<font color=\"#CC0000\">Welcome Message</font> | <a href=\"?welcomemsg=1\">ENABLE</a><br/>";
						}
					}
					else if (sFunctionName == "e_badassjoinmsg") // Badassjoinmessage
					{
						sOutput << "<h2>BADASS(ADMIN) JOIN-MESSAGE</h2>";

						if (!incoming.queries["badassjoinmsg"].empty())
						{
							if (incoming.queries["badassjoinmsg"] == "1")
								g->settings->bAdminJoinMsg = true;
							else
								g->settings->bAdminJoinMsg = false;

							sql->updateServerSetting("badassjoinmessage", g->settings->bAdminJoinMsg ? "1" : "0");
						}

						if (g->settings->bAdminJoinMsg)
							sOutput << "<font color=\"#00CC00\">Badass(Admin) Join Message</font> | <a href=\"?badassjoinmsg=0\">DISABLE</a><br/>";
						else
							sOutput << "<font color=\"#CC0000\">Badass(Admin) Join Message</font> | <a href=\"?badassjoinmsg=1\">ENABLE</a><br/>";
					}
					else if (sFunctionName == "e_votes") // Votes
					{
						sOutput << "<h2>VOTES</h2>";

						if (!incoming.queries["votekick"].empty())
						{
							if (incoming.queries["votekick"] == "1")
								g->settings->bVoteKick = true;
							else
								g->settings->bVoteKick = false;

							sql->updateServerSetting("votekick", g->settings->bVoteKick ? "1" : "0");
						}

						if (g->settings->bVoteKick)
							sOutput << "<font color=\"#00CC00\">Votekick</font> | <a href=\"?votekick=0\">DISABLE</a><br/>";
						else
							sOutput << "<font color=\"#CC0000\">Votekick</font> | <a href=\"?votekick=1\">ENABLE</a><br/>";
					}
				}
				else if (sFunctionName[0] == 'l')
				{
					if (sFunctionName == "l_livechat")
					{
						if (!incoming.queries["msg"].empty())
						{
							g->say(incoming.queries["msg"]);
	
							Battlefield::struct_chat chat;
							chat.sName = sIncomingCookieUser.empty() ? "server" : sIncomingCookieUser;
							chat.bMessageToTeam = false;
							chat.cIsOnTeam = 0;
							chat.sTime = helper::getDateTimeAsString();
							chat.sMessage = incoming.queries["msg"];

							g->addChatMessage(chat);
							sql->log(chat.sName, chat.sMessage, LOG_CHAT);
						}

						sOutput << "<script>var auto_refresh = setInterval(function(){$('#ajax_chat').fadeOut('fast').load('!livechat').fadeIn(\"fast\");}, 2000);$(document).ready(function() {$('#ajax_chat').hide().load('!livechat').fadeIn(\"slow\")});</script>"
								<< "<div id=\"ajax_chat\">Loading Chat...</div>"
								<< "<div style=\"margin-top: 20px;\"><h2>Send Chat-Message</h2><form class=\"niceform\" action=\"l_livechat\" method=\"post\">"
								<< "<fieldset>"
								<< "<dl>"
								<< "<dt><label for=\"msg\">Send Message:</label></dt>"
								<< "<dd><input type=\"text\" name=\"msg\" size=\"54\"/></dd>"
								<< "</dl>"
								<< "<dl class=\"submit\">"
								<< "<input type=\"submit\" name=\"submit\" value=\"Send Message\"/>"
								<< "</dl>"
								<< "</fieldset>"
								<< "</form></div>";
					}
					else if (sFunctionName == "l_log")
					{
						std::vector<Database::struct_log> logList = sql->logList(15000);

						sOutput << "<h2>LAST 15000 LogEntries</h2>";

						sOutput << "<table id=\"table\" class=\"datatable\" style=\"width:100%\" cellpadding=\"0\" cellspacing=\"0\">";
						sOutput << "<thead>";
						sOutput << "<tr>";
						sOutput << "<th></th>";
						sOutput << "<th>Time</th>";
						sOutput << "<th>Name</th>";
						sOutput << "<th>Message</th>";
						sOutput << "</tr>";
						sOutput << "</thead>";
						sOutput << "<tbody>";

						for (std::vector<Database::struct_log>::const_iterator it = logList.begin(); it != logList.end(); ++it) 
						{
							if ((*it).ucType == LOG_ADMIN)
							{
								switch ((*it).ucSubType)
								{
								case LOG_ADMIN_KILL:
									sOutput << "<tr style=\"background-color: #bbffbb;\">";
									break;
								
								case LOG_ADMIN_KICK:
									sOutput << "<tr style=\"background-color: #ffddbb;\">";
									break;

								case LOG_ADMIN_BAN:
									sOutput << "<tr style=\"background-color: #ffaaaa;\">";
									break;
									
								case LOG_ADMIN_SAY:
									sOutput << "<tr style=\"background-color: #ffffbb;\">";
									break;

								case LOG_ADMIN_FMOVE:
									sOutput << "<tr style=\"background-color: #ffffaa;\">";
									break;

								case LOG_ADMIN_YELL:
									sOutput << "<tr style=\"background-color: #ffffbb;\">";
									break;
								}
							}
							else
							{
								sOutput << "<tr>";
							}
							
							sOutput << "<td>" << (*it).uiID << "</td>";
							sOutput << "<td>" << (*it).sTime << "</td>";
							sOutput << "<td>" << (*it).sName << "</td>";
							sOutput << "<td>" << (*it).sMessage << "</td>";
							sOutput << "</tr>";
						}
						sOutput << "</tbody></table>";

						logList.clear();
					}
				}
				else
				{
					sOutput << jQueryAlert("Unknown function <u>" + sFunctionName + "</u>");
				}
			}
		}
		else
		{
			sOutput << sHeader_1;
			sOutput << sHeader_2;
			sOutput << "Welcome User | <a href=\"../\" class=\"serverlist\">Administrate (RSP)</a>";
			sOutput << sHeader_3;
			sOutput << jQueryAlert("Server not running: " + gamecontainer->sError);
		}

		sOutput << sFooter;
		return (sOutput.str());
	}
	
	std::string real_function_rsp(std::string sFunctionName, const incoming_things& incoming, outgoing_things& outgoing)
	{
		ostringstream sOutput;
		sOutput << sHeader_RSP;

		std::string sPassword = Config::Webserver::rsp_password;
		std::string sIncomingQueryPassword = incoming.queries["pass"], sIncomingCookiePassword = incoming.cookies["pass"];

//#if defined _WIN32 || defined _WIN64
		if (!sIncomingQueryPassword.empty())
			helper::security::getCleanString(sIncomingQueryPassword);
		if (!sIncomingCookiePassword.empty())
			helper::security::getCleanString(sIncomingCookiePassword);
//#endif

		if (sIncomingCookiePassword.length() < 2)
		{
			if (sIncomingQueryPassword == sPassword)
				outgoing.cookies["pass"] = sPassword;
			else
				return (sLoginRSP);
		}
		else
		{
			outgoing.cookies["pass"] = sPassword;
		}

		size_t positionOfExtendedCharacter = sFunctionName.find('?');
		if (positionOfExtendedCharacter != std::string::npos)
			sFunctionName = sFunctionName.substr(0, positionOfExtendedCharacter);

		if (sFunctionName == "list")
		{
			Battlefield::gamestate::state state;
			sOutput << "<h2>SERVERLIST</h2>";
			sOutput << "<table id=\"rounded-corner\" summary=\"Serverlist\">"
					<< "<thead>"
					<< "<tr>"
					<< "<th scope=\"col\" class=\"rounded-company\">Name</th>"
					<< "<th scope=\"col\" class=\"rounded\">Address</th>"
					<< "<th scope=\"col\" class=\"rounded\">Port</th>"
					<< "<th scope=\"col\" class=\"rounded\">Owner</th>"
					<< "<th scope=\"col\" class=\"rounded\">Playercount</th>"
					<< "<th scope=\"col\" class=\"rounded\">Status</th>"
					<< "<th scope=\"col\" class=\"rounded\">Start/Stop</th>"
					<< "<th scope=\"col\" class=\"rounded-q4\">Delete</th>"
					<< "</tr>"
					<< "</thead>"
					<< "<tfoot>"
					<< "<tr>"
					<< "<td colspan=\"6\" class=\"rounded-foot-left\"><em>This is still a work in progress!!!</em></td>"
					<< "<td class=\"rounded-foot-right\">&nbsp;</td>"
					<< "</tr>"
					<< "</tfoot>"
					<< "<tbody>";

			for (std::vector<struct_gamestatus*>::const_iterator it = gGameServers.begin(); it != gGameServers.end(); ++it) 
			{
				state = (*it)->state;
				sOutput << "<tr><td>" << (*it)->name << "</td>";
				sOutput << "<td>" << (*it)->g->settings->sBF3_Server_Host << "</td>";
				sOutput << "<td>" << (*it)->g->settings->usBF3_Server_Port << "</td>";
				sOutput << "<td>" << (*it)->g->settings->sOwner << "</td>";
				sOutput << "<td>" << static_cast<int>((*it)->g->count.ucPlayer) << "</td>";
				if (state == Battlefield::gamestate::ACTIVE)
				{
					sOutput << "<td>Server is running</td>";
					sOutput << "<td><a class=\"ask\" href=\"stopserver?server=" << (*it) << "\"><img src=\"images/stop.png\" alt=\"\" title=\"\" border=\"0\" /></a></td>";
				}
				else if (state == Battlefield::gamestate::EMPTY)
				{
					sOutput << "<td>Server is running, but empty</td>";
					sOutput << "<td><a class=\"ask\" href=\"stopserver?server=" << (*it) << "\"><img src=\"images/stop.png\" alt=\"\" title=\"\" border=\"0\" /></a></td>";
				}
				else
				{
					sOutput << "<td><img src=\"images/user_logout.png\" alt=\"\" title=\"\" border=\"0\" />" << (*it)->sError << "</td>";
					sOutput << "<td><a href=\"startserver?server=" << (*it) << "\"><img src=\"images/start.png\" alt=\"\" title=\"\" border=\"0\" /></a></td>";
				}

				sOutput << "<td><a href=\"deleteserver?server=" << (*it) << "\" class=\"ask\"><img src=\"images/trash.png\" alt=\"\" title=\"\" border=\"0\" /></a></td>";
				sOutput << "</tr>";
			}
			sOutput << "</tbody></table>";

			sOutput << "<div style=\"margin-top: 20px;\"><h2>Add Server</h2><form class=\"niceform\" action=\"addserver\" method=\"post\">"
					<< "<fieldset>"
					<< "<dl>"
					<< "<dt><label for=\"gs_address\">GameServer-Address:</label></dt>"
					<< "<dd><input type=\"text\" name=\"gs_address\" size=\"54\"/></dd>"
					<< "</dl>"
					<< "<dl>"
					<< "<dt><label for=\"gs_port\">RCON-Port:</label></dt>"
					<< "<dd><input type=\"text\" name=\"gs_port\" size=\"54\"/></dd>"
					<< "</dl>"
					<< "<dl>"
					<< "<dt><label for=\"gs_pwd\">GameServer-PWD:</label></dt>"
					<< "<dd><input type=\"text\" name=\"gs_pwd\" size=\"54\"/></dd>"
					<< "</dl>"
					<< "<dl>"
					<< "<dt><label for=\"owner\">Owner:</label></dt>"
					<< "<dd><input type=\"text\" name=\"owner\" size=\"54\"/></dd>"
					<< "</dl>"
					<< "<dl>"
					<< "<dt><label for=\"name\">Server name:</label></dt>"
					<< "<dd><input type=\"text\" name=\"name\" size=\"54\"/></dd>"
					<< "</dl>"
					<< "<dl class=\"submit\">"
					<< "<input type=\"submit\" name=\"submit\" value=\"Add\"/>"
					<< "</dl>"
					<< "</fieldset>"
					<< "</form></div>";
		}
		else if (sFunctionName == "deleteserver")
		{
			if (!incoming.queries["server"].empty())
			{
				outgoing.headers["Refresh"] = "0; url=list\r\n";
				sOutput << "Redirecting...";
			}
		}
		else if (sFunctionName == "addserver")
		{
			std::string gs_address = incoming.queries["gs_address"];
			unsigned short gs_port = dlib::string_cast<unsigned short>(incoming.queries["gs_port"]);
			std::string gs_pwd = incoming.queries["gs_pwd"];
			std::string owner = incoming.queries["owner"]; 
			std::string name = incoming.queries["name"]; 

			outgoing.headers["Refresh"] = "0; url=list\r\n";
			sOutput << "Not supported";
		}
		else if (sFunctionName == "startserver")
		{
			if (!incoming.queries["server"].empty())
			{
				outgoing.headers["Refresh"] = "0; url=list\r\n";

				string str;
				stringstream s (stringstream::in | stringstream::out);
				for (std::vector<struct_gamestatus*>::const_iterator it = gGameServers.begin(); it != gGameServers.end(); ++it) 
				{
					s<<(*it);
					s>>str;

					if (str == incoming.queries["server"])
					{
						dlib::create_new_thread(thread_game, dlib::ref(*it));
						break;
					}

					s.clear();
				}

				sOutput << "Redirecting...";
			}
		}
		else if (sFunctionName == "stopserver")
		{
			if (!incoming.queries["server"].empty())
			{
				outgoing.headers["Refresh"] = "0; url=list\r\n";

				string str;
				stringstream s (stringstream::in | stringstream::out);
				for (std::vector<struct_gamestatus*>::const_iterator it = gGameServers.begin(); it != gGameServers.end(); ++it) 
				{
					s<<(*it);
					s>>str;

					if (str == incoming.queries["server"])
					{
						(*it)->sError = "Stopped by RSP!";
						(*it)->state = Battlefield::gamestate::INACTIVE;
						break;
					}

					s.clear();
				}

				sOutput << "Redirecting...";
			}
		}
		else
		{
			sOutput << "Unknown function <u>" << sFunctionName << "</u>";
		}
		
		sOutput << sFooter;
		return (sOutput.str());
	}

	std::string read_requested_file(std::string sFileName, const incoming_things& incoming, outgoing_things& outgoing)
	{
		std::string sFileType = sFileName.substr(sFileName.find_last_of('.')+1);

		if (sFileType == "jpg")
		{
			std::string sBinary;
#if defined _WIN32 || defined _WIN64
			ifstream file ("www/" + sFileName, ios::binary);
#else
			ifstream file (("www/" + sFileName).c_str(), ios::binary);
#endif

			if (file.is_open())
			{
				outgoing.headers["Content-Type"] = "image/jpeg";
				outgoing.headers["Content-Length"] = file.tellg();

				char ch;
				while (file.get(ch), !file.eof())
				{
					sBinary += ch;
				}

				file.close();
				return (sBinary);
			}
		}
		else if (sFileType == "gif")
		{
			std::string sBinary;
#if defined _WIN32 || defined _WIN64
			ifstream file ("www/" + sFileName, ios::binary);
#else
			ifstream file (("www/" + sFileName).c_str(), ios::binary);
#endif

			if (file.is_open())
			{
				outgoing.headers["Content-Type"] = "image/gif";
				outgoing.headers["Content-Length"] = file.tellg();

				char ch;
				while (file.get(ch), !file.eof())
				{
					sBinary += ch;
				}

				file.close();
				return (sBinary);
			}
		}
		else if (sFileType == "png")
		{
			std::string sBinary;
#if defined _WIN32 || defined _WIN64
			ifstream file ("www/" + sFileName, ios::binary);
#else
			ifstream file (("www/" + sFileName).c_str(), ios::binary);
#endif

			if (file.is_open())
			{
				outgoing.headers["Content-Type"] = "image/png";
				outgoing.headers["Content-Length"] = file.tellg();

				char ch;
				while (file.get(ch), !file.eof())
				{
					sBinary += ch;
				}

				file.close();
				return (sBinary);
			}
		}
		else if (sFileType == "html" || sFileType == "htm")
		{
			return (read_file_html(sFileName));
		}
		else if (sFileType == "css")
		{
			outgoing.headers["Content-Type"] = "text/css";
			return (read_file_html(sFileName));
		}
		else if (sFileType == "js")
		{
			outgoing.headers["Content-Type"] = "text/javascript";
			return (read_file_html(sFileName));
		}
		else
		{
			return (read_file_plain(sFileName));
		}

		return (s404);
	}

	// ToDo: Check for ../ and other evil things...
	class web_server : public server::http_1a_c
	{
		const std::string on_request ( 
			const incoming_things& incoming,
			outgoing_things& outgoing
		)
		{
			std::string sPath;
			
			try
			{
				if (incoming.request_type == "GET" || incoming.request_type == "POST")
				{
					sPath = &incoming.path[1];
					size_t slashPos = sPath.find('/');
					size_t pointPos = sPath.find('.');

					if (sPath.empty())
					{
						return ("<head><meta http-equiv=\"refresh\" content=\"0; URL=-rsp/list\"></head>");
					}
					else if (pointPos != std::string::npos)
					{
						sPath.erase(0, slashPos+1);
					}
					else if (slashPos != std::string::npos)
					{
						if (sPath[0] == '-')
						{
							std::string server = sPath.substr(1, slashPos-1);
							for (std::vector<struct_gamestatus*>::const_iterator it = gGameServers.begin(); it != gGameServers.end(); ++it) 
							{
								if ((*it)->name == server)
									return (real_function((*it), &sPath[slashPos+1], incoming, outgoing));
							}

							if (sPath[1] == 'r' && sPath[2] == 's' && sPath[3] == 'p')
							{
								if (sPath.find('.') == std::string::npos)
								{
									if (sPath[4] == '/')
										return (real_function_rsp(&sPath[5], incoming, outgoing));
								}
								else
								{
									sPath = &sPath[5];
								}
							}
						}
					}
					
					return (read_requested_file(sPath, incoming, outgoing));
					/*
					char cPosition, cPosition2;
					sPath = &incoming.path[1];
					if (!sPath.empty())
					{
						std::string sFileType;
						cPosition2 = sPath.find_last_of('?');
						cPosition = sPath.find_last_of('.');
						if (cPosition != -1 && cPosition2 == -1)
						{
							sFileType = sPath.substr(cPosition + 1);
						}

						if (sPath[0] == 's')
						{
							if (sFileType.empty())
							{
								ucServerID = sPath[1]-48;

								if (ucServerID < gGameServers.size())
								{
									cPosition = sPath.find('?');
									if (cPosition != -1)
									{
										sPath = sPath.substr(3, cPosition - 3);
										return (real_function(ucServerID, sPath, incoming, outgoing));
									}
									else
									{
										return (real_function(ucServerID, &sPath[3], incoming, outgoing));
									}
								}
								else
								{
									return (sUnknownServer);
								}
							}
							else
							{
								sPath = sPath.substr(sPath.find('/'));
							}
						}
						else if (sPath[0] == 'r' && sPath[1] == 's' && sPath[2] == 'p')
						{
							if (sFileType.empty())
							{
								cPosition = sPath.find('?');
								if (cPosition != -1)
									sPath = sPath.substr(4, cPosition - 4);
								else
									sPath = &sPath[4];

								return (real_function_rsp(sPath, incoming, outgoing));
							}
							else
							{
								sPath = sPath.substr(sPath.find('/'));
							}
						}

						if (cPosition != -1)
							return (read_requested_file(sPath, sFileType, incoming, outgoing));
					}
					*/
				}

				return (s404);
			}
			catch (exception& e)
			{
				return (e.what());
			}
		}
	};

	web_server our_web_server;

	void thread(void*) // web_server& the_server
	{
		try
		{
			// Start the server.  start() blocks until the server is shutdown
			// by a call to clear()
			our_web_server.start();
		}
		catch (socket_error& e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, string("Socket error while starting server: ") + e.what());
		}
		catch (exception& e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, string("Error while starting server: ") + e.what());
		}
	}

	void start(const short csPort)
	{
		try
		{
			our_web_server.set_listening_port(csPort); // boost::lexical_cast<unsigned char>(incoming.queries["level"])

			// Load default html files
			s404 = load_critical_html_file("www/404.html");
			sLogin = load_critical_html_file("www/login.html");
			sLoginRSP = load_critical_html_file("www/login_rsp.html");
			sHeader_1 = load_critical_html_file("www/header_1.html");
			sHeader_2 = load_critical_html_file("www/header_2.html");
			sHeader_3 = load_critical_html_file("www/header_3.html");
			sHeader_RSP = load_critical_html_file("www/header_rsp.html");
			sFooter = load_critical_html_file("www/footer.html");
			sUnknownServer = load_critical_html_file("www/unknownserver.html");

			// create a thread that will start the server.   The ref() here allows us to pass 
			// our_web_server into the threaded function by reference.
			//thread_function t(thread, dlib::ref(our_web_server));
			dlib::create_new_thread(thread, 0);
		}
		catch (exception& e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
	}

	void stop()
	{
		try
		{
			// this will cause the server to shut down 
			our_web_server.clear();
		}
		catch (exception& e)
		{
			helper::log::error(__FILE__, __FUNCTION__, __LINE__, e.what());
		}
	}
}
