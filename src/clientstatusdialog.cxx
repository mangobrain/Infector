// Copyright 2009 Philip Allison <sane@not.co.uk>

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
#include <cerrno>
#include <cstring>
#include <sstream>
#include <memory>

// Library headers
#include <gtkmm.h>
#include <libglademm.h>

// System headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Project headers
#include "gametype.hxx"
#include "socket.hxx"

//
// Implementation
//

// Convenience function for showing an error popup
void ClientStatusDialog::errPop(const char *err) const
{
	Glib::ustring message(err);
	Gtk::MessageDialog *m = new Gtk::MessageDialog((Gtk::Window&)(*this), message, false,
		Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	m->run();
	m->hide();
	delete m;
}

// Override base classes' on_response method to
// disconnect our own event handlers from the server socket
void ClientStatusDialog::on_response(int response_id)
{
	sockeventconn.disconnect();
	if (response_id != Gtk::RESPONSE_OK)
		serversock.reset();
}

// Constructor - called by glademm by get_widget_derived
ClientStatusDialog::ClientStatusDialog(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml> &refXml)
	: Gtk::Dialog(cobject)
{
	refXml->get_widget("clientportspin", m_pPortSpin);
	refXml->get_widget("clientaddrentry", m_pAddressEntry);
	refXml->get_widget("clientconnectbutton", m_pConnectButton);
	
	refXml->get_widget("csgamedescription", m_pGameDescription);
	refXml->get_widget("csgamedetailsframe", m_pGameDetailsFrame);
	
	refXml->get_widget("csbluelabel", m_pBlueLabel);
	refXml->get_widget("csyellowlabel", m_pYellowLabel);
	
	refXml->get_widget("csredclient", m_pRedClient);
	refXml->get_widget("csgreenclient", m_pGreenClient);
	refXml->get_widget("csblueclient", m_pBlueClient);
	refXml->get_widget("csyellowclient", m_pYellowClient);

	// XXX Set default value of server port spin button
	// - doesn't seem to work from within Glade
	m_pPortSpin->set_value(49152);

	// Link the Connect button with the onConnect method for connecting to specified server
	m_pConnectButton->signal_clicked().connect(sigc::mem_fun(this, &ClientStatusDialog::onConnect));

	setDefaults();
}

// Set GUI to default state
void ClientStatusDialog::setDefaults()
{
	m_pPortSpin->set_sensitive();
	m_pAddressEntry->set_sensitive();
	m_pAddressEntry->grab_focus();
	m_pConnectButton->set_sensitive();
	m_pConnectButton->grab_default();
	m_pGameDetailsFrame->hide();
	
	// No. of bytes of network input we know we need == size of header
	bytesremaining = 11;
	
	// Have we received game details yet?
	detailsreceived = false;
}

// Connect button click event handler
void onConnect()
{
	m_pPortSpin->set_sensitive(false);
	m_pAddressEntry->set_sensitive(false);
	m_pConnectButton->set_sensitive(false);

	// Convert port number to string
	std::ostringstream ostr;
	ostr << m_pPortSpin->get_value_as_int() << std::ends;

	// Connect to specified server

	// Start by looking up given address
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	// Use IPv4 or v6
	hints.ai_family = AF_UNSPEC;
	// Use TCP
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	// Get a socket address on any available interface type
	// Service (port) is specified numerically
	hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV;
	// Do it
	addrinfo* results;
	int result = getaddrinfo(NULL, ostr.str().c_str(), &hints, &results);
	
	if (result)
	{
		// Something went wrong :(
		errPop(gai_strerror(errno));
		freeaddrinfo(results);
		response(Gtk::RESPONSE_CANCEL);
	} else {
		// It worked - try to connect using the first resolved address
		// Create a socket
		int s = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
		if (s < 0)
		{
			errPop(strerror(errno));
			response(Gtk::RESPONSE_CANCEL);
			return;
		}
		// Connect to server address
		if (connect(s, results->ai_addr, results->ai_addrlen) < 0)
		{
			errPop(strerror(errno));
			response(Gtk::RESPONSE_CANCEL);
			close(s);
			return;
		}
		// Create a Socket object from it
		serversock.reset(new Socket(s));
		// Attach the underlying IOChannel to our event handler
		sockeventconn = Glib::signal_io.connect(
			sigc::mem_fun(this, &ClientStatusDialog::handleServerSock),
				 Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);
	}
}

// Handle events on the server socket
bool ClientStatusDialog::handleServerSock(Glib::IOCondition cond)
{
	if (cond != Glib::IO_IN)
	{
		errPop("Error on server socket");
		response(Gtk::RESPONSE_CANCEL);
		return false;
	} else {
		// s|h (board shape)
		// 2|4 (num. players)
		// 4 player descriptions, each two bytes:
		//    0 - local human/none
		//    1 - AI
		//    2 - remote
		//    Second byte is either 0 or client address length for type 2
		// Client's player number (0 - 4, 0 for connected but unassigned)
		// Header total: 11 bytes
		//
		// Client addresses (in order, one for each type 2)
		
		size_t netbufsize = netbuf.length();
		size_t read = 0;
		if (bytesremaining > 0)
		{
			// Grab header; calculate remaining number of bytes (player
			// addresses), retrieve those too; when we have a full set of
			// data, update the GUI
			std::auto_ptr<char> buf(new char[bytesremaining]);
			try {
				serversock->getChannel()->read(buf, bytesremaining, read);
			}
			catch (Glib::IOChannelError &e)
			{
				errPop("Error reading from server");
				response(Gtk::RESPONSE_CANCEL);
				return false;
			}
			if (read == 0)
			{
				errPop("Server disconnected");
				response(Gtk::RESPONSE_CANCEL);
				return false;
			}
			bytesremaining -= read;
			netbuf.append(buf, read);
			if (bytesremaining == 0)
			{
				if (netbuf.length() == 11)
				{
					// Parse header to find out how much more data we need
					// Bytes 3, 5, 7 and 9 (from 0) contain player address lengths
					bytesremaining += (size_t)(netbuf.at(3));
					bytesremaining += (size_t)(netbuf.at(5));
					bytesremaining += (size_t)(netbuf.at(7));
					bytesremaining += (size_t)(netbuf.at(9));
				} else {
					// Parse full game details and update GUI
					m_pGameDetailsFrame->show();
					if (netbuf.at(1) == 2)
					{
						m_pBlueLabel->hide(); m_pBlueClient->hide();
						m_pYellowLabel->hide(); m_pYellowClient->hide();
					} else {
						m_pBlueLabel->show(); m_pBlueClient->show();
						m_pYellowLabel->show(); m_pYellowClient->show();
					}
					// Set client address labels
					if ((size_t)(netbuf.at(3)) > 0)
					else
				}
			}
			return true;
		} else {
			errPop("Unexpected data from server");
			response(Gtk::RESPONSE_CANCEL);
			return false;
		}
	}
}
