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
#include <functional>
#include <iterator>
#include <string>

#include <libtorrent/session_status.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>

#include <mlib/libtorrent.hpp>

#include "common.hpp"
#include "daemon_settings.hpp"
#include "daemon_types.hpp"



// Torrent -->
	Torrent::Torrent(
		const Torrent_full_id& full_id,
		const lt::torrent_handle& handle,
		const m::lt::Torrent_metadata& torrent_metadata,
		const Torrent_settings& settings
	)
	:
		id(full_id.id),
		serial_number(full_id.serial_number),

		handle(handle),

		encoding(settings.encoding),
		publisher_url(torrent_metadata.publisher_url),

		seeding(false),

		time_added(settings.time_added),
		time_seeding(settings.time_seeding),

		files_revision(FIRST_REVISION),
		files_settings(settings.files_settings),

		trackers_revision(FIRST_REVISION),

		download_settings_revision(FIRST_REVISION),
		download_settings(settings.download_settings),

		total_download(settings.total_download),
		total_payload_download(settings.total_payload_download),
		total_upload(settings.total_upload),
		total_payload_upload(settings.total_payload_upload),
		total_failed(settings.total_failed),
		total_redundant(settings.total_redundant),

		bytes_done(settings.bytes_done),
		bytes_done_on_last_torrent_finish(settings.bytes_done_on_last_torrent_finish)
	{
		try
		{
			this->name = handle.name();
		}
		catch(lt::invalid_handle)
		{
			MLIB_LE();
		}
	}



	Download_settings Torrent::get_download_settings(void) const
	{
		return Download_settings(this->download_settings, this->handle.is_sequential_download());
	}



	std::string Torrent::get_download_path(void) const
	{
		try
		{
			return LT2U(this->handle.save_path().string());
		}
		catch(lt::invalid_handle)
		{
			MLIB_LE();
		}
	}



	Torrent_full_id Torrent::get_full_id(void) const
	{
		return Torrent_full_id(this->id, this->serial_number);
	}



	Torrent_info Torrent::get_info(void) const
	{
		return Torrent_info(*this);
	}



	bool Torrent::is_belong_to(Torrents_group group) const
	{
		switch(group)
		{
			case NONE:
				return false;
				break;

			case ALL:
				return true;
				break;

			case DOWNLOADS:
			{
				Torrent_info torrent_info(*this);
				return torrent_info.requested_size != torrent_info.downloaded_requested_size;
			}
			break;

			case UPLOADS:
			{
				Torrent_info torrent_info(*this);
				return torrent_info.requested_size == torrent_info.downloaded_requested_size;
			}
			break;

			default:
				MLIB_LE();
				break;
		}
	}



	bool Torrent::is_paused(void) const
	{
		try
		{
			return this->handle.is_paused();
		}
		catch(lt::invalid_handle)
		{
			MLIB_LE();
		}
	}



	void Torrent::sync_files_settings(void)
	{
		std::vector<int> files_priorities;

		std::transform(
			this->files_settings.begin(), this->files_settings.end(),
			back_inserter(files_priorities),
			std::mem_fun_ref(&Torrent_file_settings::convert_to_libtorrent_prioritry)
		);

		try
		{
			this->handle.prioritize_files(files_priorities);
		}
		catch(lt::invalid_handle e)
		{
			MLIB_LE();
		}
	}
// Torrent <--



// Daemon_statistics -->
	Daemon_statistics::Daemon_statistics(void)
	:
		session_start_time(time(NULL)),
		statistics_start_time(session_start_time),

		download(0),
		payload_download(0),

		total_download(0),
		total_payload_download(0),

		upload(0),
		payload_upload(0),

		total_upload(0),
		total_payload_upload(0),

		failed(0),
		total_failed(0),

		redundant(0),
		total_redundant(0)
	{
	}



	void Daemon_statistics::reset(const lt::session_status& libtorrent_session_status)
	{
		this->session_start_time = time(NULL);
		this->statistics_start_time = this->session_start_time;

		this->download = -libtorrent_session_status.total_download;
		this->payload_download = -libtorrent_session_status.total_payload_download;

		this->total_download = 0;
		this->total_payload_download = 0;

		this->upload = -libtorrent_session_status.total_upload;
		this->payload_upload = -libtorrent_session_status.total_payload_upload;

		this->total_upload = 0;
		this->total_payload_upload = 0;

		this->failed = -libtorrent_session_status.total_failed_bytes;
		this->total_failed = 0;

		this->redundant = -libtorrent_session_status.total_redundant_bytes;
		this->total_redundant = 0;
	}
// Daemon_statistics <--

