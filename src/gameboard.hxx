// Copyright 2008 Philip Allison <sane@not.co.uk>

//    This file is part of Infector.
//
//    Infector is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Infector is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Infector.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __INFECTOR_GAMEBOARD_HXX__
#define __INFECTOR_GAMEBOARD_HXX__

class GameBoard: public Gtk::DrawingArea
{
	public:
		// Constructor - called by glademm by get_widget_derived
		GameBoard(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml> &refXml);

		// Signals we can emit
		sigc::signal<void, const int, const int> square_clicked;

		// Call this when a new game is started - will grab initial details
		// and connect game event handlers to the instance's signals.
		void newGame(Game *g, const BoardState *b, const GameType *gt);
		
		// Call this when a game has ended
		void endGame();

	private:
		// Callbacks for GTK/GDK events
		bool on_expose_event(GdkEventExpose *event);
		bool onClick(GdkEventButton *event);
		bool onResize(GdkEventConfigure *event);

		// Callbacks for various game events
		void onMoveMade(const int start_x, const int start_y, const int end_x, const int end_y, const bool gameover);
		void onInvalidMove();
		
		// Set the background pixmap of the widget to the empty board
		void setBackground();

		// Default & current state of game board
		GameType m_DefaultGameType;
		BoardState m_DefaultBoardState;
		const BoardState *m_pBoardState;
		const GameType *m_pGameType;
		int bw, bh;
};

#endif
