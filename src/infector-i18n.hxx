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

#ifndef __INFECTOR_I18N_HXX__
#define __INFECTOR_I18N_HXX__

// If native language support is enabled, include libintl.h;
// otherwise, define various functions/macros to no-ops to ease
// the burden on code readability.

#ifdef ENABLE_NLS
#	include <libintl.h>
#	define _(str) gettext(str)
#	ifdef gettext_noop
#		define N_(str) gettext_noop(str)
#	else
#		define N_(str) (str)
#	endif
#else
#	define _(str) (str)
#	define N_(str) (str)
#endif

#endif