// Copyright 2008-2009, 2012, 2018 Philip Allison <mangobrain@googlemail.com>

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

#ifndef INFECTOR_BOARDSTATE_HXX
#define INFECTOR_BOARDSTATE_HXX

class Game;

// Struct for storing a single game move
struct move
{
	int source_x;
	int source_y;
	int dest_x;
	int dest_y;
	move(const int sx, const int sy, const int dx, const int dy)
		:source_x(sx), source_y(sy), dest_x(dx), dest_y(dy)
	{};
	move() {};
};

class BoardState
{
	public:
		BoardState(GameType *gt);
		
		// Property accessors
		piece getPieceAt(const int x, const int y) const;
		void setPieceAt(const int x, const int y, const piece p);
		piece getPlayer() const;
		void getSelectedSquare(int &x, int &y) const;
		void setSelectedSquare(const int x, const int y);
		void clearSelection();
		
		int getInitialOffset() const;
		
		// Advance to the next player's turn and return the new current player
		piece nextPlayer();
		
		// Calculate whether one square is adjacent to another within 1 or 2 squares
		// Return 0 (not adjacent), 1 ("clone" distance) or 2 ("jump" distance)
		unsigned int getAdjacency(const int ax, const int ay, const int bx, const int by) const;

		// Enumerate available moves for the given player
		// Set "stop" to true to stop as soon as one move is found
		std::vector<move> getPossibleMoves(const piece player, const bool stop = false) const;

		// Can the given player actually move?
		bool canMove(const piece player) const;

		// Get current scores
		void getScores(int& p1, int& p2, int& p3, int& p4) const;
		
	private:
		// Game info
		piece current_player;
		GameType *m_pGameType;

		// Board state - store each column with an explicit vertical offset (for
		// supporting hexagonal boards)
		std::vector<std::pair<int, std::vector<piece> > > pieces;
		int xsel, ysel;

		// Scores
		int m_Score1, m_Score2, m_Score3, m_Score4;
};

#endif
