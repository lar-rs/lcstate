## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src po

ultradocdir = ${datarootdir}/doc/ultracontrol
dist_ultradoc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS


INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = \
	$(INTLTOOL_FILES)

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)


tags_source = $(shell find src -type f -name '*.c' -print)

tags:
		gcc -I/usr/include/gio-unix-2.0/ -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/ultimate-1.0 -I/usr/include/mktbus-1.0 -I/usr/include/mktlib-2.0 -M $(tags_source) | sed -e "s/[\\ ]/\n/g" | \
				sed -e "/^$/d" -e "/\.o:[ \t]*$/d" | \
				ctags -L - --c++-kinds=+p --fields=+iaS --extra=+q
