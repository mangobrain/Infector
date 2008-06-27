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
	// If the game is over (the next player who can move is the one who moved last),
	// or we've reached the cut-off depth,
	// evaluate the current position and return a score.
	bool stop = (depth == 0);
	piece p = b->getPlayer();
	bool gameover = false;
	while (!(b->canMove(p)))
	{
		p = b->nextPlayer();
		if (p == lastplayer)
		{
			gameover = true;
			break;
		}
	}
	if (stop || gameover)
	{
		int s1, s2, s3, s4;
		b->getScores(s1, s2, s3, s4);
		move bestmove;
		bestmove.source_x = 0; bestmove.source_y = 0; bestmove.dest_x = 0; bestmove.dest_y = 0;
		
		// If we've played in the maximising player's favour, return positive
		// If we've played in the opponent's favour, return negative

		int us, them;
		switch (maximisingplayer)
		{
			case player_1:
				us = s1; them = s2 + s3 + s4;
				break;
			case player_2:
				us = s2; them = s1 + s3 + s4;
				break;
			case player_3:
				us = s3; them = s2 + s1 + s4;
				break;
			default:
				us = s4; them = s2 + s3 + s1;
		}
		
		if (us >= them)
			return std::pair<int, move>(us - them, bestmove);
		else
			return std::pair<int, move>(-(them - us), bestmove);
	}

	std::vector<move> moves(b->getPossibleMoves(p));
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
		new_b->setPieceAt(i->dest_x, i->dest_y, p);
			
		// Keep searching down the tree
		new_b->nextPlayer();
		std::pair<int, move> newscore = minimax(new_b, depth - 1, maximisingplayer, p);
		delete new_b;
		
		if (p == maximisingplayer)
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

void AI::onMoveMade(const int start_x, const int start_y, const int end_x, const int end_y, const bool gameover)
{
	if (gameover)
		return;

	if (m_pBoardState->isAIPlayer(m_pBoardState->getPlayer()))
	{
		BoardState* new_b = new BoardState(*m_pBoardState);
		std::pair<int, move> m = minimax(new_b, m_pBoardState->getNumPlayers(), m_pBoardState->getPlayer(), m_pBoardState->getPlayer());
		delete new_b;
		
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
