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


#if !MLIB_ENABLE_GTK_BUILDER_EMULATION
	#include <gtk/gtk.h>

// TODO FIXME
//	#error Not tested yet
#endif

#include <mlib/main.hpp>

#include "builder.hpp"


namespace m { namespace gtk { namespace builder {

Builder create(const char* file, int line, const std::string& filename, const Glib::ustring& root)
{
	#if MLIB_ENABLE_GTK_BUILDER_EMULATION
		try
		{
			return Gnome::Glade::Xml::create(filename, root);
		}
		catch(Gnome::Glade::XmlError& e)
		{
			m::error(file, line, e.what());
		}
	#else
		try
		{
			return Gtk::Builder::create_from_file(filename, root);
		}
		catch(Gtk::BuilderError& e)
		{
			m::error(file, line, e.what());
		}
	#endif
}



Glib::ustring get_file_name(const Builder& builder)
{
	#if MLIB_ENABLE_GTK_BUILDER_EMULATION
		return builder->get_filename();
	#else
		// TODO FIXME
		return "";
	#endif
}



Gtk::Widget* get_widget(const char* file, int line, const Builder& builder, const Glib::ustring &name)
{
	Gtk::Widget* widget = NULL;

	builder->get_widget(name, widget);

	if(!widget)
		getting_widget_error(file, line, builder, name);

	return widget;
}

}}}

