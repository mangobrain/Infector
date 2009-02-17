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

EXTRA_DIST = infector.svg

dist-hook: $(SIZES:%=infector-%.png)
	cp -p $(srcdir)/infector-*.png $(distdir)/

# Sizes (NxN) at which to render and install application icon
SIZES = 16 22 24 32 48 64 72 96 128

# Rule for generating a particular size from original SVG.
# Also do this at distribution time so we don't depend on RSVG being
# present on users' systems.
infector-%.png: infector.svg
	rsvg -w $* -h $* $< $@

# Use xdg-utils to install and uninstall pre-rendered icons.
# I'd love to use it for the scalable icon as well, but
# support for that appears to have been dropped:
# http://lists.freedesktop.org/archives/portland/2006-September/000862.html
# https://bugs.freedesktop.org/show_bug.cgi?id=7837

install-data-local: $(SIZES:%=infector-%.png)
	for i in $(SIZES); do \
		xdg-icon-resource install --novendor --noupdate --size $$i \
			infector-$$i.png infector; \
	done
	xdg-icon-resource forceupdate

uninstall-local:
	for i in $(SIZES); do \
		xdg-icon-resource uninstall --noupdate --size $$i infector; \
	done
	xdg-icon-resource forceupdate

# Pre-rendered icons are considered built by the maintainer, because
# they are generated (not handmade) but distributed.  Delete them during
# maintainerclean.
maintainer-clean-local:
	rm -f infector-*.png
