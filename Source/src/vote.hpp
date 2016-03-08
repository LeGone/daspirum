/* 
 * Copyright (C) 2012 Raffael Holz LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "bf3_socket.hpp"

namespace Battlefield
{
	enum enum_TypeOfVote
	{
		VOTEKICK,
		VOTEMAP
	};

	struct struct_Vote
	{
		std::string voter;
		unsigned char choice;
	};

	class game;
	class Vote
	{
		enum_TypeOfVote typeOfVote;
		bool voteActive;
		int voteTime;
		std::vector<struct_Vote> voters;

		// Votekick
		std::string targetName;

		public:

			// Active Game
			game *g;

			Vote();

			bool start(enum_TypeOfVote typeOfVote);
			void startVoteKick(std::string targetName);
			void finish();
			void think();
			void addVoter(std::string voter, unsigned char choice);

			// Inlines
			inline bool isActive()
			{
				return (voteActive);
			}
	};
}