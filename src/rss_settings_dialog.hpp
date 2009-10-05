/**************************************************************************
*                                                                         *
*   Flush - GTK-based BitTorrent client                                   *
*   http://sourceforge.net/projects/flush                                 *
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


#ifndef HEADER_RSS_WINDOW
	#define HEADER_RSS_WINDOW

#if 0
	/*
	#include <boost/shared_ptr.hpp>

	#include <gtkmm/dialog.h>
	#include <gtkmm/window.h>

	#include <libglademm/xml.h>

	#include <mlib/gtk/dialog.hpp>
	*/



	namespace Rss_settings_dialog_aux { class Impl; }


	/// Объект, хранящий все настройки Rss_settings_dialog.
	class Rss_settings_dialog_settings: private Virtual
	{
		public:
			Rss_settings_dialog_settings(void);


		public:
	};


	/// Окно задания настроек RSS.
	class Rss_settings_dialog: public m::gtk::Dialog
	{
	public:
		typedef Categories_view_settings Settings;

	private:
		typedef Categories_view_aux::Impl Impl;


	public:
		Categories_view(const Settings& settings);


	private:
		boost::scoped_ptr<Impl>	impl;


		private:
			typedef Rss_settings_dialog_aux::Private Private;


		public:
			Rss_settings_dialog(BaseObjectType* cobject, const m::gtk::Builder& builder);


		private:
			boost::shared_ptr<Private>	priv;


		public:
			/// Предназначена для инициализации виджета после конструирования
			/// его из Glade-представления.
			virtual
			void	init(Gtk::Window& parent_window, Temporary_action action, Torrents_group group);


		public:
			/// Возвращает установленное в данный момент время.
			Time	get_time(void) const;

		private:
			/// Обработчик сигнала на закрытие диалога.
			void	on_response_cb(int response_id) const;
	};
#endif

#endif

