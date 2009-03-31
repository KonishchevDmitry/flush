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


#ifndef HEADER_DAEMON
	#define HEADER_DAEMON

	#include <deque>
	#include <string>
	#include <vector>

	#include "daemon_session.hpp"

	class Daemon: public Daemon_session
	{
		public:
			Daemon(const std::string& config_dir_path);


		public:
			/// Возвращает подробную информацию о торренте.
			Torrent_details				get_torrent_details(const Torrent_id& torrent_id) const throw(m::Exception);

			/// Возвращает директорию, в которую был скачан торрент.
			std::string					get_torrent_download_path(const Torrent_id& torrent_id) throw(m::Exception);

			/// Записывает в files список файлов торрента, а в statuses текущий статус файлов.
			/// Если переданная ревизия равна текущей, то не пишет в files ничего и заполняет
			/// только statuses.
			/// @return - текущую ревизию.
			Revision					get_torrent_files_info(const Torrent_id& torrent_id, std::vector<Torrent_file> *files, std::vector<Torrent_file_status>* statuses, Revision revision) const throw(m::Exception);

			/// Возвращает информацию о пирах торрента.
			void						get_torrent_peers_info(const Torrent_id& torrent_id, std::vector<Torrent_peer_info>& peers_info) const throw(m::Exception);

			/// Записывает в download_settings текущие параметры скачивания
			/// торрента, а в revision текущую ревизию параметров скачивания,
			/// если revision не равна текущей ревизии пареметров скачивания
			/// торрента.
			/// @return - true, если данные были записаны (ревизии не равны).
			bool						get_torrent_new_download_settings(const Torrent_id& torrent_id, Revision* revision, Download_settings* download_settings) const throw(m::Exception);

			/// Записывает в trackers текущие трекеры торрента, а в revision
			/// текущую ревизию трекеров, если revision не равна текущей ревизии
			/// трекеров.
			/// @return - true, если данные были записаны (ревизии не равны).
			bool						get_torrent_new_trackers(const Torrent_id& torrent_id, Revision* revision, std::vector<std::string>* trackers) const throw(m::Exception);

			/// Производит необходимые действия с переданными ей торрентами.
			void						process_torrents(const std::vector<Torrent_id>& torrents_ids, Torrent_process_action action) throw(m::Exception);

			/// Устанавливает параметры копирования файлов торрента по завершении их скачивания.
			void						set_copy_when_finished(const Torrent_id& torrent_id, bool copy, const std::string& to) throw(m::Exception);

			/// Устанавливает флаг скачивания для файлов с идентификаторами files_ids.
			void						set_files_download_status(const Torrent_id& torrent_id, const std::vector<int>& files_ids, bool download) throw(m::Exception);

#if 0
			/// Устанавливает новые пути для файлов.
			void						set_files_new_paths(const Torrent_id& torrent_id, const std::vector<Torrent_file>& files_new_paths) throw(m::Exception);
#endif

			/// Устанавливает приоритет для файлов, идентификторы которых содержатся в списке files_id.
			void						set_files_priority(const Torrent_id& torrent_id, const std::vector<int>& files_ids, const Torrent_file_settings::Priority priority) throw(m::Exception);

			/// Устанавливает, режим скачивания торрента - последовательный или
			/// в зависимости от доступности частей торрента.
			void						set_sequential_download(const Torrent_id& torrent_id, bool value) throw(m::Exception);

			/// Задает список трекеров торрента.
			void						set_torrent_trackers(const Torrent_id& torrent_id, const std::vector<std::string>& trackers) throw(m::Exception);

			/// Запускает демон.
			void						start(void) throw(m::Exception);

		private:
			/// Приостанавливает работу с торрентом.
			void						pause_torrent(const Torrent_id& torrent_id) throw(m::Exception);

			/// Возобновляет работу с торрентом.
			void						resume_torrent(const Torrent_id& torrent_id) throw(m::Exception);
	};

#endif
