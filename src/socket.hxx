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

#ifndef INFECTOR_SOCKET_HXX
#define INFECTOR_SOCKET_HXX

class Socket : public Glib::Object
{
	public:
		// Constructor - take socket, set options & construct IOChannel
		Socket(const int socket);
		
		// Return whether or not we still have data to send
		bool readyForOutput() const
		{
			return (m_buffer.length() == 0);
		};
		
		// Put data in internal buffer & send it, using non-blocking I/O
		void writeChars(const char *data, const size_t amount);
		
		// Get socket - used as a kind of object ID, DO NOT use for
		// performing I/O directly on the socket
		const int getSocket() const
		{
			return m_socket;
		};
		
		// Get reference to internal IOChannel, for connecting signal handlers
		// to IOConditions
		const Glib::RefPtr<Glib::IOChannel> &getChannel() const
		{
			return m_pIOChannel;
		};
		
		// Signal emitted on write error
		sigc::signal<void, const Glib::ustring&> write_error;
		
	private:
		int m_socket;
		Glib::RefPtr<Glib::IOChannel> m_pIOChannel;
		
		std::string m_buffer;

		sigc::connection output_handler_connection;
		
		// Handler for when the socket becomes writeable -
		// send data from the buffer if we have any; otherwise, we are
		// ready for more data
		bool handleIOOut(Glib::IOCondition cond);
};

class ClientSocket : public Socket
{
	public:
		// Constructor - take socket, string represenation of address
		// and player colour; construct underlying Socket
		ClientSocket(const int socket, const Glib::ustring &address, const piece player)
			: Socket(socket), m_address(address), m_player(player)
		{};
		
		// Return player colour
		piece getPlayer() const
		{
			return m_player;
		};
		
		// Set player colour
		void setPlayer(const piece p)
		{
			m_player = p;
		};
		
		// Get address string
		const Glib::ustring &getAddress() const
		{
			return m_address;
		};
		
	private:
		Glib::ustring m_address;
		piece m_player;
};

#endif
