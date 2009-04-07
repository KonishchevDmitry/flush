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
#ifndef HEADER_MLIB_GTK_TOOLBAR
	#define HEADER_MLIB_GTK_TOOLBAR

	/// Слегка облегчает работу с Gtk::Toolbar.

	#include <string>

	#include <gtkmm/toolbar.h>

	#include <mlib/errors.hpp>
	#include "types.hpp"


	namespace m { namespace gtk { namespace toolbar {

	/// Аналогичен Gtk::ToolbarStyle, за исключением того, что включает стиль
	/// DEFAULT.
	enum Style {
		DEFAULT,
		ICONS,
		TEXT,
		BOTH,
		BOTH_HORIZ
	};


	/// Преобразовывает Style в Gtk::ToolbarStyle.
	Gtk::ToolbarStyle	convert(Style style);

	/// Преобразовывает Gtk::ToolbarStyle в Style.
	Style				convert(Gtk::ToolbarStyle style);

	/// Возвращает стиль по его строковому представлению.
	/// В случае ошибки возвращает стиль DEFAULT.
	Style				get_style_from_string(const std::string& string) throw(m::Exception);

	/// Возвращает строковое имя типа.
	std::string			get_style_string_representation(Style style);

	/// Устанавливает для панели инструментов стиль style.
	void				set_style(Gtk::Toolbar& toolbar, Style style);

	}}}

#endif
#endif

