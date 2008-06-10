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
#include "game.hxx"
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

	// The widget is painted directly by us, but we'll leave the library
	// to do its own double buffering, thanks.
	set_app_paintable(true);

	// Resize the board to the default 8x8
	pieces.resize(8);
	for (std::vector<std::vector<piece> >::iterator i = pieces.begin(); i != pieces.end(); ++i)
	{
		i->resize(8);
	}
	
	// Set the widget's background to the emtpy checkerboard so that
	// we don't have to redraw it in every single expose event.
	// The widget needs to be realised before we can get its Gdk::Window as a
	// drawable, so have setBackground called after signal_realize has fired.
	signal_realize().connect_notify(sigc::mem_fun(*this, &GameBoard::setBackground), true);
	
	// The background will need to be redrawn whenever the widget is resized
	signal_configure_event().connect(sigc::mem_fun(*this, &GameBoard::onResize), true);
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
		cr->set_antialias(Cairo::ANTIALIAS_NONE);

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
				switch (pieces.at(j).at(i))
				{
					case player_1:
						cr->set_source_rgb(1, 0, 0);
						break;
					case player_2:
						cr->set_source_rgb(0, 1, 0);
						break;
					case player_3:
						cr->set_source_rgb(0, 0, 1);
						break;
					case player_4:
						cr->set_source_rgb(1, 1, 0);
						break;
					case player_none:
						x += xinc;
						continue;
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

// Call this when a new game is started - will grab initial details
// and connect game event handlers to the instance's signals.
void GameBoard::newGame(Game* g)
{
	// TODO - Implement signal handlers and uncomment this
	/*g->move_made.connect(sigc::mem_fun(*this, &GameBoard::onMoveMade));
	g->invalid_move.connect(sigc::mem_fun(*this, &GameBoard::onInvalidMove));
	g->select_piece.connect(sigc::mem_fun(*this, &GameBoard::onSelectPiece));*/

	// Resize board - TODO: grab size from Game instance
	for (std::vector<std::vector<piece> >::iterator i = pieces.begin(); i != pieces.end(); ++i)
	{
		for (std::vector<piece>::iterator j = i->begin(); j != i->end(); ++j)
			*j = player_none;
	}

	// Set default board state
	pieces[0][7] = player_1;
	pieces[7][0] = player_1;
	pieces[0][0] = player_2;
	pieces[7][7] = player_2;

	// Refresh the board
	queue_draw();
}

// Set the widget's background pixmap to the empty board
void GameBoard::setBackground()
{
	// Get the window associated with the display area widget
	Glib::RefPtr<Gdk::Window> window = get_window();
	// Create a pixmap covering the size of the widget
	Gtk::Allocation alloc = get_allocation();
	int w(alloc.get_width());
	int h(alloc.get_height());
	Glib::RefPtr<Gdk::Pixmap> pixmap = Gdk::Pixmap::create(window, w, h, -1);
	// Get Cairo context for drawing on the pixmap
	Cairo::RefPtr<Cairo::Context> cr = pixmap->create_cairo_context();
	cr->set_antialias(Cairo::ANTIALIAS_NONE);
	
	// Draw board on pixmap
	double x = 0, y = 0;
	double xinc = (double)w/8.0;
	double yinc = (double)h/8.0;

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
	
	// Set pixmap as widget background
	window->set_back_pixmap(pixmap, false);
}

// Redraw the background at the current size when resized
bool GameBoard::onResize(GdkEventConfigure *event)
{
	setBackground();
	return true;
}
