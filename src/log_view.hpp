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


#ifndef HEADER_LOG_VIEW
	#define HEADER_LOG_VIEW

	#include <deque>

	#include <gtkmm/textview.h>

	#include "common.hpp"



	/// Виджет для отображения лога.
	class Log_view: public Session_info_widget, public Gtk::TextView
	{
		public:
			Log_view(void);
		

		private:
			/// Буфер TextView.
			Glib::RefPtr<Gtk::TextBuffer>	buffer;

			/// Метка на последний символ в буфере.
			Glib::RefPtr<Gtk::TextMark>		end_mark;

			/// Максимальное количество строк,
			/// которое будет хранить лог.
			/// < 0 - неограниченное количество.
			int								max_lines;

			/// Пул неотображенных сообщений.
			std::deque<Daemon_message>		messages;


		public:
			/// Добавляет сообщение демона в лог.
			void		add_message(const Daemon_message& message);

			/// Устанавливает максимальное количество строк.
			/// < 0 - неограниченное количество.
			void		set_max_lines(const int& max_lines);

			/// Инициирует обновление виджета.
			void		update(const Torrent_id& torrent_id);

		private:
			/// Удаляет лишние строки из лога.
			void		remove_old_lines(void);

			/// Удаляет лишние сообщения из очереди неотображенных
			/// сообщений.
			void		remove_old_messages(void);
	};

#endif

