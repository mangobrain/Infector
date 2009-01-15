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
#include <cstring>
#include <sstream>
#include <cerrno>
#include <list>

// Library headers
#include <gtkmm.h>
#include <libglademm.h>

// Project headers
#include "serverstatusdialog.hxx"

// System headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

//
// Implementation
//

// Convenience function for showing an error popup
void ServerStatusDialog::errPop(const char* err) const
{
	Glib::ustring message(err);
	Gtk::MessageDialog *m = new Gtk::MessageDialog((Gtk::Window&)(*this), message, false,
		Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	m->run();
	m->hide();
	delete m;
}

// Constructor
ServerStatusDialog::ServerStatusDialog(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml> &refXml)
	: Gtk::Dialog(cobject)
{
	refXml->get_widget("serverportspin", m_pPortSpin);
	refXml->get_widget("serverportapplybutton", m_pApplyButton);
	refXml->get_widget("serverstartbutton", m_pStartButton);
	refXml->get_widget("servercancelbutton", m_pCancelButton);
	
	// Link the Apply button with the onApply method for opening a listen port
	m_pApplyButton->signal_clicked().connect(sigc::mem_fun(*this, &ServerStatusDialog::onApply));
	
	// XXX Set default value of server port spin button
	// - doesn't seem to work from within Glade
	m_pPortSpin->set_value(49152);
}

// Set controls back to default state
void ServerStatusDialog::on_response(int response_id)
{
	// Apply button is enabled and default
	m_pApplyButton->set_sensitive();
	m_pApplyButton->grab_default();
	
	// Port spin button is enabled and focussed
	m_pPortSpin->set_sensitive();
	m_pPortSpin->grab_focus();
	
	// OK button is disabled
	m_pStartButton->set_sensitive(false);
	
	// Close all listening sockets by removing all references to their IOChannels
	for (std::list<Glib::RefPtr<Glib::IOChannel> >::iterator i = serversocks.begin(); i != serversocks.end(); ++i)
	{
		i->reset();
	}
	for (std::list<sigc::connection>::iterator i = eventconns.begin(); i != eventconns.end(); ++i)
	{
		i->disconnect();
	}
	serversocks.clear();
	
	// TODO If response is cancel, close any open accepted sockets too.
	// If response is OK, disconnect event handlers from accepted sockets,
	// delete our references to them, but do not close them.
}

// Port apply button clicked
// Open listening port and start accepting clients
void ServerStatusDialog::onApply()
{
	// Disable port spinner and apply button
	m_pPortSpin->set_sensitive(false);
	m_pApplyButton->set_sensitive(false);
	
	// Convert port number to string
	std::ostringstream ostr;
	ostr << m_pPortSpin->get_value_as_int() << std::ends;
	
	// Open listening port
	
	// Start by enumerating available IP versions (v4/v6)
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	// Use IPv4 or v6
	hints.ai_family = AF_UNSPEC;
	// Use TCP
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	// Get a listening socket address on all available interface types
	// Service (port) is specified numerically
	hints.ai_flags = AI_ADDRCONFIG | AI_PASSIVE | AI_NUMERICSERV;
	// Do it
	addrinfo* results;
	int result = getaddrinfo(NULL, ostr.str().c_str(), &hints, &results);
	if (result)
	{
		// Something went wrong :(
		errPop(gai_strerror(result));
		freeaddrinfo(results);
		response(Gtk::RESPONSE_CANCEL);
	} else {
		// Open listening sockets on all returned addresses
		addrinfo* current = results;
		while (current != NULL)
		{
			// Create a socket
			SOCKET s = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
			if (s == INVALID_SOCKET)
			{
				errPop(strerror(errno));
				response(Gtk::RESPONSE_CANCEL);
				break;
			} else {
				// Bind it to the current address
				if (bind(s, current->ai_addr, current->ai_addrlen) == SOCKET_ERROR)
				{
					errPop(strerror(errno));
					response(Gtk::RESPONSE_CANCEL);
					close(s);
					break;
				} else {
					// Start it listening
					if (listen(s, 10) == SOCKET_ERROR)
					{
						errPop(strerror(errno));
						response(Gtk::RESPONSE_CANCEL);
						close(s);
						break;
					} else {
						// Connect it to signal handlers to be monitored
#ifndef MINGW
						Glib::RefPtr<Glib::IOChannel> ioc = Glib::IOChannel::create_from_fd(s);
#else
						Glib::RefPtr<Glib::IOChannel> ioc = Glib::IOChannel::create_from_win32_socket(s);
#endif
						// Bind the socket as one of the event handler's arguments, or we can't call accept()
						eventconns.push_back(Glib::signal_io().connect(sigc::bind(sigc::mem_fun(this, &ServerStatusDialog::handleServerSocks), s),
							ioc, Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL));

						// Close underlying socket automatically when IOChannel is destroyed,
						// and store IOChannel reference in our list of server sockets
						ioc->set_close_on_unref(true);
						serversocks.push_back(ioc);
					}
				}
			}
			// Go to next address
			current = current->ai_next;
		}
		// Discard socket addresses
		freeaddrinfo(results);
	}
}

// Handle incoming connections on server sockets
bool ServerStatusDialog::handleServerSocks(Glib::IOCondition cond, SOCKET s)
{
	if (cond == Glib::IO_IN)
	{
		sockaddr_storage newaddr;
		socklen_t newaddrlen = sizeof(newaddr);
		SOCKET newsock = accept(s, (sockaddr*) &newaddr, &newaddrlen);
		if (newsock == INVALID_SOCKET)
		{
			errPop(strerror(errno));
			response(Gtk::RESPONSE_CANCEL);
			return true;
		}
		// TODO Check sa_family of newaddr to see if it's a sockaddr_in
		// or a sockaddr_in6.  Store the socket and its address somewhere
		// such that it can be returned to calling code.
		// Create an IOChannel (which won't close on dereference) so we can
		// monitor to see if the client disconnects or errors.
		// Add client to GUI, including options to pick players.
		errPop("Accept success ;-)");
	} else {
		errPop("Error on listening socket");
		response(Gtk::RESPONSE_CANCEL);
	}
	return true;
}
