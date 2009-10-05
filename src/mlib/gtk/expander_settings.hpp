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


#ifndef HEADER_MLIB_GTK_EXPANDER_SETTINGS
#define HEADER_MLIB_GTK_EXPANDER_SETTINGS


#if !MLIB_ENABLE_LIBS_FORWARDS
	#include <gtkmm/expander.h>
#endif

#include <mlib/main.hpp>


namespace m { namespace gtk {

class Expander_settings: private m::Virtual
{
	public:
		Expander_settings(bool expanded = false);


	public:
		bool	expanded;

	public:
		void	get(const Gtk::Expander& expander);
		void	set(Gtk::Expander& expander) const;
};

}}

#endif

