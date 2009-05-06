AC_INIT([Flush], [0.5], [konishchev@gmail.com])


AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])


dnl foreign option tells Automake that this package is not follow the GNU
dnl Standards. GNU packages should always distribute additional files such as
dnl NEWS, README, etc.
dnl
dnl dist-bzip2 tells Automake also to create tar.bz2 archives during 'make dist'
dnl and 'make distcheck'.
AM_INIT_AUTOMAKE([-Wall -Werror foreign dist-bzip2])


AC_PROG_AWK
AC_PROG_SED
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_LANG([C++])

PKG_PROG_PKG_CONFIG
AM_GNU_GETTEXT([external])


dnl libconfig -->
	libconfig_dir_name="libconfig"
	libconfig_package_dir="$srcdir/$libconfig_dir_name"
	libconfig_min_version="1.3"

	if test -d "$libconfig_package_dir"
	then
		package_libconfig=yes
	else
		package_libconfig=no
	fi
dnl libconfig <--

dnl libtorrent -->
	libtorrent_dir_name="libtorrent"
	libtorrent_package_dir="$srcdir/$libtorrent_dir_name"
	libtorrent_min_version="0.14"

	if test -d "$libtorrent_package_dir"
	then
		package_libtorrent=yes
	else
		package_libtorrent=no
	fi
dnl libtorrent <--

dnl Configuring command line options -->
	AC_ARG_ENABLE(debug_mode,
		[AS_HELP_STRING([--enable-debug-mode],
			[enable debug mode for application])],
		[debug_mode=$enableval],
		[debug_mode=no]
	)

	AC_ARG_ENABLE(develop_mode,
		[AS_HELP_STRING([--enable-develop-mode],
			[enable develop mode for application (this mode is only for developer usage)])],
		[develop_mode=$enableval],
		[develop_mode=no]
	)

	AC_ARG_ENABLE(bundle_package,
		[AS_HELP_STRING([--disable-bundle-package],
			[disable creating bundle package which includes libconfig and libtorrent-rasterbar (only for package creating)])],
		[
			if test "X$enableval" = "Xyes"
			then
				disable_bundle_package=no
			else
				disable_bundle_package=yes
			fi
		],
		[disable_bundle_package=no]
	)

	AC_ARG_ENABLE([system_libconfig],
		[AS_HELP_STRING([--enable-system-libconfig],
			[use system libconfig library (only for bundle package)])],
		[
			if test "X$enableval" = "Xyes"
			then
				package_libconfig=no
			else
				package_libconfig=yes
			fi
		]
	)

	AC_ARG_ENABLE([system_libtorrent],
		[AS_HELP_STRING([--enable-system-libtorrent],
			[use system libtorrent-rasterbar library (only for bundle package)])],
		[
			if test "X$enableval" = "Xyes"
			then
				package_libtorrent=no
			else
				package_libtorrent=yes
			fi
		]
	)

	AC_ARG_ENABLE([inotify],
		[AS_HELP_STRING([--disable-inotify],
			[disable inotify support for application])],
		[
			if test "X$enableval" = "Xyes"
			then
				enable_inotify=yes
			else
				enable_inotify=no
			fi
		],
		[
			enable_inotify=yes
		]
	)


	if test "X$develop_mode" = "Xyes"
	then
		dnl Debug mode always enabled in develop mode
		debug_mode=yes
	fi

	if test "X$disable_bundle_package" = "Xyes"
	then
		package_libconfig=no
		package_libtorrent=no
	fi

	AC_MSG_NOTICE([debug mode: $debug_mode])
	AC_MSG_NOTICE([develop mode: $develop_mode])
	AC_MSG_NOTICE([disable bundle package: $disable_bundle_package])
	AC_MSG_NOTICE([using package libconfig: $package_libconfig])
	AC_MSG_NOTICE([using package libtorrent: $package_libtorrent])
dnl Configuring command line options <--


dnl config.h -->
	AH_TOP([
#ifndef HEADER_CONFIG
#define HEADER_CONFIG
	])

	AH_TEMPLATE([APP_NAME], [Application name])
	AC_DEFINE_UNQUOTED([APP_NAME], ["$PACKAGE_NAME"])

	AH_TEMPLATE([APP_UNIX_NAME], [Application UNIX name])
	AC_DEFINE_UNQUOTED([APP_UNIX_NAME], ["$PACKAGE"])

	AH_TEMPLATE([APP_MAJOR_VERSION], [Application major version number])
	APP_MAJOR_VERSION=`echo "$PACKAGE_VERSION" | awk -F. '{ print $1; }'`
	AC_DEFINE_UNQUOTED([APP_MAJOR_VERSION], [$APP_MAJOR_VERSION])

	AH_TEMPLATE([APP_MINOR_VERSION], [Application minor version number])
	APP_MINOR_VERSION=`echo "$PACKAGE_VERSION" | awk -F. '{ print $2; }'`
	AC_DEFINE_UNQUOTED([APP_MINOR_VERSION], [$APP_MINOR_VERSION])

	AH_TEMPLATE([APP_SUB_MINOR_VERSION], [Application sub-minor version number])
	APP_SUB_MINOR_VERSION=`echo "$PACKAGE_VERSION" | awk -F. '{ if($3 == "") print 0; else print $3; }'`
	AC_DEFINE_UNQUOTED([APP_SUB_MINOR_VERSION], [$APP_SUB_MINOR_VERSION])

	AH_TEMPLATE([APP_VERSION_STRING], [Application version string])
	AC_DEFINE_UNQUOTED([APP_VERSION_STRING], ["$PACKAGE_VERSION"])


	AH_TEMPLATE([DEBUG_MODE], [Is application compiling in debug mode])
	AH_TEMPLATE([MLIB_DEBUG_MODE], [Is mlib compiling in debug mode])
	if test "X$debug_mode" = "Xyes"
	then
		AC_DEFINE([DEBUG_MODE])
		AC_DEFINE([MLIB_DEBUG_MODE])
	fi

	AH_TEMPLATE([DEVELOP_MODE], [Is application compiling in develop mode])
	AH_TEMPLATE([MLIB_DEVELOP_MODE], [Is mlib compiling in develop mode])
	if test "X$develop_mode" = "Xyes"
	then
		AC_DEFINE([DEVELOP_MODE])
		AC_DEFINE([MLIB_DEVELOP_MODE])
	fi

	AH_TEMPLATE([_LARGEFILE_SOURCE], [Large files support])
	AC_DEFINE([_LARGEFILE_SOURCE])

	AH_TEMPLATE([_FILE_OFFSET_BITS], [Large files support])
	AC_DEFINE([_FILE_OFFSET_BITS], [64])

	AH_TEMPLATE([MLIB_ENABLE_ALIASES], [Enable short functions and classes names from mlib])
	AC_DEFINE([MLIB_ENABLE_ALIASES])

	AH_TEMPLATE([MLIB_ENABLE_DBUS], [Enable DBus support for mlib])
	AC_DEFINE([MLIB_ENABLE_DBUS])

	AH_TEMPLATE([MLIB_ENABLE_INOTIFY], [Enable libinotify in mlib])
	if test "X$enable_inotify" = "Xyes"
	then
		AC_DEFINE([MLIB_ENABLE_INOTIFY])
	fi

	AH_TEMPLATE([MLIB_ENABLE_LIBCONFIG], [Enable libconfig support for mlib])
	AC_DEFINE([MLIB_ENABLE_LIBCONFIG])

	AH_TEMPLATE([MLIB_ENABLE_LIBTORRENT], [Enable libtorrent support for mlib])
	AC_DEFINE([MLIB_ENABLE_LIBTORRENT])

	AH_TEMPLATE([MLIB_ENABLE_GTK], [Enable GTK support for mlib])
	AC_DEFINE([MLIB_ENABLE_GTK])

	AH_TEMPLATE([MLIB_ENABLE_GLADE], [Enable Glade support for mlib])
	AC_DEFINE([MLIB_ENABLE_GLADE])

	AH_BOTTOM([
#endif
	])
dnl config.h <--


AM_CONDITIONAL([DEVELOP_MODE], [test "X$develop_mode" = "Xyes"])


dnl Available libraries -->
	if test "X$enable_inotify" = "Xyes"
	then
		AC_CHECK_HEADER([sys/inotify.h],
			[],
			[
				AC_MSG_ERROR([unable to find inotify library. You may compile package without inotify library if it is not exists in your distro by --disable-inotify configure script option.])
			]
		)
	fi
dnl Available libraries <--


dnl compiler flags -->
	if test "X$develop_mode" = "Xyes"
	then
		CXXFLAGS="$CXXFLAGS -O0 -g -Wall"
	fi

	COMMON_CPPFLAGS=""
	APP_DEPENDENCIES=""
	APP_LDADD=""

	dnl Including config.h anywhere
	COMMON_CPPFLAGS="-include \$(top_builddir)/config.h -I \$(top_srcdir)/src"

	dnl libraries -->
		dnl boost -->
			AX_BOOST_BASE([1.34])
			if test -z "$BOOST_LDFLAGS"
			then
				AC_MSG_ERROR([unable to find Boost library.])
			fi
			COMMON_CPPFLAGS="$COMMON_CPPFLAGS $BOOST_CPPFLAGS"
			APP_LDADD="$APP_LDADD $BOOST_LDFLAGS"

			AX_BOOST_FILESYSTEM
			if test -z "$BOOST_FILESYSTEM_LIB"
			then
				AC_MSG_ERROR([unable to find Boost.Filesystem library.])
			fi
			APP_LDADD="$APP_LDADD $BOOST_FILESYSTEM_LIB"

			AX_BOOST_THREAD
			if test -z "$BOOST_THREAD_LIB"
			then
				AC_MSG_ERROR([unable to find Boost.Thread library.])
			fi
			APP_LDADD="$APP_LDADD $BOOST_THREAD_LIB"
		dnl boost <--

		dnl DBus -->
			PKG_CHECK_MODULES([dbus], [dbus-1])
			COMMON_CPPFLAGS="$COMMON_CPPFLAGS $dbus_CFLAGS"
			APP_LDADD="$APP_LDADD $dbus_LIBS"

			AC_SUBST([DBUSXX_LDADD], ['$(top_builddir)/dbus-c++/src/.libs/libdbus-c++-1.a'])
			AC_SUBST([DBUSXX_XML2CPP], ['$(top_builddir)/dbus-c++/tools/dbusxx-xml2cpp'])
			COMMON_CPPFLAGS="$COMMON_CPPFLAGS -I \$(top_srcdir)/dbus-c++/include"
			APP_LDADD="$APP_LDADD $DBUSXX_LDADD"
		dnl DBus <--

		dnl GTK -->
			PKG_CHECK_MODULES([gthread], [gthread-2.0])
			COMMON_CPPFLAGS="$COMMON_CPPFLAGS $gthread_CFLAGS"
			APP_LDADD="$APP_LDADD $gthread_LIBS"

			PKG_CHECK_MODULES([gtkmm], [gtkmm-2.4])
			COMMON_CPPFLAGS="$COMMON_CPPFLAGS $gtkmm_CFLAGS"
			APP_LDADD="$APP_LDADD $gtkmm_LIBS"
		dnl GTK <--

		dnl Glade -->
			PKG_CHECK_MODULES([glademm], [libglademm-2.4])
			COMMON_CPPFLAGS="$COMMON_CPPFLAGS $glademm_CFLAGS"
			APP_LDADD="$APP_LDADD $glademm_LIBS"
		dnl Glade <--

		dnl libconfig -->
			if test "X$package_libconfig" = "Xyes"
			then
				LIBCONFIG_LIBRARIES="\$(top_builddir)/$libconfig_dir_name/.libs/libconfig++.a"
				COMMON_CPPFLAGS="$COMMON_CPPFLAGS -I \$(top_srcdir)/$libconfig_dir_name"
				APP_DEPENDENCIES="$APP_DEPENDENCIES $LIBCONFIG_LIBRARIES"
				APP_LDADD="$APP_LDADD $LIBCONFIG_LIBRARIES"
			else
				PKG_CHECK_MODULES([libconfigduo], [libconfig++ >= $libconfig_min_version])
				COMMON_CPPFLAGS="$COMMON_CPPFLAGS $libconfigduo_CFLAGS"
				APP_LDADD="$APP_LDADD $libconfigduo_LIBS"
			fi
		dnl libconfig <--

		dnl libtorrent -->
			if test "X$package_libtorrent" = "Xyes"
			then
				CHECK_SSL
				COMMON_CPPFLAGS="$COMMON_CPPFLAGS $SSL_CXXFLAGS"
				APP_LDADD="$APP_LDADD $SSL_LDFLAGS $SSL_LIBS"

				ACX_PTHREAD
				COMMON_CPPFLAGS="$COMMON_CPPFLAGS $PTHREAD_CFLAGS"
				APP_LDADD="$APP_LDADD $PTHREAD_LIBS"

				COMMON_CPPFLAGS="$COMMON_CPPFLAGS -DTORRENT_USE_OPENSSL -DTORRENT_LINKING_SHARED"
				COMMON_CPPFLAGS="$COMMON_CPPFLAGS -I \$(top_srcdir)/$libtorrent_dir_name/include -I \$(top_srcdir)/$libtorrent_dir_name/include/libtorrent"

				LIBTORRENT_LIBRARIES="\$(top_builddir)/$libtorrent_dir_name/src/.libs/libtorrent-rasterbar.a"
				APP_LDADD="$APP_LDADD -lrt"
				APP_LDADD="$APP_LDADD $LIBTORRENT_LIBRARIES"
				APP_DEPENDENCIES="$APP_DEPENDENCIES $LIBTORRENT_LIBRARIES"
			else
				dnl TODO: remove bad named libtorrent-rasterbar-0.14 when it package
				dnl will become obsolete
				PKG_CHECK_MODULES([libtorrent_rasterbar], [libtorrent-rasterbar-0.14 >= $libtorrent_min_version], [], [
					PKG_CHECK_MODULES([libtorrent_rasterbar], [libtorrent-rasterbar >= $libtorrent_min_version])
				])
				COMMON_CPPFLAGS="$COMMON_CPPFLAGS $libtorrent_rasterbar_CFLAGS"
				APP_LDADD="$APP_LDADD $libtorrent_rasterbar_LIBS"
			fi
		dnl libtorrent <--
	dnl libraries <--

	dnl Gettext libraries
	APP_LDADD="$APP_LDADD $LIBINTL"

	MLIB_CPPFLAGS="$COMMON_CPPFLAGS"
	MLIB_LIBRARIES="\$(top_builddir)/src/mlib/libmlib.a \$(top_builddir)/src/mlib/gtk/libmlib_gtk.a"
	APP_CPPFLAGS="$COMMON_CPPFLAGS -I \$(top_srcdir)/src/mlib/src -include \$(top_srcdir)/src/common.hpp"
	APP_DEPENDENCIES="$APP_DEPENDENCIES $MLIB_LIBRARIES"
	APP_LDADD="$APP_LDADD $MLIB_LIBRARIES"

	AC_SUBST([MLIB_CPPFLAGS])
	AC_SUBST([APP_DEPENDENCIES])
	AC_SUBST([APP_CPPFLAGS])
	AC_SUBST([APP_LDADD])
dnl compiler flags <--


dnl Libraries inside of package -->
	PACKAGE_LIBRARIES_SUBDIRS="dbus-c++"
	AC_CONFIG_SUBDIRS([dbus-c++])

	if test "X$package_libconfig" = "Xyes"
	then
		PACKAGE_LIBRARIES_SUBDIRS="$PACKAGE_LIBRARIES_SUBDIRS $libconfig_dir_name"
		AC_CONFIG_SUBDIRS([libconfig])
	fi

	if test "X$package_libtorrent" = "Xyes"
	then
		PACKAGE_LIBRARIES_SUBDIRS="$PACKAGE_LIBRARIES_SUBDIRS $libtorrent_dir_name"
		AC_CONFIG_SUBDIRS([libtorrent])
	fi

	AC_SUBST([PACKAGE_LIBRARIES_SUBDIRS])
dnl Libraries inside of package <--


AC_CONFIG_FILES([
	glade/Makefile
	man/Makefile
	man/ru/Makefile
	po/Makefile.in
	src/Makefile
	src/mlib/Makefile
	src/mlib/gtk/Makefile
	flush.desktop
	Makefile
])

AC_OUTPUT

