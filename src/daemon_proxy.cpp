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


#include <algorithm>
#include <deque>
#include <functional>
#include <string>
#include <vector>

#include "mlib/libtorrent.hpp"

#include "daemon.hpp"
#include "daemon_proxy.hpp"
#include "daemon_types.hpp"



Daemon_proxy::Daemon_proxy(const std::string& daemon_config_path)
:
	daemon(new Daemon(daemon_config_path))
{
	/// Обработчик сигнала на приход сообщения от демона (libtorrent).
	this->daemon->messages_signal.connect(sigc::mem_fun(*this, &Daemon_proxy::on_messages_callback));
}



Daemon_proxy::~Daemon_proxy(void)
{
	delete this->daemon;
}



void Daemon_proxy::add_torrent(const std::string& torrent_path, const New_torrent_settings& torrent_settings) throw(m::Exception)
{
	this->daemon->add_torrent(torrent_path, torrent_settings);
}



Speed Daemon_proxy::get_rate_limit(Traffic_type traffic_type) const throw(m::Exception)
{
	return this->daemon->get_rate_limit(traffic_type);
}



Session_status Daemon_proxy::get_session_status(void) const throw(m::Exception)
{
	return this->daemon->get_session_status();
}



Daemon_settings Daemon_proxy::get_settings(void) const throw(m::Exception)
{
	return this->daemon->get_settings();
}



Torrent_details Daemon_proxy::get_torrent_details(const Torrent_id& torrent_id) const throw(m::Exception)
{
	return this->daemon->get_torrent_details(torrent_id);
}



std::string Daemon_proxy::get_torrent_download_path(const Torrent_id& torrent_id) const throw(m::Exception)
{
	return this->daemon->get_torrent_download_path(torrent_id);
}



Revision Daemon_proxy::get_torrent_files_info(const Torrent_id& torrent_id, std::vector<Torrent_file> *files, std::vector<Torrent_file_status>* statuses, Revision revision) const throw(m::Exception)
{
	Revision new_revision = this->daemon->get_torrent_files_info(torrent_id, files, statuses, revision);

	// Проверяем полученные данные
	if(new_revision != revision)
	{
		for(size_t i = 0; i < files->size(); i++)
		{
			std::string& path = (*files)[i].path;

			// m::Exception
			path = m::lt::get_torrent_file_path(path);
		}
	}

	return new_revision;
}



void Daemon_proxy::get_torrent_peers_info(const Torrent_id& torrent_id, std::vector<Torrent_peer_info>& peers_info) const throw(m::Exception)
{
	this->daemon->get_torrent_peers_info(torrent_id, peers_info);
}



bool Daemon_proxy::get_torrent_new_download_settings(const Torrent_id& torrent_id, Revision* revision, Download_settings* download_settings) const throw(m::Exception)
{
	return this->daemon->get_torrent_new_download_settings(torrent_id, revision, download_settings);
}



bool Daemon_proxy::get_torrent_new_trackers(const Torrent_id& torrent_id, Revision* revision, std::vector<std::string>* trackers) const throw(m::Exception)
{
	return this->daemon->get_torrent_new_trackers(torrent_id, revision, trackers);
}



void Daemon_proxy::get_torrents(std::vector<Torrent_info>& torrents_info) throw(m::Exception)
{
	this->daemon->get_torrents_info(torrents_info);
}



void Daemon_proxy::on_messages_callback(void)
{
	std::deque<Daemon_message> messages;
	this->daemon->get_messages(messages);
	this->daemon_messages_signal(messages);
}



void Daemon_proxy::process_torrents(const std::vector<Torrent_id>& torrents_ids, Torrent_process_action action) throw(m::Exception)
{
	this->daemon->process_torrents(torrents_ids, action);
}



void Daemon_proxy::reset_statistics(void) throw(m::Exception)
{
	this->daemon->reset_statistics();
}



void Daemon_proxy::set_copy_when_finished(const Torrent_id& torrent_id, bool copy, const std::string& to) throw(m::Exception)
{
	this->daemon->set_copy_when_finished(torrent_id, copy, to);
}



void Daemon_proxy::set_files_download_status(const Torrent_id& torrent_id, const std::vector<int>& files_ids, bool download) throw(m::Exception)
{
	this->daemon->set_files_download_status(torrent_id, files_ids, download);
}



#if 0
void Daemon_proxy::set_files_new_paths(const Torrent_id& torrent_id, const std::vector<Torrent_file>& files_new_paths) throw(m::Exception)
{
	this->daemon->set_files_new_paths(torrent_id, files_new_paths);
}
#endif



void Daemon_proxy::set_files_priority(const Torrent_id& torrent_id, const std::vector<int>& files_ids, const Torrent_file_settings::Priority priority) throw(m::Exception)
{
	this->daemon->set_files_priority(torrent_id, files_ids, priority);
}



void Daemon_proxy::set_rate_limit(Traffic_type traffic_type, Speed speed) throw(m::Exception)
{
	this->daemon->set_rate_limit(traffic_type, speed);
}



void Daemon_proxy::set_settings(const Daemon_settings& settings) throw(m::Exception)
{
	this->daemon->set_settings(settings);
}



void Daemon_proxy::set_sequential_download(const Torrent_id& torrent_id, bool value) throw(m::Exception)
{
	this->daemon->set_sequential_download(torrent_id, value);
}



void Daemon_proxy::set_torrent_trackers(const Torrent_id& torrent_id, const std::vector<std::string>& trackers) throw(m::Exception)
{
	this->daemon->set_torrent_trackers(torrent_id, trackers);
}



void Daemon_proxy::start(void) throw(m::Exception)
{
	this->daemon->start();
}



void Daemon_proxy::start_torrents(const Torrents_group group) throw(m::Exception)
{
	this->daemon->start_torrents(group);
}



void Daemon_proxy::stop_torrents(const Torrents_group group) throw(m::Exception)
{
	this->daemon->stop_torrents(group);
}

