SUBDIRS = @PACKAGE_LIBRARIES_SUBDIRS@ icons man mlib po src ui


ACLOCAL_AMFLAGS = -I m4

# Added by gettextize
EXTRA_DIST = build-aux/config.rpath


desktopdir = $(datadir)/applications
desktop_DATA = @PACKAGE@.desktop


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
		msgmerge --force-po --no-wrap --no-fuzzy-matching --update --backup=off "$$lang.po" "@PACKAGE@" || exit 1; \
	done
## gettext <--


## loop -->
.PHONY: loop

loop:
	bash -c ' \
	ulimit -c unlimited; \
	check_files=$$(find $(srcdir)/src -name "*.cpp" -o -name "*.h" -o -name "*.hh" -o -name "*.hpp" -o -name "*.hxx"); \
	\
	while true; \
	do \
		new_files=$$(ls -l --time-style="+%Y.%m.%d %H:%M:%S" $$check_files); \
		\
		if [ "$$files" != "$$new_files" ]; \
		then \
			make --no-print-directory --keep-going -j2 && echo "OK"; \
		fi; \
		\
		files="$$new_files"; \
		read -t 1 input && MALLOC_CHECK_=3 ./src/@PACKAGE@; \
	done'
## loop <--

