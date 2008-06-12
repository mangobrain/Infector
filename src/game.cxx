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

// Library headers
#include <gtkmm.h>
#include <libglademm.h>

// System headers

// Project headers
#include "boardstate.hxx"
#include "game.hxx"
#include "gameboard.hxx"

//
// Implementation
//

// Constructor
Game::Game(GameBoard* b, const piece lastplayer, const int bw, const int bh)
	: m_BoardState(lastplayer, bw, bh)
{
	// All signals will be auto-disconnected on destruction, because
	// this class inherits from sigc::trackable, so don't bother
	// storing returned connection objects.

	// Connect square clicked handler to game board instance
	b->square_clicked.connect(sigc::mem_fun(*this, &Game::onSquareClicked));
	
	b->newGame(this, &m_BoardState);
}

// Board square clicked
void Game::onSquareClicked(const int x, const int y)
{
	// If the current player is clicking on their own piece, highlight it.
	if (m_BoardState.getPieceAt(x, y) == m_BoardState.getPlayer())
	{
		m_BoardState.setSelectedPiece(x, y);
		select_piece();
		return;
	}
	
	// Is a piece currently highlighted?
	int xsel, ysel;
	m_BoardState.getSelectedPiece(xsel, ysel);
	if (xsel == -1 || ysel == -1)
		// Nope, and they haven't clicked one of their own.
		invalid_move();
	else {
		// Yes! Is it a valid move?
		if (((xsel == x) && (ysel == y)) || (m_BoardState.getPieceAt(x, y) != player_none))
			// Nope - they tried to move the piece onto itself, or the
			// destination square isn't empty.
			invalid_move();
		else {
			bool clone = false;
			if ((abs(xsel - x) <= 1) && (abs(ysel - y) <= 1))
				// Yes - they moved one square, so clone it.
				clone = true;
			else if ((abs(xsel - x) <= 2) && (abs(ysel - y) <= 2))
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
					if ((xx < 0) || (xx >= m_BoardState.getWidth()) || (yy < 0) || (yy >= m_BoardState.getHeight()))
						continue;
					if (m_BoardState.getPieceAt(xx, yy) == player_none)
						continue;
					m_BoardState.setPieceAt(xx, yy, m_BoardState.getPlayer());
				}
			}
			
			// Can the next player actually move?
			// If not, skip until we find someone who can.
			// If we come full circle, the game has ended.
			piece endplayer = m_BoardState.getPlayer();
			piece nextplayer = m_BoardState.nextPlayer();
			bool gameover = false;
			while (!canMove(nextplayer))
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

// Can the given player actually move?
// A player can move if there is an empty square within 2 squares
// of one of their pieces.
bool Game::canMove(const piece player) const
{
	bool result = false;
	int bw, bh;
	bw = m_BoardState.getWidth();
	bh = m_BoardState.getHeight();
	for (int x = 0; x < bw; ++x)
	{
		for (int y = 0; y < bh; ++y)
		{
			if (m_BoardState.getPieceAt(x, y) != player)
				continue;
			for (int xx = x - 2; xx <= x + 2; ++xx)
			{
				for (int yy = y - 2; yy <= y + 2; ++yy)
				{
					if ((xx < 0) || (xx >= bw) || (yy < 0) || (yy >= bh))
						continue;
					if (m_BoardState.getPieceAt(xx, yy) == player_none)
					{
						result = true;
						break;
					}
				}
				if (result)
					break;
			}
			if (result)
				break;
		}
		if (result)
			break;
	}
	return result;
}
