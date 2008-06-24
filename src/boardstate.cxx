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
#include <cstdlib>
#include <bitset>

// System headers

// Project headers
#include "boardstate.hxx"

//
// Globals
//

// "Adjacency map" for hexagonal boards
static const char *map =
	"00222"
	"02112"
	"21012"
	"21120"
	"22200";

//
// Implementation
//

BoardState::BoardState(const piece lastplayer, const int width, const int height, const bool hexagonal, const std::bitset<4> &aiplayers)
	: current_player(player_1), m_lastplayer(lastplayer), xsel(-1), ysel(-1), bw(width), bh(height),
	m_hexagonal(hexagonal), m_aiplayers(aiplayers)
{
	if (m_hexagonal)
	{

		// Hexagonal board
		// Store each column with its own unique length and vertical offset, so
		// that we don't waste any memory.  Basically, we have a square board
		// with the corners chopped off, where columns are represented as a set
		// of vectors of unequal length:

		// (0, 0) * / ***** |
		//         / ****** |
		// height / ******* |  width
		//       / ******** |
		//      / ********* |
		//        ********
		//        *******
		//        ******
		//        *****   *  ((width + height) - 1, (width + height) - 1)
		
		// Number or columns
		pieces.resize((bw + bh) - 1);
		// First column is w squares heigh - "height" refers to length of diagonal edge
		int current_height = bw;
		// Columns get longer, until the midpoint, then start shrinking again
		bool growing = true;
		// Until we get past the midpoint, we also have a vertical offset
		int current_offset = bh - 1;
		
		for (std::vector<std::pair<int, std::vector<piece> > >::iterator i = pieces.begin(); i != pieces.end(); ++i)
		{
			i->first = current_offset;
			i->second.resize(current_height);
			if (growing)
			{
				++current_height;
				--current_offset;
				if (current_height == (bw + bh) -1)
					growing = false;
			} else {
				--current_height;
			}
		}
		
		// Now store the *actual* dimensions of the board, as if it were a square
		// with the corners cut off.
		bh = (width + height) - 1;
		bw = bh;
		
		// Place starting pieces at the corners.
		// Can only have two players (fairly) on a hexagonal board, so
		// don't bother switching based on lastplayer.
		// We aren't taking into account vertical offsets
		// here, so zero in the column isn't always zero in the "square".
		pieces[0].second[0] = player_2;
		pieces[0].second[width - 1] = player_1;
		pieces[height - 1].second[0] = player_1;
		pieces[height - 1].second[bh - 1] = player_2;
		pieces[bw - 1].second[0] = player_2;
		pieces[bw - 1].second[width - 1] = player_1;
		m_lastplayer = player_2;

	} else {

		// Traditional square board with a player at each corner
		pieces.resize(bw);
		for (std::vector<std::pair<int, std::vector<piece> > >::iterator i = pieces.begin(); i != pieces.end(); ++i)
		{
			i->second.resize(bh);
			i->first = 0;
		}
		
		// Place starting pieces
		// Can only have 2 or 4 players on a square board
		switch (lastplayer)
		{
			case no_such_square:
			case player_none:
			case player_1:
			case player_2:
				pieces[0].second[0] = player_2;
				pieces[bw - 1].second[bh - 1] = player_2;
				pieces[0].second[bh - 1] = player_1;
				pieces[bw - 1].second[0] = player_1;
				m_lastplayer = player_2;
				break;
			case player_3:
			case player_4:
				pieces[0].second[0] = player_3;
				pieces[bw - 1].second[bh - 1] = player_2;
				pieces[0].second[bh - 1] = player_1;
				pieces[bw - 1].second[0] = player_4;
				m_lastplayer = player_4;
		}
	}
}

// Property accessors
piece BoardState::getPieceAt(const int x, const int y) const
{
	// Take into account unallocated squares in hexagonal board
	if ((x < 0) || (x >= bw))
		return no_such_square;
	int offset_y = y - pieces.at(x).first;
	if ((offset_y < 0) || (offset_y >= pieces.at(x).second.size()))
		return no_such_square;

	return pieces.at(x).second.at(offset_y);
}

void BoardState::setPieceAt(const int x, const int y, const piece p)
{
	// Take into account unallocated squares in hexagonal board
	if ((x < 0) || (x >= bw))
		return;
	int offset_y = y - pieces.at(x).first;
	if ((offset_y < 0) || (offset_y >= pieces.at(x).second.size()))
		return;

	pieces[x].second[offset_y] = p;
}

int BoardState::getWidth() const
{
	return bw;
}

int BoardState::getHeight() const
{
	return bh;
}

piece BoardState::getPlayer() const
{
	return current_player;
}

bool BoardState::isAIPlayer(const piece player) const
{
	switch (player)
	{
		case player_1:
			return m_aiplayers.test(0);
		case player_2:
			return m_aiplayers.test(1);
		case player_3:
			return m_aiplayers.test(2);
		default:
			return m_aiplayers.test(3);
	}
}

bool BoardState::isHexagonal() const
{
	return m_hexagonal;
}

void BoardState::getSelectedSquare(int &x, int &y) const
{
	x = xsel;
	y = ysel;
}

int BoardState::getInitialOffset() const
{
	return pieces.at(0).first;
}

void BoardState::setSelectedSquare(const int x, const int y)
{
	// Take into account unallocated squares in hexagonal board
	if ((x < 0) || (x >= bw))
		return;
	int offset_y = y - pieces.at(x).first;
	if ((offset_y < 0) || (offset_y >= pieces.at(x).second.size()))
		return;

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

// Calculate whether one square is adjacent to another within 1 or 2 squares
// Return 0 (not adjacent), 1 ("clone" distance) or 2 ("jump" distance)
unsigned int BoardState::getAdjacency(const int ax, const int ay, const int bx, const int by) const
{
	// Can't move a piece onto itself
	if ((ax == bx) && (ay == by))
		return 0;

	// Range checking, including chopped-off corners on hexagonal board

	if ((ax < 0) || (ax >= bw))
		return 0;
	int offset_y = ay - pieces.at(ax).first;
	if ((offset_y < 0) || (offset_y >= pieces.at(ax).second.size()))
		return 0;
	
	if ((bx < 0) || (bx >= bw))
		return 0;
	offset_y = by - pieces.at(bx).first;
	if ((offset_y < 0) || (offset_y >= pieces.at(bx).second.size()))
		return 0;

	// Now the actual shape-dependent adjacency test
	if (m_hexagonal)
	{
		// Look it up in the adjacency map.
		// First convert to coordinates in the range (0, 0) to (4, 4)
		int cx = (bx - ax) + 2;
		int cy = (by - ay) + 2;
		if ((cx < 0) || (cx > 4) || (cy < 0) || (cy > 4))
			return 0;
		// Then use the span to convert to an index into the linear map
		int index = (cy * 5) + cx;
		switch (map[index])
		{
			case '1':
				return 1;
			case '2':
				return 2;
			default:
				return 0;
		}		
	} else {
		// Square board? simple. :)
		if ((abs(bx - ax) <= 1) && (abs(by - ay) <= 1))
			return 1;
		else if ((abs(bx - ax) <= 2) && (abs(by - ay) <= 2))
			return 2;
		else
			return 0;
	}
}

// Enumerate available moves for the given player
// Set "stop" to true to stop as soon as one move is found
std::vector<move> BoardState::getPossibleMoves(const piece player, const bool stop) const
{
	std::vector<move> results;
	for (int x = 0; x < bw; ++x)
	{
		for (int y = 0; y < bh; ++y)
		{
			if (getPieceAt(x, y) != player)
				continue;
			for (int xx = x - 2; xx <= x + 2; ++xx)
			{
				for (int yy = y - 2; yy <= y + 2; ++yy)
				{
					
					if ((getAdjacency(x, y, xx, yy) > 0)
						&& (getPieceAt(xx, yy) == player_none))
					{
						results.push_back(move(x, y, xx, yy));
						if (stop)
							return results;
					}
				}
			}
		}
	}
	return results;
}

// Can the given player actually move?
// A player can move if there is an empty square within a
// distance of 2 from one of their pieces.
bool BoardState::canMove(const piece player) const
{
	return (getPossibleMoves(player, true).size() > 0);
}
