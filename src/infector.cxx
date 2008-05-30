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
	
	Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(PKGDATADIR "/infector.glade");
	
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
	: Gtk::Window(cobject), m_refXml(refXml)
{
}
