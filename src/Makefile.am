# Copyright 2008 Philip Allison <sane@not.co.uk>

#    This file is part of infector.
#
#    infector is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    infector is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with infector.  If not, see <http://www.gnu.org/licenses/>.

bin_PROGRAMS = infector

infector_SOURCES = infector.cxx gameboard.cxx game.cxx boardstate.cxx \
	newgamedialog.cxx ai.cxx
infector_CXXFLAGS = $(GTKMM_CFLAGS) $(GLADEMM_CFLAGS) $(AM_CXXFLAGS)
infector_LDADD = $(GTKMM_LIBS) $(GLADEMM_LIBS) $(AM_LIBS)
infector_CPPFLAGS = -DPKGDATADIR='"$(pkgdatadir)"' $(AM_CPPFLAGS)
