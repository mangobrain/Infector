dnl Copyright 2008-2009, 2012, 2015 Philip Allison <mangobrain@googlemail.com>

dnl    This file is part of Infector.
dnl
dnl    Infector is free software: you can redistribute it and/or modify
dnl    it under the terms of the GNU General Public License as published by
dnl    the Free Software Foundation, either version 3 of the License, or
dnl    (at your option) any later version.
dnl
dnl    Infector is distributed in the hope that it will be useful,
dnl    but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl    GNU General Public License for more details.
dnl
dnl    You should have received a copy of the GNU General Public License
dnl    along with Infector.  If not, see <http://www.gnu.org/licenses/>.

dnl # Specify autoconf version requirements
dnl # Not strictly necessary - but it's the version on my system,
dnl # so for now, we'll pretend it's the required version.
AC_PREREQ([2.69])

dnl # Initialise autoconf and automake
dnl # Also specify automake version requirements (again, the version used
dnl # development will suffice for now).
AC_INIT([infector],[0.4],[mangobrain@googlemail.com])
AC_CONFIG_SRCDIR([src/infector.cxx])
AC_CONFIG_MACRO_DIR([acinclude])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.15 silent-rules])
AM_SILENT_RULES([yes])

dnl # Infector is written in C++, of course.  :)
AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG([C++])

dnl # Make config.h from config.h.in.  (`autoheader' generates the latter.)
AC_CONFIG_HEADERS([config.h])

dnl # Output the command-line options given to this script
AC_DEFINE_UNQUOTED([CONFIGURE_OPTS],["$ac_configure_args"],[Options given to ./configure])

dnl # Use intltool for i18n.
IT_PROG_INTLTOOL([0.51])

dnl # Infector uses gtkmm.
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([GTKMM], [gtkmm-3.0])

dnl # Are we building on MinGW?
AC_MSG_CHECKING(host os)
AC_CANONICAL_HOST
AC_MSG_RESULT($host_os)
win=false
case "$host_os" in
	mingw*)
		win=true
		LIBS="-lwsock32 -lws2_32 ${LIBS}"
		CXXFLAGS="-mthreads -Wl,-subsystem,windows ${CXXFLAGS}"
		AC_DEFINE([MINGW],[1],[Define if building for Windows (MinGW)])
		AC_DEFINE([_WIN32_WINNT],[0x0502],[Define if building for Windows])
		AC_DEFINE([WINVER],[0x0502],[Define if building for Windows])
		;;
	*)
		;;
esac
AM_CONDITIONAL(MINGW, test "x$win" = "xtrue")

dnl # Sed is used to parse infector.desktop.in
AC_PROG_SED

dnl # Variables necessary for i18n.
AC_SUBST([GETTEXT_PACKAGE], [infector])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [gettext package name])

dnl # Available translations
ALL_LINGUAS="cs"
AM_GLIB_GNU_GETTEXT

dnl # All done.  Spit out the result.
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	data/Makefile
	po/Makefile.in
])
dnl	doc/Makefile
AC_OUTPUT
