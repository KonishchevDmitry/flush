SUBDIRS = base

if MLIB_ENABLE_GTK
SUBDIRS += gtk
endif

noinst_LIBRARIES = libmlib.a

libmlib_a_SOURCES = \
	async_fs.cpp \
	async_fs.hpp \
	fs.cpp \
	fs.hpp \
	fs_watcher.cpp \
	fs_watcher.hpp \
	main.hpp \
	math.hpp \
	math.hh \
	misc.cpp \
	misc.hh \
	misc.hpp \
	misc.hxx \
	mutex.hpp \
	mutex.hh \
	signals_holder.cpp \
	signals_holder.hpp\
	string.cpp \
	string.hpp \
	sys.cpp \
	sys.hh \
	sys.hpp

if MLIB_ENABLE_DBUS
libmlib_a_SOURCES += \
	dbus.cpp \
	dbus.hpp
endif

if MLIB_ENABLE_LIBCONFIG
libmlib_a_SOURCES += \
	libconfig.cpp \
	libconfig.hpp
endif

if MLIB_ENABLE_LIBTORRENT
libmlib_a_SOURCES += \
	libtorrent.cpp \
	libtorrent.hpp \
	libtorrent.hxx
endif

if MLIB_ENABLE_SQLITE
libmlib_a_SOURCES += \
	sqlite.cpp \
	sqlite.hpp \
	sqlite.hxx
endif

libmlib_a_CPPFLAGS = @MLIB_CPPFLAGS@
libmlib_a_CXXFLAGS = @MLIB_CXXFLAGS@

