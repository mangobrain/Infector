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

#ifndef __INFECTOR_SERVERSTATUSDIALOG_HXX__
#define __INFECTOR_SERVERSTATUSDIALOG_HXX__

#ifndef MINGW
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

class ServerStatusDialog: public Gtk::Dialog
{
	public:
		// Constructor - called by glademm by get_widget_derived
		ServerStatusDialog(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml> &refXml);
		
		// Set controls back to default state
		void setToDefaults();
	
	private:
		Gtk::SpinButton *m_pPortSpin;
		Gtk::Button *m_pStartButton;
		Gtk::Button *m_pCancelButton;
		Gtk::Button *m_pApplyButton;
		
		// Event handlers
		void onApply();
		
		// Override base classes' on_response method to reset the dialogue state
		void on_response(int response_id);
		
		// Convenience function for showing an error popup
		void errPop(const char* err) const;
		
		// IOChannel references for our listening sockets
		std::list<Glib::RefPtr<Glib::IOChannel> > serversocks;
};

#endif
