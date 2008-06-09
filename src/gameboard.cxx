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

// Constructor
GameBoard::GameBoard(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml> &refXml)
	: Gtk::DrawingArea(cobject)
{
	// Connect mouse click events to the onClick handler
	signal_button_press_event().connect(sigc::mem_fun(*this, &GameBoard::onClick));
}

// Expose (redraw) event handler
bool GameBoard::on_expose_event(GdkEventExpose *event)
{
	// Get the window associated with the display area widget
	Glib::RefPtr<Gdk::Window> window = get_window();
	if (window)
	{
		// Create a Cairo context for drawing onto the window
		Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

		// Set clip region for the context so we only really repaint
		// the area which needs to be redrawn
		cr->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
		cr->clip();

		// Draw checkerboard pattern scaled up to widget's current size
		Gtk::Allocation alloc = get_allocation();
		const int w(alloc.get_width());
		const int h(alloc.get_height());
		double x = 0, y = 0;
		double xinc = (const double)w/8.0;
		double yinc = (const double)h/8.0;

		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				if (i % 2)
				{
					if (j % 2)
						cr->set_source_rgb(0.8, 0.8, 0.8);
					else
						cr->set_source_rgb(0.5, 0.5, 0.5);
				} else {
					if (j % 2)
						cr->set_source_rgb(0.5, 0.5, 0.5);
					else
						cr->set_source_rgb(0.8, 0.8, 0.8);
				}
				cr->rectangle(x, y, xinc, yinc);
				cr->fill();
				x += xinc;
			}
			y += yinc;
			x = 0;
		}

		// Board outline
		cr->set_source_rgb(0, 0, 0);
		cr->rectangle(0, 0, w, h);
		cr->stroke();
	}
	return true;
}

// Mouse click handler
bool GameBoard::onClick(GdkEventButton *event)
{
	// Work out which square was clicked on
	Gtk::Allocation alloc = get_allocation();
	const int w(alloc.get_width());
	const int h(alloc.get_height());
	double xinc = (const double)w/8.0;
	double yinc = (const double)h/8.0;
	int x = (int)(event->x / xinc);
	int y = (int)(event->y / yinc);

	// Emit the square_clicked signal with these coordinates
	square_clicked(x, y);
	
	return true;
}
