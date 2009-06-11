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
#ifndef HEADER_MLIB_GTK_LINK_BUTTON
	#define HEADER_MLIB_GTK_LINK_BUTTON

	#include <gtkmm/box.h>


	namespace m { namespace gtk {

	/// В Gtk::LinkButton и GtkLinkButton по какой-то причине не работает
	/// функция set_uri(). Данная обертка позволяет это сделать.
	class Link_button: public Gtk::HBox
	{
		public:
			Link_button(const Glib::ustring& uri = "");


		private:
			Gtk::LinkButton*	link_button;
			sigc::signal<void>	clicked_signal;


		public:
			/// Аналог Gtk::LinkButton::get_uri().
			Glib::ustring		get_uri(void) const;

		#if GTK_CHECK_VERSION(2, 14, 0)
			/// Аналог Gtk::LinkButton::get_visited().
			bool				get_visited(void) const;
		#endif

			/// Аналог Gtk::LinkButton::set_uri().
			void				set_uri(const Glib::ustring& uri);

		#if GTK_CHECK_VERSION(2, 14, 0)
			/// Аналог Gtk::LinkButton::set_visited().
			void				set_visited(bool visited = true);
		#endif

			/// Аналог Gtk::LinkButton::signal_clicked().
			sigc::signal<void>&	signal_clicked(void);

		private:
			/// Создает новую кнопку внутри контейнера.
			void				recreate(const Glib::ustring& uri);

			/// Обработчик сигнала на нажатие кнопки.
			void				on_clicked_cb(void);
	};

	}}

#endif
#endif

