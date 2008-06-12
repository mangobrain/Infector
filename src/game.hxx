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

#ifndef __INFECTOR_GAME_HXX__
#define __INFECTOR_GAME_HXX__

class GameBoard;

// Class for main game logic, tying together players and the GUI
class Game: public sigc::trackable
{
	public:
		// Constructor - pass in game board so we can pick up
		// on emitted signals when it is clicked, and pass in
		// game properties (board size & number of players)
		Game(GameBoard* b, const piece lastplayer, const int bw, const int bh);
	
		// Signals we can emit
		sigc::signal<void, const int, const int, const int, const int, const bool>
			move_made;
		sigc::signal<void> invalid_move;
		sigc::signal<void> invalid_piece_selection;
		sigc::signal<void> select_piece;

	private:
		// Board state
		BoardState m_BoardState;

		// Event handlers
		// Board clicked
		void onSquareClicked(const int x, const int y);
		
		// Can the given player actually move?
		bool canMove(const piece player) const;
};

#endif
