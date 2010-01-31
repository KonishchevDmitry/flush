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


#ifndef HEADER_MLIB_GTK_WINDOW
	#define HEADER_MLIB_GTK_WINDOW

	#include <gtk/gtkwindow.h>

	#include <gtkmm/window.h>

	#include <mlib/gtk/main.hpp>
	#include <mlib/gtk/window_settings.hpp>


	namespace m { namespace gtk {

	class Window: public Gtk::Window
	{
		public:
			typedef Window_settings Settings;


		public:
			Window(const std::string& title, const Settings& settings = Settings(), int width = -1, int height = -1, int border_width = m::gtk::WINDOW_BORDER_WIDTH);
			Window(GtkWindow* parent_window, const std::string& title, const Settings& settings = Settings(), int width = -1, int height = -1, int border_width = m::gtk::WINDOW_BORDER_WIDTH);
			Window(Gtk::Window& parent_window, const std::string& title, const Settings& settings = Settings(), int width = -1, int height = -1, int border_width = m::gtk::WINDOW_BORDER_WIDTH);


		private:
			/// Инициализатор, который вызывают конструкторы.
			void	init(GtkWindow* parent_window, const std::string& title, const Settings& settings, int width, int height, int border_width);

		public:
			/// Аналог Gtk::Window::set_title(), но в дополнение форматирует
			/// заголовок окна.
			void	set_title(const Glib::ustring& title);

			/// Сохраняет текущие настройки Window.
			void	save_settings(Settings& settings) const;
	};

	}}

#endif

