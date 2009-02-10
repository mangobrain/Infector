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

#ifndef __INFECTOR_CLIENTSTATUSDIALOG_HXX__
#define __INFECTOR_CLIENTSTATUSDIALOG_HXX__

class ClientStatusDialog: public Gtk::Dialog
{
	public:
		// Constructor - called by glademm by get_widget_derived
		ClientStatusDialog(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml> &refXml);

		// Return game details
		void getGameType(GameType &gt) const {
			gt = m_GameType;
		};

		// Return a reference pointer to the server connection
		const Glib::RefPtr<Socket> &getServerSocket() const
		{
			return serversock;
		};
		
		// Clear our reference to server connection
		void clearServerSocketRef()
		{
			serversock.reset();
		};
		
		// Set GUI to default state
		void setDefaults();
		
	private:
		Gtk::SpinButton *m_pPortSpin;
		Gtk::Entry *m_pAddressEntry;
		Gtk::Label *m_pRedClient;
		Gtk::Label *m_pGreenClient;
		Gtk::Label *m_pBlueClient;
		Gtk::Label *m_pYellowClient;
		Gtk::Label *m_pBlueLabel;
		Gtk::Label *m_pYellowLabel;
		Gtk::Label *m_pGameDescription;
		Gtk::Button *m_pConnectButton;
		Gtk::VBox *m_pGameDetailsFrame;

		// Game details received from server
		GameType m_GameType;
		
		// Connection to server
		Glib::RefPtr<Socket> serversock;
		
		// Socket event handler connections
		sigc::connection sockeventconn;
		
		// Buffer for reading data from server
		std::string netbuf;
		
		// Have we received a full set of game details?
		bool detailsreceived;
		
		// If not, how much data is left to parse?
		size_t bytesremaining;

		// Reference to the Glade XML we were created from
		Glib::RefPtr<Gnome::Glade::Xml> m_refXml;

		// Override base classes' on_response method to
		// disconnect our own event handlers from the server socket
		void on_response(int response_id);
		
		// Handle server socket events
		bool handleServerSock(Glib::IOCondition cond);

		// Convenience function for showing an error popup
		void errPop(const char *err) const;
		
		// Event handler for connect button
		void onConnect();
};

#endif
