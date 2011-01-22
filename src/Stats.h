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

#ifndef	_SDLMILLE_STATS_H
#define	_SDLMILLE_STATS_H

#include <sys/stat.h>
#include <fstream>
#include <SDL.h>

namespace	_SDLMille
{

const	int		STAT_CAPTIONS_SIZE	= 7,
				AVERAGE_EVERY = 20;	/*	Playing one perfect hand per minute, 8 hours a day, it would take over 100 years
										to overload AverageHandScore	*/
const	Uint16	STATS_VERSION = 1;

const	char	STAT_CAPTIONS[STAT_CAPTIONS_SIZE][40] =
					{	"High Hand Score", "High Game Score", "Average Hand Score", "Average Game Score", "Draws", "Losses", "Wins"	};
enum	{OUTCOME_WON = 0, OUTCOME_DRAW, OUTCOME_LOST, OUTCOME_NOT_OVER};

class	Stats
{
public:
			Stats			(void);
	void	GetStats		(Uint32 &HighHand, Uint32 &HighGame, Uint32 &AverageHand, Uint32 &AverageGame, Uint32 &Draws, Uint32 &Losses, Uint32 &Wins);
	void	ProcessHand		(Uint8 Outcome, Uint16 HandScore, Uint16 GameScore);
private:
	bool	Load			(void);
	bool	Save			(void);

	Uint8	RunningHandCount;

	Uint16	HighGameScore,
			HighHandScore,
			RunningHandScore;

	Uint32	AverageGameScore,
			AverageHandScore,
			GameAveragesDone,
			HandAveragesDone,
			RunningGameScore,
			DrawCount,
			LossCount,
			WinCount;
};

}

#endif