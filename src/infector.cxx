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
#include <memory>
#include <cstdlib>

// Library headers
#include <gtkmm.h>
#include <libglademm.h>

// System headers

// Project headers
#include "game.hxx"
#include "gameboard.hxx"
#include "gamewindow.hxx"

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
	// Try to open an email client with xdg-email.
	Glib::ustring command("xdg-email \"");
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
		Gtk::Dialog *pNewGameDialog;
		m_refXml->get_widget("newgamedialog", pNewGameDialog);
		m_pNewGameDialog.reset(pNewGameDialog);
		// XXX Set default items for our ComboBoxes.
		// Doing this in the Glade XML itself causes errors.
		Gtk::ComboBox *pComboBox;
		m_refXml->get_widget("numplayerscombo", pComboBox);
		pComboBox->set_active(0);	// 2 players
		m_refXml->get_widget("boardsizecombo", pComboBox);
		pComboBox->set_active(0);	// 8x8 board
		m_refXml->get_widget("redcombo", pComboBox);
		pComboBox->set_active(0);	// one human..
		m_refXml->get_widget("greencombo", pComboBox);
		pComboBox->set_active(1);	// ..versus the AI
		m_refXml->get_widget("bluecombo", pComboBox);
		pComboBox->set_active(0);	// others are human if enabled
		m_refXml->get_widget("yellowcombo", pComboBox);
		pComboBox->set_active(0);
	}
	
	// Block whilst showing the dialogue, then hide it when it's dismissed
	int response = m_pNewGameDialog->run();
	m_pNewGameDialog->hide();

	// Process the response from the dialogue
	if (response == Gtk::RESPONSE_OK)
	{
		// Stop the current running game and start a new one
		delete m_pGame.get();
		m_pGame.reset(new Game(m_pBoard));
		m_pBoard->newGame(m_pGame.get());
	}
}
