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


#include <stack>
#include <algorithm>

#include <gtkmm/liststore.h>

#include <mlib/gtk/misc.hpp>
#include <mlib/gtk/tree_view.hpp>

#include "client_settings.hpp"
#include "daemon_proxy.hpp"
#include "main.hpp"
#include "torrent_peers_view.hpp"



// Torrent_peers_view_model_columns -->
	Torrent_peers_view_model_columns::Torrent_peers_view_model_columns(void)
	{
		this->set_search_column(this->ip);

		add(this->ip);
		add(this->client);

		add(this->download_speed);
		add(this->download_speed_string);

		add(this->payload_download_speed);
		add(this->payload_download_speed_string);

		add(this->upload_speed);
		add(this->upload_speed_string);

		add(this->payload_upload_speed);
		add(this->payload_upload_speed_string);

		add(this->total_payload_download);
		add(this->total_payload_download_string);

		add(this->total_payload_upload);
		add(this->total_payload_upload_string);

		add(this->availability);
		add(this->availability_string);

		add(this->hash_fails);
		add(this->hash_fails_string);
	}
// Torrent_peers_view_model_columns <--



// Torrent_peers_view_columns -->
	Torrent_peers_view_columns::Torrent_peers_view_columns(const Torrent_peers_view_model_columns& model_columns)
	:
		ip(_("IP"), model_columns.ip),
		client(_("Client"), model_columns.client),
		download_speed(_Q("Download speed|Down speed"), model_columns.download_speed_string),
		payload_download_speed(_Q("Download speed (payload)|Down speed (data)"), model_columns.payload_download_speed_string),
		upload_speed(_Q("Upload speed|Up speed"), model_columns.upload_speed_string),
		payload_upload_speed(_Q("Upload speed (payload)|Up speed (data)"), model_columns.payload_upload_speed_string),
		total_payload_download(_Q("Total download (payload)|Down data"), model_columns.total_payload_download_string),
		total_payload_upload(_Q("Total upload (payload)|Up data"), model_columns.total_payload_upload_string),
		availability(_("Availability"), availability_renderer),
		hash_fails(_("Hash fails"), model_columns.hash_fails_string)
	{
		M_GTK_TREE_VIEW_ADD_STRING_COLUMN(ip, _("IP address"))
		M_GTK_TREE_VIEW_ADD_STRING_COLUMN(client, _("Client"))

		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(download_speed, _("Download speed"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(payload_download_speed, _("Download speed (payload)"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(upload_speed, _("Upload speed"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(payload_upload_speed, _("Upload speed (payload)"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(total_payload_download, _("Total download (payload)"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(total_payload_upload, _("Total upload (payload)"))

		this->add("availability", &this->availability, _("Availability"));
		this->availability.set_sort_column(model_columns.availability);
		this->availability.add_attribute(this->availability_renderer.property_value(), model_columns.availability);
		this->availability.add_attribute(this->availability_renderer.property_text(), model_columns.availability_string);

		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(hash_fails, _("Hash fails"))
	}
// Torrent_peers_view_columns <--



// Torrent_peers_view -->
	Torrent_peers_view::Torrent_peers_view(const Torrents_view_settings& settings)
	:
		m::gtk::Tree_view<Torrent_peers_view_columns, Torrent_peers_view_model_columns, Gtk::ListStore>(settings),
		last_show_zero_values_setting(false)
	{
	}



	void Torrent_peers_view::save_settings(Torrent_peers_view_settings& settings) const
	{
		m::gtk::Tree_view<Torrent_peers_view_columns, Torrent_peers_view_model_columns, Gtk::ListStore>::save_settings(settings);
	}



	void Torrent_peers_view::update(const Torrent_id& torrent_id)
	{
		if(torrent_id)
		{
			std::vector<Torrent_peer_info> peers;
			bool show_zero_values = get_client_settings().gui.show_zero_values;
			bool show_zero_setting_changed = show_zero_values != this->last_show_zero_values_setting;


			this->last_show_zero_values_setting = show_zero_values;

			// Получаем всю необходимую нам информацию -->
				try
				{
					get_daemon_proxy().get_torrent_peers_info(torrent_id, peers);
				}
				catch(m::Exception& e)
				{
					// Этого торрента уже вполне может не оказаться, т. к. между
					// обновлением списка торрентов и этим запросом торрент мог
					// быть удален, например, функцией автоматического удаления
					// старых торрентов.
					MLIB_D(_C("Updating torrent files list failed. %1", EE(e)));
					this->model->clear();
					return;
				}
			// Получаем всю необходимую нам информацию <--

			// Входим в режим редактирования
			this->editing_start();

			Gtk::TreeIter row_it = this->model->get_iter("0");
			std::vector<Torrent_peer_info>::const_iterator peer_it = peers.begin();
			std::vector<Torrent_peer_info>::const_iterator peer_end_it = peers.end();

			while(peer_it != peer_end_it)
			{
				bool force_update = false;
				bool zeros_force_update = show_zero_setting_changed;

				// Добавляем новые строки, если их меньше
				// чем нужно.
				if(!row_it)
				{
					row_it = this->model->append();

					force_update = true;
					zeros_force_update = true;
				}

				Gtk::TreeRow row = *row_it;
				const Torrent_peer_info& peer = *peer_it;


				m::gtk::update_row(row, this->model_columns.ip, peer.ip);
				m::gtk::update_row(row, this->model_columns.client, peer.client);


				if(m::gtk::update_row(row, this->model_columns.download_speed, peer.download_speed) || zeros_force_update)
					m::gtk::update_row(row, this->model_columns.download_speed_string, m::speed_to_string(peer.download_speed, show_zero_values));

				if(m::gtk::update_row(row, this->model_columns.payload_download_speed, peer.payload_download_speed) || zeros_force_update)
					m::gtk::update_row(row, this->model_columns.payload_download_speed_string, m::speed_to_string(peer.payload_download_speed, show_zero_values));

				if(m::gtk::update_row(row, this->model_columns.upload_speed, peer.upload_speed) || zeros_force_update)
					m::gtk::update_row(row, this->model_columns.upload_speed_string, m::speed_to_string(peer.upload_speed, show_zero_values));

				if(m::gtk::update_row(row, this->model_columns.payload_upload_speed, peer.payload_upload_speed) || zeros_force_update)
					m::gtk::update_row(row, this->model_columns.payload_upload_speed_string, m::speed_to_string(peer.payload_upload_speed, show_zero_values));


				if(m::gtk::update_row(row, this->model_columns.total_payload_download, peer.total_payload_download) || zeros_force_update)
					m::gtk::update_row(row, this->model_columns.total_payload_download_string, m::size_to_string(peer.total_payload_download, show_zero_values));

				if(m::gtk::update_row(row, this->model_columns.total_payload_upload, peer.total_payload_upload) || zeros_force_update)
					m::gtk::update_row(row, this->model_columns.total_payload_upload_string, m::size_to_string(peer.total_payload_upload, show_zero_values));


				if(m::gtk::update_row(row, this->model_columns.availability, peer.availability) || force_update)
					m::gtk::update_row(row, this->model_columns.availability_string, m::to_string(peer.availability) + " %");

				if(m::gtk::update_row(row, this->model_columns.hash_fails, peer.hash_fails) || zeros_force_update)
				{
					m::gtk::update_row(
						row, this->model_columns.hash_fails_string,
						show_zero_values || peer.hash_fails ? m::to_string(peer.hash_fails) : ""
					);
				}


				peer_it++;
				row_it++;
			}

			// Удаляем лишние строки -->
				while(row_it)
					row_it = this->model->erase(row_it);
			// Удаляем лишние строки <--
			
			// Выходим из режима редактирования
			this->editing_end();
		}
		else
			this->model->clear();
	}
// Torrent_peers_view <--

