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
#include <cstdlib>
#include <algorithm>
#include <deque>

// Library headers
#include <gtkmm.h>
#include <libglademm.h>

// System headers

// Project headers
#include "gametype.hxx"
#include "boardstate.hxx"
#include "game.hxx"
#include "gameboard.hxx"
#include "ai.hxx"
#include "socket.hxx"

//
// Implementation
//

// Constructor
Game::Game(GameBoard* b, GameType &gt)
	: m_GameType(gt), m_BoardState(&m_GameType), haveserversocket(false), netbufsize(0), m_pAI(NULL)
{
	// All signals will be auto-disconnected on destruction, because
	// this class inherits from sigc::trackable, so don't bother
	// storing returned connection objects.

	// Connect square clicked handler to game board instance
	b->square_clicked.connect(sigc::mem_fun(this, &Game::onSquareClicked));
	
	// Clear the board and set its initial state for this game
	b->newGame(this, &m_BoardState, &m_GameType);
	
	// Create an AI object if necessary
	if (gt.anyPlayersOfType(pt_ai))
	{
		m_pAI.reset(new AI(this, &m_BoardState, &m_GameType));
		m_pAI->square_clicked.connect(sigc::mem_fun(this, &Game::onSquareClicked));
	}
}

// Extra initialisation for servers - give list of client sockets
void Game::giveClientSockets(const std::deque<Glib::RefPtr<ClientSocket> > &clientsocks)
{
	// Take a copy of client sockets, and connect up network event handlers
	for (std::deque<Glib::RefPtr<ClientSocket> >::const_iterator i = clientsocks.begin();
		i != clientsocks.end(); ++i)
	{
		m_ClientSockets.push_back(*i);
		Glib::signal_io().connect(
			sigc::bind(sigc::mem_fun(this, &Game::handleClientSocks), *i),
				(*i)->getChannel(), Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);
		(*i)->write_error.connect(sigc::mem_fun(this, &Game::clientWriteError));
	}
}

// Extra initialisation for clients - give server socket
void Game::giveServerSocket(const Glib::RefPtr<Socket> &serversock)
{
	// Take a reference to the socket
	m_ServerSocket = serversock;
	haveserversocket = true;

	// Set up network event handlers
	Glib::signal_io().connect(
		sigc::mem_fun(this, &Game::handleServerSock), m_ServerSocket->getChannel(),
			Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);
	m_ServerSocket->write_error.connect(sigc::mem_fun(this, &Game::serverWriteError));
}

// Write error occurred on client socket
void Game::clientWriteError(const Glib::ustring &e)
{
	m_ClientSockets.clear();
	Glib::ustring msg(_("Write error on client socket: "));
	msg.append(e);
	network_error(msg);
}

// Write error occurred on server socket
void Game::serverWriteError(const Glib::ustring &e)
{
	m_ServerSocket.reset();
	Glib::ustring msg(_("Write error on server socket: "));
	msg.append(e);
	network_error(msg);
}

// Handle events on client sockets
bool Game::handleClientSocks(Glib::IOCondition cond, Glib::RefPtr<ClientSocket> sock)
{
	if (cond != Glib::IO_IN)
	{
		m_ClientSockets.clear();
		network_error(_("I/O error on client socket"));
		return false;
	}
	else
	{
		// TODO - Only listen for input events on sockets we're actually
		// interested in (i.e. the current player's socket, if they're remote),
		// so that we don't interfere with immediate event pumping?
		// This could be an important consideration if the sleep below doesn't
		// give the board a chance to redraw between "clicks", because the way
		// in which it works will have to be overhauled (have an idle event
		// handler to initiate the timer for the second sleep?  If so, do the
		// same for AI::onMoveMade & AI::makeMove).
		// This may not be strictly necessary, but might be nice.
	
		// It's an input event.  Read in either all or part of a move,
		// depending on whether or not we already have part of the move
		// in our buffer.
		
		// If it isn't this client's turn, a network error has occurred.
		if (m_BoardState.getPlayer() != sock->getPlayer())
		{
			m_ClientSockets.clear();
			network_error(_("Client disconnected or unexpected data received"));
			return false;
		}
		
		size_t read = 0;
		try {
			sock->getChannel()->read(netbuf + netbufsize, 4 - netbufsize, read);
		}
		catch (Glib::IOChannelError &e)
		{
			m_ClientSockets.clear();
			network_error(_("Error reading from client socket"));
			return false;
		}
		if (read == 0)
		{
			m_ClientSockets.clear();
			network_error(_("Client disconnected"));
			return false;
		}
		netbufsize += read;
		
		if (netbufsize == 4)
		{
			// We read in all the data.  Display the move that was made.
			netbufsize = 0;
			// Make the player's move, if it's valid.
			if (validMove(netbuf[0], netbuf[1], netbuf[2], netbuf[3]))
			{
				// Highlight the square we're going to move then
				// make the actual move in 0.5 second time increments
				// (to let people see what's going on).
				onSquareClicked(netbuf[0], netbuf[1]);
				// Let the board redraw to highlight the selected piece
				while (Gtk::Main::events_pending())
					Gtk::Main::iteration();
				Glib::usleep(500000);
				onSquareClicked(netbuf[2], netbuf[3]);

				// Echo move to other connected clients.
				// Yes, this does introduce a delay into clients viewing
				// their opponent's moves, but it's the simplest way to
				// ensure we don't start receiving another move before
				// the game state has advanced to the next player's turn.
				for (std::deque<Glib::RefPtr<ClientSocket> >::const_iterator i = m_ClientSockets.begin();
					i != m_ClientSockets.end(); ++i)
				{
					// Don't send the move back to the client we received it from
					if (sock != (*i))
						(*i)->writeChars(netbuf, 4);
				}
			}
		}

		return true;
	}
}

// Handle events on the server socket
bool Game::handleServerSock(Glib::IOCondition cond)
{
	if (cond != Glib::IO_IN)
	{
		m_ServerSocket.reset();
		network_error(_("I/O error on server socket"));
		return false;
	}
	else
	{
		// Are we expecting to be sent a move at the moment?
		if (m_GameType.isPlayerType(m_BoardState.getPlayer(), pt_local))
		{
			m_ServerSocket.reset();
			network_error(_("Server disconnected or unexpected data received"));
			return false;
		}
		
		// Read in all or part of a move
		size_t read = 0;
		try {
			m_ServerSocket->getChannel()->read(netbuf + netbufsize, 4 - netbufsize, read);
		}
		catch (Glib::IOChannelError &e)
		{
			m_ServerSocket.reset();
			network_error(_("Error reading from server socket"));
			return false;
		}
		if (read == 0)
		{
			m_ServerSocket.reset();
			network_error(_("Server disconnected"));
			return false;
		}
		netbufsize += read;
		
		if (netbufsize == 4)
		{
			// We read in all the data.  Display the move that was made.
			netbufsize = 0;
			// Make the player's move, if it's valid.
			if (validMove(netbuf[0], netbuf[1], netbuf[2], netbuf[3]))
			{
				// Highlight the square we're going to move then
				// make the actual move in 0.5 second time increments
				// (to let people see what's going on).
				onSquareClicked(netbuf[0], netbuf[1]);
				// Let the board redraw to highlight the selected piece
				while (Gtk::Main::events_pending())
					Gtk::Main::iteration();
				Glib::usleep(500000);
				onSquareClicked(netbuf[2], netbuf[3]);
			}
		}

		return true;
	}
}

// Is a given move valid for the current player?
bool Game::validMove(const int ax, const int ay, const int bx, const int by) const
{
	if (m_BoardState.getPieceAt(ax, ay) != m_BoardState.getPlayer())
		return false;
	if (ax == bx && ay == by)
		return false;
	if (m_BoardState.getPieceAt(bx, by) != pc_player_none)
		return false;
	if (m_BoardState.getAdjacency(ax, ay, bx, by) == 0)
		return false;
	return true;
}

// Board square clicked
void Game::onSquareClicked(const int x, const int y)
{
	// If the current player is clicking on their own piece, highlight it.
	if (m_BoardState.getPieceAt(x, y) == m_BoardState.getPlayer())
	{
		m_BoardState.setSelectedSquare(x, y);
		select_piece();
		return;
	}
	
	// Is a piece currently highlighted?
	int xsel, ysel;
	m_BoardState.getSelectedSquare(xsel, ysel);
	if (xsel == -1 || ysel == -1)
		// Nope, and they haven't clicked one of their own.
		invalid_move();
	else {
		// Yes! Is it a valid move?
		if (((xsel == x) && (ysel == y)) || (m_BoardState.getPieceAt(x, y) != pc_player_none))
			// Nope - they tried to move the piece onto itself, or the
			// destination square isn't empty/doesn't exist.
			invalid_move();
		else {
			bool clone = false;
			unsigned int distance = m_BoardState.getAdjacency(x, y, xsel, ysel);
			if (distance == 1)
				// Yes - they moved one square, so clone it and update score.
				clone = true;
			else if (distance == 2)
				// Yes - they moved two squares, so jump it.
				clone = false;
			else
			{
				// Nope - the square's empty, but it's out of range.
				invalid_move();
				return;
			}
			// If we get here, a valid move was chosen, so update the board
			// and advance the state of the game.
			m_BoardState.setPieceAt(x, y, m_BoardState.getPlayer());
			if (!clone)
				m_BoardState.setPieceAt(xsel, ysel, pc_player_none);
			m_BoardState.clearSelection();
			
			// Can the next player actually move?
			// If not, skip until we find someone who can.
			// If we come full circle, the game has ended.
			piece endplayer = m_BoardState.getPlayer();
			piece nextplayer = m_BoardState.nextPlayer();
			bool gameover = false;
			while (!m_BoardState.canMove(nextplayer))
			{
				nextplayer = m_BoardState.nextPlayer();
				if (nextplayer == endplayer)
				{
					gameover = true;
					break;
				}
			}
			
			// TODO - Change this to pass in a move structure.
			// Will mean changing all onMoveMade signal handlers.
			move_made(xsel, ysel, x, y, gameover);

			// Send move to network clients if we're a server and it
			// was a local player/AI that made the move (the move has
			// already been echoed if it was received over the network)
			if (!m_ClientSockets.empty())
			{
				playertype pt = m_GameType.typeOf(endplayer);
				if (pt == pt_ai || pt == pt_local)
				{
					netbuf[0] = xsel;
					netbuf[1] = ysel;
					netbuf[2] = x;
					netbuf[3] = y;
					for (std::deque<Glib::RefPtr<ClientSocket> >::const_iterator i = m_ClientSockets.begin();
						i != m_ClientSockets.end(); ++i)
					{
						(*i)->writeChars(netbuf, 4);
					}
				}

				// If the game has ended, close the client sockets.
				if (gameover)
					m_ClientSockets.clear();
			}

			// Send move to server if we're a client and it
			// was a local player.
			if (haveserversocket && m_GameType.typeOf(endplayer) == pt_local)
			{
				netbuf[0] = xsel;
				netbuf[1] = ysel;
				netbuf[2] = x;
				netbuf[3] = y;
				m_ServerSocket->writeChars(netbuf, 4);
			}
		}
	}
}

const BoardState& Game::getBoardState() const
{
	return m_BoardState;
}
