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


#ifndef HEADER_TORRENT_PEERS_VIEW
	#define HEADER_TORRENT_PEERS_VIEW

	#include <gtkmm/cellrendererprogress.h>
	#include <gtkmm/liststore.h>
	#include <gtkmm/treemodelcolumn.h>
	#include <gtkmm/treeviewcolumn.h>

	#include <mlib/gtk/tree_view.hpp>

	#include "client_settings.hpp"



	class Torrent_peers_view_model_columns: public m::gtk::Tree_view_model_columns
	{
		public:
			Torrent_peers_view_model_columns(void);


		public:

			Gtk::TreeModelColumn<Glib::ustring>				uid;


			Gtk::TreeModelColumn<Glib::ustring>				ip;
			Gtk::TreeModelColumn<Glib::ustring>				client;


			Gtk::TreeModelColumn<Speed>						download_speed;
			Gtk::TreeModelColumn<Glib::ustring>				download_speed_string;

			Gtk::TreeModelColumn<Speed>						payload_download_speed;
			Gtk::TreeModelColumn<Glib::ustring>				payload_download_speed_string;

			Gtk::TreeModelColumn<Speed>						upload_speed;
			Gtk::TreeModelColumn<Glib::ustring>				upload_speed_string;

			Gtk::TreeModelColumn<Speed>						payload_upload_speed;
			Gtk::TreeModelColumn<Glib::ustring>				payload_upload_speed_string;


			Gtk::TreeModelColumn<Size>						total_payload_download;
			Gtk::TreeModelColumn<Glib::ustring>				total_payload_download_string;

			Gtk::TreeModelColumn<Size>						total_payload_upload;
			Gtk::TreeModelColumn<Glib::ustring>				total_payload_upload_string;


			Gtk::TreeModelColumn<int>						availability;
			Gtk::TreeModelColumn<Glib::ustring>				availability_string;

			Gtk::TreeModelColumn<int>						hash_fails;
			Gtk::TreeModelColumn<Glib::ustring>				hash_fails_string;
	};



	class Torrent_peers_view_columns: public m::gtk::Tree_view_columns
	{
		public:
			Torrent_peers_view_columns(const Torrent_peers_view_model_columns& model_columns);


		public:
			Gtk::CellRendererProgress	availability_renderer;

			Gtk::TreeViewColumn			ip;
			Gtk::TreeViewColumn			client;
			Gtk::TreeViewColumn			download_speed;
			Gtk::TreeViewColumn			payload_download_speed;
			Gtk::TreeViewColumn			upload_speed;
			Gtk::TreeViewColumn			payload_upload_speed;
			Gtk::TreeViewColumn			total_payload_download;
			Gtk::TreeViewColumn			total_payload_upload;
			Gtk::TreeViewColumn			availability;
			Gtk::TreeViewColumn			hash_fails;
	};



	class Torrent_peers_view
	:
		public Torrent_info_widget,
		public m::gtk::Tree_view<Torrent_peers_view_columns, Torrent_peers_view_model_columns, Gtk::ListStore>
	{
		public:
			Torrent_peers_view(const Torrent_peers_view_settings& settings);


		private:
			/// Значение, которое имела опция "show_zero_values" в прошлый раз.
			bool				last_show_zero_values_setting;

			/// Обновляет строку TreeView.
			void				update_row(Gtk::TreeRow &row, const std::string& uid, const Torrent_peer_info& peer, bool force_update, bool zeros_force_update, bool show_zero_values);

		public:
			/// Сохраняет настройки виджета.
			void				save_settings(Torrent_peers_view_settings& settings) const;

			/// Инициирует обновление виджета.
			virtual void		update(const Torrent_id& torrent_id);
	};
#endif

