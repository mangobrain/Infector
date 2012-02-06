// Copyright 2009 Philip Allison <mangobrain@googlemail.com>

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
#include <cerrno>
#include <cstring>
#include <sstream>

// Library headers
#include <gtkmm.h>

// System headers
#include <sys/types.h>
#ifdef MINGW
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#endif

// Project headers
#include "gametype.hxx"
#include "socket.hxx"
#include "clientstatusdialog.hxx"

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
	{
		delete serversock;
		serversock = NULL;
	}
}

// Constructor - called by glademm by get_widget_derived
ClientStatusDialog::ClientStatusDialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refXml)
	: Gtk::Dialog(cobject), serversock(NULL)
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
	bytesremaining = 13;
	
	// Have we received game details yet?
	detailsreceived = false;
	netbuf.clear();
}

// Connect button click event handler
void ClientStatusDialog::onConnect()
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
	// TODO - support v6 on Vista?
#ifdef MINGW
	hints.ai_family = AF_INET;
#else
	hints.ai_family = AF_UNSPEC;
#endif
	// Use TCP
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	// Get a socket address on any available interface type
	// Service (port) is specified numerically
	// TODO - Make this test more fine-grained, as ADDRCONFIG and
	// NUMERICSERV *are* available on Vista and above
#ifdef MINGW
	hints.ai_flags = 0;
#else
	hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV;
#endif
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
#ifdef MINGW
			closesocket(s);
#else
			close(s);
#endif
			return;
		}
		// Create a Socket object from it
		serversock = new Socket(s);
		// Attach the underlying IOChannel to our event handler
		sockeventconn = Glib::signal_io().connect(
			sigc::mem_fun(this, &ClientStatusDialog::handleServerSock),
				 serversock->getChannel(),
				 	Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);
	}
}

const char *getLabel(char p)
{
	switch (p)
	{
		case 0:
			return _("Host");
		case 1:
			return _("Computer");
		default:
			return _("<i>Empty</i>");
	}
}

// Handle events on the server socket
bool ClientStatusDialog::handleServerSock(Glib::IOCondition cond)
{
	if (cond != Glib::IO_IN)
	{
		errPop(_("Error on server socket"));
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
		// width & height
		// Header total: 13 bytes
		//
		// Client addresses (in order, one for each type 2)
		
		size_t read = 0;
		if (bytesremaining > 0)
		{
			// Grab header; calculate remaining number of bytes (player
			// addresses), retrieve those too; when we have a full set of
			// data, update the GUI
			char *buf = new char[bytesremaining];
			try {
				serversock->getChannel()->read(buf, bytesremaining, read);
			}
			catch (Glib::IOChannelError &e)
			{
				errPop(_("Error reading from server"));
				response(Gtk::RESPONSE_CANCEL);
				delete[] buf;
				return false;
			}
			if (read == 0)
			{
				errPop(_("Server disconnected"));
				response(Gtk::RESPONSE_CANCEL);
				delete[] buf;
				return false;
			}

			bytesremaining -= read;
			netbuf.append(buf, read);
			delete[] buf;
			
			// If we've already received at least one full update,
			// and we receive a '1', take it as meaning that the
			// host has clicked "OK" on the server status dialogue.
			if (detailsreceived && netbuf.length() == 1 && netbuf.at(0) == 1)
			{
				response(Gtk::RESPONSE_OK);
				return false;
			}

			// We've received either a full header, or a full complete update
			if (bytesremaining == 0)
			{
				// Full header
				if (netbuf.length() == 13)
				{
					// Validate header data
					//
					// If it isn't a square or a hexagonal board,
					// isn't two or four players, is a hexagonal board
					// with other than two players, has any unrecognised
					// player types, has a client player number greater
					// than the number of players, or gives addresses for
					// players local to the server (local/AI), it isn't
					// valid game data.
					
					if ((netbuf.at(0) != 's' && netbuf.at(0) != 'h')
						|| (netbuf.at(1) != 2 && netbuf.at(1) != 4)
						|| (netbuf.at(0) == 'h' && netbuf.at(1) != 2)
						|| (netbuf.at(2) > 2) || (netbuf.at(4) > 2)
						|| (netbuf.at(6) > 2) || (netbuf.at(8) > 2)
						|| (netbuf.at(10) > netbuf.at(1))
						|| (netbuf.at(2) != 2 && netbuf.at(3) != 0)
						|| (netbuf.at(4) != 2 && netbuf.at(5) != 0)
						|| (netbuf.at(6) != 2 && netbuf.at(7) != 0)
						|| (netbuf.at(8) != 2 && netbuf.at(9) != 0))
					{
						errPop(_("Invalid data from server"));
						response(Gtk::RESPONSE_CANCEL);
						return false;
					}
					
					// Parse header to find out how much more data we need
					// Bytes 3, 5, 7 and 9 (from 0) contain player address lengths
					
					bytesremaining += (size_t)(netbuf.at(3));
					bytesremaining += (size_t)(netbuf.at(5));
					bytesremaining += (size_t)(netbuf.at(7));
					bytesremaining += (size_t)(netbuf.at(9));
					
					// Fill in GameType structure
					m_GameType.square = (netbuf.at(0) == 's');
					
					// The "local" player is indicated by our client number,
					// all other players are remote
					if (netbuf.at(2) == 2 && netbuf.at(10) == 1)
						m_GameType.player_1 = pt_local;
					else
						m_GameType.player_1 = pt_remote;
					if (netbuf.at(4) == 2 && netbuf.at(10) == 2)
						m_GameType.player_2 = pt_local;
					else
						m_GameType.player_2 = pt_remote;
					if (netbuf.at(1) == 4)
					{
						if (netbuf.at(6) == 2 && netbuf.at(10) == 3)
							m_GameType.player_3 = pt_local;
						else
							m_GameType.player_3 = pt_remote;
						if (netbuf.at(8) == 2 && netbuf.at(10) == 4)
							m_GameType.player_4 = pt_local;
						else
							m_GameType.player_4 = pt_remote;
					} else {
						m_GameType.player_3 = pt_none;
						m_GameType.player_4 = pt_none;
					}
					
					m_GameType.w = netbuf.at(11);
					m_GameType.h = netbuf.at(12);
				}
				
				// Full update (could be full header with no address info
				// attached, hence the second "if" rather than an "else")
				if (bytesremaining == 0)
				{
					detailsreceived = true;
					
					// Parse full game details and update GUI
					// Set client address labels
					
					size_t rl = (size_t)(netbuf.at(3));
					size_t gl = (size_t)(netbuf.at(5));
					size_t bl = (size_t)(netbuf.at(7));
					size_t yl = (size_t)(netbuf.at(9));
					if (rl > 0)
						m_pRedClient->set_label(netbuf.substr(13, rl).c_str());
					else
						m_pRedClient->set_label(getLabel(netbuf.at(2)));
					if (gl > 0)
						m_pGreenClient->set_label(netbuf.substr(13 + rl, gl).c_str());
					else
						m_pGreenClient->set_label(getLabel(netbuf.at(4)));
					m_pGameDetailsFrame->show();
					if (netbuf.at(1) == 2)
					{
						m_pBlueLabel->hide(); m_pBlueClient->hide();
						m_pYellowLabel->hide(); m_pYellowClient->hide();
					} else {
						m_pBlueLabel->show(); m_pBlueClient->show();
						m_pYellowLabel->show(); m_pYellowClient->show();
						if (bl > 0)
							m_pBlueClient->set_label(
								netbuf.substr(13 + rl + gl, bl).c_str());
						else
							m_pBlueClient->set_label(getLabel(netbuf.at(6)));
						if (yl > 0)
							m_pYellowClient->set_label(
								netbuf.substr(13 + rl + gl + bl, yl).c_str());
						else
							m_pYellowClient->set_label(getLabel(netbuf.at(8)));
					}
					
					// Mark client's own details in bold
					Glib::ustring boldclient("<b>");
					Gtk::Label *clientlabel = NULL;
					
					if (m_GameType.player_1 == pt_local)
						clientlabel = m_pRedClient;
					else if (m_GameType.player_2 == pt_local)
						clientlabel = m_pGreenClient;
					else if (m_GameType.player_3 == pt_local)
						clientlabel = m_pBlueClient;
					else if (m_GameType.player_4 == pt_local)
						clientlabel = m_pYellowClient;
					
					if (clientlabel != NULL)
					{
						boldclient.append(clientlabel->get_label());
						boldclient.append("</b>");
						clientlabel->set_label(boldclient);
					}
					
					// Build game description string
					Glib::ustring description;
					if (m_GameType.square)
						description = _("Square board, ");
					else
						description = _("Hexagonal board, ");
					description += Glib::ustring::compose("%1x%2, ", m_GameType.w, m_GameType.h);
					if ((!m_GameType.square) || (m_GameType.player_3 == pt_none))
						description += _("2 players");
					else
						description += _("4 players");
					m_pGameDescription->set_label(description);
					
					// Get ready to receive an updated set of game details
					
					bytesremaining = 13;
					netbuf.clear();
				}
			}
			return true;
		} else {
			errPop(_("Unexpected data from server"));
			response(Gtk::RESPONSE_CANCEL);
			return false;
		}
	}
}
