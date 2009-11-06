dnl /**************************************************************************
dnl *                                                                         *
dnl *   MLib - library of some useful things for internal usage               *
dnl *                                                                         *
dnl *   Copyright (C) 2009, Konishchev Dmitry                                 *
dnl *   http://konishchevdmitry.blogspot.com/                                 *
dnl *                                                                         *
dnl *   This program is free software; you can redistribute it and/or modify  *
dnl *   it under the terms of the GNU General Public License as published by  *
dnl *   the Free Software Foundation; either version 3 of the License, or     *
dnl *   (at your option) any later version.                                   *
dnl *                                                                         *
dnl *   This program is distributed in the hope that it will be useful,       *
dnl *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
dnl *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
dnl *   GNU General Public License for more details.                          *
dnl *                                                                         *
dnl **************************************************************************/



dnl	SYNOPSIS
dnl
dnl		MLIB_INIT
dnl
dnl
dnl	DESCRIPTION
dnl
dnl		Initializes mlib macroses.
dnl
dnl
dnl	This macro sets:
dnl
dnl		MLIB_CPPFLAGS=''
dnl		MLIB_CFLAGS=''
dnl		MLIB_CXXFLAGS=''
dnl		MLIB_LDADD=''
dnl
AC_DEFUN([MLIB_INIT],
[
	if test "$#" != "0"
	then
		AC_MSG_ERROR([[MLIB_INIT usage error at line $LINENO: macros takes zero arguments.]])
	fi

	dnl Clearing all output variables -->
		MLIB_CPPFLAGS=''
		MLIB_CFLAGS=''
		MLIB_CXXFLAGS=''
		MLIB_LDADD=''
	dnl Clearing all output variables <--
])



dnl	SYNOPSIS
dnl
dnl		MLIB_SET_CUSTOM_LIBRARY([[LIBRARY_NAME]], [[LIBRARY_CPPFLAGS]], [[LIBRARY_LDADD]])
dnl
dnl
dnl	DESCRIPTION
dnl
dnl		Sets custom CPPFLAGS and LDADD for library LIBRARY_NAME.
dnl
AC_DEFUN([MLIB_SET_CUSTOM_LIBRARY],
[
	if test "$#" != "3"
	then
		AC_MSG_ERROR([[MLIB_SET_CUSTOM_LIBRARY usage error at line $LINENO: macros takes three arguments.]])
	else
		eval "mlib_$1_is_custom=yes"
		eval "mlib_$1_CPPFLAGS='$2'"
		eval "mlib_$1_LDADD='$3'"
	fi
])



dnl	SYNOPSIS
dnl
dnl		MLIB_FEATURE_ENABLE([[FEATURE_NAME]])
dnl
dnl
dnl	DESCRIPTION
dnl
dnl		Enables mlib feature FEATURE_NAME.
dnl
AC_DEFUN([MLIB_FEATURE_ENABLE],
[
	if test "$#" != "1"
	then
		AC_MSG_ERROR([[MLIB_FEATURE_ENABLE usage error at line $LINENO: macros takes one argument.]])
	else
		mlib_feature="$1"
		mlib_feature_value="yes"
	fi

	case "$mlib_feature" in
		"debug_mode")
			mlib_debug_mode="$mlib_feature_value";;
		"develop_mode")
			mlib_develop_mode="$mlib_feature_value";;

		"async_fs")
			mlib_enable_async_fs="$mlib_feature_value";;
		"dbus")
			mlib_enable_dbus="$mlib_feature_value";;
		"fs_watcher")
			mlib_enable_fs_watcher="$mlib_feature_value";;
		"gtk")
			mlib_enable_gtk="$mlib_feature_value";;
		"gtk_builder")
			mlib_enable_gtk_builder="$mlib_feature_value";;
		"gtk_builder_emulation")
			mlib_enable_gtk_builder_emulation="$mlib_feature_value";;
		"libconfig")
			mlib_enable_libconfig="$mlib_feature_value";;
		"libtorrent")
			mlib_enable_libtorrent="$mlib_feature_value";;
		"sqlite")
			mlib_enable_sqlite="$mlib_feature_value";;

		*)
			AC_MSG_ERROR([[MLIB_FEATURE_ENABLE usage error at line $LINENO: unknown feature name '$mlib_feature'.]])
	esac
])



dnl	SYNOPSIS
dnl
dnl		MLIB_CONFIGURE
dnl
dnl
dnl	DESCRIPTION
dnl
dnl		Configures mlib library. This macros must be called after MLIB_INIT and
dnl		all calls of MLIB_SET_CUSTOM_LIBRARY and MLIB_FEATURE_ENABLE.
dnl
dnl
dnl	This macro requires:
dnl
dnl		AC_CONFIG_HEADERS's call
dnl		AC_CONFIG_HEADERS's config.h include in the MLIB_CPPFLAGS
dnl		PKG_PROG_PKG_CONFIG's call
dnl
dnl
dnl	And appends flags to:
dnl
dnl		MLIB_CPPFLAGS
dnl		MLIB_CFLAGS
dnl		MLIB_CXXFLAGS
dnl		MLIB_LDADD
dnl
AC_DEFUN([MLIB_CONFIGURE],
[
	if test "$#" != "0"
	then
		AC_MSG_ERROR([[MLIB_CONFIGURE usage error at line $LINENO: macros takes zero arguments.]])
	fi


	mlib_path="mlib"

	AC_MSG_NOTICE([[Configuring mlib...]])


	dnl Features -->

		dnl Debug mode -->
			AM_CONDITIONAL([MLIB_DEBUG_MODE], [test "X$mlib_debug_mode" = "Xyes"])
			AH_TEMPLATE([MLIB_DEBUG_MODE], [Is mlib compiling in the debug mode])

			if test "X$mlib_debug_mode" = "Xyes"
			then
				AC_DEFINE([MLIB_DEBUG_MODE])
			else
				mlib_debug_mode="no"
			fi

			AC_MSG_NOTICE([[[mlib config] Debug mode: $mlib_debug_mode]])
		dnl Debug mode <--

		dnl Develop mode -->
			AM_CONDITIONAL([MLIB_DEVELOP_MODE], [test "X$mlib_develop_mode" = "Xyes"])
			AH_TEMPLATE([MLIB_DEVELOP_MODE], [Is mlib compiling in the develop mode])
			AH_TEMPLATE([MLIB_ENABLE_LIBS_FORWARDS],
				[Disable including libraries forward declarations for compile time reducing] )

			if test "X$mlib_develop_mode" = "Xyes"
			then
				AC_DEFINE([MLIB_DEVELOP_MODE])
				AC_DEFINE([MLIB_ENABLE_LIBS_FORWARDS])
			else
				mlib_develop_mode="no"
			fi

			AC_MSG_NOTICE([[[mlib config] Develop mode: $mlib_develop_mode]])
		dnl Develop mode <--


		dnl Async FS -->
			AM_CONDITIONAL([MLIB_ENABLE_ASYNC_FS], [test "X$mlib_enable_async_fs" = "Xyes"])
			AH_TEMPLATE([MLIB_ENABLE_ASYNC_FS], [Enable Async FS support for mlib])

			if test "X$mlib_enable_async_fs" = "Xyes"
			then
				AC_DEFINE([MLIB_ENABLE_ASYNC_FS])
			else
				mlib_enable_async_fs="no"
			fi

			AC_MSG_NOTICE([[[mlib config] Enable Async FS: $mlib_enable_async_fs]])
		dnl Async FS <--

		dnl DBus -->
			AM_CONDITIONAL([MLIB_ENABLE_DBUS], [test "X$mlib_enable_dbus" = "Xyes"])
			AH_TEMPLATE([MLIB_ENABLE_DBUS], [Enable DBus support for mlib])

			if test "X$mlib_enable_dbus" = "Xyes"
			then
				AC_DEFINE([MLIB_ENABLE_DBUS])
			else
				mlib_enable_dbus="no"
			fi

			AC_MSG_NOTICE([[[mlib config] Enable DBus: $mlib_enable_dbus]])
		dnl DBus <--

		dnl FS watcher -->
			AM_CONDITIONAL([MLIB_ENABLE_FS_WATCHER], [test "X$mlib_enable_fs_watcher" = "Xyes"])
			AH_TEMPLATE([MLIB_ENABLE_FS_WATCHER], [Enable FS watcher support for mlib])

			if test "X$mlib_enable_fs_watcher" = "Xyes"
			then
				AC_DEFINE([MLIB_ENABLE_FS_WATCHER])
			else
				mlib_enable_fs_watcher="no"
			fi

			AC_MSG_NOTICE([[[mlib config] Enable FS watcher: $mlib_enable_fs_watcher]])
		dnl FS watcher <--

		dnl GTK -->
			AM_CONDITIONAL([MLIB_ENABLE_GTK], [test "X$mlib_enable_gtk" = "Xyes"])
			AH_TEMPLATE([MLIB_ENABLE_GTK], [Enable GTK support for mlib])

			if test "X$mlib_enable_gtk" = "Xyes"
			then
				AC_DEFINE([MLIB_ENABLE_GTK])
			else
				mlib_enable_gtk="no"
			fi

			AC_MSG_NOTICE([[[mlib config] Enable GTK: $mlib_enable_gtk]])
		dnl GTK <--

		dnl GtkBuilder -->
			AM_CONDITIONAL([MLIB_ENABLE_GTK_BUILDER], [test "X$mlib_enable_gtk_builder" = "Xyes"])
			AH_TEMPLATE([MLIB_ENABLE_GTK_BUILDER], [Enable GtkBuilder support for mlib])

			if test "X$mlib_enable_gtk_builder" = "Xyes"
			then
				AC_DEFINE([MLIB_ENABLE_GTK_BUILDER])
			else
				mlib_enable_gtk_builder="no"
			fi

			AC_MSG_NOTICE([[[mlib config] Enable GtkBuilder: $mlib_enable_gtk_builder]])
		dnl GtkBuilder <--

		dnl GtkBuilder emulation -->
			AM_CONDITIONAL([MLIB_ENABLE_GTK_BUILDER_EMULATION], [test "X$mlib_enable_gtk_builder_emulation" = "Xyes"])
			AH_TEMPLATE([MLIB_ENABLE_GTK_BUILDER_EMULATION], [Enable GtkBuilder emulation for mlib])

			if test "X$mlib_enable_gtk_builder_emulation" = "Xyes"
			then
				AC_DEFINE([MLIB_ENABLE_GTK_BUILDER_EMULATION])
			else
				mlib_enable_gtk_builder_emulation="no"
			fi

			AC_MSG_NOTICE([[[mlib config] Enable GtkBuilder emulation: $mlib_enable_gtk_builder_emulation]])
		dnl GtkBuilder emulation <--

		dnl Inotify -->
			AC_CHECK_HEADER([[sys/inotify.h]],
				[[ mlib_enable_inotify=yes ]],
				[[ mlib_enable_inotify=no ]]
			)

			AH_TEMPLATE([MLIB_ENABLE_INOTIFY], [Enable inotify in mlib])
			if test "X$mlib_enable_inotify" = "Xyes"
			then
				AC_DEFINE([MLIB_ENABLE_INOTIFY])
			fi

			AC_MSG_NOTICE([[[mlib config] Enable inotify: $mlib_enable_inotify]])
		dnl Inotify <--

		dnl libconfig -->
			AM_CONDITIONAL([MLIB_ENABLE_LIBCONFIG], [test "X$mlib_enable_libconfig" = "Xyes"])
			AH_TEMPLATE([MLIB_ENABLE_LIBCONFIG], [Enable libconfig support for mlib])

			if test "X$mlib_enable_libconfig" = "Xyes"
			then
				AC_DEFINE([MLIB_ENABLE_LIBCONFIG])
			else
				mlib_enable_libconfig="no"
			fi

			AC_MSG_NOTICE([[[mlib config] Enable libconfig: $mlib_enable_libconfig]])
		dnl libconfig <--

		dnl libtorrent -->
			AM_CONDITIONAL([MLIB_ENABLE_LIBTORRENT], [test "X$mlib_enable_libtorrent" = "Xyes"])
			AH_TEMPLATE([MLIB_ENABLE_LIBTORRENT], [Enable libtorrent support for mlib])

			if test "X$mlib_enable_libtorrent" = "Xyes"
			then
				AC_DEFINE([MLIB_ENABLE_LIBTORRENT])
			else
				mlib_enable_libtorrent="no"
			fi

			AC_MSG_NOTICE([[[mlib config] Enable libtorrent: $mlib_enable_libtorrent]])
		dnl libconfig <--

		dnl SQLite -->
			AM_CONDITIONAL([MLIB_ENABLE_SQLITE], [test "X$mlib_enable_sqlite" = "Xyes"])
			AH_TEMPLATE([MLIB_ENABLE_SQLITE], [Enable SQLite support for mlib])

			if test "X$mlib_enable_sqlite" = "Xyes"
			then
				AC_DEFINE([MLIB_ENABLE_SQLITE])
			else
				mlib_enable_sqlite="no"
			fi

			AC_MSG_NOTICE([[[mlib config] Enable SQLite: $mlib_enable_sqlite]])
		dnl SQLite <--

	dnl Features <--


	dnl libraries -->

		dnl boost -->
			AX_BOOST_BASE([[1.34]])
			if test -z "$BOOST_LDFLAGS"
			then
				AC_MSG_ERROR([[Unable to find Boost library.]])
			fi
			MLIB_CPPFLAGS="$MLIB_CPPFLAGS $BOOST_CPPFLAGS"
			MLIB_LDADD="$MLIB_LDADD $BOOST_LDFLAGS"

			AX_BOOST_FILESYSTEM
			if test -z "$BOOST_FILESYSTEM_LIB"
			then
				AC_MSG_ERROR([[Unable to find Boost.Filesystem library.]])
			fi
			MLIB_LDADD="$MLIB_LDADD $BOOST_FILESYSTEM_LIB"

			AX_BOOST_SIGNALS
			if test -z "$BOOST_SIGNALS_LIB"
			then
				AC_MSG_ERROR([[Unable to find Boost.Signals library.]])
			fi
			MLIB_LDADD="$MLIB_LDADD $BOOST_SIGNALS_LIB"

			AX_BOOST_THREAD
			if test -z "$BOOST_THREAD_LIB"
			then
				AC_MSG_ERROR([[Unable to find Boost.Thread library.]])
			fi
			MLIB_LDADD="$MLIB_LDADD $BOOST_THREAD_LIB"
		dnl boost <--

		dnl DBus -->
			if test "X$mlib_enable_dbus" = "Xyes"
			then
				PKG_CHECK_MODULES([dbus], [dbus-1])
				MLIB_CPPFLAGS="$MLIB_CPPFLAGS $dbus_CFLAGS"
				MLIB_LDADD="$MLIB_LDADD $dbus_LIBS"

				if test "X$mlib_dbus_cxx_is_custom" = "Xyes"
				then
					MLIB_CPPFLAGS="$MLIB_CPPFLAGS $mlib_dbus_cxx_CPPFLAGS"
					MLIB_LDADD="$MLIB_LDADD $mlib_dbus_cxx_LDADD"
				else
					PKG_CHECK_MODULES([dbus_cxx], [dbus-c++])
					MLIB_CPPFLAGS="$MLIB_CPPFLAGS $dbus_cxx_CFLAGS"
					MLIB_LDADD="$MLIB_LDADD $dbus_cxx_LIBS"
				fi
			fi
		dnl DBus <--

		dnl Gettext
		MLIB_LDADD="$MLIB_LDADD $LIBINTL"

		dnl GTK -->
			if test "X$mlib_enable_gtk" = "Xyes"
			then
				PKG_CHECK_MODULES([gtkmm], [gtkmm-2.4])
				MLIB_CPPFLAGS="$MLIB_CPPFLAGS $gtkmm_CFLAGS"
				MLIB_LDADD="$MLIB_LDADD $gtkmm_LIBS"

				dnl Glade -->
					if test "X$mlib_enable_gtk_builder" = "Xyes" -a "X$mlib_enable_gtk_builder_emulation" = "Xyes"
					then
						PKG_CHECK_MODULES([glademm], [libglademm-2.4])
						MLIB_CPPFLAGS="$MLIB_CPPFLAGS $glademm_CFLAGS"
						MLIB_LDADD="$MLIB_LDADD $glademm_LIBS"
					fi
				dnl Glade <--
			fi
		dnl GTK <--

		dnl libconfig -->
			if test "X$mlib_enable_libconfig" = "Xyes"
			then
				if test "X$mlib_libconfig_is_custom" = "Xyes"
				then
					MLIB_CPPFLAGS="$MLIB_CPPFLAGS $mlib_libconfig_CPPFLAGS"
					MLIB_LDADD="$MLIB_LDADD $mlib_libconfig_LDADD"
				else
					PKG_CHECK_MODULES([libconfig], [libconfig++ >= 1.3])
					MLIB_CPPFLAGS="$MLIB_CPPFLAGS $libconfig_CFLAGS"
					MLIB_LDADD="$MLIB_LDADD $libconfig_LIBS"
				fi
			fi
		dnl libconfig <--

		dnl libtorrent -->
			if test "X$mlib_enable_libtorrent" = "Xyes"
			then
				if test "X$mlib_libtorrent_is_custom" = "Xyes"
				then
					MLIB_CPPFLAGS="$MLIB_CPPFLAGS $mlib_libtorrent_CPPFLAGS"
					MLIB_LDADD="$MLIB_LDADD $mlib_libtorrent_LDADD"
				else
					dnl TODO: remove bad named libtorrent-rasterbar-0.14 when it package
					dnl will become obsolete
					PKG_CHECK_MODULES([libtorrent_rasterbar], [libtorrent-rasterbar-0.14 >= 0.14], [], [
						PKG_CHECK_MODULES([libtorrent_rasterbar], [libtorrent-rasterbar >= 0.14])
					])
					MLIB_CPPFLAGS="$MLIB_CPPFLAGS $libtorrent_rasterbar_CFLAGS"
					MLIB_LDADD="$MLIB_LDADD $libtorrent_rasterbar_LIBS"
				fi
			fi
		dnl libtorrent <--

	dnl libraries <--


	dnl Flags -->
		dnl mlib includes
		MLIB_CPPFLAGS="$MLIB_CPPFLAGS -I \$(top_srcdir)/$mlib_path/include"

		dnl mlib libraries -->
			MLIB_LDADD="$MLIB_LDADD \$(top_builddir)/$mlib_path/src/base/libmlib_base.a"
			MLIB_LDADD="$MLIB_LDADD \$(top_builddir)/$mlib_path/src/libmlib.a"

			if test "X$mlib_enable_gtk" = "Xyes"
			then
				MLIB_LDADD="$MLIB_LDADD \$(top_builddir)/$mlib_path/src/gtk/libmlib_gtk.a"
			fi
		dnl mlib libraries <--

		AC_SUBST([MLIB_CPPFLAGS])
		AC_SUBST([MLIB_CFLAGS])
		AC_SUBST([MLIB_CXXFLAGS])
		AC_SUBST([MLIB_LDADD])
	dnl Flags <--


	AC_CONFIG_FILES([
		mlib/src/base/libs_forwards/Makefile
		mlib/src/base/Makefile
		mlib/src/gtk/Makefile
		mlib/src/Makefile
		mlib/Makefile
	])


	AC_MSG_NOTICE([[mlib have been successfully configured.]])
])

