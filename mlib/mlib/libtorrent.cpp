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


#ifdef MLIB_ENABLE_LIBTORRENT

	#include <algorithm>
	#include <exception>

	#include <boost/lexical_cast.hpp>
	#include <boost/optional.hpp>

	#include <libtorrent/bencode.hpp>
	#include <libtorrent/entry.hpp>
	#include <libtorrent/escape_string.hpp>
	#include <libtorrent/torrent_handle.hpp>
	#include <libtorrent/torrent_info.hpp>
	#include <libtorrent/version.hpp>

	#include <mlib/fs.hpp>
	#include <mlib/main.hpp>
	#include <mlib/misc.hpp>
	#include <mlib/string.hpp>

	#include "libtorrent.hpp"

#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 15, 0)
	#include <boost/system/error_code.hpp>
#endif



namespace m {


namespace libtorrent {

// Torrent_file -->
	Torrent_file::Torrent_file(int id, const std::string& path, Size size)
	:
		id(id),
		path(path),
		size(size)
	{
	}



	std::string Torrent_file::get_path(void)
	{
		return this->path;
	}



	void Torrent_file::set_path(std::string path)
	{
		this->path = path;
	}
// Torrent_file <--



Torrent_metadata::Torrent_metadata(const lt::torrent_info& info, const std::string& publisher_url)
:
	info(info),
	publisher_url(publisher_url)
{
}



Torrent_metadata get_magnet_metadata(const std::string& magnet)
{
	MLIB_D(_C("Getting magent link's '%1' info...", magnet));

	if(!is_magnet_uri(magnet))
		M_THROW(_("Invalid magnet URI."));

	try
	{
		// libtorrent'овские функции могут сгенерировать очень много
		// различных исключений исключений.

		// Хэш -->
			sha1_hash info_hash;
			std::string info_hash_string;

			{
				std::string btih_prefix = "urn:btih:";
				boost::optional<std::string> btih = lt::url_has_argument(magnet, "xt");

				if(btih && btih->find(btih_prefix) == 0)
				{
					*btih = btih->substr(btih_prefix.size());

					if(btih->size() == 40)
						info_hash = boost::lexical_cast<sha1_hash>(*btih);
					else
						info_hash.assign(lt::base32decode(*btih));
				}
				else
					M_THROW(_("there is no 'xt=urn:btih:' field in it"));
			}

			info_hash_string = boost::lexical_cast<std::string>(info_hash);
			MLIB_D(_C("\tInfo hash: %1.", info_hash_string));
		// Хэш <--

		lt::torrent_info info(info_hash);

		// Имя -->
			{
				boost::optional<std::string> name = lt::url_has_argument(magnet, "dn");

				#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 15, 0)
				{
					boost::system::error_code error;
					lt::file_storage files = info.files();

					if(name)
					{
						files.set_name( lt::unescape_string(name->c_str(), error) );

						if(error)
							files.set_name(info_hash_string);
					}
					else
						files.set_name(info_hash_string);

					info.remap_files(files);
				}
				#else
					if(name)
						info.files().set_name( lt::unescape_string(name->c_str()) );
					else
						info.files().set_name(info_hash_string);
				#endif
			}

			MLIB_D(_C("\tName: '%1'.", info.name()));
		// Имя <--

		// Трекер -->
		{
			boost::optional<std::string> tracker = url_has_argument(magnet, "tr");

			if(tracker)
			{
			#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 15, 0)
				boost::system::error_code error;
				std::string tracker_url = unescape_string(tracker->c_str(), error);
			#else
				std::string tracker_url = unescape_string(tracker->c_str());
			#endif
				MLIB_D(_C("\tTracker: '%1'.", tracker_url));
				info.add_tracker(tracker_url);
			}
		}
		// Трекер <--

		return Torrent_metadata(info);
	}
	catch(std::exception& e)
	{
		M_THROW(__("Invalid magnet URI: %1.", EE(e)));
	}
}



std::vector<std::string> get_torrent_downloaded_files_paths(const libtorrent::torrent_handle& torrent_handle, const std::vector<bool>& interested_files)
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
		if(interested_files[i] && files_progress[i] == it->size)
		#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 16, 0)
			torrent_files.push_back( LT2U(it->filename()) );
		#else
			torrent_files.push_back( LT2U(it->path.string()) );
		#endif

	return torrent_files;
}



std::string get_torrent_file_path(const std::string& file_path)
{
	Path path = Path("/" + file_path).normalize();
	std::string path_string = path.string();

	if(!path.is_absolute() || path_string == "/")
		M_THROW(__("Invalid torrent file path '%1'.", file_path));

	return path_string;
}



std::vector<Torrent_file> get_torrent_files(const lt::torrent_info& torrent_info)
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
	#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 16, 0)
		torrent_file.path = get_torrent_file_path( LT2U(it->filename()) );
	#else
		torrent_file.path = get_torrent_file_path( LT2U(it->path.string()) );
	#endif
		torrent_file.size = it->size;
		torrent_files.push_back(torrent_file);
	}

	return torrent_files;
}



std::vector<Torrent_file> get_torrent_files(const std::string& torrent_path, const std::string& encoding)
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
	#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 16, 0)
		torrent_files_paths.push_back( LT2U(it->filename()) );
	#else
		torrent_files_paths.push_back( LT2U(it->path.string()) );
	#endif

	return torrent_files_paths;
}



Torrent_metadata get_torrent_metadata(const m::Buffer& torrent_data, const std::string& encoding)
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
			const lazy_entry* entry;

			entry = torrent_entry.dict_find_string("publisher-url");

			if(!entry)
				entry = torrent_entry.dict_find_string("comment");

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

				#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 15, 0)
				{
					lt::file_storage files = torrent_info.files();
					files.set_name(torrent_name);
					torrent_info.remap_files(files);
				}
				#else
					torrent_info.files().set_name(torrent_name);
				#endif
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
						#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
						if(storage.at(0).path.string() != path)
							storage.rename_file(file_id, path);
						#elif M_LT_GET_VERSION() < M_GET_VERSION(0, 16, 0)
						if(storage.at(0).path.string() != path)
							torrent_info.rename_file(file_id, path);
						#else
						if(storage.at(0).path != path)
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
					#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
					if(storage.at(0).path.string() != path)
						storage.rename_file(0, path);
					#elif M_LT_GET_VERSION() < M_GET_VERSION(0, 16, 0)
					if(storage.at(0).path.string() != path)
						torrent_info.rename_file(0, path);
					#else
					if(storage.at(0).path != path)
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
				#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 16, 0)
					const std::string path = it->filename();
					const std::string new_path = U2LT(it->filename());
				#else
					const std::string path = it->path.string();
					const std::string new_path = U2LT(it->path.string());
				#endif

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



Torrent_metadata get_torrent_metadata(const std::string& torrent_uri, const std::string& encoding)
{
	if(is_magnet_uri(torrent_uri))
		return get_magnet_metadata(torrent_uri);
	else
	{
		m::Buffer torrent_data;

		try
		{
			// Генерирует m::Exception
			torrent_data.load_file(torrent_uri);

			// Генерирует m::Exception
			return get_torrent_metadata(torrent_data, encoding);
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Error while reading torrent file '%1': %2.", torrent_uri, EE(e)));
		}
	}
}



std::vector<std::string> get_torrent_trackers(const libtorrent::torrent_handle& torrent_handle)
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



Version get_version(void)
{
	return ::m::get_version(LIBTORRENT_VERSION_MAJOR, LIBTORRENT_VERSION_MINOR, 0);
}



bool is_magnet_uri(const std::string& uri)
{
	std::string magnet_prefix = "magnet:";

	if(uri.size() > magnet_prefix.size() && uri.substr(0, magnet_prefix.size()) == magnet_prefix)
		return true;
	else
		return false;
}


}



std::string EE(const libtorrent::invalid_encoding& error)
{
	return _("bad torrent data");
}



#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 15, 0)
	std::string EE(const libtorrent::libtorrent_exception& error)
	{
		return __("libtorrent error code [%1]", error.error());
	}
#else
	std::string EE(const libtorrent::duplicate_torrent& error)
	{
		return _("torrent is already exists in this session");
	}



	std::string EE(const libtorrent::invalid_torrent_file& error)
	{
		return _("this is not a torrent file");
	}
#endif


}

#endif

