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


#ifndef HEADER_SETTINGS_WINDOW
#define HEADER_SETTINGS_WINDOW

	#include <string>

	#include <gtkmm/box.h>
	#include <gtkmm/checkbutton.h>
#if !MLIB_ENABLE_LIBS_FORWARDS
	#include <gtkmm/linkbutton.h>
#endif
	#include <gtkmm/filechooserbutton.h>
	#include <gtkmm/filechooserdialog.h>
	#include <gtkmm/notebook.h>
	#include <gtkmm/treestore.h>
	#include <gtkmm/window.h>

	#include <mlib/gtk/dialog.hpp>
	#include <mlib/gtk/tree_view.hpp>

	#include "common.hpp"



namespace Settings_window_aux { class Private; }

/// Окно изменения настроек клиента и демона.
class Settings_window: public m::gtk::Dialog
{
	private:
		typedef Settings_window_aux::Private Private;

		enum Section {
			CLIENT,
			CLIENT_MAIN,
			CLIENT_GUI,
			CLIENT_GUI_MISC,
			CLIENT_GUI_STATUS_BAR,
			DAEMON,
			DAEMON_NETWORK,
			DAEMON_NETWORK_MISC,
			DAEMON_NETWORK_IP_FILTER,
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
		~Settings_window(void);


	private:
		Private*						priv;

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
		// client <--


	private:
		/// Обновляет виджеты, отвечающие за автоматическую очистку от
		/// старых торрентов.
		void			auto_clean_widgets_update_for(const Auto_clean_type& type, Gtk::Button* type_button, Gtk::Label* type_label, Gtk::Widget* widget);

		/// Закрывает окно.
		void			close(void);

		/// Приводит виджеты окна в соответствие с настройками.
		void			load_settings(void);

		/// Обработчик сигнала на изменение действия, которое необходимо
		/// произвести, когда сработает выбранное условие "устаревания"
		/// торрента.
		void			on_auto_clean_max_ratio_clicked_cb(void);

		/// Обработчик сигнала на изменение действия, которое необходимо
		/// произвести, когда сработает выбранное условие "устаревания"
		/// торрента.
		void			on_auto_clean_max_seeding_time_clicked_cb(void);

		/// Обработчик сигнала на изменение действия, которое необходимо
		/// произвести, когда сработает выбранное условие "устаревания"
		/// торрента.
		void			on_auto_clean_max_seeding_torrents_clicked_cb(void);

		/// Обработчик сигнала на переключение флажка "автоматически загружать торренты".
		void			on_auto_load_torrents_toggled_callback(void);

		/// Обработчик сигнала на переключение флажка "копировать
		/// автоматически загружаемые торренты в...".
		void			on_auto_load_torrents_copy_to_toggled_callback(void);

		/// Обработчик сигнала на нажатие на кнопку "Cancel".
		void			on_cancel_button_callback(void);

		// Обработчик сигнала на смену раздела.
		void 			on_change_section_callback(const std::pair<Gtk::LinkButton*, Gtk::TreePath>& target);

		/// Обработчик сигнала на закрытие окна.
		bool			on_close_callback(GdkEventAny* event);

		/// Обработчик сигнала на переключение флажка "Copy on finished to path".
		void			on_copy_finished_to_path_toggled_callback(void);

		/// Обработчик сигнала на нажатие на кнопку "OK".
		void			on_ok_button_callback(void);

		// Обработчик сигнала на изменение выделенного раздела.
		void 			on_section_changed_callback(void);

		/// Обработчик сигнала на переключение флажка "Show tray icon".
		void			on_show_tray_icon_toggled_callback(void);

		/// Переносит свойства виджетов в настройки.
		void			save_settings(void);

		/// Обновляет GUI кнопки, определяющей метод избавления от старых
		/// торрентов.
		void			set_auto_clean_button_type(Gtk::Button& button, const Auto_clean_type& type);
};

#endif

