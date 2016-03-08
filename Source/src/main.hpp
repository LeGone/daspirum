/* 
 * Copyright (C) 2012 Raffael Holz aka LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#ifndef MAIN_HPP
#define MAIN_HPP

#include <vector>
#include "battlefield3.hpp"

#define BUILD 1

struct struct_gamestatus
{
	Battlefield::game *g;
	Battlefield::gamestate::state state;
	std::string name;
	std::string sError;
};

extern std::vector<struct_gamestatus*> gGameServers;
void thread_game(void *);

#endif
