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
#include <cstdlib>
#include <ctime>

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
	: m_pBoardState(bs)
{
	game->move_made.connect(sigc::mem_fun(*this, &AI::onMoveMade));
	
	// Seed the random number generator (moves are picked at random if there
	// are multiple possibilities with the same score, such as in the opening)
	srand(time(NULL));
	
	// Make a move if it's our turn first
	onMoveMade(0, 0, 0, 0, false);
}

bool movecmp(const std::pair<move, int> &a, const std::pair<move, int> &b)
{
	return a.second > b.second;
}

void AI::onMoveMade(const int start_x, const int start_y, const int end_x, const int end_y, const bool gameover)
{
	if (gameover)
		return;

	piece me = m_pBoardState->getPlayer();
	if (m_pBoardState->isAIPlayer(me))
	{
		// Get all possible moves, and allocate a second vector for storing them plus their scores
		std::vector<move> moves(m_pBoardState->getPossibleMoves(me));
		std::vector<std::pair<move, int> > scoredmoves;
		scoredmoves.reserve(moves.size());
		
		// Score all possible moves and pick one of the best
		
		int s1, s2, s3, s4;
		m_pBoardState->getScores(s1, s2, s3, s4);
		
		for (std::vector<move>::iterator i = moves.begin(); i != moves.end(); ++i)
		{
			int score = 0;
		
			// Simulate the current move
			BoardState new_b(*m_pBoardState);
			int adj = new_b.getAdjacency(i->source_x, i->source_y, i->dest_x, i->dest_y);
			if (adj == 2)
				new_b.setPieceAt(i->source_x, i->source_y, player_none);
			new_b.setPieceAt(i->dest_x, i->dest_y, me);
			
			// Calculate how many squares we capture and score 4 points for each
			int new_s1, new_s2, new_s3, new_s4;
			new_b.getScores(new_s1, new_s2, new_s3, new_s4);
			switch (me)
			{
				case player_1:
					score += (new_s1 - s1) * 5;
					break;
				case player_2:
					score += (new_s2 - s2) * 5;
					break;
				case player_3:
					score += (new_s3 - s3) * 5;
					break;
				default:
					score += (new_s4 - s4) * 5;
			}
			
			// Now look at all squares and determine whether
			// the board overall is in good or bad shape from our point of view
			for (int y = 0; y < new_b.getHeight(); ++y)
			{
				for (int x = 0; x < new_b.getWidth(); ++x)
				{
					piece thisone = new_b.getPieceAt(x, y);
					if (thisone == no_such_square)
						continue;

					int distance_one_our_pieces = 0;
					int distance_two_our_pieces = 0;
					int distance_one_holes = 0;
					int distance_one_enemy_pieces = 0;
					int distance_two_enemy_pieces = 0;

					for (int yy = y - 1; yy <= y + 1; ++yy)
					{
						for (int xx = x - 1; xx <= x + 1; ++xx)
						{
							piece thatone = new_b.getPieceAt(xx, yy);
							int adj = new_b.getAdjacency(x, y, xx, yy);
							if (thatone == no_such_square && adj == 1)
								++distance_one_holes;
							else if (thatone != no_such_square && thatone != player_none)
							{
								if (adj == 1)
								{
									if (thatone == me)
										++distance_one_our_pieces;
									else
										++distance_one_enemy_pieces;
								}
								else if (adj == 2)
								{
									if (thatone == me)
										++distance_two_our_pieces;
									else
										++distance_two_enemy_pieces;
								}
							}
						}
					}
					
					if (thisone == me)
						// Score points for defending our own pieces
						score += (distance_one_our_pieces + distance_one_holes) * 2;
					else if (thisone != player_none)
						// Score points for being able to capture enemies
						score += (distance_two_our_pieces == 0) ? 0 : 1;
					else if (distance_two_enemy_pieces > 0 || distance_one_enemy_pieces > 0)
					{
						// Lose points if we can be captured - based on both number of pieces and how limiting it is to our game						
						if (distance_one_our_pieces > 0)
						{
							score -= distance_one_our_pieces * 4;

							BoardState new_bb(new_b);
							new_bb.setPieceAt(x, y, (me == player_1) ? player_2 : player_1);
							
							int currmoves = new_b.getPossibleMoves(me).size();
							int nextmoves = new_bb.getPossibleMoves(me).size();
							
							score -= (currmoves - nextmoves) / 10;
						}
					}
					
					// Score for giving ourselves a lot of future options
					//score += new_b.getPossibleMoves(me).size() / 80;
				}
			}
			
			scoredmoves.push_back(std::pair<move, int>(*i, score));
		}
		
		// Pick our best move.  Shuffle the list first so we don't know
		// what will come out on top if we have multiple moves with the same high score.
		std::random_shuffle(scoredmoves.begin(), scoredmoves.end());
		std::stable_sort(scoredmoves.begin(), scoredmoves.end(), movecmp);
		
		m = scoredmoves.at(0).first;
		
		// Highlight the square we're going to move then
		// make the actual move in 0.5 time increments (to let people see)
		selectpiece = true;
		Glib::signal_timeout().connect(sigc::mem_fun(*this, &AI::makeMove), 500);
	}
}

bool AI::makeMove()
{
	if (selectpiece)
	{
		// Select the piece we want to move
		square_clicked(m.source_x, m.source_y);
		selectpiece = false;
		return true;
	} else {
		// Move it
		square_clicked(m.dest_x, m.dest_y);
		// Don't keep firing the timer after this call
		return false;
	}
}
