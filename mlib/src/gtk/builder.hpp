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


#ifndef HEADER_MLIB_GTK_BUILDER
#define HEADER_MLIB_GTK_BUILDER

#include <memory>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#ifndef MLIB_ENABLE_LIBS_FORWARDS
	#include <gtkmm/widget.h>
#endif

#include "builder.hxx"


#define MLIB_GTK_BUILDER_CREATE(args...) ::m::gtk::builder::create(__FILE__, __LINE__, args)
#define MLIB_GTK_BUILDER_GET_WIDGET(args...) ::m::gtk::builder::get_widget(__FILE__, __LINE__, args)
#define MLIB_GTK_BUILDER_GET_WIDGET_DERIVED(args...) ::m::gtk::builder::get_widget_derived(__FILE__, __LINE__, args)


namespace m { namespace gtk { namespace builder {

/// Аналог Gnome::Glade::Xml::create, но только в случае неудачи
/// аварийно завершает работу программы.
Builder			create(const char* file, int line, const std::string& filename, const Glib::ustring& root = Glib::ustring());

/// Возвращает путь к файлу, из которого был загружен builder. В случае ошибки
/// возвращает "".
Glib::ustring	get_file_name(const Builder& builder);

/// Аналог Gnome::Glade::Xml::get_widget, но не проверяет widget на NULL и в
/// случае неудачи аварийно завершает работу программы.
Gtk::Widget*	get_widget(const char* file, int line, const Builder& builder, const Glib::ustring &name);

/// Аналог Gnome::Glade::Xml::get_widget, но не проверяет widget на NULL и в
/// случае неудачи аварийно завершает работу программы.
template<class T_widget>
T_widget*		get_widget(const char* file, int line, const Builder& builder, const Glib::ustring& name, T_widget*& widget);

/// Аналог Gnome::Glade::Xml::get_widget_derived, но не проверяет widget на
/// NULL и в случае неудачи аварийно завершает работу программы.
template<class T_widget>
T_widget*		get_widget_derived(const char* file, int line, const Builder& builder, const Glib::ustring& name, T_widget*& widget);

}}}

#include "builder.hh"

#endif

