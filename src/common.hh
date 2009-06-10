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


// Daemon_message -->
	Daemon_message::Type Daemon_message::get_type(void) const
	{
		return this->type;
	}



	const std::string& Daemon_message::get(void) const
	{
		return this->message;
	}



	Daemon_message::operator const std::string&(void) const
	{
		return this->get();
	}
// Daemon_message <--



// Torrent_id -->
	Torrent_id::Torrent_id(void)
	{
	}



	Torrent_id::Torrent_id(const std::string& hash)
	: std::string(hash)
	{
	}



	Torrent_id::Torrent_id(const Glib::ustring& hash)
	: std::string(static_cast<std::string>(hash))
	{
	}
// Torrent_id <--



// Torrent_full_id -->
	Torrent_full_id::Torrent_full_id(const Torrent_id& id, size_t serial_number)
	:
		id(id),
		serial_number(serial_number)
	{
	}
// Torrent_full_id <--



// Torrent_info_widget -->
	void Torrent_info_widget::torrent_changed(const Torrent_id& torrent_id)
	{
		this->update(torrent_id);
	}
// Torrent_info_widget <--



// Session_info_widget -->
	void Session_info_widget::torrent_changed(const Torrent_id& torrent_id)
	{
	}
// Session_info_widget <--



// Torrent_file_settings -->
	Torrent_file_settings::Torrent_file_settings(void)
	:
		download(true),
		priority(Torrent_file_settings::get_default_priority())
	{
	}



	Torrent_file_settings::Torrent_file_settings(bool download, Priority priority)
	:
		download(download),
		priority(priority)
	{
	}



	Torrent_file_settings::Priority Torrent_file_settings::get_default_priority(void)
	{
		return NORMAL;
	}



	std::string Torrent_file_settings::get_default_priority_localized_name(void)
	{
		return Torrent_file_settings::get_priority_name(Torrent_file_settings::get_default_priority());
	}



	std::string Torrent_file_settings::get_default_priority_name(void)
	{
		return Torrent_file_settings::get_priority_localized_name(Torrent_file_settings::get_default_priority());
	}



	std::string Torrent_file_settings::get_priority_localized_name(void) const
	{
		return Torrent_file_settings::get_priority_localized_name(priority);
	}



	std::string Torrent_file_settings::get_priority_name(void) const
	{
		return Torrent_file_settings::get_priority_name(priority);
	}



	void Torrent_file_settings::set_priority_by_name(const std::string& name) throw(m::Exception)
	{
		this->priority = Torrent_file_settings::get_priority_by_name(name);
	}
// Torrent_file_settings <--



// Torrent_file_status -->
	Torrent_file_status::Torrent_file_status(bool download, Priority priority, Size downloaded)
	:
		Torrent_file_settings(download, priority),
		downloaded(downloaded)
	{
	}



	Torrent_file_status::Torrent_file_status(const Torrent_file_settings& settings, Size downloaded)
	:
		Torrent_file_settings(settings),
		downloaded(downloaded)
	{
	}
// Torrent_file_status <--



Speed get_lt_rate_limit(Speed rate)
{
	if(rate == -1)
		return -1;
	else
		return rate * 1024;
}



Speed get_rate_limit_from_lt(Speed rate)
{
	if(rate == -1)
		return -1;
	else
		return rate / 1024;
}

