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
#include <bitset>

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
	
	// When the board shape is changed, show/hide the player 3 & 4 type controls
	// and the number-of-players controls
	refXml->get_widget("boardshapecombo", m_pBoardShape);
	m_pBoardShape->signal_changed().connect(sigc::mem_fun(*this, &NewGameDialog::onChangeShape));	
	
	refXml->get_widget("bluelabel", m_pPlayer3Label);
	refXml->get_widget("yellowlabel", m_pPlayer4Label);
	refXml->get_widget("playernumlabel", m_pPlayerNumLabel);
	refXml->get_widget("redcombo", m_pPlayer1Box);
	refXml->get_widget("greencombo", m_pPlayer2Box);
	refXml->get_widget("bluecombo", m_pPlayer3Box);
	refXml->get_widget("yellowcombo", m_pPlayer4Box);
	refXml->get_widget("boardsizecombo", m_pBoardSize);

	// XXX Set default items for our ComboBoxes.
	// Doing this in the Glade XML itself causes errors.
	
	// Default board size to 8x8, shape square
	m_pBoardSize->set_active(1);
	m_pBoardShape->set_active(0);

	// 2 players	
	m_pNumPlayers->set_active(0);
	
	// Red, blue & yellow: human, green: computer
	m_pPlayer3Box->set_active(0);
	m_pPlayer4Box->set_active(0);
	Gtk::ComboBox *pPlayer1Box;
	Gtk::ComboBox *pPlayer2Box;
	refXml->get_widget("redcombo", pPlayer1Box);
	refXml->get_widget("greencombo", pPlayer2Box);	
	pPlayer1Box->set_active(0);
	pPlayer2Box->set_active(1);
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

void NewGameDialog::onChangeShape()
{
	if (m_pBoardShape->get_active_row_number() == 0)
	{
		// Square
		m_pBoardSize->set_active(1);
		m_pNumPlayers->show();
		m_pPlayerNumLabel->show();
		onChangePlayers();
	} else {
		// Hexagonal
		m_pBoardSize->set_active(0);
		m_pNumPlayers->hide();
		m_pPlayerNumLabel->hide();
		m_pPlayer3Label->hide();
		m_pPlayer4Label->hide();
		m_pPlayer3Box->hide();
		m_pPlayer4Box->hide();
	}
}

piece NewGameDialog::getLastPlayer() const
{
	if ((m_pNumPlayers->get_active_row_number() == 0) || (m_pBoardShape->get_active_row_number() == 1))
		return player_2;
	else
		return player_4;
}

void NewGameDialog::getBoardSize(int &w, int &h) const
{
	switch (m_pBoardSize->get_active_row_number())
	{
		case 0:
			w = 5; h = 5;
			break;
		case 1:
			w = 8; h = 8;
			break;
		case 2:
			w = 10; h = 10;
			break;
		case 3:
			w = 14; h = 14;
			break;
		default:
			w = 20; h = 20;
	}
}

bool NewGameDialog::isBoardHexagonal() const
{
	return m_pBoardShape->get_active_row_number() == 1;
}

// Get a bit set indicating whether each of the possible four players is an AI
const std::bitset<4> NewGameDialog::getAIPlayers() const
{
	std::bitset<4> results;
	results.reset();
	if (m_pPlayer1Box->get_active_row_number() == 1)
		results.set(0);
	if (m_pPlayer2Box->get_active_row_number() == 1)
		results.set(1);
	if (m_pPlayer3Box->get_active_row_number() == 1)
		results.set(2);
	if (m_pPlayer4Box->get_active_row_number() == 1)
		results.set(3);
	
	return results;
}
