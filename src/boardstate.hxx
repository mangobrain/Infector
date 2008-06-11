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

#ifndef __INFECTOR_BOARDSTATE_HXX__
#define __INFECTOR_BOARDSTATE_HXX__

// Enumerated type for board square states
enum piece
{
	player_none,
	player_1,
	player_2,
	player_3,
	player_4
};

class BoardState
{
	public:
		BoardState(const piece lastplayer, const int width, const int height);
		
		// Property accessors
		piece getPieceAt(const int x, const int y) const;
		int getWidth() const;
		int getHeight() const;
		piece getPlayer() const;
		
		// Advance to the next player's turn and return the new current player
		piece nextPlayer();
		
	private:
		// Game info
		piece current_player;
		piece m_lastplayer;
		std::vector<std::vector<piece> > pieces;
};

#endif
