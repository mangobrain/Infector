// Copyright 2008-2009, 2012, 2018 Philip Allison <mangobrain@googlemail.com>

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


//
// Includes
//

// Standard
#include <config.h>

// Library headers
#include <gtkmm.h>

// Project headers
#include "gametype.hxx"
#include "newgamedialog.hxx"

//
// Implementation
//

// Constructor
NewGameDialog::NewGameDialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refXml)
	: Gtk::Dialog(cobject)
{
	// When the number of players is changed, show/hide the player 3 & 4 type controls
	refXml->get_widget("numplayerscombo", m_pNumPlayers);
	m_pNumPlayers->signal_changed().connect(sigc::mem_fun(this, &NewGameDialog::onChangePlayers));
	
	// When the board shape is changed, show/hide the player 3 & 4 type controls
	// and the number-of-players controls
	refXml->get_widget("boardshapecombo", m_pBoardShape);
	m_pBoardShape->signal_changed().connect(sigc::mem_fun(this, &NewGameDialog::onChangeShape));	
	
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

void NewGameDialog::getGameType(GameType &gt) const
{
	// Set width & height
	switch (m_pBoardSize->get_active_row_number())
	{
		case 0:
			gt.w = 5; gt.h = 5;
			break;
		case 1:
			gt.w = 8; gt.h = 8;
			break;
		case 2:
			gt.w = 10; gt.h = 10;
			break;
		case 3:
			gt.w = 14; gt.h = 14;
			break;
		default:
			gt.w = 20; gt.h = 20;
	}
	
	// Set square board flag
	gt.square = (m_pBoardShape->get_active_row_number() == 0);

	// Set type of players 1 and 2
	switch (m_pPlayer1Box->get_active_row_number())
	{
		case 0:
			gt.player_1 = pt_local;
			break;
		case 1:
			gt.player_1 = pt_ai;
			break;
		default:
			gt.player_1 = pt_remote;
	}
	switch (m_pPlayer2Box->get_active_row_number())
	{
		case 0:
			gt.player_2 = pt_local;
			break;
		case 1:
			gt.player_2 = pt_ai;
			break;
		default:
			gt.player_2 = pt_remote;
	}
	
	// Set types of players 3 and 4 if it's a 4-player game
	if (gt.square && (m_pNumPlayers->get_active_row_number() == 1))
	{
		switch (m_pPlayer3Box->get_active_row_number())
		{
			case 0:
				gt.player_3 = pt_local;
				break;
			case 1:
				gt.player_3 = pt_ai;
				break;
			default:
				gt.player_3 = pt_remote;
		}
		switch (m_pPlayer4Box->get_active_row_number())
		{
			case 0:
				gt.player_4 = pt_local;
				break;
			case 1:
				gt.player_4 = pt_ai;
				break;
			default:
				gt.player_4 = pt_remote;
		}
	}
}
