noinst_LIBRARIES = libmlib_gtk.a

libmlib_gtk_a_SOURCES = \
	action.cpp \
	action.hpp \
	controlled_list.cpp \
	controlled_list.hpp \
	dialog.cpp \
	dialog.hpp \
	dialog_settings.hpp \
	dispatcher.cpp \
	dispatcher.hh \
	dispatcher.hpp \
	dispatcher.hxx \
	expander_settings.cpp \
	expander_settings.hpp \
	expander_settings.hxx \
	link_button.cpp \
	link_button.hpp \
	main.cpp \
	main.hpp \
	misc.cpp \
	misc.hh \
	misc.hpp \
	misc.hxx \
	paned.cpp \
	paned.hpp \
	paned_settings.cpp \
	paned_settings.hpp \
	signal_proxy.hh \
	signal_proxy.hpp \
	signal_proxy.hxx\
	toolbar.cpp \
	toolbar.hpp \
	tree_view.cpp \
	tree_view.hh \
	tree_view.hpp \
	tree_view_settings.cpp \
	tree_view_settings.hpp \
	vbox.cpp \
	vbox.hpp \
	window.cpp \
	window.hpp \
	window_settings.cpp \
	window_settings.hpp

if MLIB_ENABLE_GTK_BUILDER
libmlib_gtk_a_SOURCES  += \
	builder.cpp \
	builder.hh \
	builder.hpp \
	builder.hxx
endif

libmlib_gtk_a_CPPFLAGS = @MLIB_CPPFLAGS@
libmlib_gtk_a_CFLAGS = @MLIB_CFLAGS@
libmlib_gtk_a_CXXFLAGS = @MLIB_CXXFLAGS@

