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
#include <cstdlib>
#include <memory>

// Library headers
#include <gtkmm.h>

// System headers

// Project headers
#include "gametype.hxx"
#include "boardstate.hxx"
#include "game.hxx"
#include "gameboard.hxx"
#include "ai.hxx"

//
// Implementation
//

// Constructor
GameBoard::GameBoard(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refXml)
	: Gtk::DrawingArea(cobject), m_DefaultGameType(), m_DefaultBoardState(&m_DefaultGameType),
		m_pBoardState(NULL), m_pGameType(NULL), bw(m_DefaultGameType.w), bh(m_DefaultGameType.h)
{
	// Connect mouse click events to the onClick handler
	signal_button_press_event().connect(sigc::mem_fun(this, &GameBoard::onClick));
	set_sensitive(true);

	// The widget is painted directly by us, but we'll leave the library
	// to do its own double buffering, thanks.
	set_app_paintable(true);
	
	// Set the widget's background to the emtpy checkerboard so that
	// we don't have to redraw it in every single expose event.
	// The widget needs to be realised before we can get its Gdk::Window as a
	// drawable, so have setBackground called after signal_realize has fired.
	signal_realize().connect_notify(sigc::mem_fun(this, &GameBoard::setBackground), true);
	
	// The background will need to be redrawn whenever the widget is resized
	signal_configure_event().connect(sigc::mem_fun(this, &GameBoard::onResize), true);
	
	// Don't put pieces on the default board - it looks like a game is in play
	m_DefaultBoardState.setPieceAt(0, 0, pc_player_none);
	m_DefaultBoardState.setPieceAt(0, 7, pc_player_none);
	m_DefaultBoardState.setPieceAt(7, 0, pc_player_none);
	m_DefaultBoardState.setPieceAt(7, 7, pc_player_none);
	m_DefaultBoardState.clearSelection();
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
		int w(alloc.get_width());
		int h(alloc.get_height());

		const BoardState *current = ((m_pBoardState == NULL) ? &m_DefaultBoardState : m_pBoardState);
		if (!((m_pGameType == NULL) ? m_DefaultGameType.square : m_pGameType->square))
		{
			// Horrid hexagonal board
			double x = 0;
			double y = (double)h / 2;
			double xinc = (double)w / ((bw - current->getInitialOffset()) * 2);
			double yinc = xinc;
			double hxinc = xinc / 2;
			double hyinc = yinc / 2;
			
			for (int i = 0; i < bh; ++i)
			{
				y = ((double) h / 2) + (i * hyinc);
				x = ((hxinc * 2) * (i - current->getInitialOffset())) + (hxinc / 2);
				for (int j = 0; j < bw; ++j)
				{
					if (current->getPieceAt(j, i) != pc_no_such_square)
					{
						bool draw = true;
						float r = 0, g = 0, b = 0;
						switch (current->getPieceAt(j, i))
						{
							case pc_player_1:
								cr->set_source_rgb(1, 0, 0);
								r = 0.8;
								break;
							case pc_player_2:
								cr->set_source_rgb(0, 1, 0);
								g = 0.8;
								break;
							case pc_player_3:
								cr->set_source_rgb(0, 0, 1);
								b = 0.8;
								break;
							case pc_player_4:
								cr->set_source_rgb(1, 1, 0);
								r = 0.8; g = 0.8;
								break;
							default:
								draw = false;
						}
						if (draw)
						{
							cr->move_to(x, y);
							cr->rel_line_to(hxinc, hyinc);
							cr->rel_line_to(hxinc, 0);
							cr->rel_line_to(hxinc, -hyinc);
							cr->rel_line_to(-hxinc, -hyinc);
							cr->rel_line_to(-hxinc, 0);
							
							cr->close_path();
							cr->fill_preserve();
							cr->clip_preserve();
							cr->set_source_rgb(r, g, b);
							cr->unset_dash();
							cr->stroke();
							cr->reset_clip();
						}
						// Highlight currently selected square and possible moves
						int xsel, ysel;
						current->getSelectedSquare(xsel, ysel);
						draw = false;
						bool self = false;
						if (i == ysel && j == xsel)
						{
							cr->set_source_rgb(1, 1, 1);
							draw = true;
							self = true;
						}
						else if (xsel != -1 && ysel != -1)
						{
							unsigned int distance = current->getAdjacency(xsel, ysel, j, i);
							if (distance == 1)
							{
								cr->set_source_rgb(1, 0.5, 1);
								draw = true;
							}
							else if (distance == 2)
							{
								cr->set_source_rgb(1, 0, 1);
								draw = true;
							}
						}
						if (self || (draw && (current->getPieceAt(j, i) == pc_player_none)))
						{
							std::vector<double> dashes;
							dashes.push_back(2);
							cr->set_dash(dashes, 0);
							cr->arc(x + (hxinc * 1.5), y, hxinc / 2.0, 0, 2 * M_PI);
							cr->stroke();
						}
					}
					x += (hxinc * 2);
					y -= hyinc;
				}
			}
		} else {
			// Traditional square board
			double x = 0, y = 0;
			double xinc = (double)w/bw;
			double yinc = (double)h/bh;
			double hxinc = xinc / 2.0;
			double hyinc = yinc / 2.0;
			for (int i = 0; i < bh; ++i)
			{
				for (int j = 0; j < bw; ++j)
				{
					bool draw = true;
					float r = 0, g = 0, b = 0;
					switch (current->getPieceAt(j, i))
					{
						case pc_player_1:
							cr->set_source_rgb(1, 0, 0);
							r = 0.8;
							break;
						case pc_player_2:
							cr->set_source_rgb(0, 1, 0);
							g = 0.8;
							break;
						case pc_player_3:
							cr->set_source_rgb(0, 0, 1);
							b = 0.8;
							break;
						case pc_player_4:
							cr->set_source_rgb(1, 1, 0);
							r = 0.8; g = 0.8;
							break;
						default:
							draw = false;
					}
					if (draw)
					{
						cr->rectangle(x, y, xinc, yinc);
						cr->fill_preserve();
						cr->clip_preserve();
						cr->set_source_rgb(r, g, b);
						cr->unset_dash();
						cr->stroke();
						cr->reset_clip();
					}
					
					// Highlight currently selected square and possible moves
					int xsel, ysel;
					current->getSelectedSquare(xsel, ysel);
					draw = false;
					bool self = false;
					if (i == ysel && j == xsel)
					{
						cr->set_source_rgb(1, 1, 1);
						draw = true;
						self = true;
					}
					else if (xsel != -1 && ysel != -1)
					{
						unsigned int distance = current->getAdjacency(xsel, ysel, j, i);
						if (distance == 1)
						{
							cr->set_source_rgb(1, 0.5, 1);
							draw = true;
						}
						else if (distance == 2)
						{
							cr->set_source_rgb(1, 0, 1);
							draw = true;
						}
					}
					if (self || (draw && (current->getPieceAt(j, i) == pc_player_none)))
					{
						std::vector<double> dashes;
						dashes.push_back(2);
						cr->set_dash(dashes, 0);
						cr->arc(x + hxinc, y + hyinc, hxinc / 2.0, 0, 2 * M_PI);
						cr->stroke();
					}
					
					x += xinc;
				}
				y += yinc;
				x = 0;
			}
		}
	}
	return true;
}

// Mouse click handler
bool GameBoard::onClick(GdkEventButton *event)
{
	if (m_pBoardState == NULL || m_pGameType == NULL ||
		!(m_pGameType->isPlayerType(m_pBoardState->getPlayer(), pt_local)))
	{
		return true;
	}

	// Work out which square was clicked on
	Gtk::Allocation alloc = get_allocation();
	const int w(alloc.get_width());
	const int h(alloc.get_height());
	
	const BoardState *current = ((m_pBoardState == NULL) ? &m_DefaultBoardState : m_pBoardState);
	if (!((m_pGameType == NULL) ? m_DefaultGameType.square : m_pGameType->square))
	{
		// Horrid hexagonal board
		double x = 0;
		double y = (double)h / 2;
		double xinc = (double)w / ((bw - current->getInitialOffset()) * 2);
		double yinc = xinc;
		double hxinc = xinc / 2;
		double hyinc = yinc / 2;
		
		for (int i = 0; i < bh; ++i)
		{
			y = ((double) h / 2) + (i * hyinc);
			x = ((hxinc * 2) * (i - current->getInitialOffset())) + (hxinc / 2);
			for (int j = 0; j < bw; ++j)
			{
				if (current->getPieceAt(j, i) != pc_no_such_square)
				{
					// Point-in-hexagon test is split into three parts:
					// - if relative x coord is between (hxinc) and (2 * hxinc), point in rectangle
					// - if relative x coord is between 0 and hxinc, we're testing the left-hand diagonals
					// - if relative x coord is between (2 * hxinc) and (3 * hxinc), we're testing
					//   the right-hand diagonals
					// Diagonals are at 45 degrees, so testing for being between the lines is easy.
					int relx = (int)(event->x - x);
					int rely = (int)(event->y - y);
					if (relx >= hxinc && relx <= (hxinc * 2))
					{
						if (rely <= hyinc && rely >= -hyinc)
						{
							square_clicked(j, i);
							return true;
						}
					}
					else if (relx > 0 && relx < hxinc)
					{
						if (abs(rely) <= relx)
						{
							square_clicked(j, i);
							return true;
						}
					}
					else if (relx > (hxinc * 2) && relx <= (hxinc * 3))
					{
						if (abs(rely) <= ((hxinc * 3) - relx))
						{
							square_clicked(j, i);
							return true;
						}
					}
				}
				x += (hxinc * 2);
				y -= hyinc;
			}
		}
	} else {
		// Traditional square board
		double xinc = (const double)w/bw;
		double yinc = (const double)h/bh;
		int x = (int)(event->x / xinc);
		int y = (int)(event->y / yinc);

		// Emit the square_clicked signal with these coordinates
		square_clicked(x, y);
	}
	
	return true;
}

// Call this when a new game is started - will grab initial details
// and connect game event handlers to the instance's signals.
void GameBoard::newGame(Game *g, const BoardState *b, const GameType *gt)
{
	// TODO - Implement signal handlers and uncomment this
	g->move_made.connect(sigc::mem_fun(this, &GameBoard::onMoveMade));
	//g->invalid_move.connect(sigc::mem_fun(this, &GameBoard::onInvalidMove));
	g->select_piece.connect(sigc::mem_fun(this, &GameBoard::queue_draw));

	// Store pointer to shared board state
	m_pBoardState = b;
	m_pGameType = gt;

	// Store board width & height locally to keep code clean
	bw = m_pGameType->w;
	bh = m_pGameType->h;

	// Store board shape so that later on, when the game has ended,
	// the board doesn't magically flip from hexagonal back to square
	// on the next on_expose
	m_DefaultGameType.square = m_pGameType->square;

	// Refresh the board
	setBackground();
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
	//cr->set_antialias(Cairo::ANTIALIAS_NONE);

	// Draw board on pixmap
	const BoardState *current = ((m_pBoardState == NULL) ? &m_DefaultBoardState : m_pBoardState);
	if (!((m_pGameType == NULL) ? m_DefaultGameType.square : m_pGameType->square))
	{
		// Horrid hexagonal board
		Glib::RefPtr<const Gtk::Style> style = get_style();
		Gdk::Color bg = style->get_bg(Gtk::STATE_NORMAL);
		cr->set_source_rgb(bg.get_red_p(), bg.get_green_p(), bg.get_blue_p());
		cr->rectangle(0, 0, w, h);
		cr->fill();

		double x = 0;
		double y = (double)h / 2;
		double xinc = (double)w / ((bw - current->getInitialOffset()) * 2);
		double yinc = xinc;
		double hxinc = xinc / 2;
		double hyinc = yinc / 2;
		
		for (int i = 0; i < bh; ++i)
		{
			y = ((double) h / 2) + (i * hyinc);
			x = ((hxinc * 2) * (i - current->getInitialOffset())) + (hxinc / 2);
			for (int j = 0; j < bw; ++j)
			{
				if (current->getPieceAt(j, i) != pc_no_such_square)
				{
					if (i % 2)
					{
						if (j % 2)
							cr->set_source_rgb(0.8, 0.8, 0.8);
						else
							cr->set_source_rgb(0.5, 0.5, 0.5);
					} else {
						if (j % 2)
							cr->set_source_rgb(0.4, 0.4, 0.4);
						else
							cr->set_source_rgb(0.7, 0.7, 0.7);
					}
					
					cr->move_to(x, y);
					cr->rel_line_to(hxinc, hyinc);
					cr->rel_line_to(hxinc, 0);
					cr->rel_line_to(hxinc, -hyinc);
					cr->rel_line_to(-hxinc, -hyinc);
					cr->rel_line_to(-hxinc, 0);
					
					cr->close_path();
					cr->fill();
				}
				x += (hxinc * 2);
				y -= hyinc;
			}
		}
	} else {
		// Traditional square board	
		double x = 0, y = 0;
		double xinc = (double)w/bw;
		double yinc = (double)h/bh;
		
		cr->set_source_rgb(0.8, 0.8, 0.8);
		cr->rectangle(0, 0, w, h);
		cr->fill();

		for (int i = 0; i < bh; ++i)
		{
			for (int j = 0; j < bw; ++j)
			{
				bool draw = true;
				if (i % 2)
				{
					if (j % 2)
						draw = false;
					else
						cr->set_source_rgb(0.5, 0.5, 0.5);
				} else {
					if (j % 2)
						cr->set_source_rgb(0.5, 0.5, 0.5);
					else
						draw = false;
				}
				if (draw)
				{
					cr->rectangle(x, y, xinc, yinc);
					cr->fill();
				}
				x += xinc;
			}
			y += yinc;
			x = 0;
		}
	}
	
	// Set pixmap as widget background
	window->set_back_pixmap(pixmap, false);
}

// Redraw the background at the current size when resized
bool GameBoard::onResize(GdkEventConfigure *event)
{
	setBackground();
	return true;
}

void GameBoard::endGame()
{
	m_pGameType = NULL;
}

void GameBoard::onMoveMade(const int start_x, const int start_y, const int end_x, const int end_y, const bool gameover)
{
	// TODO - Some form of animation?
	// Redraw the board immediately
	queue_draw();
	while (Gtk::Main::events_pending())
		Gtk::Main::iteration();
	
	// Disable clicking when game is over or it's not a local player's turn - until next game starts
	if (gameover)
		endGame();
}
