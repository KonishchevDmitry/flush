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


Torrent_metadata::Torrent_metadata(const lt::torrent_info& info, const std::string& publisher_url)
:
	info(info),
	publisher_url(publisher_url)
{
}



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



std::vector<Torrent_file> get_torrent_files(const std::string& torrent_path, const std::string& encoding) throw(m::Exception)
{
	return get_torrent_files(get_torrent_metadata(torrent_path, encoding).info);
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



Torrent_metadata get_torrent_metadata(const m::Buffer& torrent_data, const std::string& encoding) throw(m::Exception)
{
	class Invalid_torrent_file {};

	try
	{
		lt::lazy_entry torrent_entry;
		lt::lazy_bdecode(torrent_data.get_data(), torrent_data.get_cur_ptr(), torrent_entry);

		std::string publisher_url;
		lt::torrent_info torrent_info(torrent_entry);


		// В libtorrent::torrent_info libtorrent записывает строки в той
		// кодировке, в которой они присутствуют в *.torrent файле, при этом
		// "ломая" исходную кодировку функцией проверки на UTF-8-валидность.
		// Нам же необходимо, чтобы все интересующие нас строки были в UTF-8.

		const lazy_entry* info_entry;

		// Инфо-секция -->
			if(torrent_entry.type() != lt::lazy_entry::dict_t)
				throw Invalid_torrent_file();

			if( !(info_entry = torrent_entry.dict_find_dict("info")) )
				throw Invalid_torrent_file();
		// Инфо-секция <--

		// publisher_url -->
		{
			const lazy_entry* entry = torrent_entry.dict_find_string("publisher-url");

			if(entry)
			{
				publisher_url = entry->string_value();

				if(!m::is_valid_utf(publisher_url) || !m::is_url_string(publisher_url))
					publisher_url = "";
			}
		}
		// publisher_url <--

		// Торрент записан не в UTF-8 кодировке
		if(encoding != MLIB_UTF_CHARSET_NAME)
		{
			std::string torrent_name;

			// Имя -->
			{
				const lazy_entry* name_entry;
			
				if( (name_entry = info_entry->dict_find_string("name.utf-8")) )
					torrent_name = m::validate_utf(name_entry->string_value());
				else if( (name_entry = info_entry->dict_find_string("name")) )
					torrent_name = m::convert(name_entry->string_value(), MLIB_UTF_CHARSET_NAME, encoding);
				else
					throw Invalid_torrent_file();

				if(torrent_name.empty())
					throw Invalid_torrent_file();

				torrent_info.files().set_name(torrent_name);
			}
			// Имя <--

			// Файлы -->
			{
				const std::string lt_files_encoding = m::get_libtorrent_files_charset();

				#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
					lt::file_storage& storage = torrent_info.files();
				#else
					const lt::file_storage& storage = torrent_info.files();
				#endif

				bool utf = false;
				const lazy_entry* files_entry;

				if(
					( (files_entry = info_entry->dict_find_list("files.utf-8")) && (utf = true) ) ||
					( (files_entry = info_entry->dict_find_list("files")) )
				)
				{
					std::string files_prefix = "/" + torrent_name;

					if(storage.num_files() != files_entry->list_size())
						throw Invalid_torrent_file();

					for(size_t file_id = 0, size = files_entry->list_size(); file_id < size; ++file_id)
					{
						std::string path;

						const lazy_entry* file_entry = files_entry->list_at(file_id);
						const lazy_entry* path_entry;

						if(file_entry->type() != lt::lazy_entry::dict_t)
							throw Invalid_torrent_file();

						if(!(
							( (path_entry = file_entry->dict_find_list("path.utf-8")) && (utf = true) ) ||
							( path_entry = file_entry->dict_find_list("path"))
						))
							throw Invalid_torrent_file();

						for(size_t i = 0, end = path_entry->list_size(); i < end; ++i)
						{
							const lazy_entry* path_element_entry = path_entry->list_at(i);

							if(path_element_entry->type() != lt::lazy_entry::string_t)
								throw Invalid_torrent_file();

							if(path_element_entry->string_value().empty())
								throw Invalid_torrent_file();

							path += "/" + path_element_entry->string_value();
						}

						// libtorrent не поддерживает не-UTF-8 локали, поэтому приходится
						// делать за него всю работу самим.
						// -->
							if(utf)
								path = U2LT( Path(files_prefix + path).normalize().string() );
							else
							{
								path = U2LT(Path(
									files_prefix + m::convert(path, MLIB_UTF_CHARSET_NAME, encoding)
								).normalize().string());
							}
						// <--

						// Лучше не делать лишних переименований - в
						// какой-то версии libtorrent эти пути не
						// проверялись на соответствие и впоследствии
						// возникала ошибка.
						if(storage.at(0).path.string() != path)
						#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
							storage.rename_file(file_id, path);
						#else
							torrent_info.rename_file(file_id, path);
						#endif
					}
				}
				else
				{
					if(storage.num_files() != 1)
						throw Invalid_torrent_file();

					std::string path = U2LT(Path("/" + torrent_name).normalize().string());

					// Лучше не делать лишних переименований - в
					// какой-то версии libtorrent эти пути не
					// проверялись на соответствие и впоследствии
					// возникала ошибка.
					if(storage.at(0).path.string() != path)
					#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
						storage.rename_file(0, path);
					#else
						torrent_info.rename_file(0, path);
					#endif
				}
			}
			// Файлы <--
		}
		// Торрент записан в UTF-8 кодировке
		else
		{
			// libtorrent не поддерживает не-UTF-8 локали, поэтому приходится
			// делать за него всю работу самим.
			//
			// -->
			{
				#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
					lt::file_storage& storage = torrent_info.files();
				#endif

				lt::torrent_info::file_iterator it = torrent_info.begin_files();
				lt::torrent_info::file_iterator end_it = torrent_info.end_files();

				for(size_t i = 0; it != end_it; ++it, ++i)
				{
					const std::string path = it->path.string();
					const std::string new_path = U2LT(it->path.string());

					// Лучше не делать лишних переименований - в какой-то
					// версии libtorrent эти пути не проверялись на
					// соответствие и впоследствии возникала ошибка.
					if(path != new_path)
					#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
						storage.rename_file(i, new_path);
					#else
						torrent_info.rename_file(i, new_path);
					#endif
				}
			}
			// <--
		}

		return Torrent_metadata(torrent_info, publisher_url);
	}
	catch(lt::invalid_encoding& e)
	{
		M_THROW(EE(e));
	}
	catch(lt::invalid_torrent_file& e)
	{
		M_THROW(EE(e));
	}
	catch(Invalid_torrent_file&)
	{
		M_THROW(_("invalid torrent file"));
	}
}



Torrent_metadata get_torrent_metadata(const std::string& torrent_path, const std::string& encoding) throw(m::Exception)
{
	m::Buffer torrent_data;

	try
	{
		// Генерирует m::Exception
		torrent_data.load_file(torrent_path);

		// Генерирует m::Exception
		return get_torrent_metadata(torrent_data, encoding);
	}
	catch(m::Exception& e)
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

