/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
*                                                                         *
*   Copyright (C) 2009, Konishchev Dmitry                                 *
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


#ifdef MLIB_ENABLE_GLADE
#ifndef HEADER_MLIB_GTK_GLADE_FWD
#define HEADER_MLIB_GTK_GLADE_FWD

	#if MLIB_ENABLE_FORWARDS
		#include <mlib/forwards/glademm.hpp>
	#else
		#include <libglademm/xml.h>
	#endif

	#include <mlib/main.hpp>

	namespace m { namespace glade {
		typedef ::Glib::RefPtr< ::Gnome::Glade::Xml > Glade_xml;
	}}

	#ifdef MLIB_ENABLE_ALIASES
		using m::glade::Glade_xml;
	#endif

#endif
#endif


