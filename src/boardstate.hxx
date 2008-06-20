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

#ifndef __INFECTOR_BOARDSTATE_HXX__
#define __INFECTOR_BOARDSTATE_HXX__

class Game;

// Enumerated type for board square states
enum piece
{
	player_none,
	player_1,
	player_2,
	player_3,
	player_4,
	no_such_square
};

class BoardState
{
	public:
		BoardState(const piece lastplayer, const int width, const int height, const bool hexagonal);
		
		// Property accessors
		piece getPieceAt(const int x, const int y) const;
		void setPieceAt(const int x, const int y, const piece p);
		int getWidth() const;
		int getHeight() const;
		piece getPlayer() const;
		bool isHexagonal() const;
		void getSelectedSquare(int &x, int &y) const;
		void setSelectedSquare(const int x, const int y);
		void clearSelection();
		
		int getInitialOffset() const;
		
		// Advance to the next player's turn and return the new current player
		piece nextPlayer();
		
		// Calculate whether one square is adjacent to another within 1 or 2 squares
		// Return 0 (not adjacent), 1 ("clone" distance) or 2 ("jump" distance)
		unsigned int getAdjacency(const int ax, const int ay, const int bx, const int by) const;
		
	private:
		// Game info
		piece current_player;
		piece m_lastplayer;
		// Board state - store each column with an explicit vertical offset (for
		// supporting hexagonal boards)
		std::vector<std::pair<int, std::vector<piece> > > pieces;
		int xsel, ysel;
		int bw, bh;
		bool m_hexagonal;
};

#endif
