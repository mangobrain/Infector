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

// System headers

// Project headers
#include "gamewindow.hxx"

//
// Implementation
//

// Entry point
int main(int argc, char *argv[])
{
	Gtk::Main kit(argc, argv);
	GameWindow gw;
	Gtk::Main::run(gw);
	return 0;
}


//
// GameWindow class
//

GameWindow::GameWindow()
	: m_refActionGroup(Gtk::ActionGroup::create()), m_refUIManager(Gtk::UIManager::create())
{
	// Create menu actions inside the action group
	m_refActionGroup->add(Gtk::Action::create("MenuFile", "_File"));
	m_refActionGroup->add(Gtk::Action::create("Quit", Gtk::Stock::QUIT));
	
	// Add the action group to a UI manager
	m_refUIManager->insert_action_group(m_refActionGroup);
	
	// Get the window to respond to keyboard shortcuts
	add_accel_group(m_refUIManager->get_accel_group());
	
	// Describe the visual layout of the menu
	Glib::ustring ui_info = 
		"<ui>"
		"	<menubar name='MenuBarMain'>"
		"		<menu action='MenuFile'>"
		"			<menuitem action='Quit' />"
		"		</menu>"
		"	</menubar>"
		"</ui>";
	m_refUIManager->add_ui_from_string(ui_info);
	
	// Add the root vbox to the window
	add(m_vbox);
	
	// Add the menu bar to the top of the vbox
	m_vbox.pack_start(m_refUIManager->get_widget("/MenuBarMain");
	
	show_all_children();
}
