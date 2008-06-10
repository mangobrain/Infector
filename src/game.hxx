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
		// on emitted signals when it is clicked
		Game(GameBoard* b);
	
		// Signals we can emit
		sigc::signal<void, const int, const int, const int, const int>
			move_made;
		sigc::signal<void> invalid_move;
		sigc::signal<void, const int, const int> select_piece;

	private:
		// Pointer to game board
		GameBoard* m_pBoard;

		// Event handlers
		// Board clicked
		void onSquareClicked(const int x, const int y);
};

#endif
