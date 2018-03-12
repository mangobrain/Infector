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

#ifndef INFECTOR_SERVERSTATUSDIALOG_HXX
#define INFECTOR_SERVERSTATUSDIALOG_HXX

class ServerStatusDialog: public Gtk::Dialog
{
	public:
		// Constructor - called by glademm by get_widget_derived
		ServerStatusDialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refXml);
		
		// Destructor - destroy all client sockets
		~ServerStatusDialog()
		{
			for (std::deque<ClientSocket*>::iterator i = clientsockets.begin();
				i != clientsockets.end(); ++i)
			{
				delete *i;
			}
		};
		
		// Set game description label text, and set Clients and Game Details
		// tables to their initial states
		void setGameDetails(const GameType &gt);
		
		// Get client socket pointer list - for retrieval by calling code
		// once a network game has successfully been established
		const std::deque<ClientSocket*> &getClientSockets() const
		{
			return clientsockets;
		};
		
		// Clear out pointers to client sockets
		void clearClientSocketRefs()
		{
			clientsockets.clear();
		};
	
	private:
		Gtk::Label *m_paClientLabels[4];
		Gtk::ComboBoxText m_aClientComboBoxes[4];
		Gtk::Button *m_paClientKickButtons[4];
		Gtk::Table *m_pClientTable;
		Gtk::Label *m_pBlueLabel;
		Gtk::Label *m_pYellowLabel;
		Gtk::Label *m_pRedClient;
		Gtk::Label *m_pGreenClient;
		Gtk::Label *m_pBlueClient;
		Gtk::Label *m_pYellowClient;
		Gtk::Label *m_pGameDescription;
		Gtk::SpinButton *m_pPortSpin;
		Gtk::Button *m_pStartButton;
		Gtk::Button *m_pApplyButton;
		
		const GameType *m_pGameType;
		
		int redrow, greenrow, bluerow, yellowrow;
		
		// Number of remote players in game (and hence number
		// if client connections we should accept)
		size_t requiredclients;
		
		// Event handlers
		void onApply();
		
		// Override base classes' on_response method to reset the dialogue state
		void on_response(int response_id);
		
		// Convenience function for showing an error popup
		// TODO - Unify with the one in ClientStatusDialog; could
		// possibly also be used by GameWindow::onNetworkError.
		void errPop(const char *err) const;
		
		// IOChannel references for our sockets
		std::list<Glib::RefPtr<Glib::IOChannel> > serverchannels;
		std::deque<ClientSocket*> clientsockets;
		
		// Unassigned players
		std::list<piece> remoteplayers;
		
		// Connection objects corresponding to socket event handlers
		std::list<sigc::connection> servereventconns;
		
		// Store the client socket along with client socket event handler
		// connections, so we can delete them individually when
		// clients disconnect
		std::list<std::pair<const int, sigc::connection> > clienteventconns;
		std::list<sigc::connection> clienterrconns;
		
		// Connection objects corresponding to client combo box event handlers
		std::list<sigc::connection> clientcomboconns;
		
		// Connection objects corresponding to client kick button event handlers
		std::list<sigc::connection> clientkickconns;
		
		// Socket event handlers
		bool handleServerSocks(Glib::IOCondition cond, const int s);
		bool handleClientSocks(Glib::IOCondition cond, const int s);
		void clientWriteError(const Glib::ustring &e, const int s);
		
		// Client player selection box event handler
		void onClientComboChange(const size_t id);
		
		// Client kick button event handler
		void onKickClient(const int s);
		
		// Ensure GUI - connected clients list, game description,
		// etc. - is up to date with the latest client state.
		void setGUIFromClientState();
		
		// Remote all references to a connected client given their socket
		void removeClient(const int s);
		
		// Send game description to clients over network
		void sendGameDetails();
};

#endif
