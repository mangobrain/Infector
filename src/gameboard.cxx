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
#include "gameboard.hxx"

//
// Implementation
//

GameBoard::GameBoard(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml> &refXml)
	: Gtk::DrawingArea(cobject)
{
}

bool GameBoard::on_expose_event(GdkEventExpose *event)
{
	Glib::RefPtr<Gdk::Window> window = get_window();
	if (window)
	{
		Gtk::Allocation alloc = get_allocation();
		const int w(alloc.get_width());
		const int h(alloc.get_height());
		double x = 0, y = 0;
		double xinc = (const double)w/8.0;
		double yinc = (const double)h/8.0;

		Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				if (i % 2)
				{
					if (j % 2)
						cr->set_source_rgb(1, 0, 0);
					else
						cr->set_source_rgb(0, 0, 1);
				} else {
					if (j % 2)
						cr->set_source_rgb(0, 0, 1);
					else
						cr->set_source_rgb(1, 0, 0);
				}
				cr->rectangle(x, y, xinc, yinc);
				cr->fill();
				x += xinc;
			}
			y += yinc;
			x = 0;
		}
	}
	return true;
}
