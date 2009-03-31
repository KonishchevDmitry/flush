/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
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


#ifdef MLIB_ENABLE_LIBTORRENT

#include <algorithm>

#include <libtorrent/bencode.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/version.hpp>

#include "fs.hpp"
#include "libtorrent.hpp"
#include "misc.hpp"
#include "string.hpp"
#include "types.hpp"



namespace m { namespace libtorrent {


std::vector<std::string> get_torrent_downloaded_files_paths(const libtorrent::torrent_handle& torrent_handle) throw(libtorrent::invalid_handle)
{
	std::vector<std::string> torrent_files;

	std::vector<libtorrent::size_type> files_progress;

	lt::torrent_info torrent_info = torrent_handle.get_torrent_info();
	lt::torrent_info::file_iterator it = torrent_info.begin_files();
	lt::torrent_info::file_iterator end_it = torrent_info.end_files();

	torrent_files.reserve(torrent_info.num_files());

	// Получаем информацию о состоянии файлов
	torrent_handle.file_progress(files_progress);

	MLIB_A(files_progress.size() == size_t(torrent_info.num_files()));

	for(size_t i = 0; it != end_it; it++, i++)
		if(files_progress[i] == it->size)
			torrent_files.push_back( LT2U(it->path.string()) );

	return torrent_files;
}



std::string get_torrent_file_path(const std::string& file_path) throw(m::Exception)
{
	Path path = Path("/" + file_path).normalize();
	std::string path_string = path.string();

	if(!path.is_absolute() || path_string == "/")
		M_THROW(__("Invalid torrent file path '%1'.", file_path));
	
	return path_string;
}



std::vector<Torrent_file> get_torrent_files(const lt::torrent_info& torrent_info) throw(m::Exception)
{
	Torrent_file torrent_file;
	std::vector<Torrent_file> torrent_files;
	lt::torrent_info::file_iterator it = torrent_info.begin_files();
	lt::torrent_info::file_iterator end_it = torrent_info.end_files();

	torrent_files.reserve(torrent_info.num_files());

	for(int i = 0; it != end_it; it++, i++)
	{
		torrent_file.id = i;
		// m::Exception
		torrent_file.path = get_torrent_file_path( LT2U(it->path.string()) );
		torrent_file.size = it->size;
		torrent_files.push_back(torrent_file);
	}

	return torrent_files;
}



std::vector<Torrent_file> get_torrent_files(const std::string& torrent_path) throw(m::Exception)
{
	return get_torrent_files(get_torrent_info(torrent_path));
}



std::vector<std::string> get_torrent_files_paths(const libtorrent::torrent_info& torrent_info)
{
	std::vector<std::string> torrent_files_paths;
	lt::torrent_info::file_iterator it = torrent_info.begin_files();
	lt::torrent_info::file_iterator end_it = torrent_info.end_files();

	torrent_files_paths.reserve(torrent_info.num_files());

	for(; it != end_it; it++)
		torrent_files_paths.push_back( LT2U(it->path.string()) );

	return torrent_files_paths;
}



lt::torrent_info get_torrent_info(const std::string& torrent_path) throw(m::Exception)
{
	try
	{
		m::Buffer file_buf;
		#if M_LT_GET_MAJOR_MINOR_VERSION() < M_GET_VERSION(0, 14, 0)
			lt::entry torrent_entry;
		#else
			lt::lazy_entry torrent_entry;
		#endif

		file_buf.load_file(torrent_path);
		#if M_LT_GET_MAJOR_MINOR_VERSION() < M_GET_VERSION(0, 14, 0)
			torrent_entry = lt::bdecode(file_buf.get_data(), file_buf.get_cur_ptr());
		#else
			 lt::lazy_bdecode(file_buf.get_data(), file_buf.get_cur_ptr(), torrent_entry);
		#endif
		
		lt::torrent_info torrent_info(torrent_entry);

		// libtorrent не поддерживает не-UTF-8 локали, поэтому приходится
		// делать за него всю работу самим.
		//
		// В libtorrent::torrent_info libtorrent записывает файлы в кодировке
		// UTF-8. Мы же перегоняем их в ту кодировку, в которой они должны быть
		// на самом деле.
		// -->
		{
			#if M_LT_GET_VERSION() < M_GET_VERSION(0, 15, 0)
				lt::file_storage& storage = torrent_info.files();
			#endif

			lt::torrent_info::file_iterator it = torrent_info.begin_files();
			lt::torrent_info::file_iterator end_it = torrent_info.end_files();

			#if M_LT_GET_VERSION() < M_GET_VERSION(0, 15, 0)
				for(int i = 0; it != end_it; it++, i++)
					storage.rename_file(i, U2LT(it->path.string()));
			#else
				for(int i = 0; it != end_it; it++, i++)
					torrent_info.rename_file(i, U2LT(it->path.string()));
			#endif
		}
		// <--
		
		return torrent_info;
	}
	catch(m::Exception& e)
	{
		M_THROW(__("Error while reading torrent file '%1': %2.", torrent_path, EE(e)));
	}
	catch(lt::invalid_encoding e)
	{
		M_THROW(__("Error while reading torrent file '%1': %2.", torrent_path, EE(e)));
	}
	catch(lt::invalid_torrent_file e)
	{
		M_THROW(__("Error while reading torrent file '%1': %2.", torrent_path, EE(e)));
	}
}



std::vector<std::string> get_torrent_trackers(const libtorrent::torrent_handle& torrent_handle) throw(lt::invalid_handle)
{
	std::vector<std::string> trackers;

	// Генерирует lt::invalid_handle
	std::vector<lt::announce_entry> announces = torrent_handle.trackers();

	trackers.reserve(announces.size());

	for(size_t i = 0; i < announces.size(); i++)
		trackers.push_back(announces[i].url);
	
	return trackers;
}



std::vector<std::string> get_torrent_trackers(const libtorrent::torrent_info& torrent_info)
{
	std::vector<std::string> trackers;

	std::vector<lt::announce_entry> announces = torrent_info.trackers();

	trackers.reserve(announces.size());

	for(size_t i = 0; i < announces.size(); i++)
		trackers.push_back(announces[i].url);
	
	return trackers;
}

}}

#endif

