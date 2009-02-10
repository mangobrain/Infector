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

// Language headers
#include <cstring>
#include <sstream>
#include <cerrno>
#include <list>
#include <deque>
#include <algorithm>
#include <functional>

// Library headers
#include <gtkmm.h>
#include <libglademm.h>

// Project headers
#include "gametype.hxx"
#include "socket.hxx"
#include "serverstatusdialog.hxx"

// System headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

//
// Implementation
//

// Convenience function for showing an error popup
void ServerStatusDialog::errPop(const char *err) const
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
	: Gtk::Dialog(cobject), m_pGameType(NULL), redrow(0), greenrow(0), bluerow(0), yellowrow(0), requiredclients(0)
{
	refXml->get_widget("serverportspin", m_pPortSpin);
	refXml->get_widget("serverportapplybutton", m_pApplyButton);
	refXml->get_widget("serverstartbutton", m_pStartButton);
	refXml->get_widget("ssgamedescription", m_pGameDescription);
	
	refXml->get_widget("serverclientstable", m_pClientTable);
	
	refXml->get_widget("ssclientlabel1", m_paClientLabels[0]);
	refXml->get_widget("ssclientlabel2", m_paClientLabels[1]);
	refXml->get_widget("ssclientlabel3", m_paClientLabels[2]);
	refXml->get_widget("ssclientlabel4", m_paClientLabels[3]);
	
	refXml->get_widget("ssclientkick1", m_paClientKickButtons[0]);
	refXml->get_widget("ssclientkick2", m_paClientKickButtons[1]);
	refXml->get_widget("ssclientkick3", m_paClientKickButtons[2]);
	refXml->get_widget("ssclientkick4", m_paClientKickButtons[3]);
	
	refXml->get_widget("ssbluelabel", m_pBlueLabel);
	refXml->get_widget("ssyellowlabel", m_pYellowLabel);
	
	refXml->get_widget("ssredclient", m_pRedClient);
	refXml->get_widget("ssgreenclient", m_pGreenClient);
	refXml->get_widget("ssblueclient", m_pBlueClient);
	refXml->get_widget("ssyellowclient", m_pYellowClient);
	
	// Link the Apply button with the onApply method for opening a listen port
	m_pApplyButton->signal_clicked().connect(sigc::mem_fun(this, &ServerStatusDialog::onApply));
	
	// XXX Set default value of server port spin button
	// - doesn't seem to work from within Glade
	m_pPortSpin->set_value(49152);
	
	// Attach combo boxes for assigning clients to player colours
	// to the client info display table.
	for (size_t i = 0; i < 4; ++i)
		m_pClientTable->attach(m_aClientComboBoxes[i], 1, 2, i, i + 1);
}

// Set player description labels in the game details area.
// Return true if the player slot is filled (AI, local, or a connected remote player)
bool setPlayerDescription(const playertype pt, Gtk::Label *label, int target_row = 0,
	Gtk::ComboBoxText *clientcombos = NULL, Gtk::Label **clientaddrs = NULL)
{
	switch (pt)
	{
		case pt_ai:
			label->set_label("Computer");
			return true;
		case pt_local:
			label->set_label("Local");
			return true;
		case pt_none:
			return false;
		default:
			{
				// If the player is a remote client, set it to Empty,
				// unless one of the client combo boxes is set to the
				// row for this player - in which case set it to the
				// description of that client.
				label->set_label("<i>Empty</i>");
				if (clientcombos != NULL)
				{
					for (size_t i = 0; i < 4; ++i)
					{
						if (clientcombos[i].get_active_row_number() == target_row)
						{
							label->set_label(clientaddrs[i]->get_label());
							return true;
						}
					}
				}
				return false;
			}
	}
}

// Called whenever one of the client type combo boxes is changed
void ServerStatusDialog::onClientComboChange(const size_t id)
{
	// Ensure we do not have two players trying to play the same
	// colour by unsetting any other combo boxes with the same active row
	// Keep ClientSocket structures up to date as well
	int row = m_aClientComboBoxes[id].get_active_row_number();
	if (row == redrow)
			clientsockets[id]->setPlayer(pc_player_1);
	else if (row == greenrow)
			clientsockets[id]->setPlayer(pc_player_2);
	else if (row == bluerow)
			clientsockets[id]->setPlayer(pc_player_3);
	else if (row == yellowrow)
			clientsockets[id]->setPlayer(pc_player_4);
	else
			clientsockets[id]->setPlayer(pc_player_none);
	for (size_t i = 0; i < 4; ++i)
	{
		if (i == id)
			continue;
		if (m_aClientComboBoxes[i].get_active_row_number() == row)
		{
			m_aClientComboBoxes[i].set_active(0);
			if (i < clientsockets.size())
				clientsockets[i]->setPlayer(pc_player_none);
		}
	}
	
	setGUIFromClientState();
}

// Set game description label text, and set Clients and Game Details
// tables to their initial states
void ServerStatusDialog::setGameDetails(const GameType &gt)
{
	// TODO - Remove this and have a hand-over function which lets calling code
	// retrieve our ClientSocket RefPtrs then clears the local list, which
	// must be called if the dialogue response is OK
	clientsockets.clear();

	m_pGameType = &gt;

	// Build game description string
	Glib::ustring description;
	if (gt.square)
		description = "Square board, ";
	else
		description = "Hexagonal board, ";
	description += Glib::ustring::compose("%1x%2, ", gt.w, gt.h);
	if ((!gt.square) || (gt.player_3 == pt_none))
		description += "2 players";
	else
		description += "4 players";
	
	// Put it on UI
	m_pGameDescription->set_label(description);
	
	// Hide blue & yellow player labels if it's a 2 player game
	if ((!gt.square) || (gt.player_3 == pt_none))
	{
		m_pBlueLabel->hide(); m_pBlueClient->hide();
		m_pYellowLabel->hide(); m_pYellowClient->hide();
	} else {
		m_pBlueLabel->show(); m_pBlueClient->show();
		m_pYellowLabel->show(); m_pYellowClient->show();
	}
	
	// Work out how many client connections to accept
	requiredclients = 0;
	if (gt.player_1 == pt_remote)
	{
		++requiredclients;
	}
	if (gt.player_2 == pt_remote)
	{
		++requiredclients;
	}
	if (gt.square)
	{
		if (gt.player_3 == pt_remote)
		{
			++requiredclients;;
		}
		if (gt.player_4 == pt_remote)
		{
			++requiredclients;
		}
	}
	
	// Disconnect all client combo box event handlers before manually
	// modifying their state, so we don't get into a recursion loop
	for (std::list<sigc::connection>::iterator i = clientcomboconns.begin(); i != clientcomboconns.end(); ++i)
		i->disconnect();
	clientcomboconns.clear();
	
	// Put player selection text on client combo boxes.
	// Store which row of the client combo boxes corresponds to each player.
	redrow = -1; greenrow = -1; yellowrow = -1; bluerow = -1;
	for (size_t i = 0; i < 4; ++i)
	{
		m_aClientComboBoxes[i].clear_items();
		m_aClientComboBoxes[i].append_text("None");
		int currentrow = 1;
		if (gt.player_1 == pt_remote)
		{
			m_aClientComboBoxes[i].append_text("Red");
			redrow = currentrow++;
		}
		if (gt.player_2 == pt_remote)
		{
			m_aClientComboBoxes[i].append_text("Green");
			greenrow = currentrow++;
		}
		if (gt.square)
		{
			if (gt.player_3 == pt_remote)
			{
				m_aClientComboBoxes[i].append_text("Blue");
				bluerow = currentrow++;
			}
			if (gt.player_4 == pt_remote)
			{
				m_aClientComboBoxes[i].append_text("Yellow");
				yellowrow = currentrow++;
			}
		}
	}
	
	setGUIFromClientState();
}

// Run when dialog response is given
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

	// Detach all event handlers - even if response wasn't cancel, calling
	// code will want to install its own for client sockets
	for (std::list<sigc::connection>::iterator i = servereventconns.begin(); i != servereventconns.end(); ++i)
		i->disconnect();
	for (std::list<std::pair<const int, sigc::connection> >::iterator i = clienteventconns.begin(); i != clienteventconns.end(); ++i)
		i->second.disconnect();
	for (std::list<sigc::connection>::iterator i = clienterrconns.begin(); i != clienterrconns.end(); ++i)
		i->disconnect();
	servereventconns.clear();
	clienteventconns.clear();
	clienterrconns.clear();
	
	// Close all listening sockets by removing all references to their IOChannels
	for (std::list<Glib::RefPtr<Glib::IOChannel> >::iterator i = serverchannels.begin(); i != serverchannels.end(); ++i)
	{
		i->reset();
	}
	serverchannels.clear();
	
	// If response is anything other than OK, close any open accepted sockets too.
	if (response_id != Gtk::RESPONSE_OK)
		clientsockets.clear();
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
			int s = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
			if (s < 0)
			{
				errPop(strerror(errno));
				response(Gtk::RESPONSE_CANCEL);
				break;
			} else {
				// Bind it to the current address
				int val = 1;
				setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
				if (bind(s, current->ai_addr, current->ai_addrlen) < 0)
				{
					errPop(strerror(errno));
					response(Gtk::RESPONSE_CANCEL);
					close(s);
					break;
				} else {
					// Start it listening
					if (listen(s, 10) < 0)
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
						servereventconns.push_back(Glib::signal_io().connect(sigc::bind(sigc::mem_fun(this, &ServerStatusDialog::handleServerSocks), s),
							ioc, Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL));

						// Close underlying socket automatically when IOChannel is destroyed,
						// and store IOChannel reference in our list of server sockets
						ioc->set_close_on_unref(true);
						serverchannels.push_back(ioc);
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

// Handle disconnection of client sockets
bool ServerStatusDialog::handleClientSocks(Glib::IOCondition cond, const int s)
{
	// Clients shouldn't be writing down their sockets at this stage
	removeClient(s);
	
	// Return false to disconnect from event handler
	return false;
}

// Handle write errors to client sockets
void ServerStatusDialog::clientWriteError(const Glib::ustring &e, const int s)
{
	// Disconnect the failed client and show an error message
	removeClient(s);
	Glib::ustring m("Client write error: ");
	m.append(e);
	errPop(m.c_str());
}

// Function template for comparing first member of a pair to
// a given integer.  Used with std::bind2nd to form a functor
// for std::find_if, to locate clients by socket in clientchannels &
// clienteventconns lists.  Could be replaced with compose1 and
// select1st if those were standard rather than SGI extensions.
template <class A> bool equalTo(const std::pair<const int, A> a, int b)
{
	return a.first == b;
}

// Similar for finding the ClientSocket for the given socket
bool sockEqualTo(const Glib::RefPtr<ClientSocket> a, int b)
{
	return a->getSocket() == b;
}

// Remote all references to a connected client given their socket
void ServerStatusDialog::removeClient(const int s)
{
	// Remove connection object for event handler from list
	std::list<std::pair<const int, sigc::connection> >::iterator eventconn =
		std::find_if(clienteventconns.begin(), clienteventconns.end(),
			std::bind2nd(std::ptr_fun(&equalTo<sigc::connection>), s)
	);
	// Disconnect event handler *before* returning - otherwise, when
	// invoked from onKickClient, closing the socket invokes handleClientSocks,
	// which invokes this again, and so on...
	eventconn->second.disconnect();
	clienteventconns.erase(eventconn);
	
	// Close socket & delete IO channel
	std::deque<Glib::RefPtr<ClientSocket> >::iterator ioc =
		std::find_if(clientsockets.begin(), clientsockets.end(),
			std::bind2nd(std::ptr_fun(&sockEqualTo), s)
	);
	clientsockets.erase(ioc);
	
	setGUIFromClientState();
}

// Handle incoming connections on server sockets
bool ServerStatusDialog::handleServerSocks(Glib::IOCondition cond, const int s)
{
	if (cond == Glib::IO_IN)
	{
		sockaddr_storage newaddr;
		socklen_t newaddrlen = sizeof(newaddr);
		memset(&newaddr, 0, sizeof(newaddr));
		int newsock = accept(s, (sockaddr*) &newaddr, &newaddrlen);
		if (newsock < 0)
		{
			errPop(strerror(errno));
			response(Gtk::RESPONSE_CANCEL);
			return true;
		}
		
		// Should we accept this socket, or is the server full?
		if (clientsockets.size() == requiredclients)
		{
			// TODO - Send client a "server full" message
			close(newsock);
			return true;
		}
		
		// Check ss_family of newaddr to see if it's a sockaddr_in
		// or a sockaddr_in6.  Convert the client address to a string
		// accordingly.
		char buf[INET6_ADDRSTRLEN];
		const char *result;
		switch (newaddr.ss_family)
		{
			case AF_INET:
				result = inet_ntop(newaddr.ss_family,
					&(((sockaddr_in *) &newaddr)->sin_addr), buf, INET_ADDRSTRLEN);
				break;
			default:
				result = inet_ntop(newaddr.ss_family,
					&(((sockaddr_in6 *) &newaddr)->sin6_addr), buf, INET6_ADDRSTRLEN);
		}
		if (result == NULL)
		{
			errPop(strerror(errno));
			response(Gtk::RESPONSE_CANCEL);
			return true;
		}
		Glib::ustring clientaddr(buf);
		
		// Automatically assign first unassigned colour to client
		ClientSocket *newclient = new ClientSocket(newsock, clientaddr, remoteplayers.front());
		remoteplayers.pop_front();
		clientsockets.push_back(Glib::RefPtr<ClientSocket>(newclient));
		
		// Connect the socket to an event handler, listening to see if the client disconnects
		std::pair<const int, sigc::connection> eventconn(newsock,
			Glib::signal_io().connect(
				sigc::bind(sigc::mem_fun(this, &ServerStatusDialog::handleClientSocks), newsock),
					newclient->getChannel(), Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL)
		);
		clienteventconns.push_back(eventconn);
		
		// Also connect to the socket's write error signal, to catch
		// errors during asynchronous writes
		clienterrconns.push_back(newclient->write_error.connect(
			sigc::bind(sigc::mem_fun(this, &ServerStatusDialog::clientWriteError), newsock)
		));
		
		setGUIFromClientState();
	} else {
		errPop("Error on listening socket");
		response(Gtk::RESPONSE_CANCEL);
	}
	return true;
}

// Ensure GUI - connected clients list, game description, etc. - is up to
// date with the latest client state.
void ServerStatusDialog::setGUIFromClientState()
{
	// Rebuild the needs-assigning list
	remoteplayers.clear();
	if (m_pGameType->player_1 == pt_remote)
		remoteplayers.push_back(pc_player_1);
	if (m_pGameType->player_2 == pt_remote)
		remoteplayers.push_back(pc_player_2);
	if (m_pGameType->square)
	{
		if (m_pGameType->player_3 == pt_remote)
			remoteplayers.push_back(pc_player_3);
		if (m_pGameType->player_4 == pt_remote)
			remoteplayers.push_back(pc_player_4);
	}
	for (std::deque<Glib::RefPtr<ClientSocket> >::const_iterator i = clientsockets.begin(); i != clientsockets.end(); ++i)
	{
		remoteplayers.remove((*i)->getPlayer());
	}

	// Disconnect all client combo box event handlers before manually
	// modifying their state, so we don't get into a recursion loop
	for (std::list<sigc::connection>::iterator i = clientcomboconns.begin(); i != clientcomboconns.end(); ++i)
		i->disconnect();
	clientcomboconns.clear();

	// Add clients to GUI, including options to pick players.
	for (size_t i = 0; i < 4; ++i)
	{
		m_aClientComboBoxes[i].set_active(0);
		if (i >= clientsockets.size())
		{
			m_paClientLabels[i]->hide();
			m_paClientKickButtons[i]->hide();
			m_aClientComboBoxes[i].hide();
		} else {
			m_paClientLabels[i]->set_label(clientsockets[i]->getAddress());
			m_paClientLabels[i]->show();
			m_paClientKickButtons[i]->show();
			switch (clientsockets[i]->getPlayer())
			{
				case pc_player_none:
					m_aClientComboBoxes[i].set_active(0);
					break;
				case pc_player_1:
					m_aClientComboBoxes[i].set_active(redrow);
					break;
				case pc_player_2:
					m_aClientComboBoxes[i].set_active(greenrow);
					break;
				case pc_player_3:
					m_aClientComboBoxes[i].set_active(bluerow);
					break;
				default:
					m_aClientComboBoxes[i].set_active(yellowrow);
			}
			m_aClientComboBoxes[i].show();
		}
	}

	// Update the game details text indicating colours & clients
	bool allSlotsFilled = true;
	allSlotsFilled &= setPlayerDescription(m_pGameType->player_1, m_pRedClient, redrow, m_aClientComboBoxes, m_paClientLabels);
	allSlotsFilled &= setPlayerDescription(m_pGameType->player_2, m_pGreenClient, greenrow, m_aClientComboBoxes, m_paClientLabels);
	if (m_pGameType->square && m_pGameType->player_3 != pt_none)
	{
		allSlotsFilled &= setPlayerDescription(m_pGameType->player_3, m_pBlueClient, bluerow, m_aClientComboBoxes, m_paClientLabels);
		allSlotsFilled &= setPlayerDescription(m_pGameType->player_4, m_pYellowClient, yellowrow, m_aClientComboBoxes, m_paClientLabels);
	}
	
	// Send game description to clients over network
	sendGameDetails();
	
	// Allow starting the game if all slots are filled
	// TODO - Only allow the OK button to be clicked if all client
	// sockets are ready for output, as that means they have all
	// received the latest game details.  Add a signal to client
	// sockets which is emitted when writes finish, and connect
	// a callback method to it here.
	m_pStartButton->set_sensitive(allSlotsFilled);

	// Reconnect client combo box event handlers
	for (size_t i = 0; i < 4; ++i)
		clientcomboconns.push_back(m_aClientComboBoxes[i].signal_changed().connect(sigc::bind(sigc::mem_fun(this, &ServerStatusDialog::onClientComboChange), i)));
	
	// Disconnect & reconnect all client kick button event handlers
	for (std::list<sigc::connection>::iterator i = clientkickconns.begin(); i != clientkickconns.end(); ++i)
		i->disconnect();
	clientkickconns.clear();
	size_t buttonindex = 0;
	// Pass client socket into event handler function, as this is what removeClient expects as its argument
	for (std::deque<Glib::RefPtr<ClientSocket> >::const_iterator i = clientsockets.begin(); i != clientsockets.end(); ++i)
		clientkickconns.push_back(
			m_paClientKickButtons[buttonindex++]->signal_clicked().connect(
				sigc::bind(sigc::mem_fun(this, &ServerStatusDialog::onKickClient), (*i)->getSocket())
			)
		);
}

// Client disconnect button event handler
void ServerStatusDialog::onKickClient(const int s)
{
	removeClient(s);
}

// Send game description to clients over network
void ServerStatusDialog::sendGameDetails()
{
	if (clientsockets.size() == 0)
		return;

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
	
	std::string message;
	if (m_pGameType->square)
	{
		message.append(1, 's');
		if (m_pGameType->player_3 == pt_none)
			message.append(1, 2);
		else
			message.append(1, 4);
	} else
		message.append("h\2");

	int players = 2;
	if (m_pGameType->square && m_pGameType->player_3 != pt_none)
		players = 4;

	for (int i = 0; i < players; ++i)
	{
		playertype p;

		if (i == 0)
			p = m_pGameType->player_1;
		else if (i == 1)
			p = m_pGameType->player_2;
		else if (i == 2)
			p = m_pGameType->player_3;
		else
			p = m_pGameType->player_4;

		if (p == pt_local)
			message.append(1, 0).append(1, 0);
		else if (p == pt_ai)
			message.append(1, 1).append(1, 0);
		else
			message.append(1, 2).append(1, 0);
	}
	
	// Append two empty player descriptions if necessary -
	// we want a static header size
	if (players == 2)
		message.append(4, 0);
	
	// Append - for now - a dummy 11th byte
	message.append(1, 0);

	message.append(1, m_pGameType->w);
	message.append(1, m_pGameType->h);
	
	std::string redaddr, greenaddr, blueaddr, yellowaddr;
	for (std::deque<Glib::RefPtr<ClientSocket> >::const_iterator i = clientsockets.begin(); i != clientsockets.end(); ++i)
	{
		if ((*i)->getPlayer() == pc_player_1)
		{
			redaddr.assign((*i)->getAddress().c_str(), (*i)->getAddress().bytes());
			message[3] = (char) ((*i)->getAddress().bytes());
		}
		else if ((*i)->getPlayer() == pc_player_2)
		{
			greenaddr.assign((*i)->getAddress().c_str(), (*i)->getAddress().bytes());
			message[5] = (char) ((*i)->getAddress().bytes());
		}
		else if ((*i)->getPlayer() == pc_player_3)
		{
			blueaddr.assign((*i)->getAddress().c_str(), (*i)->getAddress().bytes());
			message[7] = (char) ((*i)->getAddress().bytes());
		}
		else if ((*i)->getPlayer() == pc_player_4)
		{
			blueaddr.assign((*i)->getAddress().c_str(), (*i)->getAddress().bytes());
			message[9] = (char) ((*i)->getAddress().bytes());
		}
	}
	message.append(redaddr).append(greenaddr).append(blueaddr).append(yellowaddr);

	// Send message out to each client, including the client's player number
	for (std::deque<Glib::RefPtr<ClientSocket> >::iterator i = clientsockets.begin(); i != clientsockets.end(); ++i)
	{
		if ((*i)->getPlayer() == pc_player_1)
			message[10] = 1;
		else if ((*i)->getPlayer() == pc_player_2)
			message[10] = 2;
		else if ((*i)->getPlayer() == pc_player_3)
			message[10] = 3;
		else if ((*i)->getPlayer() == pc_player_4)
			message[10] = 4;
		else
			message[10] = 0;

		(*i)->writeChars(message.c_str(), message.length());
	}
}
