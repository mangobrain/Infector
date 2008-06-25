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

#ifndef __INFECTOR_AI_HXX__
#define __INFECTOR_AI_HXX__

class Game;
class BoardState;
struct move;

// One entry in the tree of enumerated moves
struct treenode
{
	// Move made to get here from parent
	move m;
	// Player who made the move
	piece player;
	// State board is left in
	BoardState *b;
	// Heuristic score of this move
	unsigned int score;
	// Moves the next player can make from this starting point
	std::list<treenode*> children;
};

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
		bool makeMove() const;
		int next_x, next_y;
		
		// Root node of enumerated move tree
		treenode *root;
		// Current set of leaf nodes
		std::list<treenode*> leaves;
};

#endif
