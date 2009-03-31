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


#ifndef HEADER_MAIN_WINDOW
	#define HEADER_MAIN_WINDOW

	#include <queue>

	#include <gtkmm/statusbar.h>
	#include <gtkmm/statusicon.h>
	#include <gtkmm/uimanager.h>
	#include <gtkmm/window.h>

	#include <mlib/gtk/types.hpp>
	#include <mlib/gtk/window.hpp>

	#include "client_settings.hpp"



	class Main_window: public m::gtk::Window
	{
		public:
			Main_window(const Main_window_settings& settings);


		private:
			/// Определяет, было ли хотя бы один раз отображено окно, или за
			/// все время работы программы пользоватьель его так и не видел.
			bool							has_been_showed;

			/// Status bar
			Gtk::Statusbar					status_bar;

			/// Иконка в трее.
			Glib::RefPtr<Gtk::StatusIcon>	tray;

			Torrents_viewport*				torrents_viewport;

			Glib::RefPtr<Gtk::UIManager>	ui_manager;

			/// Окно редактирования настроек.
			Settings_window*				settings_window;

			/// Текущая "привязка" сигнала на обновление GUI.
			sigc::connection				gui_update_timeout_connection;


		public:
			/// Добавляет сообщение демона в виджет, занимащийся отображанием
			/// этих сообщений.
			void	add_daemon_message(const Daemon_message& message) const;

			/// Производит все необходимые в зависимости от настроек действия
			/// по открытию торрента.
			void	open_torrent(const std::string& torrent_path);

			/// Cохраняет текущие настройки клиента.
			void	save_settings(void);

			/// Инициирует обновление GUI.
			void	update_gui(void);

		private:
			/// При выборе пункта меню "Изменить максимальную скорость скачивания/отдачи".
			void	on_change_rate_limit_callback(Traffic_type traffic_type);

			/// Обработчик сигнала на закрытие окна.
			bool	on_close_callback(GdkEventAny* event);

			/// Обработчик сигнала на создание торрент файла.
			void	on_create_callback(void);

			/// Обработчик сигнала на обновление GUI.
			bool	on_gui_update_timeout(void);

			/// Обработчик сигнала на открытие торрент файла.
			void	on_open_callback(void);

			/// Обработчик сигнала на закрытие окна для выбора торрент файла.
			void	on_open_response_callback(int response_id, Gtk::FileChooserDialog* dialog);

			/// При выборе пункта меню "Приостановить все торренты".
			void	on_pause_torrents_callback(Torrents_group group);

			/// Обработчик на нажатие на одну из кнопок (элементов меню),
			/// закрывающих приложение.
			void	on_quit_callback(void);

			/// При выборе пункта меню "Возобновить все торренты".
			void	on_resume_torrents_callback(Torrents_group group);

			/// Обработчик сигнала на автоматическое сохранение настроек.
			bool	on_save_settings_timeout(void);

			/// Обработчик сигнала на отображение About диалога.
			void	on_show_about_dialog_callback(void);

			/// Обработчик сигнала на открытие окна настроек.
			void	on_show_settings_window_callback(void);

			/// Обработчик сигнала на отображение статистики.
			void	on_show_statistics_callback(void);

			/// Обработчик нажатия левой кнопки мыши по значку в трее.
			void	on_tray_activated(void);

			/// Обработчик нажатия правой кнопки мыши по значку в трее.
			void	on_tray_popup_menu(int button, int activate_time);

			/// Задает интервал обновления GUI.
			void	set_gui_update_interval(int interval);

			/// Отображает окно.
			void	show(void);

			/// Отображает окно и все дочерние виджеты.
			void	show_all(void);

			/// Отображает или скрывает иконку из трея.
			void	show_tray_icon(bool show);
	};

#endif

