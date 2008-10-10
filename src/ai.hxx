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

#ifndef __INFECTOR_AI_HXX__
#define __INFECTOR_AI_HXX__

class Game;
class BoardState;

class AI : public sigc::trackable
{
	public:
		AI(Game *game, const BoardState *bs);
	
		// Signals we can emit
		sigc::signal<void, const int, const int> square_clicked;
	
	private:
		// Pointer to current board state
		const BoardState *m_pBoardState;
		
		// Event handlers
		void onMoveMade(const int start_x, const int start_y, const int end_x, const int end_y, const bool gameover);
		
		// Make move after timer has fired (so human players can observe selected piece first)
		bool makeMove();
		bool selectpiece;
		move m;
};

#endif
