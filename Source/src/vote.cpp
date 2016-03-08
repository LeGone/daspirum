/* 
 * Copyright (C) 2012 Raffael Holz LeGone - All Rights Reserved
 * http://www.legone.name
 *
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 */

#include "vote.hpp"
#include "battlefield3.hpp"

#define NO 0
#define YES 1

namespace Battlefield
{
	Vote::Vote()
	{
		voteActive = false;
	}

	bool Vote::start(enum_TypeOfVote typeOfVote)
	{
		if (voteActive)
		{
			g->say("Unable to start a vote! Already active.");
			return (false);
		}

		this->typeOfVote = typeOfVote;
		voteTime = 30;

		voteActive = true;

		return (true);
	}

	void Vote::startVoteKick(std::string targetName)
	{
		if (g->settings->bVoteKick && start(VOTEKICK))
		{
			this->targetName = targetName;

			g->say("A vote to kick " + targetName + " has been started");
		}
	}

	void Vote::finish()
	{
		if (typeOfVote == VOTEKICK)
		{
			unsigned char yes = 0, no = 0;
			int voterCount = voters.size();

			if (voterCount > g->count.ucPlayer/2)
			{
				for (int i=0; i<voterCount; i++)
				{
					if (voters.at(i).choice == YES)
						yes++;
					else
						no++;
				}

				if (yes > no)
					g->player_kick(targetName, "Vote-kicked!");
				else
					g->say("Vote canceled! To less yes-votes.");
			}
			else
				g->say("Vote canceled! To less voters.");
		}

		voteActive = false;
		voters.clear();
	}

	void Vote::think()
	{
		if (!voteActive)
			return;

		voteTime--;
		if (voteTime == 0)
			finish();
	}

	void Vote::addVoter(std::string voter, unsigned char choice)
	{
		if (!voteActive)
		{
			g->say("No active votes!", "player", voter);
			return;
		}

		unsigned int voterCount = voters.size();
		for (unsigned int i=0; i<voterCount; i++)
		{
			if (voters.at(i).voter == voter)
			{
				g->say("Already voted!", "player", voter);
				return;
			}
		}

		struct_Vote vote;
		vote.voter = voter;
		vote.choice = choice;

		voters.push_back(vote);
		g->say("Successfully voted!", "player", voter);
	}
}