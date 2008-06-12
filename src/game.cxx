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
#include <iostream>

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
			if ((abs(xsel - x) <= 1) && (abs(ysel - y) <= 1))
			{
				// Yes - they moved one square, so clone it.
				m_BoardState.setPieceAt(x, y, m_BoardState.getPlayer());
				m_BoardState.clearSelection();
				m_BoardState.nextPlayer();
				move_made(xsel, ysel, x, y);
			}
			else if ((abs(xsel - x) <= 2) && (abs(ysel - y) <= 2))
			{
				// Yes - they moved two squares.
				m_BoardState.setPieceAt(x, y, m_BoardState.getPlayer());
				m_BoardState.setPieceAt(xsel, ysel, player_none);
				m_BoardState.clearSelection();
				m_BoardState.nextPlayer();
				move_made(xsel, ysel, x, y);
			}
		}
	}
}
