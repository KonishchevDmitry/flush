AC_INIT([Flush], [0.9.11], [konishchev@gmail.com])


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
MLIB_INIT


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

	if test "X$develop_mode" = "Xyes"; then
		MLIB_FEATURE_ENABLE([[develop_mode]])

		dnl Debug mode always enabled in develop mode
		debug_mode=yes
	fi

	if test "X$debug_mode" = "Xyes"; then
		MLIB_FEATURE_ENABLE([[debug_mode]])
	fi
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

	AH_TEMPLATE([DEBUG_MODE], [Is application compiling in the debug mode])
	if test "X$debug_mode" = "Xyes"; then
		AC_DEFINE([DEBUG_MODE])
	fi

	AH_TEMPLATE([DEVELOP_MODE], [Is application compiling in the develop mode])
	if test "X$develop_mode" = "Xyes"; then
		AC_DEFINE([DEVELOP_MODE])
	fi

	AH_TEMPLATE([_LARGEFILE_SOURCE], [Large files support])
	AC_DEFINE([_LARGEFILE_SOURCE])

	AH_TEMPLATE([_FILE_OFFSET_BITS], [Large files support])
	AC_DEFINE([_FILE_OFFSET_BITS], [64])

	AH_BOTTOM([
#endif
	])
dnl config.h <--


dnl Compiler flags -->
	dnl Clearing all flags -->
		COMMON_CPPFLAGS=''
		COMMON_CFLAGS=''
		COMMON_CXXFLAGS=''
		COMMON_LDADD=''

		APP_CPPFLAGS=''
		APP_CFLAGS=''
		APP_CXXFLAGS=''
		APP_DEPENDENCIES=''
		APP_LDADD=''
	dnl Clearing all flags <--

	dnl Including config.h anywhere
	COMMON_CPPFLAGS="$COMMON_CPPFLAGS -include \$(top_builddir)/config.h"

	APP_CPPFLAGS="$APP_CPPFLAGS -I \$(top_srcdir)/src"

	dnl Develop mode flags -->
		if test "X$develop_mode" = "Xyes"; then
			CXXFLAGS="$CXXFLAGS -O0 -g"

			dnl Some of this options are not supported by older versions of GCC
			COMMON_CXXFLAGS="$COMMON_CXXFLAGS -fdiagnostics-show-option"
			COMMON_CXXFLAGS="$COMMON_CXXFLAGS -Wall -Winit-self -Wpointer-arith -Wlogical-op -Wcast-qual -Wcast-align -Wnon-virtual-dtor"
			COMMON_CXXFLAGS="$COMMON_CXXFLAGS -Werror=reorder -Werror=main -Werror=missing-braces -Werror=array-bounds -Werror=sequence-point -Werror=return-type"

			dnl -Winline -Weffc++ -Wold-style-cast -Wsign-conversion -Wconversion -Wfloat-equal -Wstack-protector -Woverloaded-virtual

			dnl GCC forgets about -Wno-* options when -O* flag is specified in CXXFLAGS
			COMMON_CXXFLAGS="$COMMON_CXXFLAGS -Wextra -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-empty-body"
		fi
	dnl Develop mode flags <--

	dnl Libraries -->
		dnl appindicator -->
			APPINDICATOR_REQUIRED=0.0.7

			AC_ARG_ENABLE(appindicator,
				AS_HELP_STRING([--enable-appindicator[=@<:@no/auto/yes@:>@]], [application indicator support]),
				[enable_appindicator=$enableval],
				[enable_appindicator="auto"]
			)

			if test "x$enable_appindicator" = "xauto"; then
				PKG_CHECK_EXISTS([appindicator-0.1 >= $APPINDICATOR_REQUIRED],
					enable_appindicator="yes",
					enable_appindicator="no"
				)
			fi

			if test "x$enable_appindicator" = "xyes"; then
				PKG_CHECK_EXISTS([appindicator-0.1 >= $APPINDICATOR_REQUIRED],,
					 AC_MSG_ERROR([appindicator-0.1 is not installed]))
				PKG_CHECK_MODULES(appindicator, appindicator-0.1 >= $APPINDICATOR_REQUIRED)

				APP_CPPFLAGS="$APP_CPPFLAGS $appindicator_CFLAGS"
				APP_LDADD="$APP_LDADD $appindicator_LIBS"

				AC_DEFINE(HAVE_APP_INDICATOR, 1, [Have AppIndicator])
			fi
		dnl appindicator <--

		dnl boost -->
			AX_BOOST_BASE([1.34])
			if test -z "$BOOST_LDFLAGS"; then
				AC_MSG_ERROR([Unable to find Boost library.])
			fi
			APP_CPPFLAGS="$APP_CPPFLAGS $BOOST_CPPFLAGS"
			APP_LDADD="$APP_LDADD $BOOST_LDFLAGS"

			AX_BOOST_FILESYSTEM
			if test -z "$BOOST_FILESYSTEM_LIB"; then
				AC_MSG_ERROR([Unable to find Boost.Filesystem library.])
			fi
			APP_LDADD="$APP_LDADD $BOOST_FILESYSTEM_LIB"

			AX_BOOST_THREAD
			if test -z "$BOOST_THREAD_LIB"; then
				AC_MSG_ERROR([Unable to find Boost.Thread library.])
			fi
			APP_LDADD="$APP_LDADD $BOOST_THREAD_LIB"

			AX_BOOST_SYSTEM
			if test -z "$BOOST_SYSTEM_LIB"; then
				AC_MSG_ERROR([Unable to find Boost.System library.])
			fi
			APP_LDADD="$APP_LDADD $BOOST_SYSTEM_LIB"
		dnl boost <--

		dnl DBus -->
			PKG_CHECK_MODULES([dbus], [dbus-1])
			APP_CPPFLAGS="$APP_CPPFLAGS $dbus_CFLAGS"
			APP_LDADD="$APP_LDADD $dbus_LIBS"

			DBUSXX_CPPFLAGS='-I $(top_srcdir)/dbus-c++/include'
			DBUSXX_LIBRARIES='$(top_builddir)/dbus-c++/src/.libs/libdbus-c++-1.a'
			AC_SUBST([DBUSXX_XML2CPP], [['$(top_builddir)/dbus-c++/tools/dbusxx-xml2cpp']])

			APP_CPPFLAGS="$APP_CPPFLAGS $DBUSXX_CPPFLAGS"
			APP_DEPENDENCIES="$APP_DEPENDENCIES $DBUSXX_LIBRARIES"
			APP_LDADD="$APP_LDADD $DBUSXX_LIBRARIES"

			MLIB_SET_CUSTOM_LIBRARY([[dbus_cxx]], [[$DBUSXX_CPPFLAGS]], [[$DBUSXX_LIBRARIES]])
		dnl DBus <--

		dnl Gettext
		APP_LDADD="$APP_LDADD $LIBINTL"

		dnl GTK -->
			PKG_CHECK_MODULES([gthread], [gthread-2.0])
			APP_CPPFLAGS="$APP_CPPFLAGS $gthread_CFLAGS"
			APP_LDADD="$APP_LDADD $gthread_LIBS"

			PKG_CHECK_MODULES([gtkmm], [gtkmm-2.4])
			APP_CPPFLAGS="$APP_CPPFLAGS $gtkmm_CFLAGS"
			APP_LDADD="$APP_LDADD $gtkmm_LIBS"
		dnl GTK <--

		dnl libconfig -->
			PKG_CHECK_MODULES([libconfig], [libconfig++ >= 1.3])
			APP_CPPFLAGS="$APP_CPPFLAGS $libconfig_CFLAGS"
			APP_LDADD="$APP_LDADD $libconfig_LIBS"
		dnl libconfig <--

		dnl libnotify -->
			PKG_CHECK_MODULES([libnotify], [libnotify])
			APP_CPPFLAGS="$APP_CPPFLAGS $libnotify_CFLAGS"
			APP_LDADD="$APP_LDADD $libnotify_LIBS"
		dnl libnotify <--

		dnl libtorrent -->
			PKG_CHECK_MODULES([libtorrent_rasterbar], [libtorrent-rasterbar >= 0.14])
			APP_CPPFLAGS="$APP_CPPFLAGS $libtorrent_rasterbar_CFLAGS"
			APP_LDADD="$APP_LDADD $libtorrent_rasterbar_LIBS"
		dnl libtorrent <--

		dnl openssl -->
		CHECK_SSL
		APP_CPPFLAGS="$APP_CPPFLAGS $SSL_CXXFLAGS"
		APP_LDADD="$APP_LDADD $SSL_LDFLAGS $SSL_LIBS"
		dnl openssl <--
	dnl Libraries <--

	MLIB_CPPFLAGS="$COMMON_CPPFLAGS $MLIB_CPPFLAGS"
	MLIB_CFLAGS="$COMMON_CPPFLAGS $MLIB_CFLAGS"
	MLIB_CXXFLAGS="$COMMON_CXXFLAGS $MLIB_CXXFLAGS"
dnl Compiler flags <--

dnl Conditionals
AM_CONDITIONAL([DEVELOP_MODE], [test "X$develop_mode" = "Xyes"])

dnl Configuring mlib -->
	MLIB_FEATURE_ENABLE([[async_fs]])
	MLIB_FEATURE_ENABLE([[dbus]])
	MLIB_FEATURE_ENABLE([[fs_watcher]])
	MLIB_FEATURE_ENABLE([[gtk]])
	MLIB_FEATURE_ENABLE([[gtk_builder]])
	MLIB_FEATURE_ENABLE([[gtk_builder_emulation]])
	MLIB_FEATURE_ENABLE([[libconfig]])
	MLIB_FEATURE_ENABLE([[libtorrent]])
	MLIB_CONFIGURE
dnl Configuring mlib <--

dnl Application flags -->
	APP_CPPFLAGS="$MLIB_CPPFLAGS $APP_CPPFLAGS"
	APP_CFLAGS="$MLIB_CFLAGS $APP_CFLAGS"
	APP_CXXFLAGS="$MLIB_CXXFLAGS $APP_CXXFLAGS"
	APP_DEPENDENCIES="$MLIB_DEPENDENCIES $APP_DEPENDENCIES"
	APP_LDADD="$MLIB_LDADD $APP_LDADD"

	AC_SUBST([APP_CPPFLAGS])
	AC_SUBST([APP_CFLAGS])
	AC_SUBST([APP_CXXFLAGS])
	AC_SUBST([APP_DEPENDENCIES])
	AC_SUBST([APP_LDADD])
dnl Application flags <--

dnl Libraries inside of the package
PACKAGE_LIBRARIES_SUBDIRS="dbus-c++"
AC_CONFIG_SUBDIRS([dbus-c++])
AC_SUBST([PACKAGE_LIBRARIES_SUBDIRS])

dnl Summary report
AC_MSG_NOTICE([debug mode: $debug_mode])
AC_MSG_NOTICE([develop mode: $develop_mode])

AC_CONFIG_FILES([
	icons/Makefile
	man/Makefile
	man/ru/Makefile
	po/Makefile.in
	src/Makefile
	ui/Makefile
	flush.desktop
	Makefile
])

AC_OUTPUT

