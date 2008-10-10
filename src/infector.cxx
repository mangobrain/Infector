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


//
// Includes
//

// Standard
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

// Language headers
#include <memory>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <bitset>

// Library headers
#include <gtkmm.h>
#include <libglademm.h>

// System headers

// Project headers
#include "boardstate.hxx"
#include "game.hxx"
#include "gameboard.hxx"
#include "newgamedialog.hxx"
#include "gamewindow.hxx"
#include "ai.hxx"

//
// Implementation
//

void onAboutURL(Gtk::AboutDialog &d, const Glib::ustring &url)
{
	// XXX This is really, really hackish.
	// Try to open the clicked URL with xdg-open then gnome-open.
	Glib::ustring command("xdg-open ");
	command += url;
	command += " || gnome-open ";
	command += url;
	system(command.c_str());
}

void onAboutEmail(Gtk::AboutDialog &d, const Glib::ustring &addr)
{
	// XXX This is also really, really hackish.
	// Try to open an email client with xdg-email then gnome-open.
	Glib::ustring command("xdg-email \"");
	command += addr;
	command += "\" || gnome-open \"mailto:";
	command += addr;
	command += "\"";
	system(command.c_str());
}

// Entry point
int main(int argc, char *argv[])
{
	Gtk::Main kit(argc, argv);

	// Install hooks for clicked URLs and email addresses in about dialogues
	Gtk::AboutDialog::set_url_hook(sigc::ptr_fun(onAboutURL));
	Gtk::AboutDialog::set_email_hook(sigc::ptr_fun(onAboutEmail));

	// Load main Glade file
	Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(PKGDATADIR "/infector.glade");
	
	// Instantiate main window & run Glib main loop
	GameWindow *pGw;
	refXml->get_widget_derived("mainwindow", pGw);
	kit.run(*pGw);
	
	delete pGw;
	return 0;
}


//
// GameWindow class
//

GameWindow::GameWindow(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml> &refXml)
	: Gtk::Window(cobject), m_refXml(refXml), m_pAboutDialog(NULL), m_pNewGameDialog(NULL),
	m_pGame(NULL)
{
	// Link the "About" menu item to the onAbout method
	Gtk::MenuItem *pAbout;
	m_refXml->get_widget("aboutmenuitem", pAbout);
	pAbout->signal_activate().connect(sigc::mem_fun(*this, &GameWindow::onAbout));

	// Link the "New" menu item & button to the onNewGame method
	Gtk::MenuItem *pNewGame;
	m_refXml->get_widget("newgamemenuitem", pNewGame);
	pNewGame->signal_activate().connect(sigc::mem_fun(*this, &GameWindow::onNewGame));
	Gtk::ToolButton *pNewGameButton;
	m_refXml->get_widget("newtoolbutton", pNewGameButton);
	pNewGameButton->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::onNewGame));
	
	// Link the "Quit" menu item to the hide method
	Gtk::MenuItem *pQuit;
	m_refXml->get_widget("quitmenuitem", pQuit);
	pQuit->signal_activate().connect(sigc::ptr_fun(&Gtk::Main::quit));

	// Grab pointer to the widget on which the board is drawn
	m_refXml->get_widget_derived("drawingarea", m_pBoard);
	
	// Get bars for showing scores during play
	m_refXml->get_widget("redscorebar", m_pRedStatusbar);
	m_refXml->get_widget("greenscorebar", m_pGreenStatusbar);
	m_refXml->get_widget("bluescorebar", m_pBlueStatusbar);
	m_refXml->get_widget("yellowscorebar", m_pYellowStatusbar);
	m_refXml->get_widget("statusbar", m_pStatusbar);
}

// "About" event handler 
void GameWindow::onAbout()
{
	// Instantiate the about dialogue if not already done
	// XXX Not 100% sure I should do this once and keep the instance,
	// but calling get_widget multiple times doesn't seem to work.
	if (m_pAboutDialog.get() == NULL)
	{
		Gtk::AboutDialog *pAboutDialog;
		m_refXml->get_widget("aboutdialog", pAboutDialog);
		m_pAboutDialog.reset(pAboutDialog);
	}
	
	// Block whilst showing the dialogue, then hide it when it's dismissed
	m_pAboutDialog->run();
	m_pAboutDialog->hide();
}

// New game event handler
void GameWindow::onNewGame()
{
	// Instantiate the about dialogue if not already done
	if (m_pNewGameDialog.get() == NULL)
	{
		NewGameDialog *pNewGameDialog;
		m_refXml->get_widget_derived("newgamedialog", pNewGameDialog);
		m_pNewGameDialog.reset(pNewGameDialog);
	}
	
	// Block whilst showing the dialogue, then hide it when it's dismissed
	int response = m_pNewGameDialog->run();
	m_pNewGameDialog->hide();

	// Process the response from the dialogue
	if (response == Gtk::RESPONSE_OK)
	{
		// Stop the current running game and start a new one
		int w, h;
		m_pNewGameDialog->getBoardSize(w, h);
		m_pGame.reset(new Game(m_pBoard, m_pNewGameDialog->getLastPlayer(), w, h, m_pNewGameDialog->isBoardHexagonal(), m_pNewGameDialog->getAIPlayers()));
		onMoveMade(0, 0, 0, 0, false);
		m_pGame->move_made.connect(sigc::mem_fun(this, &GameWindow::onMoveMade));
	}
}

// Move made - update status bar, and show popup if game has ended
void GameWindow::onMoveMade(const int ax, const int ay, const int bx, const int by, const bool gameover)
{
	int s1, s2, s3, s4;
	m_pGame->getBoardState().getScores(s1, s2, s3, s4);
	m_pRedStatusbar->pop();
	m_pGreenStatusbar->pop();
	m_pBlueStatusbar->pop();
	m_pYellowStatusbar->pop();
	m_pStatusbar->pop();
	if (s1 >= 0)
	{
		std::ostringstream os; os << "R: " << s1;
		m_pRedStatusbar->push(os.str());
	} else
		m_pRedStatusbar->push("N/A");
	if (s2 >= 0)
	{
		std::ostringstream os; os << "G: " << s2;
		m_pGreenStatusbar->push(os.str());
	} else
		m_pGreenStatusbar->push("N/A");
	if (s3 >= 0)
	{
		std::ostringstream os; os << "B: " << s3;
		m_pBlueStatusbar->push(os.str());
	} else
		m_pBlueStatusbar->push("N/A");
	if (s4 >= 0)
	{
		std::ostringstream os; os << "Y: " << s4;
		m_pYellowStatusbar->push(os.str());
	} else
		m_pYellowStatusbar->push("N/A");
	
	if (gameover)
	{
		m_pStatusbar->push("Game over");
		Glib::ustring message("Tie: ");
		
		if (s1 > s2 && s1 > s3 && s1 > s4)
			message = "Red wins!";
		else if (s2 > s1 && s2 > s3 && s2 > s4)
			message = "Green wins!";
		else if (s3 > s2 && s3 > s1 && s3 > s4)
			message = "Blue wins!";
		else if (s4 > s2 && s4 > s3 && s4 > s1)
			message = "Yellow wins!";
		else {
			// Work out who the tie is between
			int max = std::max(std::max(s1, s2), std::max(s3, s4));
			std::ostringstream tiemsg;
			if (s1 == max)
				tiemsg << "red, ";
			if (s2 == max)
				tiemsg << "green, ";
			if (s3 == max)
				tiemsg << "blue, ";
			if (s4 == max)
				tiemsg << "yellow, ";
			message.append(tiemsg.str().substr(0, tiemsg.str().length() - 2));
		}
			
		Gtk::MessageDialog *m = new Gtk::MessageDialog(*this, message, false,
			Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
		m->run();
		m->hide();
		delete m;
	} else {
		switch (m_pGame->getBoardState().getPlayer())
		{
			case player_1:
				m_pStatusbar->push("Red to play");
				break;
			case player_2:
				m_pStatusbar->push("Green to play");
				break;
			case player_3:
				m_pStatusbar->push("Blue to play");
				break;
			default:
				m_pStatusbar->push("Yellow to play");
				break;
		}
	}
}
