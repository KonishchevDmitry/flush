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


#ifdef MLIB_ENABLE_GTK

#include <gtk/gtk.h>

#include <gtkmm/action.h>

#include "action.hpp"


namespace m { namespace gtk {

Glib::RefPtr<Gtk::Action> create_action_with_icon_name(const Glib::ustring& name, const Glib::ustring& icon_name, const Glib::ustring& label, const Glib::ustring& tooltip)
{
	#if GTK_CHECK_VERSION(2, 16, 0)
		return Gtk::Action::create_with_icon_name(name, icon_name, label, tooltip);
	#else
		Glib::RefPtr<Gtk::Action> action;

		action = Gtk::Action::create(name, label, tooltip);
		g_object_set(G_OBJECT(action->gobj()), "icon-name", icon_name.c_str(), NULL);

		return action;
	#endif
}

}}

#endif

