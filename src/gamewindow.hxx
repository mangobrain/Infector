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

#ifndef __INFECTOR_GAMEWINDOW_HXX__
#define __INFECTOR_GAMEWINDOW_HXX__

class GameWindow: public Gtk::Window
{
	public:
		// Constructor - called by glademm by get_widget_derived
		GameWindow(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml> &refXml);
	private:
		// Reference to the Glade XML we were created from
		Glib::RefPtr<Gnome::Glade::Xml> m_refXml;

		// Pointers to various dialogues, instantiated as needed
		std::auto_ptr<Gtk::AboutDialog> m_pAboutDialog;
		std::auto_ptr<NewGameDialog> m_pNewGameDialog;

		// Game board display (derived widget)
		GameBoard *m_pBoard;

		// Current running game
		std::auto_ptr<Game> m_pGame;

		// Event handlers
		void onAbout();
		void onNewGame();
};

#endif
