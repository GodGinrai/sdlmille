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
	AverageGameScore = AverageHandScore = DrawCount = GameAveragesDone = GameRemainder = HandRemainder = HandAveragesDone =
	HighGameScore = HighHandScore = LossCount = RunningGameScore = RunningHandCount = RunningHandScore = WinCount = 0;
}

void	Stats::GetStats		(Uint32 &Wins, Uint32 &Losses, Uint32 &Draws, Uint32 &HighGame, Uint32 &AverageGame, Uint32 &HighHand, Uint32 &AverageHand)
{
	Load();

	Uint32	GameCount = DrawCount + LossCount + WinCount,
			RunningGameCount = (GameCount % AVERAGE_EVERY);

	HighGame = HighGameScore;
	HighHand = HighHandScore;

	AverageHand = AverageHandScore;

	if (RunningHandCount > 0)
	{
		double	PortionDone = RunningHandCount / (double) AVERAGE_EVERY;

		if (HandAveragesDone > 0)
		{
			AverageHand *= HandAveragesDone;
			AverageHand += HandRemainder;
		}

		AverageHand += (RunningHandScore / RunningHandCount) * PortionDone;
		AverageHand /= (double) (HandAveragesDone + PortionDone);
	}

	AverageGame = AverageGameScore;

	if (RunningGameCount > 0)
	{
		double	PortionDone = RunningGameCount / (double) AVERAGE_EVERY;

		if (GameAveragesDone > 0)
		{
			AverageGame *= GameAveragesDone;
			AverageGame += GameRemainder;
		}

		AverageGame += (RunningGameScore / RunningGameCount) * PortionDone;
		AverageGame /= (double) (GameAveragesDone + PortionDone);
	}

	Draws = DrawCount;
	Losses = LossCount;
	Wins = WinCount;
}

void	Stats::ProcessHand	(Uint8 Outcome, Uint16 HandScore, Uint16 GameScore)
{
	Uint32	GameCount = DrawCount + LossCount + WinCount;

	Load();

	++RunningHandCount;

	RunningHandScore += HandScore;

	if (HandScore > HighHandScore)
		HighHandScore = HandScore;

	if (RunningHandCount == AVERAGE_EVERY)
	{
		if (HandAveragesDone > 0)
		{
			AverageHandScore *= HandAveragesDone;
			AverageHandScore += HandRemainder;
		}

		AverageHandScore += (RunningHandScore / AVERAGE_EVERY);

		++HandAveragesDone;
		HandRemainder = AverageHandScore % AVERAGE_EVERY;
		AverageHandScore /= HandAveragesDone;

		RunningHandCount = 0;
		RunningHandScore = 0;
	}

	if (Outcome < OUTCOME_NOT_OVER)
	{
		RunningGameScore += GameScore;

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
		default:
			return;
		}

		GameCount = WinCount + LossCount + DrawCount;

		if ((GameCount % AVERAGE_EVERY) == 0)
		{
			if (GameAveragesDone > 0)
			{
				AverageGameScore *= GameAveragesDone;
				AverageGameScore += GameRemainder;
			}

			AverageGameScore += (RunningGameScore / AVERAGE_EVERY);

			++GameAveragesDone;
			GameRemainder = AverageGameScore % AVERAGE_EVERY;
			AverageGameScore /= GameAveragesDone;

			RunningGameScore = 0;
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
					StatsFile.read((char*) &RunningHandCount, sizeof(Uint8));
					StatsFile.read((char*) &GameRemainder, sizeof(Uint8));
					StatsFile.read((char*) &HandRemainder, sizeof(Uint8));
					StatsFile.read((char*) &HighGameScore, sizeof(Uint16));
					StatsFile.read((char*) &HighHandScore, sizeof(Uint16));
					StatsFile.read((char*) &AverageGameScore, sizeof(Uint32));
					StatsFile.read((char*) &AverageHandScore, sizeof(Uint32));
					StatsFile.read((char*) &RunningHandScore, sizeof(Uint16));
					StatsFile.read((char*) &RunningGameScore, sizeof(Uint32));
					StatsFile.read((char*) &GameAveragesDone, sizeof(Uint32));
					StatsFile.read((char*) &HandAveragesDone, sizeof(Uint32));
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

		StatsFile.write((char*) &STATS_VERSION, sizeof(Uint16));
		StatsFile.write((char*) &RunningHandCount, sizeof(Uint8));
		StatsFile.write((char*) &GameRemainder, sizeof(Uint8));
		StatsFile.write((char*) &HandRemainder, sizeof(Uint8));
		StatsFile.write((char*) &HighGameScore, sizeof(Uint16));
		StatsFile.write((char*) &HighHandScore, sizeof(Uint16));
		StatsFile.write((char*) &AverageGameScore, sizeof(Uint32));
		StatsFile.write((char*) &AverageHandScore, sizeof(Uint32));
		StatsFile.write((char*) &RunningHandScore, sizeof(Uint16));
		StatsFile.write((char*) &RunningGameScore, sizeof(Uint32));
		StatsFile.write((char*) &GameAveragesDone, sizeof(Uint32));
		StatsFile.write((char*) &HandAveragesDone, sizeof(Uint32));
		StatsFile.write((char*) &DrawCount, sizeof(Uint32));
		StatsFile.write((char*) &LossCount, sizeof(Uint32));
		StatsFile.write((char*) &WinCount, sizeof(Uint32));

		Success = StatsFile.good();
	}

	return Success;
}

}