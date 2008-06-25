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
#include <memory>
#include <utility>
#include <vector>
#include <list>
#include <algorithm>
#include <bitset>

// Library headers
#include <glibmm.h>
#include <sigc++/sigc++.h>

// Project headers
#include "ai.hxx"
#include "boardstate.hxx"
#include "game.hxx"


//
// Implementation
//

AI::AI(Game *game, const BoardState *bs)
	: m_pBoardState(bs), next_x(0), next_y(0)
{
	game->move_made.connect(sigc::mem_fun(*this, &AI::onMoveMade));

	// Construct root node of tree
	// We don't particularly care about its properties, other than child list
	root = new treenode();
	
	// Construct initial move tree 4 levels deep
	piece p = m_pBoardState->getPlayer();
	std::vector<move> moves(m_pBoardState->getPossibleMoves(p));
	for (std::vector<move>::iterator i = moves.begin(); i != moves.end(); ++i)
	{
		treenode *node = new treenode();
		node.m = *i;
		node.player = p;
		
		// Take a copy of the current board state and simulate our move
		node.b = new BoardState(*m_pBoardState);
		int adj = node.b->getAdjacency(node.m.source_x, node.m.source_y, node.m.dest_x, node.m.dest_y);
		if (adj == 2)
			node.b->setPieceAt(node.m.source_x, node.m.source_y, player_none);
		node.b->setPieceAt(node.m.dest_x, node.m.dest_y, p);
		
		// Use heuristics to score the move
		// Prefer cloning to jumping
		node.score = (3 - adj) * 2;
		// Capture enemy pieces
		for (int xx = node.m.dest_x - 1; xx <= node.m.dest_x + 1; ++xx)
		{
			for (int yy = node.m.dest_y - 1; yy <= node.m.dest_y + 1; ++yy)
			{
				// BoardState does range checking for us
				piece capturesquare = m_BoardState.getPieceAt(xx, yy);
				if (capturesquare == no_such_square)
					continue;
				if (capturesquare == player_none)
				{
					// Enjoy exploring - a little
					node.score += 1;
					continue;
				}
				if (capturesquare == p)
				{
					// Enjoy defending
					node.score += 2;
					continue;
				}
				node.b->setPieceAt(node.m.dest_x, node.m.dest_y, p);
				// Enjoy capturing enemy pieces the most
				node.score += 3;
			}
		}
		
		// If we've rendered any opponents immobile, score big points.
		// If we ourselves are immobile - this may not be a good move.
		// Don't worry that we may only have a 2-player game - just means
		// every move gets 20 added here, so it evens out.
		if (!(node.b->canMove(player_1)))
			node.score += 10 * ((p == player_1) ? -1 : 1);
		if (!(node.b->canMove(player_2)))
			node.score += 10 * ((p == player_2) ? -1 : 1);
		if (!(node.b->canMove(player_3)))
			node.score += 10 * ((p == player_3) ? -1 : 1);
		if (!(node.b->canMove(player_4)))
			node.score += 10 * ((p == player_4) ? -1 : 1);
	}
}

void AI::onMoveMade(const int start_x, const int start_y, const int end_x, const int end_y, const bool gameover)
{
	if (m_pBoardState->isAIPlayer(m_pBoardState->getPlayer()))
	{
		std::vector<move> moves(m_pBoardState->getPossibleMoves(m_pBoardState->getPlayer()));
		std::random_shuffle(moves.begin(), moves.end());
		// Highlight the square we're going to move
		square_clicked(moves.front().source_x, moves.front().source_y);
		next_x = moves.front().dest_x;
		next_y = moves.front().dest_y;
		// Make the actual move in 0.5 seconds time (to let people see)
		Glib::signal_timeout().connect(sigc::mem_fun(*this, &AI::makeMove), 500);
	}
}

bool AI::makeMove() const
{
	square_clicked(next_x, next_y);
	// Don't keep firing the timer after this call
	return false;
}
