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
#include <cstdlib>
#include <bitset>

// Library headers
#include <gtkmm.h>
#include <libglademm.h>

// System headers

// Project headers
#include "boardstate.hxx"
#include "game.hxx"
#include "gameboard.hxx"
#include "ai.hxx"

//
// Implementation
//

// Constructor
Game::Game(GameBoard* b, const piece lastplayer, const int bw, const int bh, const bool hexagonal, const std::bitset<4> &aiplayers)
	: m_BoardState(lastplayer, bw, bh, hexagonal, aiplayers), m_Score1(-1), m_Score2(-1), m_Score3(-1), m_Score4(-1), m_pAI(NULL)
{
	// All signals will be auto-disconnected on destruction, because
	// this class inherits from sigc::trackable, so don't bother
	// storing returned connection objects.

	// Connect square clicked handler to game board instance
	b->square_clicked.connect(sigc::mem_fun(*this, &Game::onSquareClicked));
	
	b->newGame(this, &m_BoardState);
	
	// Set initial scores
	if (!hexagonal)
	{
		if (lastplayer == player_2)
		{
			m_Score1 = 2;
			m_Score2 = 2;
		} else {
			m_Score1 = 1;
			m_Score2 = 1;
			m_Score3 = 1;
			m_Score4 = 1;
		}
	} else {
		m_Score1 = 3;
		m_Score2 = 3;
	}
	
	// Create an AI object if necessary
	if (aiplayers.any())
	{
		m_pAI.reset(new AI(this, &m_BoardState));
		m_pAI->square_clicked.connect(sigc::mem_fun(*this, &Game::onSquareClicked));
	}
}

// Board square clicked
void Game::onSquareClicked(const int x, const int y)
{
	// If the current player is clicking on their own piece, highlight it.
	if (m_BoardState.getPieceAt(x, y) == m_BoardState.getPlayer())
	{
		m_BoardState.setSelectedSquare(x, y);
		select_piece();
		return;
	}
	
	// Is a piece currently highlighted?
	int xsel, ysel;
	m_BoardState.getSelectedSquare(xsel, ysel);
	if (xsel == -1 || ysel == -1)
		// Nope, and they haven't clicked one of their own.
		invalid_move();
	else {
		// Yes! Is it a valid move?
		if (((xsel == x) && (ysel == y)) || (m_BoardState.getPieceAt(x, y) != player_none))
			// Nope - they tried to move the piece onto itself, or the
			// destination square isn't empty/doesn't exist.
			invalid_move();
		else {
			bool clone = false;
			unsigned int distance = m_BoardState.getAdjacency(x, y, xsel, ysel);
			if (distance == 1)
			{
				// Yes - they moved one square, so clone it and update score.
				clone = true;
				switch (m_BoardState.getPlayer())
				{
					case player_1:
						m_Score1++;
						break;
					case player_2:
						m_Score2++;
						break;
					case player_3:
						m_Score3++;
						break;
					default:
						m_Score4++;
						break;
				}
			}
			else if (distance == 2)
				// Yes - they moved two squares, so jump it.
				clone = false;
			else
			{
				// Nope - the square's empty, but it's out of range.
				invalid_move();
				return;
			}
			// If we get here, a valid move was chosen, so update the board
			// and advance the state of the game.
			m_BoardState.setPieceAt(x, y, m_BoardState.getPlayer());
			if (!clone)
				m_BoardState.setPieceAt(xsel, ysel, player_none);
			m_BoardState.clearSelection();
			
			// Capture enemy pieces
			for (int xx = x - 1; xx <= x + 1; ++xx)
			{
				for (int yy = y - 1; yy <= y + 1; ++yy)
				{
					// BoardState does range checking for us
					piece capturesquare = m_BoardState.getPieceAt(xx, yy);
					if ((capturesquare == player_none) || (capturesquare == no_such_square))
						continue;
					if (capturesquare == m_BoardState.getPlayer())
						continue;
					// Ask BoardState whether or not the piece is adjacent
					// - it abstracts away the board shape for us
					if (m_BoardState.getAdjacency(x, y, xx, yy) == 1)
					{
						// It is. Capture it and update scores.
						piece oldplayer = m_BoardState.getPieceAt(xx, yy);
						piece newplayer = m_BoardState.getPlayer();
						m_BoardState.setPieceAt(xx, yy, newplayer);
						switch (oldplayer)
						{
							case player_1:
								m_Score1--;
								break;
							case player_2:
								m_Score2--;
								break;
							case player_3:
								m_Score3--;
								break;
							default:
								m_Score4--;
								break;
						}
						switch (newplayer)
						{
							case player_1:
								m_Score1++;
								break;
							case player_2:
								m_Score2++;
								break;
							case player_3:
								m_Score3++;
								break;
							default:
								m_Score4++;
								break;
						}
					}
				}
			}
			
			// Can the next player actually move?
			// If not, skip until we find someone who can.
			// If we come full circle, the game has ended.
			piece endplayer = m_BoardState.getPlayer();
			piece nextplayer = m_BoardState.nextPlayer();
			bool gameover = false;
			while (!m_BoardState.canMove(nextplayer))
			{
				nextplayer = m_BoardState.nextPlayer();
				if (nextplayer == endplayer)
				{
					gameover = true;
					break;
				}
			}
			
			move_made(xsel, ysel, x, y, gameover);
		}
	}
}

void Game::getScores(int& p1, int& p2, int& p3, int& p4) const
{
	p1 = m_Score1;
	p2 = m_Score2;
	p3 = m_Score3;
	p4 = m_Score4;
}

const BoardState& Game::getBoardState() const
{
	return m_BoardState;
}
