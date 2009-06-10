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


#ifdef MLIB_ENABLE_GTK
#ifndef HEADER_MLIB_GTK_ACTION
#define HEADER_MLIB_GTK_ACTION

#include <gtkmm/action.h>


namespace m { namespace gtk {

// В Ubuntu 8.04 (gtkmm-2.4 2.12) отсутствует
// Gtk::Action::create_with_icon_name().
Glib::RefPtr<Gtk::Action>	create_action_with_icon_name(const Glib::ustring& name, const Glib::ustring& icon_name, const Glib::ustring& label, const Glib::ustring& tooltip);

}}

#endif
#endif

