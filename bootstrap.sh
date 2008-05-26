#! /bin/sh

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

# Bootstrap: get a local copy ready for building, from scratch.

# Run the following, in order:
# * aclocal: create `aclocal.m4' from `configure.ac'
# * autoheader: create `config.h.in' (or other named header file) from
#   `configure.ac'
# * automake: create all `Makefile.in' files from all `Makefile.am' files,
#   apparently finding them via `configure.ac'
#   * `--add-missing' and `--copy' add copies of (not symlinks to) any
#     standard files which are missing from the distribution
# * autoconf: create `configure' from `configure.ac'
#
# One is then in a position to `configure; make; make install'. :)

aclocal && autoheader && automake --add-missing --copy && autoconf
