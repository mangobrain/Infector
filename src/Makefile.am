# Copyright 2008-2009 Philip Allison <mangobrain@googlemail.com>

#    This file is part of Infector.
#
#    Infector is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Infector is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Infector.  If not, see <http://www.gnu.org/licenses/>.

bin_PROGRAMS = infector

infector_SOURCES = infector.cxx gameboard.cxx game.cxx boardstate.cxx \
	newgamedialog.cxx ai.cxx serverstatusdialog.cxx socket.cxx \
	clientstatusdialog.cxx ai.hxx boardstate.hxx clientstatusdialog.hxx \
	gameboard.hxx game.hxx gametype.hxx gamewindow.hxx newgamedialog.hxx \
	serverstatusdialog.hxx socket.hxx infector-i18n.hxx
infector_CXXFLAGS = $(GTKMM_CFLAGS) $(GLADEMM_CFLAGS) $(AM_CXXFLAGS)
infector_LDADD = $(GTKMM_LIBS) $(GLADEMM_LIBS) $(AM_LIBS)

# Use relative paths on Windows.
if MINGW
infector_CPPFLAGS = -DINFECTOR_PKGDATADIR='"../share/infector"' \
	-DINFECTOR_LOCALEDIR='"../share/locale"' -I$(top_srcdir) $(AM_CPPFLAGS)
else
infector_CPPFLAGS = -DINFECTOR_PKGDATADIR='"$(pkgdatadir)"' \
	-DINFECTOR_LOCALEDIR='"$(localedir)"' -I$(top_srcdir) $(AM_CPPFLAGS)
endif
