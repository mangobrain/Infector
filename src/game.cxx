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
#include <iostream>

// Library headers
#include <gtkmm.h>
#include <libglademm.h>

// System headers

// Project headers
#include "gameboard.hxx"
#include "game.hxx"

//
// Implementation
//

// Constructor
Game::Game(GameBoard* b)
	:m_pBoard(b)
{
	// All signals will be auto-disconnected on destruction, because
	// this class inherits from sigc::trackable, so don't bother
	// storing returned connection objects.

	// Connect square clicked handler to game board instance
	b->square_clicked.connect(sigc::mem_fun(*this, &Game::onSquareClicked));

	std::cout << "New game" << std::endl;
}

// Board square clicked
void Game::onSquareClicked(const int x, const int y)
{
	std::cout << "Clicked: x " << x << ", y " << y << std::endl;
}
