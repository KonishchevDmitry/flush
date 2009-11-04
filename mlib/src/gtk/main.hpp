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


#ifndef HEADER_MLIB_GTK_MAIN
#define HEADER_MLIB_GTK_MAIN

#include <mlib/main.hpp>


namespace m { namespace gtk {


/// Аналог gdk_threads_enter()/gdk_threads_leave().
class Scoped_enter
{
	public:
		Scoped_enter(void);
		~Scoped_enter(void);
};



extern const unsigned int MOUSE_LEFT_BUTTON;
extern const unsigned int MOUSE_RIGHT_BUTTON;

/// Используется при создании Gtk::Window:
/// window.set_border_width(WINDOW_BORDER_WIDTH)
extern int WINDOW_BORDER_WIDTH;

/// Используется при создании Gtk::Window:
/// box.set_border_width(BOX_BORDER_WIDTH)
extern int BOX_BORDER_WIDTH;

/// Используется при создании Gtk::HBox:
/// Gtk::HBox(false, m::gtk::HBOX_SPACING)
extern int HBOX_SPACING;

/// Используется при создании Gtk::VBox:
/// Gtk::VBox(false, m::gtk::VBOX_SPACING)
extern int VBOX_SPACING;

/// Используется при формировании таблиц.
///
/// Например:
/// Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name
extern int TABLE_NAME_VALUE_SPACING;

/// Используется при формировании таблиц.
/// Величина отступа между строками.
///
/// Например:
/// Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name
/// [TABLE_ROWS_SPACING]
/// Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name
extern int TABLE_ROWS_SPACING;

/// Используется при формировании таблиц.
/// Величина отступа между колонками, содержащими пары (имя, значение).
///
/// Например:
/// Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name[TABLE_NAME_VALUE_COLUMNS_SPACING]Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name
/// [TABLE_ROWS_SPACING]
/// Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name[TABLE_NAME_VALUE_COLUMNS_SPACING]Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name
extern int TABLE_NAME_VALUE_COLUMNS_SPACING;


}}

#endif

