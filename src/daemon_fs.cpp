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

#include <mlib/fs.hpp>

#include "daemon_fs.hpp"



void Daemon_fs::create_config_dirs(void) const throw(m::Exception)
{
	// torrents dir -->
		std::string torrents_dir_path = this->get_torrents_dir_path();

		try
		{
			m::fs::mkdir_if_not_exists(torrents_dir_path);
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Can't create directory '%1': %2.", m::fs::get_abs_path_lazy(torrents_dir_path), EE(e)));
		}
	// torrents dir <--
}



std::string Daemon_fs::get_config_dir_path(void) const
{
	try
	{
		return m::fs::unix_get_cwd();
	}
	catch(m::Exception)
	{
		return ".";
	}
}



std::string Daemon_fs::get_torrent_dir_path(const Torrent_id& torrent_id) const
{
	return Path(this->get_torrents_dir_path()) / torrent_id;
}



std::string Daemon_fs::get_torrents_download_path(void) const
{
	return Path(m::fs::get_user_home_path()) / "downloads";
}



std::string Daemon_fs::get_torrents_dir_path(void) const
{
	return Path(this->get_config_dir_path()) / "torrents";
}

