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

#ifndef	_SDLMILLE_OPTIONS_H
#define	_SDLMILLE_OPTIONS_H

#include <sys/stat.h>
//#include <fstream>
#include <SDL.h>

namespace	_SDLMille
{

enum	{OPTION_CARD_CAPTIONS = 0, OPTION_FAST_GAME, OPTION_ANIMATIONS, OPTION_HARD_DIFFICULTY, OPTION_COUNT};

const	Uint16	OPTION_VERSION = 3;
const	char	OPTION_NAMES[OPTION_COUNT][16] = {"Card captions", "Fast game", "Animations", "Hard Difficulty"};

class	Options
{
public:
			Options		(void):	Opts (5)							{}
			~Options	(void)										{}
	bool	GetOpt		(int Option)								const;
	bool	ReadOpts	(void);
	bool	SaveOpts	(void)										const;
	bool	SetOpt		(int Option, bool Switch);
	bool	SwitchOpt	(int Option);
private:
	char	Opts;
};

}

#endif
