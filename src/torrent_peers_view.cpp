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

#include <mlib/main.hpp>
#if M_BOOST_GET_VERSION() >= M_GET_VERSION(1, 36, 0)
	#include <boost/unordered_map.hpp>
#else
	#include <map>
#endif

#include <gtkmm/liststore.h>

#include <mlib/gtk/misc.hpp>
#include <mlib/gtk/tree_view.hpp>
#include <mlib/string.hpp>

#include "client_settings.hpp"
#include "common.hpp"
#include "daemon_proxy.hpp"
#include "main.hpp"
#include "torrent_peers_view.hpp"



// Torrent_peers_view_model_columns -->
	Torrent_peers_view_model_columns::Torrent_peers_view_model_columns(void)
	{
		this->set_search_column(this->ip);

		add(this->uid);

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
		#if M_BOOST_GET_VERSION() >= M_GET_VERSION(1, 36, 0)
			boost::unordered_map<std::string, Torrent_peer_info> peers;
		#else
			std::map<std::string, Torrent_peer_info> peers;
		#endif
			bool show_zero_values = get_client_settings().gui.show_zero_values;
			bool show_zero_setting_changed = show_zero_values != this->last_show_zero_values_setting;


			this->last_show_zero_values_setting = show_zero_values;

			// Создаем индекс по информации о пирах -->
			{
				std::vector<Torrent_peer_info> peers_list;

				// Получаем всю необходимую нам информацию -->
					try
					{
						get_daemon_proxy().get_torrent_peers_info(torrent_id, peers_list);
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

				M_FOR_CONST_IT(peers_list, it)
				{
					size_t id = 0;
					std::string uid;
					M_ITER_TYPE(peers) peers_it;

					// Получаем уникальный идентификатор клиента -->
						do
						{
							uid = it->ip + "_" + m::to_string(id);
							peers_it = peers.find(uid);
						}
						while(peers_it != peers.end());
					// Получаем уникальный идентификатор клиента <--

					peers.insert(std::pair<std::string, Torrent_peer_info>(uid, *it));
				}
			}
			// Создаем индекс по информации о пирах <--

			// Удаляем те строки, которые уже не актуальны -->
			{
				Gtk::TreeModel::iterator it = this->model->children().begin();

				while(it)
				{
					M_ITER_TYPE(peers) peer_it = peers.find(
						it->get_value(this->model_columns.uid)
					);

					if(peer_it == peers.end())
						it = this->model->erase(it);
					else
						++it;
				}
			}
			// Удаляем те строки, которые уже не актуальны <--

			// Обновляем существующие строки -->
			{
				std::vector<Gtk::TreeRow> rows;

				// Получаем все строки модели.
				// Оперировать итераторами мы не можем, т. к. из-за сортировки
				// после каждого изменения какой-либо строки модель сортируется, и
				// итераторы начинают указывать не на те элементы (к примеру,
				// следующий итератор может указывать на ту строку, которая ранее
				// уже была обработана).
				// -->
				{
					Gtk::TreeModel::Children iters = this->model->children();

					rows.reserve(iters.size());
					M_FOR_CONST_IT(iters, it)
						rows.push_back(*it);
				}
				// <--

				M_FOR_IT(rows, it)
				{
					M_ITER_TYPE(peers) peer_it = peers.find(
						it->get_value(this->model_columns.uid)
					);

					// Обновляем старые значения
					this->update_row(*it, peer_it->first, peer_it->second, false, show_zero_setting_changed, show_zero_values);

					// Удаляем из индекса обработанную информацию
					peers.erase(peer_it);
				}
			}
			// Обновляем существующие строки <--

			// Теперь в индексе остались только пиры, которые еще не присутствуют в списке.
			// Добавляем их.
			M_FOR_CONST_IT(peers, it)
			{
				Gtk::TreeRow row = *this->model->append();
				this->update_row(row, it->first, it->second, true, true, show_zero_values);
			}
		}
		else
			this->model->clear();
	}



	void Torrent_peers_view::update_row(Gtk::TreeRow &row, const std::string& uid, const Torrent_peer_info& peer, bool force_update, bool zeros_force_update, bool show_zero_values)
	{
		m::gtk::update_row(row, this->model_columns.uid, uid);
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
			m::gtk::update_row(row, this->model_columns.availability_string, m::to_string(peer.availability) + "%");

		if(m::gtk::update_row(row, this->model_columns.hash_fails, peer.hash_fails) || zeros_force_update)
		{
			m::gtk::update_row(
				row, this->model_columns.hash_fails_string,
				show_zero_values || peer.hash_fails ? m::to_string(peer.hash_fails) : ""
			);
		}
	}
// Torrent_peers_view <--

