// Copyright 2008-2009 Philip Allison <sane@not.co.uk>

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
#include "infector-i18n.hxx"

// Language headers
#include <memory>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <list>
#include <deque>

// Library headers
#include <gtkmm.h>

// System headers
#ifdef MINGW
#include <winsock2.h>
#endif

// Project headers
#include "gametype.hxx"
#include "socket.hxx"
#include "boardstate.hxx"
#include "game.hxx"
#include "gameboard.hxx"
#include "newgamedialog.hxx"
#include "serverstatusdialog.hxx"
#include "clientstatusdialog.hxx"
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
#ifdef ENABLE_NLS
	// Tell gettext where to find messages for our application's domain
	bindtextdomain(GETTEXT_PACKAGE, __INFECTOR_LOCALEDIR);
	// Messages are all in UTF-8
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	// Switch to our application's domain for string retrieval
	textdomain(GETTEXT_PACKAGE);
#endif

#ifdef MINGW
	// Start up Winsock 2.2
	WORD wsver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(wsver, &data);
#endif

	Gtk::Main kit(argc, argv);

#ifdef MINGW
	// Find "people" icon for server status dialogue,
	// and "infector" icon for about dialogue
	Glib::RefPtr<Gtk::IconTheme> it(Gtk::IconTheme::get_default());
	it->append_search_path(__INFECTOR_PKGDATADIR);
#endif

	// Install hooks for clicked URLs and email addresses in about dialogues
	Gtk::AboutDialog::set_url_hook(sigc::ptr_fun(onAboutURL));
	Gtk::AboutDialog::set_email_hook(sigc::ptr_fun(onAboutEmail));

	// Load main GtkBuilder file
	Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(__INFECTOR_PKGDATADIR "/infector.ui");
	
	// Instantiate main window & run Glib main loop
	GameWindow *pGw;
	refXml->get_widget_derived("mainwindow", pGw);
#ifdef MINGW
	// Set window icon
	pGw->set_icon_from_file(__INFECTOR_PKGDATADIR "/infector.ico");
#endif
	kit.run(*pGw);

#ifdef MINGW
	WSACleanup();
#endif
	
	delete pGw;
	return 0;
}


//
// GameWindow class
//

GameWindow::GameWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refXml)
	: Gtk::Window(cobject), m_refXml(refXml), m_pAboutDialog(NULL), m_pNewGameDialog(NULL),
	m_pGame(NULL)
{
	// Create ActionGroup for menu & toolbar items and their actions
	m_refActGrp = Gtk::ActionGroup::create();
	m_refActGrp->add(Gtk::Action::create("NewGame", Gtk::Stock::NEW),
		sigc::mem_fun(this, &GameWindow::onNewGame));
	m_refActGrp->add(Gtk::Action::create("Connect", Gtk::Stock::CONNECT),
		sigc::mem_fun(this, &GameWindow::onConnect));
	m_refActGrp->add(Gtk::Action::create("Quit", Gtk::Stock::QUIT),
		sigc::ptr_fun(&Gtk::Main::quit));
	m_refActGrp->add(Gtk::Action::create("About", Gtk::Stock::ABOUT),
		sigc::mem_fun(this, &GameWindow::onAbout));
	m_refActGrp->add(Gtk::Action::create("GameMenu", _("_Game")));
	m_refActGrp->add(Gtk::Action::create("HelpMenu", Gtk::Stock::HELP));

	// Instantiate main menu & toolbar
	m_refUIMan = Gtk::UIManager::create();
	m_refUIMan->insert_action_group(m_refActGrp);
	add_accel_group(m_refUIMan->get_accel_group());
	m_refUIMan->add_ui_from_file(__INFECTOR_PKGDATADIR "/menu.ui");
	
	// Add menu & toolbar to main window vbox
	Gtk::Widget *pMenubar = m_refUIMan->get_widget("/MenuBar");
	Gtk::Widget *pToolbar = m_refUIMan->get_widget("/ToolBar");
	Gtk::VBox *pVBox;
	m_refXml->get_widget("vbox", pVBox);
	pVBox->pack_start(*pMenubar, Gtk::PACK_SHRINK);
	pVBox->pack_start(*pToolbar, Gtk::PACK_SHRINK);
	
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
	// Instantiate the new game dialogue if not already done
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
		GameType gt;
		m_pNewGameDialog->getGameType(gt);
	
		// If there are remote players, show the server status dialogue,
		// which will block until a suitable number of remote players are found
		// Either way, stop the current running game and start a new one
		if (gt.anyPlayersOfType(pt_remote))
		{
			// Instantiate server status dialogue if not already done
			if (m_pServerStatusDialog.get() == NULL)
			{
				ServerStatusDialog *pServerStatusDialog;
				m_refXml->get_widget_derived("serverstatusdialog", pServerStatusDialog);
				m_pServerStatusDialog.reset(pServerStatusDialog);
			}
			
			m_pServerStatusDialog->setGameDetails(gt);
			response = m_pServerStatusDialog->run();
			m_pServerStatusDialog->hide();

			if (response != Gtk::RESPONSE_OK)
				return;
			else {
				m_pGame.reset(new Game(m_pBoard, gt));
				m_pGame->giveClientSockets(m_pServerStatusDialog->getClientSockets());
				m_pServerStatusDialog->clearClientSocketRefs();
			}
		}
		else
			m_pGame.reset(new Game(m_pBoard, gt));
		
		// Set status bar to initial state
		onMoveMade(0, 0, 0, 0, false);
		
		// Connect game event handlers
		m_pGame->move_made.connect(sigc::mem_fun(this, &GameWindow::onMoveMade));
		m_pGame->network_error.connect(sigc::mem_fun(this, &GameWindow::onNetworkError));
	}
}

// Connect to network game event handler
void GameWindow::onConnect()
{
	// Instantiate the connect dialogue if not already done
	if (m_pClientStatusDialog.get() == NULL)
	{
		ClientStatusDialog *pClientStatusDialog;
		m_refXml->get_widget_derived("clientstatusdialog", pClientStatusDialog);
		m_pClientStatusDialog.reset(pClientStatusDialog);
	}

	// Block whilst showing the dialogue, then hide it when it's dismissed
	m_pClientStatusDialog->setDefaults();
	int response = m_pClientStatusDialog->run();
	m_pClientStatusDialog->hide();

	if (response == Gtk::RESPONSE_OK)
	{
		GameType gt;
		m_pClientStatusDialog->getGameType(gt);
		
		// Show a message telling the current player what colour they are
		Glib::ustring msg(_("You are playing as "));

		if (gt.player_1 == pt_local)
			msg.append(_("red"));
		else if (gt.player_2 == pt_local)
			msg.append(_("green"));
		else if (gt.player_3 == pt_local)
			msg.append(_("blue"));
		else if (gt.player_4 == pt_local)
			msg.append(_("yellow"));

		Gtk::MessageDialog *m = new Gtk::MessageDialog((Gtk::Window&)(*this),
			msg, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
		m->run();
		m->hide();
		delete m;
		
		m_pGame.reset(new Game(m_pBoard, gt));
		m_pGame->giveServerSocket(m_pClientStatusDialog->getServerSocket());
		
		// Set status bar to initial state
		onMoveMade(0, 0, 0, 0, false);
		
		// Connect game event handlers
		m_pGame->move_made.connect(sigc::mem_fun(this, &GameWindow::onMoveMade));
		m_pGame->network_error.connect(sigc::mem_fun(this, &GameWindow::onNetworkError));
	}
}

// Network error - show popup & desensitise the board
void GameWindow::onNetworkError(const Glib::ustring &e)
{
	m_pBoard->endGame();
	Gtk::MessageDialog *m = new Gtk::MessageDialog((Gtk::Window&)(*this), e, false,
		Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	m->run();
	m->hide();
	delete m;
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
		std::ostringstream os; os << _("R: ") << s1;
		m_pRedStatusbar->push(os.str());
	} else
		m_pRedStatusbar->push(_("N/A"));
	if (s2 >= 0)
	{
		std::ostringstream os; os << _("G: ") << s2;
		m_pGreenStatusbar->push(os.str());
	} else
		m_pGreenStatusbar->push(_("N/A"));
	if (s3 >= 0)
	{
		std::ostringstream os; os << _("B: ") << s3;
		m_pBlueStatusbar->push(os.str());
	} else
		m_pBlueStatusbar->push(_("N/A"));
	if (s4 >= 0)
	{
		std::ostringstream os; os << _("Y: ") << s4;
		m_pYellowStatusbar->push(os.str());
	} else
		m_pYellowStatusbar->push(_("N/A"));
	
	// Set the status bar message
	if (gameover)
	{
		// The game is over - pop up a win/draw message
		m_pStatusbar->push(_("Game over"));
		Glib::ustring message(_("Tie: "));
		
		if (s1 > s2 && s1 > s3 && s1 > s4)
			message = _("Red wins!");
		else if (s2 > s1 && s2 > s3 && s2 > s4)
			message = _("Green wins!");
		else if (s3 > s2 && s3 > s1 && s3 > s4)
			message = _("Blue wins!");
		else if (s4 > s2 && s4 > s3 && s4 > s1)
			message = _("Yellow wins!");
		else {
			// Work out who the tie is between
			int max = std::max(std::max(s1, s2), std::max(s3, s4));
			std::ostringstream tiemsg;
			if (s1 == max)
				tiemsg << _("red") << ", ";
			if (s2 == max)
				tiemsg << _("green") << ", ";
			if (s3 == max)
				tiemsg << _("blue") << ", ";
			if (s4 == max)
				tiemsg << _("yellow") << ", ";
			message.append(tiemsg.str().substr(0, tiemsg.str().length() - 2));
		}

		m_pBoard->endGame();

		Gtk::MessageDialog *m = new Gtk::MessageDialog(*this, message, false,
			Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
		m->run();
		m->hide();
		delete m;
	}
	else
	{
		// The game is not over - set the status bar to indicate whose turn
		// it is next.  Put it in bold if it's a local human player.
		Glib::ustring msg;
		bool localplayer = false;
		piece currentplayer = m_pGame->getBoardState().getPlayer();

		if (m_pGame->getGameType().typeOf(currentplayer) == pt_local)
		{
			msg.append("<b>");
			localplayer = true;
		}

		switch (currentplayer)
		{
			case pc_player_1:
				msg.append(_("Red to play"));
				break;
			case pc_player_2:
				msg.append(_("Green to play"));
				break;
			case pc_player_3:
				msg.append(_("Blue to play"));
				break;
			default:
				msg.append(_("Yellow to play"));
				break;
		}

		if (localplayer)
			msg.append("</b>");

		// XXX BIG HACK to get at the label on the status bar directly.
		// Status bars don't officially support Pango markup, but labels do.
		Gtk::Bin *b = (Gtk::Bin*)(m_pStatusbar->children()[0].get_widget());
		Gtk::Label *l = (Gtk::Label*)(b->get_child());
		l->set_markup(msg);
	}
}
