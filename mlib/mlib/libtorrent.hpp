/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
*                                                                         *
*   Copyright (C) 2009-2010, Dmitry Konishchev                            *
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


#ifndef HEADER_MLIB_LIBTORRENT
#define HEADER_MLIB_LIBTORRENT

#include <deque>

#ifndef MLIB_ENABLE_LIBS_FORWARDS
	#include <libtorrent/bencode.hpp>
	#include <libtorrent/torrent_handle.hpp>
#endif

#include <libtorrent/torrent_info.hpp>

#include <mlib/main.hpp>
#include <mlib/misc.hxx>


namespace m {


namespace libtorrent {

struct Torrent_file
{
	explicit Torrent_file(void) {}
	explicit Torrent_file(int id, const std::string& path, Size size);


	int				id;
	std::string		path;
	Size			size;


	std::string		get_path(void);

	void			set_path(std::string);

	bool operator<(const Torrent_file& file) const { return this->path < file.path; }
};



/// Предоставляет расширенную информацию о торренте, которой нет в
/// lt::torrent_info.
class Torrent_metadata
{
	public:
		Torrent_metadata(
			const lt::torrent_info& info,
			const std::string& publisher_url = ""
		);


	public:
		lt::torrent_info	info;
		std::string			publisher_url;
};



/// Возвращает Torrent_metadata, соответствующий magnet-ссылке.
/// @throw - m::Exception.
Torrent_metadata			get_magnet_metadata(const std::string& magnet);

/// Возвращает список путей скачанных файлов торрента torrent_handle.
/// @throw - libtorrent::invalid_handle.
std::vector<std::string>	get_torrent_downloaded_files_paths(const libtorrent::torrent_handle& torrent_handle, const std::vector<bool>& interested_files);

/// Преобразовывает file_path в путь, используемый для представления
/// расположения файлов торрента. Если file_path невозможно преобразовать в такой
/// путь, то генерирует исключение.
/// @throw - m::Exception.
std::string					get_torrent_file_path(const std::string& file_path);

/// Возвращает списк файлов торрента torrent_info.
/// @throw - m::Exception.
std::vector<Torrent_file>	get_torrent_files(const libtorrent::torrent_info& torrent_info);

/// Возвращает списк файлов торрента torrent_path.
/// @throw - m::Exception.
std::vector<Torrent_file>	get_torrent_files(const std::string& torrent_path, const std::string& encoding);

/// Возвращает список путей файлов торрента torrent_info.
std::vector<std::string>	get_torrent_files_paths(const libtorrent::torrent_info& torrent_info);

/// Возвращает Torrent_metadata, соответствующий торренту, данные которого
/// хранятся в буфере.
/// @throw - m::Exception.
Torrent_metadata			get_torrent_metadata(const m::Buffer& torrent_data, const std::string& encoding = MLIB_UTF_CHARSET_NAME);

/// Возвращает Torrent_metadata, соответствующий торренту torrent_uri, который
/// может быть либо файлом торрента, либо magnet-ссылкой.
/// @throw - m::Exception.
Torrent_metadata			get_torrent_metadata(const std::string& torrent_uri, const std::string& encoding = MLIB_UTF_CHARSET_NAME);

/// Возвращает список трекеров торрента.
/// @throw - lt::invalid_handle.
std::vector<std::string>	get_torrent_trackers(const libtorrent::torrent_handle& torrent_handle);

/// Возвращает список трекеров торрента.
std::vector<std::string>	get_torrent_trackers(const libtorrent::torrent_info& torrent_info);

/// Возвращает текущую версию libtorrent.
Version						get_version(void);

/// Определяет, является ли uri magnet-ссылкой.
bool						is_magnet_uri(const std::string& uri);

}


	std::string		EE(const libtorrent::invalid_encoding& error);
#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 15, 0)
	std::string		EE(const libtorrent::libtorrent_exception& error);
#else
	std::string		EE(const libtorrent::duplicate_torrent& error);
	std::string		EE(const libtorrent::invalid_torrent_file& error);
#endif


}

#endif

