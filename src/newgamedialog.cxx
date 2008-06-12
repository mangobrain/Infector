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

// Library headers
#include <gtkmm.h>
#include <libglademm.h>

// Project headers
#include "boardstate.hxx"
#include "newgamedialog.hxx"

//
// Implementation
//

// Constructor
NewGameDialog::NewGameDialog(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml> &refXml)
	: Gtk::Dialog(cobject)
{
	// When the number of players is changed, show/hide the player 3 & 4 type controls
	refXml->get_widget("numplayerscombo", m_pNumPlayers);
	m_pNumPlayers->signal_changed().connect(sigc::mem_fun(*this, &NewGameDialog::onChangePlayers));
	
	refXml->get_widget("bluelabel", m_pPlayer3Label);
	refXml->get_widget("yellowlabel", m_pPlayer4Label);
	refXml->get_widget("bluecombo", m_pPlayer3Box);
	refXml->get_widget("yellowcombo", m_pPlayer4Box);
	refXml->get_widget("boardsizecombo", m_pBoardSize);
}

void NewGameDialog::onChangePlayers()
{
	if (m_pNumPlayers->get_active_row_number() == 0)
	{
		m_pPlayer3Label->hide();
		m_pPlayer4Label->hide();
		m_pPlayer3Box->hide();
		m_pPlayer4Box->hide();
	}
	else
		show_all();
}

piece NewGameDialog::getLastPlayer() const
{
	if (m_pNumPlayers->get_active_row_number() == 0)
		return player_2;
	else
		return player_4;
}

void NewGameDialog::getBoardSize(int &w, int &h) const
{
	switch (m_pBoardSize->get_active_row_number())
	{
		case 0:
			w = 8; h = 8;
			break;
		case 1:
			w = 10; h = 10;
			break;
		case 2:
			w = 14; h = 14;
			break;
		case 3:
			w = 20; h = 20;
	}
}
