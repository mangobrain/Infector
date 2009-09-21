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

#ifndef __INFECTOR_GAMEWINDOW_HXX__
#define __INFECTOR_GAMEWINDOW_HXX__

class GameWindow: public Gtk::Window
{
	public:
		// Constructor - called by glademm by get_widget_derived
		GameWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refXml);
	private:
		// Reference to the Glade XML we were created from
		Glib::RefPtr<Gtk::Builder> m_refXml;

		// Pointers to various dialogues, instantiated as needed
		std::auto_ptr<Gtk::AboutDialog> m_pAboutDialog;
		std::auto_ptr<NewGameDialog> m_pNewGameDialog;
		std::auto_ptr<ServerStatusDialog> m_pServerStatusDialog;
		std::auto_ptr<ClientStatusDialog> m_pClientStatusDialog;

		// Game board display (derived widget)
		GameBoard *m_pBoard;

		// Current running game
		std::auto_ptr<Game> m_pGame;
		
		// Statusbars for game scores
		Gtk::Statusbar *m_pRedStatusbar;
		Gtk::Statusbar *m_pGreenStatusbar;
		Gtk::Statusbar *m_pBlueStatusbar;
		Gtk::Statusbar *m_pYellowStatusbar;
		
		// Statusbar for general info
		Gtk::Statusbar *m_pStatusbar;
		
		// UI manager and action group for menus & toolbars
		Glib::RefPtr<Gtk::UIManager> m_refUIMan;
		Glib::RefPtr<Gtk::ActionGroup> m_refActGrp;

		// Event handlers
		void onAbout();
		void onNewGame();
		void onConnect();
		void onMoveMade(const int ax, const int ay, const int bx, const int by, const bool gameover);
		void onNetworkError(const Glib::ustring &e);
};

#endif
