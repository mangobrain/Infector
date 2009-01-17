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

// System headers

// Project headers
#include "gametype.hxx"
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

BoardState::BoardState(GameType *gt)
	: current_player(pc_player_1), m_pGameType(gt), xsel(-1), ysel(-1),
		m_Score1(-1), m_Score2(-1), m_Score3(-1), m_Score4(-1)
{
	if (!(m_pGameType->square))
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
		pieces.resize((m_pGameType->w + m_pGameType->h) - 1);
		// First column is w squares heigh - "height" refers to length of diagonal edge
		int current_height = m_pGameType->w;
		// Columns get longer, until the midpoint, then start shrinking again
		bool growing = true;
		// Until we get past the midpoint, we also have a vertical offset
		int current_offset = m_pGameType->h - 1;
		
		for (std::vector<std::pair<int, std::vector<piece> > >::iterator i = pieces.begin(); i != pieces.end(); ++i)
		{
			i->first = current_offset;
			i->second.resize(current_height);
			if (growing)
			{
				++current_height;
				--current_offset;
				if (current_height == (m_pGameType->w + m_pGameType->h) -1)
					growing = false;
			} else {
				--current_height;
			}
		}
		
		// Now store the *actual* dimensions of the board, as if it were a square
		// with the corners cut off.
		int orig_h = m_pGameType->h;
		int orig_w = m_pGameType->w;
		m_pGameType->h = (m_pGameType->w + m_pGameType->h) - 1;
		m_pGameType->w = m_pGameType->h;
		
		// Place starting pieces at the corners.
		// Can only have two players (fairly) on a hexagonal board, so
		// don't bother switching based on lastplayer.
		// We aren't taking into account vertical offsets
		// here, so zero in the column isn't always zero in the "square".
		pieces[0].second[0] = pc_player_2;
		pieces[0].second[orig_w - 1] = pc_player_1;
		pieces[orig_h - 1].second[0] = pc_player_1;
		pieces[orig_h - 1].second[m_pGameType->h - 1] = pc_player_2;
		pieces[m_pGameType->w - 1].second[0] = pc_player_2;
		pieces[m_pGameType->w - 1].second[orig_w - 1] = pc_player_1;

	} else {
		// Traditional square board with a player at each corner
		pieces.resize(m_pGameType->w);
		for (std::vector<std::pair<int, std::vector<piece> > >::iterator i = pieces.begin(); i != pieces.end(); ++i)
		{
			i->second.resize(m_pGameType->h);
			i->first = 0;
		}
		
		// Place starting pieces
		// Can only have 2 or 4 players on a square board
		if (m_pGameType->player_3 == pt_none)
		{
			// 2 players
			pieces[0].second[0] = pc_player_2;
			pieces[m_pGameType->w - 1].second[m_pGameType->h - 1] = pc_player_2;
			pieces[0].second[m_pGameType->h - 1] = pc_player_1;
			pieces[m_pGameType->w - 1].second[0] = pc_player_1;
		} else {
			// 4 players
			pieces[0].second[0] = pc_player_3;
			pieces[m_pGameType->w - 1].second[m_pGameType->h - 1] = pc_player_2;
			pieces[0].second[m_pGameType->h - 1] = pc_player_1;
			pieces[m_pGameType->w - 1].second[0] = pc_player_4;
		}
	}
	
	// Set initial scores
	if (m_pGameType->square)
	{
		if (m_pGameType->player_3 == pt_none)
		{
			m_Score1 = 2;
			m_Score2 = 2;
		} else {
			m_Score1 = 1;
			m_Score2 = 1;
			m_Score3 = 1;
			m_Score4 = 1;
		}
	} else {
		m_Score1 = 3;
		m_Score2 = 3;
	}
}

// Property accessors
piece BoardState::getPieceAt(const int x, const int y) const
{
	// Take into account unallocated squares in hexagonal board
	if ((x < 0) || (x >= m_pGameType->w))
		return pc_no_such_square;
	int offset_y = y - pieces.at(x).first;
	if ((offset_y < 0) || (offset_y >= pieces.at(x).second.size()))
		return pc_no_such_square;

	return pieces.at(x).second.at(offset_y);
}

void BoardState::setPieceAt(const int x, const int y, const piece p)
{
	// Take into account unallocated squares in hexagonal board
	if ((x < 0) || (x >= m_pGameType->w))
		return;
	int offset_y = y - pieces.at(x).first;
	if ((offset_y < 0) || (offset_y >= pieces.at(x).second.size()))
		return;
	
	if (p == pc_player_none)
	{
		// If we're clearing out a square, subtract one from player's score
		switch (pieces.at(x).second.at(offset_y))
		{
			case pc_player_1:
				--m_Score1;
				break;
			case pc_player_2:
				--m_Score2;
				break;
			case pc_player_3:
				--m_Score3;
				break;
			default:
				--m_Score4;
				break;
		}
	}
	else
	{
		// Increase score for the player who's just gained a piece
		switch (p)
		{
			case pc_player_1:
				m_Score1++;
				break;
			case pc_player_2:
				m_Score2++;
				break;
			case pc_player_3:
				m_Score3++;
				break;
			default:
				m_Score4++;
		}
		
		// Capture enemy pieces
		for (int xx = x - 1; xx <= x + 1; ++xx)
		{
			int offset = -1;
			for (int yy = y - 1; yy <= y + 1; ++yy)
			{
				piece capturesquare = getPieceAt(xx, yy);
				if ((capturesquare == pc_player_none) || (capturesquare == pc_no_such_square) || (capturesquare == p))
				{
					++offset;
					continue;
				}
				if (getAdjacency(x, y, xx, yy) == 1)
				{
					// Enemy piece is adjacent - capture it and update scores
					int offset_yy = yy - pieces.at(xx).first;
					pieces[xx].second[offset_yy] = p;
					switch (capturesquare)
					{
						case pc_player_1:
							m_Score1--;
							break;
						case pc_player_2:
							m_Score2--;
							break;
						case pc_player_3:
							m_Score3--;
							break;
						default:
							m_Score4--;
					}
					switch (p)
					{
						case pc_player_1:
							m_Score1++;
							break;
						case pc_player_2:
							m_Score2++;
							break;
						case pc_player_3:
							m_Score3++;
							break;
						default:
							m_Score4++;
					}
				}
				++offset;
			}
		}
	}

	pieces[x].second[offset_y] = p;
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

int BoardState::getInitialOffset() const
{
	return pieces.at(0).first;
}

void BoardState::setSelectedSquare(const int x, const int y)
{
	// Take into account unallocated squares in hexagonal board
	if ((x < 0) || (x >= m_pGameType->w))
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
	if (((m_pGameType->player_3 == pt_none) && (current_player == pc_player_2))
		|| (current_player == pc_player_4))
	{
		current_player = pc_player_1;
	} else {
		current_player = (piece)(current_player + 1);
	}
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

	if ((ax < 0) || (ax >= m_pGameType->w))
		return 0;
	int offset_y = ay - pieces.at(ax).first;
	if ((offset_y < 0) || (offset_y >= pieces.at(ax).second.size()))
		return 0;
	
	if ((bx < 0) || (bx >= m_pGameType->w))
		return 0;
	offset_y = by - pieces.at(bx).first;
	if ((offset_y < 0) || (offset_y >= pieces.at(bx).second.size()))
		return 0;

	// Now the actual shape-dependent adjacency test
	if (!(m_pGameType->square))
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
	for (int x = 0; x < m_pGameType->w; ++x)
	{
		for (int y = 0; y < m_pGameType->h; ++y)
		{
			if (getPieceAt(x, y) != player)
				continue;
			for (int xx = x - 2; xx <= x + 2; ++xx)
			{
				for (int yy = y - 2; yy <= y + 2; ++yy)
				{
					if ((getAdjacency(x, y, xx, yy) > 0)
						&& (getPieceAt(xx, yy) == pc_player_none))
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

void BoardState::getScores(int& p1, int& p2, int& p3, int& p4) const
{
	p1 = m_Score1;
	p2 = m_Score2;
	p3 = m_Score3;
	p4 = m_Score4;
}
