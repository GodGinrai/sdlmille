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
	Reset();
}

void	Stats::Clear		(void)
{
	Reset();
	Save();
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
		double	PortionDone = RunningHandCount / (double) AVERAGE_EVERY,
				WorkingAverage = AverageHand;

		if (HandAveragesDone > 0)
		{
			WorkingAverage *= HandAveragesDone;
			WorkingAverage += HandRemainder;
		}

		WorkingAverage += (RunningHandScore / RunningHandCount) * PortionDone;
		WorkingAverage /= (double) (HandAveragesDone + PortionDone);

		AverageHand = WorkingAverage;
	}

	AverageGame = AverageGameScore;

	if (RunningGameCount > 0)
	{
		double	WorkingAverage = AverageGame,
				PortionDone = RunningGameCount / (double) AVERAGE_EVERY;

		if (GameAveragesDone > 0)
		{
			WorkingAverage *= GameAveragesDone;
			WorkingAverage += GameRemainder;
		}

		WorkingAverage += (RunningGameScore / RunningGameCount) * PortionDone;
		WorkingAverage /= (double) (GameAveragesDone + PortionDone);

		AverageGame = WorkingAverage;
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
	//using namespace std;

	struct	stat	Info;
			Uint16	SavedStatsVersion = 0;
			bool	Success = false;
	

	if (stat("stats.dat", &Info) == 0)	//File exists
	{
		FILE *StatsFile = fopen("stats.dat", "rb");
		
		if (StatsFile != 0)
		{
			fread(&SavedStatsVersion, sizeof(Uint16), 1, StatsFile);

			if (SavedStatsVersion == STATS_VERSION)
			{
				fread(&RunningHandCount, sizeof(Uint8), 1, StatsFile);
				fread(&GameRemainder, sizeof(Uint8), 1, StatsFile);
				fread(&HandRemainder, sizeof(Uint8), 1, StatsFile);
				fread(&HighGameScore, sizeof(Uint16), 1, StatsFile);
				fread(&HighHandScore, sizeof(Uint16), 1, StatsFile);
				fread(&AverageGameScore, sizeof(Uint32), 1, StatsFile);
				fread(&AverageHandScore, sizeof(Uint32), 1, StatsFile);
				fread(&RunningHandScore, sizeof(Uint16), 1, StatsFile);
				fread(&RunningGameScore, sizeof(Uint32), 1, StatsFile);
				fread(&GameAveragesDone, sizeof(Uint32), 1, StatsFile);
				fread(&HandAveragesDone, sizeof(Uint32), 1, StatsFile);
				fread(&DrawCount, sizeof(Uint32), 1, StatsFile);
				fread(&LossCount, sizeof(Uint32), 1, StatsFile);
				fread(&WinCount, sizeof(Uint32), 1, StatsFile);
			}

			fclose(StatsFile);

			Success = true;
		}
	}

	return Success;
}

void	Stats::Reset	(void)
{
	AverageGameScore = AverageHandScore = DrawCount = GameAveragesDone = GameRemainder = HandRemainder = HandAveragesDone =
	HighGameScore = HighHandScore = LossCount = RunningGameScore = RunningHandCount = RunningHandScore = WinCount = 0;
}

bool	Stats::Save		(void)
{
	//using namespace std;

	bool Success = false;
	
	FILE *StatsFile = fopen("stats.dat", "wb");

	if (StatsFile != 0)
	{
		fwrite(&STATS_VERSION, sizeof(Uint16), 1, StatsFile);
		fwrite(&RunningHandCount, sizeof(Uint8), 1, StatsFile);
		fwrite(&GameRemainder, sizeof(Uint8), 1, StatsFile);
		fwrite(&HandRemainder, sizeof(Uint8), 1, StatsFile);
		fwrite(&HighGameScore, sizeof(Uint16), 1, StatsFile);
		fwrite(&HighHandScore, sizeof(Uint16), 1, StatsFile);
		fwrite(&AverageGameScore, sizeof(Uint32), 1, StatsFile);
		fwrite(&AverageHandScore, sizeof(Uint32), 1, StatsFile);
		fwrite(&RunningHandScore, sizeof(Uint16), 1, StatsFile);
		fwrite(&RunningGameScore, sizeof(Uint32), 1, StatsFile);
		fwrite(&GameAveragesDone, sizeof(Uint32), 1, StatsFile);
		fwrite(&HandAveragesDone, sizeof(Uint32), 1, StatsFile);
		fwrite(&DrawCount, sizeof(Uint32), 1, StatsFile);
		fwrite(&LossCount, sizeof(Uint32), 1, StatsFile);
		fwrite(&WinCount, sizeof(Uint32), 1, StatsFile);
		
		fclose(StatsFile);

		Success = true;
	}

	return Success;
}

}