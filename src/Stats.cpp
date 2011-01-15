/*
This file is part of SDL Mille.

SDL Mille is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SDL Mille is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SDL Mille.  If not, see <http://www.gnu.org/licenses/>.

(See file LICENSE for full text of license)
*/

#include "Stats.h"

namespace	_SDLMille
{

	Stats::Stats	(void)
{
	DrawCount = HighGameScore = HighHandScore = LossCount = WinCount = 0;
}

void	Stats::GetStats		(Uint16 &HighHand, Uint16 &HighGame, Uint32 &Draws, Uint32 &Losses, Uint32 &Wins)
{
	Load();

	HighGame = HighGameScore;
	HighHand = HighHandScore;
	Draws = DrawCount;
	Losses = LossCount;
	Wins = WinCount;
}

void	Stats::ProcessHand	(Uint8 Outcome, Uint16 HandScore, Uint16 GameScore)
{
	Load();

	if (HandScore > HighHandScore)
		HighHandScore = HandScore;

	if (Outcome < OUTCOME_NOT_OVER)
	{
		if (GameScore > HighGameScore)
			HighGameScore = GameScore;

		switch(Outcome)
		{
		case	OUTCOME_WON:
			++WinCount;
			break;
		case	OUTCOME_LOST:
			++LossCount;
			break;
		case OUTCOME_DRAW:
			++DrawCount;
			break;
		}
	}

	Save();
}

/* Private methods */

bool	Stats::Load		(void)
{
	using namespace std;

	struct	stat	Info;
			Uint16	SavedStatsVersion = 0;
			bool	Success = false;
	

	if (stat("stats.dat", &Info) == 0)	//File exists
	{
		ifstream	StatsFile ("stats.dat", ios::in | ios::binary);
		
		if (StatsFile.is_open())
		{
			StatsFile.seekg(0);
			StatsFile.read((char *) &SavedStatsVersion, sizeof(Uint16));
			if (StatsFile.good())
			{
				if (SavedStatsVersion == STATS_VERSION)
				{
					StatsFile.read((char*) &HighGameScore, sizeof(Uint16));
					StatsFile.read((char*) &HighHandScore, sizeof(Uint16));
					StatsFile.read((char*) &DrawCount, sizeof(Uint32));
					StatsFile.read((char*) &LossCount, sizeof(Uint32));
					StatsFile.read((char*) &WinCount, sizeof(Uint32));

					Success = StatsFile.good();
				}
			}

			StatsFile.close();
		}
	}

	return Success;
}

bool	Stats::Save		(void)
{
	using namespace std;

	bool Success = false;
	ofstream StatsFile ("stats.dat", ios::out | ios::binary);

	if (StatsFile.is_open())
	{
		StatsFile.seekp(0);

		StatsFile.write((char *) &STATS_VERSION, sizeof(Uint16));
		StatsFile.write((char*) &HighGameScore, sizeof(Uint16));
		StatsFile.write((char*) &HighHandScore, sizeof(Uint16));
		StatsFile.write((char*) &DrawCount, sizeof(Uint32));
		StatsFile.write((char*) &LossCount, sizeof(Uint32));
		StatsFile.write((char*) &WinCount, sizeof(Uint32));

		Success = StatsFile.good();
	}

	return Success;
}

}