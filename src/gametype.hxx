// Copyright 2008 Philip Allison <sane@not.co.uk>

//    This file is part of Infector.
//
//    Infector is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Infector is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Infector.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __INFECTOR_GAMETYPE_HXX__
#define __INFECTOR_GAMETYPE_HXX__

// Enumerated type for players
enum playertype
{
	pt_ai,
	pt_local,
	pt_remote,
	pt_none
};

// Enumerated type for board square states
enum piece
{
	pc_player_none,
	pc_player_1,
	pc_player_2,
	pc_player_3,
	pc_player_4,
	pc_no_such_square
};

// Structure for constant information about a game
struct GameType
{
	int w, h;
	bool square;
	playertype player_1;
	playertype player_2;
	playertype player_3;
	playertype player_4;
	GameType()
		: w(8), h(8), square(true), player_1(pt_none), player_2(pt_none),
			player_3(pt_none), player_4(pt_none)
	{};
	bool anyPlayersOfType(const playertype pt) const
	{
		return ((player_1 == pt) || (player_2 == pt)
				|| (player_3 == pt) || (player_4 == pt));
	};
	bool isPlayerType(const piece pc, const playertype pt) const
	{
		switch (pc)
		{
			case pc_player_1:
				return (player_1 == pt);
			case pc_player_2:
				return (player_2 == pt);
			case pc_player_3:
				return (player_3 == pt);
			default:
				return (player_4 == pt);
		}
	};
	playertype typeOf(const piece pc) const
	{
		switch (pc)
		{
			case pc_player_1:
				return player_1;
			case pc_player_2:
				return player_2;
			case pc_player_3:
				return player_3;
			default:
				return player_4;
		}
	};
};

#endif
