// Copyright 2008-2009 Philip Allison <mangobrain@googlemail.com>

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

#ifndef __INFECTOR_GAME_HXX__
#define __INFECTOR_GAME_HXX__

class GameBoard;
class AI;
class ClientSocket;
class Socket;

// Class for main game logic, tying together players and the GUI
class Game: public sigc::trackable
{
	public:
		// Constructor - pass in game board so we can pick up
		// on emitted signals when it is clicked, and pass in
		// game properties (board size, number of players, etc.)
		Game(GameBoard *b, GameType &gt);
		
		// Destructor - destroy client and server sockets
		~Game();
		
		// Extra initialisation for servers - give list of client sockets
		void giveClientSockets(const std::deque<ClientSocket*> &clientsocks);
		
		// Extra initialisation for clients - include server socket
		void giveServerSocket(Socket *serversock);
	
		// Signals we can emit
		sigc::signal<void, const int, const int, const int, const int, const bool>
			move_made;
		sigc::signal<void> invalid_move;
		sigc::signal<void> select_piece;
		sigc::signal<void, const Glib::ustring&> network_error;
		
		const BoardState &getBoardState() const;
		const GameType &getGameType() const;

	private:
		// Game properties
		GameType m_GameType;

		// Board state
		BoardState m_BoardState;
		
		// Whether or not the game is over
		bool m_gameover;
		
		// Client sockets, if acting as network server
		std::deque<ClientSocket*> m_pClientSockets;

		// Server socket, if acting as network client
		Socket *m_pServerSocket;
		
		// Socket event handler connections
		sigc::connection serversockeventconn;
		std::deque<sigc::connection> clientsockeventconns;

		// Event handlers
		// Board clicked
		void onSquareClicked(const int x, const int y);
		// Client sockets
		bool handleClientSocks(Glib::IOCondition cond, ClientSocket *sock);
		void clientWriteError(const Glib::ustring &e);
		// Server sockets
		bool handleServerSock(Glib::IOCondition cond);
		void serverWriteError(const Glib::ustring &e);
		
		// See if a particular move is valid for the current player
		bool validMove(const int ax, const int ay, const int bx, const int by) const;

		// Destroy all client sockets and clear client socket list
		void destroyClientSockets();
		
		// Destroy server socket
		void destroyServerSocket();

		// Network input buffer
		char netbuf[4];
		// Number of bytes read for the current remote player's move
		size_t netbufsize;
		
		// AI player
		std::auto_ptr<AI> m_pAI;
};

#endif
