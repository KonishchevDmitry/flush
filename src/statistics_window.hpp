/**************************************************************************
*                                                                         *
*   Flush - GTK-based BitTorrent client                                   *
*   http://sourceforge.net/projects/flush                                 *
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


#ifndef HEADER_STATISTICS_WINDOW
#define HEADER_STATISTICS_WINDOW

#include <gtkmm/table.h>

#include <mlib/gtk/dialog.hpp>

#include "common.hpp"


/// Окно для отображения статистической информации о текущей и прошлых
/// сессиях.
class Statistics_window: public m::gtk::Dialog
{
	public:
		Statistics_window(Gtk::Window& parent_window);


	private:
		int					rows_num;
		const int			columns_num;
		Gtk::Table			table;


	public:
		// Отображает окно со статистической информацией.
		void	run(void);

	private:
		/// Добавляет одну строку в таблицу.
		void	add_row(void);

		/// Добавляет разделительную линию.
		void	attach_separator(void);

		/// Добавляет еще одно share ratio.
		void	attach_share_ratio(const std::string& name, Share_ratio ratio);

		/// Добавляет еще один размер.
		void	attach_size(const std::string& name, Size size);

		/// Добавляет еще одно время.
		void	attach_time(const std::string& name, Time time);

		/// Добавляет еще одно значение.
		void	attach_value(const std::string& name, const std::string& value);

		/// Обработчик сигнала на нажатие кнопки "Reset".
		void	on_reset_callback(void);
};

#endif

