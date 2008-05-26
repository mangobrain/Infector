dnl Copyright 2008 Philip Allison <sane@not.co.uk>

dnl    This file is part of infector.
dnl
dnl    infector is free software: you can redistribute it and/or modify
dnl    it under the terms of the GNU General Public License as published by
dnl    the Free Software Foundation, either version 3 of the License, or
dnl    (at your option) any later version.
dnl
dnl    infector is distributed in the hope that it will be useful,
dnl    but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl    GNU General Public License for more details.
dnl
dnl    You should have received a copy of the GNU General Public License
dnl    along with infector.  If not, see <http://www.gnu.org/licenses/>.

dnl # Specify autoconf version requirements
dnl # Not strictly necessary - but it's the version on my system,
dnl # so for now, we'll pretend it's the required version.
AC_PREREQ([2.59])

dnl # Initialise autoconf and automake
dnl # Also specify automake version requirements (again, the version used
dnl # development will suffice for now).
AC_INIT([infector], [0.1], [sane@not.co.uk])
AC_CONFIG_SRCDIR([src/infector.cxx])
AM_INIT_AUTOMAKE([1.8.5])

dnl # infector is written in C++, of course.  :)
AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG([C++])

dnl # Make config.h from config.h.in.  (`autoheader' generates the latter.)
AC_CONFIG_HEADERS([config.h])

dnl # Output the command-line options given to this script
AC_DEFINE_UNQUOTED([CONFIGURE_OPTS],["$ac_configure_args"],[Options given to ./configure])

dnl # infector uses gtkmm.
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([GTKMM], [gtkmm-2.4 >= 2.10])

dnl # All done.  Spit out the result.
AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
dnl	data/Makefile
dnl	doc/Makefile
AC_OUTPUT
