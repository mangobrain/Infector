// Copyright 2008-2009, 2012, 2018 Philip Allison <mangobrain@googlemail.com>

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
#include <config.h>

// Library headers
#include <glibmm.h>

// Project headers
#include "gametype.hxx"
#include "socket.hxx"

// System headers
#include <sys/types.h>
#ifdef MINGW
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

//
// Implementation
//

// Constructor - take socket, set options & construct IOChannel
Socket::Socket(const int socket)
#ifdef MINGW
	: m_socket(socket), m_pIOChannel(Glib::IOChannel::create_from_win32_socket(socket))
#else
	: m_socket(socket), m_pIOChannel(Glib::IOChannel::create_from_fd(socket))
#endif
{
	// Set TCP_NODELAY on the socket - we want data to be sent out
	// as soon as possible, regardless of the potentially tiny amounts
	// being sent.
	int val = 1;
#ifdef MINGW
	setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char*)(&val), sizeof(int));
#else
	setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(int));
#endif
	
	// Set the IOChannel to non-blocking
	m_pIOChannel->set_flags(Glib::IO_FLAG_NONBLOCK);
	
	// Get the channel to handle closing the socket for us
	m_pIOChannel->set_close_on_unref(true);
	
	// No character encoding - binary data please
	m_pIOChannel->set_encoding("");
	
	// No buffering please
	m_pIOChannel->set_buffered(false);
}

// Put data in internal buffer & send it, using non-blocking I/O
void Socket::writeChars(const char *data, const size_t amount)
{
	m_buffer.append(data, amount);
	handleIOOut(Glib::IO_OUT);
}

// Handler for when the socket becomes writeable -
// send data from the buffer if we have any; otherwise, we are
// ready for more data
bool Socket::handleIOOut(Glib::IOCondition cond)
{
	if (m_buffer.length() > 0)
	{
		// Disconnect output event handler, and send data in a loop
		// until we get IO_STATUS_AGAIN
		output_handler_connection.disconnect();
		while (true)
		{
			size_t b = 0;
			
			// Raise an error signal if we get an exception during writing.
			// This class does non-blocking writes asynchronously from the
			// code requesting the write, so the calling code is not able
			// to catch the exception itself directly.
			Glib::IOStatus s;
			try {
				s = m_pIOChannel->write(m_buffer.c_str(), m_buffer.length(), b);
			}
			catch (Glib::IOChannelError &e)
			{
				write_error(e.what());
				return false;
			}
			
			// How much data was sent?  Either truncate the buffer accordingly,
			// or exit if the answer is "everything".
			if (b < m_buffer.length())
				m_buffer.assign(m_buffer.substr(b));
			else {
				m_buffer.clear();
				return false;
			}
			
			// Otherwise, decide what to do next.
			if (s == Glib::IO_STATUS_AGAIN)
			{
				// Sending more data would block.  Monitor the IOChannel to see
				// when it next would not, and call this function again.
				output_handler_connection = Glib::signal_io().connect(
					sigc::mem_fun(*this, &ClientSocket::handleIOOut),
					m_pIOChannel, Glib::IO_OUT);
				return true;
			}
			else if (s == Glib::IO_STATUS_NORMAL)
				// Sending more data would not block - do it now.
				continue;
			// Must have been an error condition to get here.
			return false;
		}
	}
	return false;
}
