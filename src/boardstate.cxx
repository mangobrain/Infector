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
#include <utility>
#include <vector>

// System headers

// Project headers
#include "boardstate.hxx"

//
// Implementation
//

BoardState::BoardState(const piece lastplayer, const int width, const int height, const bool hexagonal)
	: current_player(player_1), m_lastplayer(lastplayer), xsel(-1), ysel(-1)
{
	if (hexagonal)
	{
		// Hexagonal board
		// Store each column with its own unique length and vertical offset, so
		// that we don't waste any memory.
		
		// Number or columns
		pieces.resize((width + height) - 1);
		// First column is w squares heigh - "height" refers to length of diagonal
		int current_height = width;
		// Columns get longer, until the midpoint, then start shrinking again
		bool growing = true;
		// When we get past the midpoint, we also start having a vertical offset
		int current_offset = 0;
		
		for (std::vector<std::pair<unsigned int, std::vector<piece> > >::iterator i = pieces.begin(); i != pieces.end(); ++i)
		{
			i->first = current_offset;
			i->second.resize(current_height);
			if (growing)
			{
				++current_height;
				if (current_height == (width + height) -1)
					growing = false;
			} else {
				--current_height;
				++current_offset;
			}
		}
	} else {
		// Traditional square board with a player at each corner
		pieces.resize(width);
		for (std::vector<std::pair<unsigned int, std::vector<piece> > >::iterator i = pieces.begin(); i != pieces.end(); ++i)
		{
			i->second.resize(height);
			i->first = 0;
		}
		switch (lastplayer)
		{
			case player_2:
				pieces[0].second[0] = player_2;
				pieces[width - 1].second[height - 1] = player_2;
				pieces[0].second[height - 1] = player_1;
				pieces[width - 1].second[0] = player_1;
				break;
			case player_4:
				pieces[0].second[0] = player_3;
				pieces[width - 1].second[height - 1] = player_2;
				pieces[0].second[height - 1] = player_1;
				pieces[width - 1].second[0] = player_4;
		}
	}
}

// Property accessors
piece BoardState::getPieceAt(const int x, const int y) const
{
	// Take into account unallocated squares in hexagonal board
	int offset_y = y - pieces.at(x).first;
	if ((offset_y < 0) || (offset_y >= pieces.at(x).second.size()))
		return player_none;

	return pieces.at(x).second.at(offset_y);
}

void BoardState::setPieceAt(const int x, const int y, const piece p)
{
	// Take into account unallocated squares in hexagonal board
	int offset_y = y - pieces.at(x).first;
	if ((offset_y < 0) || (offset_y >= pieces.at(x).second.size()))
		return;

	pieces[x].second[offset_y] = p;
}

int BoardState::getWidth() const
{
	return pieces.size();
}

int BoardState::getHeight() const
{
	// In memory, a hexagonal board is always square, just with some unallocated spaces
	// - so this isn't "correct", but that doesn't matter much, as it isn't used during rendering.
	return pieces.at(0).second.size();
}

piece BoardState::getPlayer() const
{
	return current_player;
}

void BoardState::getSelectedSquare(int &x, int &y) const
{
	x = xsel;
	y = ysel;
}

void BoardState::setSelectedSquare(const int x, const int y)
{
	xsel = x;
	ysel = y;
}

void BoardState::clearSelection()
{
	xsel = -1;
	ysel = -1;
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
