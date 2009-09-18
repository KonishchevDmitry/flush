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


#ifndef HEADER_CATEGORIES_VIEW
#define HEADER_CATEGORIES_VIEW

#include <boost/scoped_ptr.hpp>

#include <gtkmm/box.h>

#include <mlib/gtk/signal_proxy.hxx>

#include "categories_view.hxx"
#include "common.hpp"


namespace Categories_view_aux { class Impl; }

/// Фильтр, используемый для отсеивания только тех торрентов, которые в данный
/// момент интересуют пользователя.
class Categories_filter
{
	public:
		Categories_filter(int categories);


	private:
		int	categories;


	public:
		/// @return - true, если торрент интересует пользователя.
		bool	operator()(const Torrent_info& info) const;
};


/// Объект, хранящий все настройки Categories_view.
class Categories_view_settings: private m::Virtual
{
	public:
		Categories_view_settings(void);


	public:
		bool						visible;
		bool						show_names;
		bool						show_counters;
		std::vector<std::string>	selected_items;
};


/// Виджет, предоставляющий возможноть выбора определенных категорий, к которым
/// может принадлежать торрент, для последующей фильтрации списка торрентов.
class Categories_view: public Gtk::VBox
{
	public:
		typedef Categories_view_settings Settings;

	private:
		typedef Categories_view_aux::Impl Impl;


	public:
		Categories_view(const Settings& settings);


	private:
		boost::scoped_ptr<Impl>	impl;


	public:
		/// Возвращает фильтр, отсеивающий торренты в соответствии с текущим
		/// состоянием Categories_view.
		Categories_filter			get_filter(void) const;

		/// Сохраняет текущие настройки.
		void						save_settings(Settings* settings) const;

		/// Отображать или нет счетчики.
		void						show_counters(bool show = true);

		/// Отображать или нет имена категорий.
		void						show_names(bool show = true);

		/// Сигнал на изменение выделенных в данный момент категорий.
		m::gtk::Signal_proxy<void>	signal_changed(void);

		/// Сигнал, извещающий о том, что виджет нуждается в том, чтобы ему
		/// передали информацию о текущих торрентах (обязателен к подключению).
		m::gtk::Signal_proxy<void>	signal_needs_update(void);

		/// Обновляет статистику по текущим торрентам, если это необходимо.
		void						update(const std::vector<Torrent_info>& torrents);
};

#endif

