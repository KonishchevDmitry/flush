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


#ifndef HEADER_TORRENTS_VIEW
	#define HEADER_TORRENTS_VIEW

	#include <deque>
	#include <map>
	#include <vector>

	#include <gtkmm/actiongroup.h>
	#include <gtkmm/cellrendererprogress.h>
	#include <gtkmm/liststore.h>
	#include <gtkmm/treemodel.h>
	#include <gtkmm/treeview.h>
	#include <gtkmm/uimanager.h>

	#include <mlib/gtk/tree_view.hpp>

	#include "client_settings.hpp"



	class Torrents_view_model_columns: public m::gtk::Tree_view_model_columns
	{
		public:
			Torrents_view_model_columns(void);


		public:
			Gtk::TreeModelColumn<Glib::ustring>		id;
			Gtk::TreeModelColumn<Glib::ustring>		name;
			Gtk::TreeModelColumn<bool>		   		paused;
			Gtk::TreeModelColumn<Glib::ustring>		status;
			Gtk::TreeModelColumn<int>		   		progress;


			Gtk::TreeModelColumn<Size>		   		size;
			Gtk::TreeModelColumn<Glib::ustring>		size_string;

			Gtk::TreeModelColumn<Size>		   		requested_size;
			Gtk::TreeModelColumn<Glib::ustring>		requested_size_string;

			Gtk::TreeModelColumn<Size>		   		downloaded_requested_size;
			Gtk::TreeModelColumn<Glib::ustring>		downloaded_requested_size_string;

			Gtk::TreeModelColumn<int>		   		complete_percent;
			Gtk::TreeModelColumn<Glib::ustring>		complete_percent_string;


			Gtk::TreeModelColumn<Size>				total_download;
			Gtk::TreeModelColumn<Glib::ustring>		total_download_string;

			Gtk::TreeModelColumn<Size>				total_payload_download;
			Gtk::TreeModelColumn<Glib::ustring>		total_payload_download_string;

			Gtk::TreeModelColumn<Size>				total_upload;
			Gtk::TreeModelColumn<Glib::ustring>		total_upload_string;

			Gtk::TreeModelColumn<Size>				total_payload_upload;
			Gtk::TreeModelColumn<Glib::ustring>		total_payload_upload_string;

			Gtk::TreeModelColumn<Size>				total_failed;
			Gtk::TreeModelColumn<Glib::ustring>		total_failed_string;

			Gtk::TreeModelColumn<Size>				total_redundant;
			Gtk::TreeModelColumn<Glib::ustring>		total_redundant_string;


			Gtk::TreeModelColumn<Speed>				download_speed;
			Gtk::TreeModelColumn<Glib::ustring>		download_speed_string;

			Gtk::TreeModelColumn<Speed>				payload_download_speed;
			Gtk::TreeModelColumn<Glib::ustring>		payload_download_speed_string;

			Gtk::TreeModelColumn<Speed>				upload_speed;
			Gtk::TreeModelColumn<Glib::ustring>		upload_speed_string;

			Gtk::TreeModelColumn<Speed>				payload_upload_speed;
			Gtk::TreeModelColumn<Glib::ustring>		payload_upload_speed_string;


			Gtk::TreeModelColumn<Share_ratio>		share_ratio;
			Gtk::TreeModelColumn<Glib::ustring>		share_ratio_string;

			Gtk::TreeModelColumn<int>				peers_num;
			Gtk::TreeModelColumn<int>				seeds_num;


			Gtk::TreeModelColumn<Time>				time_added;
			Gtk::TreeModelColumn<Glib::ustring>		time_added_string;

			Gtk::TreeModelColumn<Time>				time_left;
			Gtk::TreeModelColumn<Glib::ustring>		time_left_string;

			Gtk::TreeModelColumn<Time>				time_seeding;
			Gtk::TreeModelColumn<Glib::ustring>		time_seeding_string;
	};



	class Torrents_view_columns: public m::gtk::Tree_view_columns
	{
		public:
			Torrents_view_columns(const Torrents_view_model_columns& model_columns);


		public:
			Gtk::TreeViewColumn			name;

			Gtk::CellRendererProgress	status_renderer;
			Gtk::TreeViewColumn			status;

			Gtk::TreeViewColumn			size;
			Gtk::TreeViewColumn			requested_size;
			Gtk::TreeViewColumn			downloaded_requested_size;
			Gtk::TreeViewColumn			complete_percent;

			Gtk::TreeViewColumn			total_download;
			Gtk::TreeViewColumn			total_payload_download;
			Gtk::TreeViewColumn			total_upload;
			Gtk::TreeViewColumn			total_payload_upload;
			Gtk::TreeViewColumn			total_failed;
			Gtk::TreeViewColumn			total_redundant;

			Gtk::TreeViewColumn			download_speed;
			Gtk::TreeViewColumn			payload_download_speed;
			Gtk::TreeViewColumn			upload_speed;
			Gtk::TreeViewColumn			payload_upload_speed;

			Gtk::TreeViewColumn			share_ratio;

			Gtk::TreeViewColumn			peers_num;
			Gtk::TreeViewColumn			seeds_num;

			Gtk::TreeViewColumn			time_added;
			Gtk::TreeViewColumn			time_left;
			Gtk::TreeViewColumn			time_seeding;
	};



	class Torrents_view: public m::gtk::Tree_view<Torrents_view_columns, Torrents_view_model_columns, Gtk::ListStore>
	{
		public:
			Torrents_view(const Torrents_view_settings& settings);


		public:
			sigc::signal<void, Torrent_id>	torrent_selected_signal;

		private:
			Glib::RefPtr<Gtk::Action>		pause_action;
			Glib::RefPtr<Gtk::Action>		resume_action;
			Glib::RefPtr<Gtk::UIManager>	ui_manager;


		public:
			/// Сохраняет настройки виджета.
			void			save_settings(Torrents_view_settings& settings) const;

			/// Иницииурет обновление виджета.
			void			update(std::vector<Torrent_info>::iterator infos_it, const std::vector<Torrent_info>::iterator& infos_end_it);

		private:
			/// Обработчик сигнала на нажатие правой кнопки мыши.
			virtual void	on_mouse_right_button_click(const GdkEventButton* const event);

			/// Обработчик сигнала на активацию строки TreeView.
			void			on_row_activated_callback(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);

			/// Обработчик сигнала на изменение списка выделенных торрентов.
			void			on_selection_changed_callback(void);

			/// Обработчик сигнала на выбор пользователем действия, которое
			/// он хочет совершить над торрентом(ами) (остановить, запустить, удалить и т. п.).
			void			torrents_process_callback(Torrent_process_action action);

			/// Обновляет строку TreeView.
			void			update_row(Gtk::TreeModel::iterator &iter, const Torrent_info& torrent_info, bool is_new_row);
	};
#endif
