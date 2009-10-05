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
#ifndef HEADER_MLIB_GTK_DIALOG
	#define HEADER_MLIB_GTK_DIALOG

	#include <gtkmm/dialog.h>
	#include <gtkmm/window.h>

	#include <mlib/gtk/dialog_settings.hpp>
	#include <mlib/gtk/main.hpp>
	#include <mlib/gtk/misc.hxx>


	namespace m { namespace gtk {

	class Dialog: public Gtk::Dialog
	{
		public:
			typedef Dialog_settings Settings;


		public:
			Dialog(BaseObjectType* cobject);
			Dialog(Gtk::Window& parent_window, const std::string& title = "", const Settings& settings = Settings(), int width = -1, int height = -1, int border_width = m::gtk::WINDOW_BORDER_WIDTH);

		public:
			/// Предназначена для инициализации виджета после конструирования
			/// его из Glade-представления.
			virtual
			void	init(Gtk::Window& parent_window, const Settings& settings = Settings());


		public:
			/// Сохраняет текущие настройки.
			void	save_settings(Settings& settings) const;
	};

	}}

#endif
#endif

