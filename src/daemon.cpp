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


#include <string>
#include <vector>

#include "common.hpp"
#include "daemon.hpp"



Daemon::Daemon(const std::string& config_dir_path)
:
	Daemon_session(config_dir_path)
{
}



Torrent_details Daemon::get_torrent_details(const Torrent_id& torrent_id) const
{
	return Daemon_session::get_torrent_details(this->get_torrent(torrent_id));
}



std::string Daemon::get_torrent_download_path(const Torrent_id& torrent_id)
{
	return this->get_torrent(torrent_id).get_download_path();
}



Revision Daemon::get_torrent_files_info(const Torrent_id& torrent_id, std::vector<Torrent_file> *files, std::vector<Torrent_file_status>* statuses, Revision revision) const
{
	return Daemon_session::get_torrent_files_info(this->get_torrent(torrent_id), files, statuses, revision);
}



void Daemon::get_torrent_peers_info(const Torrent_id& torrent_id, std::vector<Torrent_peer_info>& peers_info) const
{
	Daemon_session::get_torrent_peers_info(this->get_torrent(torrent_id), peers_info);
}



bool Daemon::get_torrent_new_download_settings(const Torrent_id& torrent_id, Revision* revision, Download_settings* download_settings) const
{
	return Daemon_session::get_torrent_new_download_settings(this->get_torrent(torrent_id), revision, download_settings);
}



bool Daemon::get_torrent_new_trackers(const Torrent_id& torrent_id, Revision* revision, std::vector<std::string>* trackers) const
{
	return Daemon_session::get_torrent_new_trackers(this->get_torrent(torrent_id), revision, trackers);
}



void Daemon::interrupt_temporary_action(bool complete)
{
	Daemon_session::interrupt_temporary_action(complete);
}



void Daemon::pause_torrent(const Torrent_id& torrent_id)
{
	Daemon_session::pause_torrent(this->get_torrent(torrent_id));
}



void Daemon::process_torrents(const std::vector<Torrent_id>& torrents_ids, Torrent_process_action action)
{
	M_FOR_CONST_IT(torrents_ids, it)
	{
		switch(action)
		{
			case PAUSE:
				this->pause_torrent(*it);
				break;

			case RESUME:
				this->resume_torrent(*it);
				break;

			case RECHECK:
				this->recheck_torrent(*it);
				break;

			case REMOVE:
				this->remove_torrent(*it);
				break;

			case REMOVE_WITH_DATA:
				this->remove_torrent_with_data(*it);
				break;

			default:
				MLIB_LE();
				break;
		}
	}
}



void Daemon::process_torrents_temporary(Temporary_action action, Torrents_group group, Time time)
{
	Daemon_session::process_torrents_temporary(action, group, time);
}



void Daemon::resume_torrent(const Torrent_id& torrent_id)
{
	Daemon_session::resume_torrent(this->get_torrent(torrent_id));
}



void Daemon::set_copy_when_finished(const Torrent_id& torrent_id, bool copy, const std::string& to)
{
	Daemon_session::set_copy_when_finished(this->get_torrent(torrent_id), copy, to);
}



void Daemon::set_files_download_status(const Torrent_id& torrent_id, const std::vector<int>& files_ids, bool download)
{
	Daemon_session::set_files_download_status(this->get_torrent(torrent_id), files_ids, download);
}



#if 0
void Daemon::set_files_new_paths(const Torrent_id& torrent_id, const std::vector<Torrent_file>& files_new_paths)
{
	Daemon_session::set_files_new_paths(this->get_torrent(torrent_id), files_new_paths);
}
#endif



void Daemon::set_files_priority(const Torrent_id& torrent_id, const std::vector<int>& files_ids, const Torrent_file_settings::Priority priority)
{
	Daemon_session::set_files_priority(this->get_torrent(torrent_id), files_ids, priority);
}



void Daemon::set_sequential_download(const Torrent_id& torrent_id, bool value)
{
	Daemon_session::set_sequential_download(this->get_torrent(torrent_id), value);
}



void Daemon::set_torrent_trackers(const Torrent_id& torrent_id, const std::vector<std::string>& trackers)
{
	Daemon_session::set_torrent_trackers(this->get_torrent(torrent_id), trackers);
}



void Daemon::start(void)
{
	try
	{
		this->create_config_dirs();
	}
	catch(m::Exception& e)
	{
		M_THROW(__("Can't start daemon. Creating config directories failed. %1", EE(e)));
	}

	Daemon_session::start();
}



void Daemon::stop(void)
{
	Daemon_session::stop();
}

