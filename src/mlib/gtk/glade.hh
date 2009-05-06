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


#include "../messages.hpp"
#include "../string.hpp"


namespace m { namespace glade {

template<class T_widget>
T_widget* get_widget(const char* file, int line, const Glib::RefPtr<Gnome::Glade::Xml>& xml, const Glib::ustring& name, T_widget*& widget)
{
	widget = NULL;

	if(!xml->get_widget(name, widget))
	{
		m::error(file, line, __(
			"Can't get widget '%1' from glade file '%2'.",
			name, xml->get_filename()
		));
	}

	return widget;
}



template<class T_widget>
T_widget* get_widget_derived(const char* file, int line, const Glib::RefPtr<Gnome::Glade::Xml>& xml, const Glib::ustring& name, T_widget*& widget)
{
	widget = NULL;

	if(!xml->get_widget_derived(name, widget))
	{
		m::error(file, line, __(
			"Can't get widget '%1' from glade file '%2'.",
			name, xml->get_filename()
		));
	}

	return widget;
}

}}

