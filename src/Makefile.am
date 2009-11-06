EXTRA_DIST = dbus_introspect.xml

bin_PROGRAMS = flush

flush_SOURCES = \
	add_torrent_dialog.cpp \
	add_torrent_dialog.hpp \
	app_icons.cpp \
	app_icons.hpp \
	application.cpp \
	application.hh \
	application.hpp \
	categories_view.cpp \
	categories_view.hpp \
	categories_view.hxx \
	client_cmd_options.cpp \
	client_cmd_options.hpp \
	client_settings.cpp \
	client_settings.hpp \
	client_settings.hxx \
	common.cpp \
	common.hh \
	common.hpp \
	create_torrent_dialog.cpp \
	create_torrent_dialog.hpp \
	daemon.cpp \
	daemon_fs.cpp \
	daemon_fs.hpp \
	daemon.hpp \
	daemon_proxy.cpp \
	daemon_proxy.hpp \
	daemon_session.cpp \
	daemon_session.hpp \
	daemon_settings.cpp \
	daemon_settings.hpp \
	daemon_types.cpp \
	daemon_types.hpp \
	gui_lib.cpp \
	gui_lib.hpp \
	ip_filter.cpp \
	ip_filter.hpp \
	log_view.cpp \
	log_view.hpp \
	main.cpp \
	main.hpp \
	main_window.cpp \
	main_window.hpp \
	open_torrent_dialog.cpp \
	open_torrent_dialog.hpp \
	settings_window.cpp \
	settings_window.hpp \
	statistics_window.cpp \
	statistics_window.hpp \
	temporary_action_dialog.cpp \
	temporary_action_dialog.hpp \
	torrent_details_view.cpp \
	torrent_details_view.hpp \
	torrent_files_view.cpp \
	torrent_files_view.hh \
	torrent_files_view.hpp \
	torrent_options_view.cpp \
	torrent_options_view.hpp \
	torrent_peers_view.cpp \
	torrent_peers_view.hpp \
	torrents_view.cpp \
	torrents_view.hpp \
	torrents_viewport.cpp \
	torrents_viewport.hpp \
	trackers_view.cpp \
	trackers_view.hpp


dbus_glues = adaptor_glue.hpp proxy_glue.hpp
nodist_flush_SOURCES = $(dbus_glues)
BUILT_SOURCES = $(dbus_glues)
CLEANFILES = $(dbus_glues)

adaptor_glue.hpp: dbus_introspect.xml
	$(DBUSXX_XML2CPP) '$<' --adaptor='$@'

proxy_glue.hpp: dbus_introspect.xml
	$(DBUSXX_XML2CPP) '$<' --proxy='$@'

flush_DEPENDENCIES = @APP_DEPENDENCIES@
flush_CPPFLAGS = @APP_CPPFLAGS@
if DEVELOP_MODE
flush_CPPFLAGS += -D APP_BIN_PATH='"$(abs_builddir)/$(PACKAGE)"'
flush_CPPFLAGS += -D APP_UI_PATH='"$(abs_top_srcdir)/ui"'
flush_CPPFLAGS += -D APP_CUSTOM_ICONS_PATH='"$(abs_top_srcdir)/icons"'
else
flush_CPPFLAGS += -D APP_BIN_PATH='"$(bindir)/$(PACKAGE)"'
flush_CPPFLAGS += -D APP_UI_PATH='"$(datadir)/$(PACKAGE)/ui"'
flush_CPPFLAGS += -D APP_CUSTOM_ICONS_PATH='"$(datadir)/$(PACKAGE)/icons"'
endif
flush_CPPFLAGS += -D APP_LOCALE_PATH='"$(localedir)"'
flush_CXXFLAGS = @APP_CXXFLAGS@
flush_LDADD = @APP_LDADD@

