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


#ifndef HEADER_SETTINGS_WINDOW
	#define HEADER_SETTINGS_WINDOW

	#include <string>

	#include <gtkmm/box.h>
	#include <gtkmm/button.h>
	#include <gtkmm/checkbutton.h>
	#include <gtkmm/entry.h>
	#include <gtkmm/filechooserbutton.h>
	#include <gtkmm/filechooserdialog.h>
	#include <gtkmm/notebook.h>
	#include <gtkmm/spinbutton.h>
	#include <gtkmm/treestore.h>
	#include <gtkmm/window.h>

	#include <mlib/gtk/dialog.hpp>
	#include <mlib/gtk/tree_view.hpp>



	/// Окно изменения настроек клиента и демона.
	class Settings_window: public m::gtk::Dialog
	{
		private:
			enum Section {
				CLIENT,
				CLIENT_MAIN,
				CLIENT_GUI,
				DAEMON,
				DAEMON_NETWORK,
				DAEMON_AUTOMATION
			};

			class Sections_view_model_columns: public m::gtk::Tree_view_model_columns
			{
				public:
					Sections_view_model_columns(void);


				public:
					Gtk::TreeModelColumn<int>				id;
					Gtk::TreeModelColumn<Glib::ustring>		name;
			};



			class Sections_view_columns: public m::gtk::Tree_view_columns
			{
				public:
					Sections_view_columns(const Sections_view_model_columns& model_columns);


				public:
					Gtk::TreeViewColumn			name;
			};



			class Sections_view: public m::gtk::Tree_view<Sections_view_columns, Sections_view_model_columns, Gtk::TreeStore>
			{
			};


		public:
			Settings_window(Gtk::Window& parent_window, Client_settings* client_settings, Daemon_settings* daemon_settings);


		private:
			Client_settings&				client_settings;
			Daemon_settings&				daemon_settings;

			Sections_view					sections_view;
			Gtk::Notebook					sections_notebook;

			// client -->
				// Main -->
					Gtk::CheckButton		show_add_torrent_dialog;
					Gtk::CheckButton		start_torrent_on_adding_check_button;

					Gtk::FileChooserDialog	download_to_dialog;
					Gtk::FileChooserButton	download_to_button;

					Gtk::CheckButton		copy_finished_to_check_button;
					Gtk::FileChooserDialog	copy_finished_to_dialog;
					Gtk::FileChooserButton	copy_finished_to_button;

					Gtk::Entry				open_command;
				// Main <--

				// GUI -->
					// Miscellaneous -->
						Gtk::CheckButton	show_speed_in_window_title;
						Gtk::CheckButton	show_tray_icon;

						Gtk::SpinButton		gui_update_interval;
						Gtk::SpinButton		max_log_lines;
					// Miscellaneous <--

					// Status bar -->
						Gtk::CheckButton	status_bar_download_speed;
						Gtk::CheckButton	status_bar_payload_download_speed;

						Gtk::CheckButton	status_bar_upload_speed;
						Gtk::CheckButton	status_bar_payload_upload_speed;

						Gtk::CheckButton	status_bar_download;
						Gtk::CheckButton	status_bar_payload_download;

						Gtk::CheckButton	status_bar_upload;
						Gtk::CheckButton	status_bar_payload_upload;

						Gtk::CheckButton	status_bar_share_ratio;
						Gtk::CheckButton	status_bar_failed;
						Gtk::CheckButton	status_bar_redundant;
					// Status bar <--
				// GUI <--
			// client <--

			// daemon -->
				// Network -->
					// Ports -->
						Gtk::SpinButton		listen_port_from;
						Gtk::SpinButton		listen_port_to;
						Gtk::Label			listen_port;
					// Ports <--

					// Extras -->
						Gtk::CheckButton	dht;
						Gtk::CheckButton	lsd;
						Gtk::CheckButton	upnp;
						Gtk::CheckButton	natpmp;
						Gtk::CheckButton	smart_ban;
						Gtk::CheckButton	pex;
					// Extras <--

					// Bandwidth -->
						Gtk::SpinButton		download_rate_limit;
						Gtk::SpinButton		upload_rate_limit;

						Gtk::SpinButton		max_uploads;
						Gtk::SpinButton		max_connections;
					// Bandwidth <--
				// Network <--

				// Automation -->
					Gtk::VBox				auto_delete_torrents_vbox;
					Gtk::CheckButton		auto_delete_torrents;
					Gtk::CheckButton		auto_delete_torrents_with_data;
					Gtk::SpinButton			auto_delete_torrents_max_seed_time;
					Gtk::SpinButton			auto_delete_torrents_max_seeds;
				// Automation <--
			// daemon <--


		private:
			/// Добавляет SpinButton.
			void			add_spin_button(Gtk::VBox& parent_vbox, const std::string& label_string, Gtk::SpinButton& spin_button, const std::pair<int, int>& range, const std::pair<int, int>& increments);

			/// Закрывает окно.
			void			close(void);

			/// Приводит виджеты окна в соответствие с настройками.
			void			load_settings(void);

			/// Обработчик сигнала на переключение флажка "автоматически удалять торренты".
			void			on_auto_delete_torrents_toggled_callback(void);

			/// Обработчик сигнала на нажатие на кнопку "Cancel".
			void			on_cancel_button_callback(void);

			// Обработчик сигнала на смену раздела.
			void 			on_change_section_callback(const std::pair<Gtk::LinkButton*, Gtk::TreePath>& target);

			/// Обработчик сигнала на закрытие окна.
			bool			on_close_callback(GdkEventAny* event);

			/// Обработчик сигнала на переключение флажка "Copy on finished to path".
			void			on_copy_finished_to_path_toggled_callback(void);

			/// Обработчик сигнала на изменение диапазона прослушиваемых портов.
			void			on_listen_port_range_change_callback(void);

			/// Обработчик сигнала на нажатие на кнопку "OK".
			void			on_ok_button_callback(void);

			// Обработчик сигнала на изменение выделенного раздела.
			void 			on_section_changed_callback(void);

			/// Переносит свойства виджетов в настройки.
			void			save_settings(void);
	};

#endif

