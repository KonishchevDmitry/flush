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


#ifndef HEADER_MLIB_GTK_PANED
#define HEADER_MLIB_GTK_PANED

#include <gtkmm/paned.h>

#include <mlib/gtk/main.hpp>
#include <mlib/gtk/paned_settings.hpp>


namespace m { namespace gtk {

class VPaned: public Gtk::VPaned
{
	public:
		typedef Paned_settings Settings;


	public:
		VPaned(const Settings& settings = Settings());
	

	public:
		/// Сохраняет текущие настройки VPaned.
		void save_settings(Settings& settings) const;
};

}}

#endif

