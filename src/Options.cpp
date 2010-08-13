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

Options::Options	(void)
{
	Opts = 1;
}

Options::~Options	(void)
{
}

bool	Options::GetOpt	(int Option)
{
	if (Option < (sizeof(char) * 8))
		return ((Opts >> Option) & 1);

	return false;
}

bool	Options::ReadOpts	(void)
{
	using namespace std;

	bool Success = false;
	struct	stat	Info;

	if (stat("options.dat", &Info) == 0)
	{
		ifstream	OptsFile ("options.dat", ios::in | ios::binary);
		
		if (OptsFile.bad())
			return false;

		OptsFile.seekg(0);
		Success = OptsFile.read(&Opts, 1);
		OptsFile.close();

		return Success;
	}

	return false;
}

bool	Options::SaveOpts	(void)
{
	using namespace std;

	bool Success = false;
	ofstream OptsFile ("options.dat", ios::out | ios::binary);

	if (OptsFile.bad())
		return false;

	OptsFile.seekp(0);
	Success = OptsFile.write(&Opts, 1);
	OptsFile.close();

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
	{
		return SetOpt(Option, !GetOpt(Option));
	}

	return false;
}

}
