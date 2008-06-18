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

#ifndef __INFECTOR_NEWGAMEDIALOG_HXX__
#define __INFECTOR_NEWGAMEDIALOG_HXX__

class NewGameDialog: public Gtk::Dialog
{
	public:
		// Constructor - called by glademm by get_widget_derived
		NewGameDialog(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml> &refXml);
		
		// Game property accessors
		piece getLastPlayer() const;
		void getBoardSize(int &w, int &h) const;
		bool isBoardHexagonal() const;

	private:
		Gtk::ComboBox *m_pNumPlayers;
		Gtk::ComboBox *m_pBoardSize;
		Gtk::ComboBox *m_pBoardShape;
		Gtk::Label *m_pPlayer3Label;
		Gtk::Label *m_pPlayer4Label;
		Gtk::Label *m_pPlayerNumLabel;
		Gtk::ComboBox *m_pPlayer3Box;
		Gtk::ComboBox *m_pPlayer4Box;

		// Event handlers
		void onChangePlayers();
		void onChangeShape();
};

#endif
