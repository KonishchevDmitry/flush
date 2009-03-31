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


#ifndef HEADER_DAEMON_FS
	#define HEADER_DAEMON_FS

	#include <string>

	class Daemon_fs
	{
		protected:
			/// Создает все необходимые директории, в которых будут
			/// храниться конфигурационные файлы, если они еще не
			/// существуют.
			void			create_config_dirs(void) const throw(m::Exception);

			/// Возвращает путь к директории с конфигурационными файлами.
			std::string		get_config_dir_path(void) const;

			/// Возвращает путь к файлам торрента torrent_id.
			std::string		get_torrent_dir_path(const Torrent_id& torrent_id) const;

			/// Возвращает путь к папке, в которую по умолчанию сохраняются файлы торрентов.
			std::string		get_torrents_download_path(void) const;

			/// Возвращает путь к папке, хранящей информацию о торретах.
			std::string		get_torrents_dir_path(void) const;
	};

#endif

