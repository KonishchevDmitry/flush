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
#ifndef HEADER_MLIB_GTK_GLADE
	#define HEADER_MLIB_GTK_GLADE

	#include <glibmm/refptr.h>
	#include <glibmm/ustring.h>

	#include <libglademm/xml.h>

	#include "types.hpp"


	#define MLIB_GLADE_CREATE(args...) m::glade::create(__FILE__, __LINE__, args)
	#define MLIB_GLADE_GET_WIDGET(args...) m::glade::get_widget(__FILE__, __LINE__, args)
	#define MLIB_GLADE_GET_WIDGET_DERIVED(args...) m::glade::get_widget_derived(__FILE__, __LINE__, args)


	namespace m { namespace glade {

	/// Аналог Gnome::Glade::Xml::create, но только в случае неудачи
	/// аварийно завершает работу программы.
	Glib::RefPtr<Gnome::Glade::Xml>	create(const char* file, int line, const std::string& filename, const Glib::ustring& root = Glib::ustring(), const Glib::ustring& domain = Glib::ustring());

	/// Аналог Gnome::Glade::Xml::get_widget, но не проверяет widget на NULL и в
	/// случае неудачи аварийно завершает работу программы.
	Gtk::Widget* 					get_widget(const char* file, int line, const Glib::RefPtr<Gnome::Glade::Xml>& xml, const Glib::ustring &name);

	/// Аналог Gnome::Glade::Xml::get_widget, но не проверяет widget на NULL и в
	/// случае неудачи аварийно завершает работу программы.
	template<class T_widget>
	T_widget*						get_widget(const char* file, int line, const Glib::RefPtr<Gnome::Glade::Xml>& xml, const Glib::ustring& name, T_widget*& widget);

	/// Аналог Gnome::Glade::Xml::get_widget_derived, но не проверяет widget на
	/// NULL и в случае неудачи аварийно завершает работу программы.
	template<class T_widget>
	T_widget*						get_widget_derived(const char* file, int line, const Glib::RefPtr<Gnome::Glade::Xml>& xml, const Glib::ustring& name, T_widget*& widget);

	}}

	#include "glade.hh"

#endif
#endif


