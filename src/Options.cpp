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

#include "Options.h"

namespace _SDLMille
{

bool	Options::GetOpt		(int Option)				const
{
	if (Option < OPTION_COUNT)
		return ((Opts >> Option) & 1);

	return false;
}

bool	Options::ReadOpts	(void)
{
	//using namespace std;

	struct	stat	Info;
	bool	Success = false;
	

	if (stat("options.dat", &Info) == 0)	//File exists
	{
		FILE *OptsFile = fopen("options.dat", "rb");
		
		if (OptsFile != 0)
		{
			Uint16	SavedOptionVersion = 0;

			fread(&SavedOptionVersion, sizeof(Uint16), 1, OptsFile);
			if ((SavedOptionVersion >= 2) && (SavedOptionVersion <= OPTION_VERSION))
			{
				fread(&Opts, sizeof(char), 1, OptsFile);
				Success = true;
			}

			fclose(OptsFile);
		}
	}

	return Success;
}

bool	Options::SaveOpts	(void)						const
{
	//using namespace std;

	bool Success = false;

	FILE *OptsFile = fopen("options.dat", "wb");

	if (OptsFile != 0)
	{
		fwrite(&OPTION_VERSION, sizeof(Uint16), 1, OptsFile);
		fwrite(&Opts, sizeof(char), 1, OptsFile);
		Success = true;
		fclose(OptsFile);
	}

	return Success;	
}

bool	Options::SetOpt		(int Option, bool Switch)
{
	if (Option < OPTION_COUNT)
	{
		if (Switch)
			Opts = Opts | (1 << Option);
		else
			Opts = Opts & ~(1 << Option);

		return true;
	}

	return false;
}

bool	Options::SwitchOpt	(int Option)
{
	if (Option < OPTION_COUNT)
		return SetOpt(Option, !GetOpt(Option));

	return false;
}

}
