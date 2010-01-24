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


#if MLIB_ENABLE_GTK_BUILDER_EMULATION
	#include <libglademm/xml.h>
#else
	#include <gtkmm/builder.h>
#endif

#include <mlib/main.hpp>


namespace m { namespace gtk { namespace builder {

namespace
{
	/// Обработчик ошибки о невозможности получить виджет по имени.
	void	getting_widget_error(const char* file, int line, const Builder& builder, const Glib::ustring& name);



	void getting_widget_error(const char* file, int line, const Builder& builder, const Glib::ustring& name)
	{
		m::error(file, line, __(
			"Can't get widget '%1' from glade file '%2'.",
			name, get_file_name(builder)
		));
	}

}



template<class T_widget>
T_widget* get_widget(const char* file, int line, const Builder& builder, const Glib::ustring& name, T_widget*& widget)
{
	widget = NULL;

	#if MLIB_ENABLE_GTK_BUILDER_EMULATION
		if(!builder->get_widget(name, widget))
			getting_widget_error(file, line, builder, name);
	#else
		builder->get_widget(name, widget);

		if(!widget)
			getting_widget_error(file, line, builder, name);
	#endif

	return widget;
}



template<class T_widget>
T_widget* get_widget_derived(const char* file, int line, const Builder& builder, const Glib::ustring& name, T_widget*& widget)
{
	widget = NULL;

	#if MLIB_ENABLE_GTK_BUILDER_EMULATION
		if(!builder->get_widget_derived(name, widget))
			getting_widget_error(file, line, builder, name);
	#else
		builder->get_widget_derived(name, widget);

		if(!widget)
			getting_widget_error(file, line, builder, name);
	#endif

	return widget;
}

}}}

