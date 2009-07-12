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

	#include <boost/scoped_ptr.hpp>

	#include <gtkmm/box.h>
	#include <gtkmm/notebook.h>
	#include <gtkmm/scrolledwindow.h>
	#include <gtkmm/togglebutton.h>

	#include <mlib/gtk/paned.hpp>
	#include <mlib/gtk/signal_proxy.hxx>

	#include "client_settings.hpp"
	#include "common.hpp"



	namespace Torrents_viewport_aux { class Private; }

	class Torrents_viewport: public Gtk::VBox
	{
		private:
			typedef Torrents_viewport_aux::Private Private;

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
			boost::scoped_ptr<Private>			priv;

			/// Содержит либо только список торрентов, либо список торрентов
			/// с информационными виджетами.
			Gtk::VBox							main_vbox;

			/// Содержит кнопки переключения информационных виджетов.
			Gtk::HBox							info_toggle_buttons_hbox;


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


		public:
			/// Возвращает виджет, отображающий лог.
			inline
			Log_view&					get_log_view(void) const
										{ return *this->log_view; }

			/// Выполняет требуемое действие над выбранными в данный момент
			/// торрентами.
			void						process_torrents(Torrent_process_action action);

			/// Отображает или скрывает список категорий торрентов.
			void						show_categories(bool show = true);

			/// Отображает или скрывает имена категорий.
			void						show_categories_names(bool show = true);

			/// Отображает или скрывает счетчики в категориях.
			void						show_categories_counters(bool show = true);

			/// Инициирует обновление виджета.
			void						update(void);

			/// Сохраняет настройки виджета.
			void						save_settings(Torrents_viewport_settings& settings) const;

			/// Сигнал на изменение списка действий, которые можно выполнить над
			/// торрентом(ами), выделенным(ми) в данный момент.
			m::gtk::Signal_proxy<void,
			Torrent_process_actions>	signal_torrent_process_actions_changed(void);

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

