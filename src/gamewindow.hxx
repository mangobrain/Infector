// Copyright 2008 Philip Allison <sane@not.co.uk>

//    This file is part of infector.
//
//    infector is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    infector is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with infector.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __INFECTOR_GAMEWINDOW_HXX__
#define __INFECTOR_GAMEWINDOW_HXX__

class GameWindow: public Gtk::Window
{
	public:
		GameWindow();
	private:
		Gtk::RefPtr<Gtk::ActionGroup> m_refActionGroup;
		Gtk::RefPtr<Gtk::UIManager> m_refUIManager;
		Gtk::VBox m_vbox;
};

#endif
