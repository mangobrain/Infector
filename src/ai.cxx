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
#include <algorithm>
#include <bitset>
#include <iostream>

// Library headers
#include <glibmm.h>
#include <sigc++/sigc++.h>

// Project headers
#include "boardstate.hxx"
#include "ai.hxx"
#include "game.hxx"


//
// Implementation
//

AI::AI(Game *game, const BoardState *bs)
	: m_pBoardState(bs), next_x(0), next_y(0)
{
	game->move_made.connect(sigc::mem_fun(*this, &AI::onMoveMade));
}

std::pair<int, move> minimax(BoardState* b, const int depth, const piece maximisingplayer, const piece lastplayer)
{
	piece orig_p = b->getPlayer();
	piece p = b->nextPlayer();
	bool gameover = false;
	while (!(b->canMove(p)))
	{
		p = b->nextPlayer();
		if (p == orig_p)
		{
			gameover = true;
			break;
		}
	}
	if ((depth == 0) || gameover)
	{
		// Return heuristic evaluation of position from the point of view
		// of the player who made the move that got us here
		move m;
		m.source_x = 0; m.source_y = 0; m.dest_x = 0; m.dest_y = 0;
		int score = 0;
		for (int x = 0; x < b->getWidth(); ++x)
		{
			for (int y = 0; y < b->getHeight(); ++y)
			{
				if (b->getPieceAt(x, y) == lastplayer)
					++score;
			}
		}
		return std::pair<int, move>(score, m);
	}
	else
	{
		std::vector<move> moves(b->getPossibleMoves(orig_p));
		int score = 0;
		bool noscore = true;
		move bestmove;
		bestmove.source_x = 0; bestmove.source_y = 0; bestmove.dest_x = 0; bestmove.dest_y = 0;
		for (std::vector<move>::iterator i = moves.begin(); i != moves.end(); ++i)
		{
			// Simulate current move
			BoardState* new_b = new BoardState(*b);
			int adj = new_b->getAdjacency(i->source_x, i->source_y, i->dest_x, i->dest_y);
			if (adj == 2)
				new_b->setPieceAt(i->source_x, i->source_y, player_none);
			new_b->setPieceAt(i->dest_x, i->dest_y, orig_p);

			// Capture enemy pieces
			for (int xx = i->dest_x - 1; xx <= i->dest_x + 1; ++xx)
			{
				for (int yy = i->dest_y - 1; yy <= i->dest_y + 1; ++yy)
				{
					// BoardState does range checking for us
					piece capturesquare = new_b->getPieceAt(xx, yy);
					if (capturesquare == no_such_square)
						continue;
					if (capturesquare == player_none)
						continue;
					if (capturesquare == orig_p)
						continue;
					if (new_b->getAdjacency(i->dest_x, i->dest_y, xx, yy) == 1)
						new_b->setPieceAt(xx, yy, orig_p);
				}
			}
			
			// Keep searching down the tree
			std::pair<int, move> newscore = minimax(new_b, depth - 1, maximisingplayer, orig_p);
			delete new_b;
			if (orig_p == maximisingplayer)
			{
				if ((newscore.first > score) || noscore)
				{
					score = newscore.first;
					bestmove = *i;
					noscore = false;
				}
			} else {
				if ((newscore.first < score) || noscore)
				{
					score = newscore.first;
					bestmove = *i;
					noscore = false;
				}
			}
		}
		return std::pair<int, move>(score, bestmove);
	}
}

void AI::onMoveMade(const int start_x, const int start_y, const int end_x, const int end_y, const bool gameover)
{
	std::cout << "AI" << std::endl;
	if (gameover)
		return;

	if (m_pBoardState->isAIPlayer(m_pBoardState->getPlayer()))
	{
		BoardState* new_b = new BoardState(*m_pBoardState);
		std::pair<int, move> m = minimax(new_b, 3, m_pBoardState->getPlayer(), m_pBoardState->getPlayer());
		
		// Highlight the square we're going to move
		square_clicked(m.second.source_x, m.second.source_y);
		next_x = m.second.dest_x;
		next_y = m.second.dest_y;
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
