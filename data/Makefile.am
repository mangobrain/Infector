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

dist_pkgdata_DATA = infector.glade

# Distribute original scalable icon, and various pre-rendered
# icons, so that we don't depend on RSVG being present for installation

# Also distribute the .desktop.in.in (un-expanded, untranslated) files.

EXTRA_DIST = infector.svg infector.desktop.in.in

dist-hook: $(SIZES:%=infector-%.png)
	cp -p $(srcdir)/infector-*.png $(distdir)/

# Sizes (NxN) at which to render and install application icon
SIZES = 16 22 24 32 48 64 72 96 128

# Rule for generating a particular size from original SVG.
# Also do this at distribution time so we don't depend on RSVG being
# present on users' systems.
infector-%.png: infector.svg
	rsvg -w $* -h $* $< $@

# Rule for generating .desktop.in file from .desktop.in.in.
# This converts the "bindir" placeholder in the ".in.in" file
# into a fully expanded literal in ".in".  The ".in" file is
# then processed for translations by intltool.
infector.desktop.in: infector.desktop.in.in
	rm -f $@ $@.tmp
	$(SED) -e s'|@bindir[@]|$(bindir)|g' $< >$@.tmp
	mv $@.tmp $@

# Translate infector.desktop.in into infector.desktop.
# This will happen on "make install", as the only thing which
# depends on infector.desktop itself is install-data-local.
@INTLTOOL_DESKTOP_RULE@

# Use xdg-utils to install and uninstall pre-rendered icons.
# I'd love to use it for the scalable icon as well, but
# support for that appears to have been dropped:
# http://lists.freedesktop.org/archives/portland/2006-September/000862.html
# https://bugs.freedesktop.org/show_bug.cgi?id=7837

# Also use xdg-util to install and uninstall .desktop file.

install-data-local: $(SIZES:%=infector-%.png) infector.desktop
	for i in $(SIZES); do \
		xdg-icon-resource install --novendor --noupdate --size $$i \
			infector-$$i.png infector; \
	done
	xdg-icon-resource forceupdate
	xdg-desktop-menu install --novendor infector.desktop

uninstall-local:
	for i in $(SIZES); do \
		xdg-icon-resource uninstall --noupdate --size $$i infector; \
	done
	xdg-icon-resource forceupdate
	xdg-desktop-menu uninstall infector.desktop

# Pre-rendered icons are considered built by the maintainer, because
# they are generated (not handmade) but distributed.  Delete them during
# maintainerclean.
maintainer-clean-local:
	rm -f infector-*.png

# .desktop file is built by make
CLEANFILES = infector.desktop infector.desktop.in
