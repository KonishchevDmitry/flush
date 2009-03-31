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


#ifndef HEADER_TORRENTS_VIEWPORT
	#define HEADER_TORRENTS_VIEWPORT

	#include <gtkmm/box.h>
	#include <gtkmm/frame.h>
	#include <gtkmm/notebook.h>
	#include <gtkmm/scrolledwindow.h>
	#include <gtkmm/togglebutton.h>

	#include <mlib/gtk/paned.hpp>

	#include "client_settings.hpp"



	class Torrents_viewport: public Gtk::VBox
	{
		private:
			/// Контейнер для хранения InfoWidget.
			struct Info_widget_handle
			{
				Info_widget_handle(const std::string& name, Gtk::ToggleButton& button, Info_widget& widget);

				std::string			name;
				Gtk::ToggleButton*	button;
				Info_widget*		widget;
			};


		public:
			Torrents_viewport(const Torrents_viewport_settings& settings);


		private:
			/// Содержит либо только список торрентов, либо список торрентов
			/// с информационными виджетами.
			Gtk::VBox							main_vbox;

			/// Содержит кнопки переключения информационных виджетов.
			Gtk::HBox							info_toggle_buttons_hbox;


			Gtk::Frame							torrents_view_frame;
			Gtk::ScrolledWindow					torrents_view_scrolled_window;
			Torrents_view*						torrents_view;


			m::gtk::VPaned						torrents_view_and_torrent_infos_vpaned;
			Gtk::Notebook						torrent_infos_notebook;


			Gtk::ToggleButton					details_toggle_button;
			Gtk::ScrolledWindow					torrent_details_view_scrolled_window;
			Torrent_details_view*				torrent_details_view;

			Gtk::ToggleButton					files_list_toggle_button;
			Gtk::ScrolledWindow					torrent_files_view_scrolled_window;
			Torrent_files_dynamic_view*			torrent_files_view;

			Gtk::ToggleButton					peers_list_toggle_button;
			Gtk::ScrolledWindow					torrent_peers_view_scrolled_window;
			Torrent_peers_view*					torrent_peers_view;

			Gtk::ToggleButton					options_toggle_button;
			Gtk::ScrolledWindow					torrent_options_view_scrolled_window;
			Torrent_options_view*				torrent_options_view;

			Gtk::ToggleButton					log_toggle_button;
			Gtk::ScrolledWindow					log_view_scrolled_window;
			Log_view*							log_view;


			/// Определяет, в каком режиме мы находимся в данный момент:
			/// в режиме отображения информационных виджетов или в обычном
			/// режиме, в котором они не отображаются.
			bool								info_mode;

			/// Определяет, происходит ли в данный момент обработка
			/// сигнала на перключение кнопки. Если да - то обработка
			/// остальных сигналов на пререключение не происходит.
			bool								toggle_in_process;

			/// Информационные виджеты.
			std::vector<Info_widget_handle>		info_widgets;

			/// Указатель на текущий информационный виджет.
			Info_widget*						current_info_widget;

			/// Выделенный в данный момент торрент.
			Torrent_id							cur_torrent_id;


		public:
			/// Возвращает виджет, отображающий лог.
			inline
			Log_view&		get_log_view(void) const
							{ return *this->log_view; }

			/// Обработчик сигнала на изменение списка выделенных торрентов.
			void			on_torrent_selected_callback(const Torrent_id& torrent_id);

			/// Инициирует обновление виджета.
			void			update(std::vector<Torrent_info>::iterator infos_it, const std::vector<Torrent_info>::iterator& infos_end_it);

			/// Сохраняет настройки виджета.
			void			save_settings(Torrents_viewport_settings& settings) const;

		private:
			/// Добавляет виджет в список информационных виджетов. Данный виджет будет отображаться
			/// при нажатии на кнопку button.
			void			add_info_widget(const std::string& name, Gtk::Widget& widget_container, Info_widget& widget, Gtk::ToggleButton& button);

			/// Обработчик сигнала нажатия на кнопку переключения информационного виджета.
			void			on_toggle_info_callback(Gtk::ToggleButton* button);

			/// Включает или выключает режим отображения информационных виджетов.
			void			set_info_mode(bool info_mode = true);
	};

#endif

