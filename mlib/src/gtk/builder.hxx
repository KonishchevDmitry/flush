/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
*                                                                         *
*   Copyright (C) 2009-2010, Dmitry Konishchev                            *
*   http://konishchevdmitry.blogspot.com/                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


#ifndef HEADER_MLIB_GTK_BUILDER_FWD
#define HEADER_MLIB_GTK_BUILDER_FWD

#if MLIB_ENABLE_FORWARDS
	#if MLIB_ENABLE_GTK_BUILDER_EMULATION
		#include <mlib/forwards/glademm.hpp>
	#else
		#include <mlib/forwards/gtkmm.hpp>
	#endif
#else
	#if MLIB_ENABLE_GTK_BUILDER_EMULATION
		#include <libglademm/xml.h>
	#else
		#include <gtkmm/builder.h>
	#endif
#endif

#include <mlib/main.hpp>

#if MLIB_ENABLE_GTK_BUILDER_EMULATION
	namespace m { namespace gtk {
		typedef ::Glib::RefPtr< ::Gnome::Glade::Xml > Builder;
	}}
#else
	namespace m { namespace gtk {
		typedef ::Glib::RefPtr< ::Gtk::Builder > Builder;
	}}
#endif

#endif

