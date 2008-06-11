// Copyright 2008 Philip Allison <sane@not.co.uk>

//    This file is part of infector.
//
//    infector is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    infector is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with infector.  If not, see <http://www.gnu.org/licenses/>.


//
// Includes
//

// Standard
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

// Language headers
#include <vector>

// System headers

// Project headers
#include "boardstate.hxx"

//
// Implementation
//

BoardState::BoardState(const piece lastplayer, const int width, const int height)
	: current_player(player_1), m_lastplayer(lastplayer)
{
	pieces.resize(width);
	for (std::vector<std::vector<piece> >::iterator i = pieces.begin(); i != pieces.end(); ++i)
	{
		i->resize(height);
	}
	switch (lastplayer)
	{
		case player_2:
			pieces[0][0] = player_2;
			pieces[width - 1][height - 1] = player_2;
			pieces[0][height - 1] = player_1;
			pieces[width - 1][0] = player_1;
			break;
		case player_4:
			pieces[0][0] = player_3;
			pieces[width - 1][height - 1] = player_2;
			pieces[0][height - 1] = player_1;
			pieces[width - 1][0] = player_4;
	}
}

// Property accessors
piece BoardState::getPieceAt(const int x, const int y) const
{
	return pieces.at(x).at(y);
}

int BoardState::getWidth() const
{
	return pieces.size();
}

int BoardState::getHeight() const
{
	return pieces.at(0).size();
}

piece BoardState::getPlayer() const
{
	return current_player;
}

// Advance to the next player's turn and return the new current player
piece BoardState::nextPlayer()
{
	if (current_player == m_lastplayer)
		current_player = player_1;
	else
		current_player = (piece)(current_player + 1);
	return current_player;
}
