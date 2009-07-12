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


#ifndef HEADER_TORRENT_OPTIONS_VIEW
	#define HEADER_TORRENT_OPTIONS_VIEW

	#include <gtkmm/box.h>

	#include "common.hpp"


	class Torrent_options_view
	:
		public Torrent_info_widget,
		public Gtk::VBox
	{
		private:
			class Gui;


		public:
			Torrent_options_view(void);
			~Torrent_options_view(void);


		private:
			/// Отображаемый в данный момент торрент.
			Torrent_id			torrent_id;

			/// Ревизия настроек скачивания отображаемого в данный момент торрента.
			Revision			download_settings_revision;

			/// Ревизия трекеров отображаемого в данный момент торрента.
			Revision			trackers_revision;

			/// Определяет, производится ли в данный момент обновление GUI.
			bool				gui_updating;

			Gui*				gui;


		public:
			/// Инициирует обновление виджета.
			void		update(const Torrent_id& torrent_id);

		private:
			/// Сбрасывает отображаемую виджетами информацию в значения по
			/// умолчанию.
			void		clear(void);

			/// Обновляет виджет, если он отображает устаревшую информацию
			/// (судя по номеру ревизии).
			///
			/// @param force - если true, то обновляет виджет даже в том
			/// случае, если, судя по номеру ревизии, он отображает актуальную
			/// информацию.
			void		custom_update(bool force = false);

			/// Обработчик сигнала на переключение флажка "Копировать файлы
			/// торрента по завершении скачивания".
			void		on_copy_when_finished_toggled_callback(void);

			/// Обработчик сигнала на нажатие на кнопку "Выбрать директорию для
			/// сохранения файлов торрента по завершении их скачивания".
			void		on_select_copy_when_finished_to_callback(void);

			/// Обработчик сигнала на переключение флажка "Последовательное
			/// скачивание".
			void		on_sequential_download_toggled_callback(void);

			/// Обработчик сигнала на изменение пользователем списка трекеров.
			void		on_trackers_changed_callback(void);
	};
#endif

