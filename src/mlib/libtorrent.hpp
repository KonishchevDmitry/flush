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
#ifndef HEADER_MLIB_LIBTORRENT
#define HEADER_MLIB_LIBTORRENT

#include <deque>
#include <string>
#include <vector>

#include <libtorrent/torrent_handle.hpp>

#include "errors.hpp"
#include "misc.hpp"
#include "types.hpp"


namespace m { namespace libtorrent {


struct Torrent_file
{
	inline
	Torrent_file(void);

	inline
	Torrent_file(int id, const std::string& path, Size size);


	int				id;
	std::string		path;
	Size			size;


	inline
	std::string		get_path(void);

	inline
	void			set_path(std::string);

	inline
	bool operator<(const Torrent_file& file) const;
};



/// Предоставляет расширенную информацию о торренте, которой нет в
/// lt::torrent_info.
class Torrent_metadata
{
	public:
		Torrent_metadata(
			const lt::torrent_info& info,
			const std::string& publisher_url
		);


	public:
		lt::torrent_info	info;
		std::string			publisher_url;
};



/// Возвращает список путей скачанных файлов торрента torrent_handle.
std::vector<std::string>	get_torrent_downloaded_files_paths(const libtorrent::torrent_handle& torrent_handle) throw(libtorrent::invalid_handle);

/// Преобразовывает file_path в путь, используемый для представления
/// расположения файлов торрента. Если file_path невозможно преобразовать в такой
/// путь, то генерирует исключение.
std::string					get_torrent_file_path(const std::string& file_path) throw(m::Exception);

/// Возвращает списк файлов торрента torrent_info.
std::vector<Torrent_file>	get_torrent_files(const libtorrent::torrent_info& torrent_info) throw(m::Exception);

/// Возвращает списк файлов торрента torrent_path.
std::vector<Torrent_file>	get_torrent_files(const std::string& torrent_path, const std::string& encoding) throw(m::Exception);

/// Возвращает список путей файлов торрента torrent_info.
std::vector<std::string>	get_torrent_files_paths(const libtorrent::torrent_info& torrent_info);

/// Возвращает libtorrent::torrent_info, соответствующий
/// торренту, данные которого хранятся в буфере.
Torrent_metadata			get_torrent_metadata(const m::Buffer& torrent_data, const std::string& encoding) throw(m::Exception);

/// Возвращает libtorrent::torrent_info, соответствующий
/// торренту torrent_path.
Torrent_metadata			get_torrent_metadata(const std::string& torrent_path, const std::string& encoding) throw(m::Exception);

/// Возвращает список трекеров торрента.
std::vector<std::string>	get_torrent_trackers(const libtorrent::torrent_handle& torrent_handle) throw(lt::invalid_handle);

/// Возвращает список трекеров торрента.
std::vector<std::string>	get_torrent_trackers(const libtorrent::torrent_info& torrent_info);

/// Возвращает текущую версию libtorrent.
inline
Version						get_version(void);

}}

#include "libtorrent.hh"

#endif
#endif

