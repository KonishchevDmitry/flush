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


#ifndef HEADER_CLIENT_SETTINGS
	#define HEADER_CLIENT_SETTINGS

	#include <string>

	#include <mlib/gtk/paned_settings.hpp>
	#include <mlib/gtk/toolbar.hpp>
	#include <mlib/gtk/tree_view_settings.hpp>
	#include <mlib/gtk/window_settings.hpp>


	#define GUI_MIN_UPDATE_INTERVAL ( 1000 / 25 )



	class Tree_view_settings: public m::gtk::Tree_view_settings
	{
		public:
			void read_config(const libconfig::Setting& config_root);
			void write_config(libconfig::Setting& config_root) const;

		private:
			void read_column_config(const libconfig::Setting& config_root, m::gtk::Tree_view_column_settings& column);
			void write_column_config(libconfig::Setting& config_root, const m::gtk::Tree_view_column_settings& column) const;
	};



	typedef Tree_view_settings Torrents_view_settings;
	typedef Tree_view_settings Torrent_files_view_settings;
	typedef Tree_view_settings Torrent_peers_view_settings;



	class Paned_settings: public m::gtk::Paned_settings
	{
		public:
			void read_config(const libconfig::Setting& config_root);
			void write_config(libconfig::Setting& config_root) const;
	};



	class Window_settings: public m::gtk::Window_settings
	{
		public:
			void read_config(const libconfig::Setting& config_root);
			void write_config(libconfig::Setting& config_root) const;
	};



	class Torrents_viewport_settings
	{
		public:
			std::string						info_widget;

			Paned_settings					torrents_view_and_torrent_infos_vpaned;
			Torrents_view_settings			torrents_view;

			Torrent_files_view_settings		torrent_files_view;
			Torrent_files_view_settings		torrent_peers_view;


		public:
			void read_config(const libconfig::Setting& config_root);
			void write_config(libconfig::Setting& config_root) const;
	};



	class Status_bar_settings
	{
		public:
			Status_bar_settings(void);


		public:
			bool	download_speed;
			bool	payload_download_speed;

			bool	upload_speed;
			bool	payload_upload_speed;

			bool	download;
			bool	payload_download;

			bool	upload;
			bool	payload_upload;

			bool	share_ratio;
			bool	failed;
			bool	redundant;


		public:
			void read_config(const libconfig::Setting& config_root);
			void write_config(libconfig::Setting& config_root) const;
	};



	class Main_window_settings
	{
		public:
			Window_settings				window;
			Torrents_viewport_settings	torrents_viewport;
			Status_bar_settings			status_bar;


		public:
			void read_config(const libconfig::Setting& config_root);
			void write_config(libconfig::Setting& config_root) const;
	};



	/// Диалог добавления торрента.
	class Add_torrent_dialog_settings
	{
		public:
			Window_settings		window;
			Tree_view_settings	torrent_files_view;


		public:
			void read_config(const libconfig::Setting& config_root);
			void write_config(libconfig::Setting& config_root) const;
	};



	/// Диалог создания торрента.
	class Create_torrent_dialog_settings
	{
		public:
			Window_settings		window;

			/// Директория, в которой лежали файлы торрента, когда в последний
			/// раз создавался очередной торрент.
			std::string			get_from;

			/// Директория, в которую в последний раз сохранялся торрент.
			std::string			save_to;


		public:
			void read_config(const libconfig::Setting& config_root);
			void write_config(libconfig::Setting& config_root) const;
	};



	class Gui_settings
	{
		public:
			Gui_settings(void);


		public:
			/// Отображать панель инструментов.
			bool							show_toolbar;

			/// Стиль панели инструментов.
			m::gtk::toolbar::Style			toolbar_style;


			/// Отображать иконку в трее.
			bool							show_tray_icon;

			/// Интервал обновления GUI.
			int								update_interval;

			/// Максимальное количество строк в логе.
			int								max_log_lines;

			/// Настройки главного окна.
			Main_window_settings			main_window;

			/// Директория, в которой находился последний открытый торрент.
			std::string						open_torrents_from;

			/// Отображать или нет окно добавления торрента
			/// при открытии *.torrent файла.
			bool							show_add_torrent_dialog;

			/// Диалог добавления торрента.
			Add_torrent_dialog_settings		add_torrent_dialog;

			/// Диалог создания торрента.
			Create_torrent_dialog_settings	create_torrent_dialog;


		public:
			void read_config(const libconfig::Setting& config_root);
			void write_config(libconfig::Setting& config_root) const;
	};



	class User_settings
	{
		public:
			User_settings(void);


		public:
			/// Начинать скачивание торрента сразу после добавления.
			bool			start_torrent_on_adding;

			/// Директория по умолчанию, в которую будут скачиваться торренты.
			std::string		download_to;

			/// Директория, в которую будут копироваться файлы торрентов
			/// после завершения скачивания.
			/// Если равна "", то копирование производиться не будет.
			std::string		copy_finished_to;

			/// Команда для открытия файлов торрента.
			std::string		open_command;


		public:
			void read_config(const libconfig::Setting& config_root, Version client_version);
			void write_config(libconfig::Setting& config_root) const;
	};



	class Client_settings
	{
		public:
			Gui_settings gui;
			User_settings user;


		public:
			void read_config(const std::string& config_path) throw(m::Exception);
			void write_config(const std::string& config_path) const throw(m::Exception);
	};

#endif

