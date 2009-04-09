SUBDIRS = @PACKAGE_LIBRARIES_SUBDIRS@ man po src

ACLOCAL_AMFLAGS = -I m4

# Added by gettextize
EXTRA_DIST = build-aux/config.rpath

desktopdir = $(datadir)/applications
desktop_DATA = @PACKAGE@.desktop

icons_16dir = $(datadir)/icons/hicolor/16x16/apps
icons_22dir = $(datadir)/icons/hicolor/22x22/apps
icons_24dir = $(datadir)/icons/hicolor/24x24/apps
icons_32dir = $(datadir)/icons/hicolor/32x32/apps
icons_48dir = $(datadir)/icons/hicolor/48x48/apps
icons_72dir = $(datadir)/icons/hicolor/72x72/apps

dist_icons_16_DATA = icons/16x16/apps/@PACKAGE@.png
dist_icons_22_DATA = icons/22x22/apps/@PACKAGE@.png
dist_icons_24_DATA = icons/24x24/apps/@PACKAGE@.png
dist_icons_32_DATA = icons/32x32/apps/@PACKAGE@.png
dist_icons_48_DATA = icons/48x48/apps/@PACKAGE@.png
dist_icons_72_DATA = icons/72x72/apps/@PACKAGE@.png


# icons cache -->
gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook:
	@ \
	if test -z "$(DESTDIR)"; \
	then \
		echo "Updating GTK icon cache..."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated. After install please run:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi
# icons cache <--


# gettext -->
.PHONY: gettext

gettext:
	make -C po update-po
	cd po || exit 1; \
	for lang in $$(cat LINGUAS | $(SED) 's/#.*$$//'); \
	do \
		if test "$$lang" = "en"; \
		then \
			msginit --no-translator --locale="$$lang" --output-file="$$lang.po" || exit 1; \
		else \
			msgmerge --force-po --no-wrap --no-fuzzy-matching --update --backup=off "$$lang.po" "@PACKAGE@" || exit 1; \
		fi; \
	done
## gettext <--


## loop -->
.PHONY: loop

loop:
	bash -c ' \
	\
	check_files=$$(find $(srcdir)/src -name "*.cpp"; find $(srcdir)/src -name "*.hpp"; find $(srcdir)/src -name "*.hh";); \
	\
	while true; \
	do \
		new_files=$$(ls -l $$check_files); \
		\
		if [ "$$files" != "$$new_files" ]; \
		then \
			make --no-print-directory --keep-going && echo "OK"; \
		fi; \
		\
		files="$$new_files"; \
		read -t 1 input && ./src/@PACKAGE@; \
	done'
## loop <--

