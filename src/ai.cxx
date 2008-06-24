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
