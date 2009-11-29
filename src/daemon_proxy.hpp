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



#ifndef HEADER_CLIENT
#define HEADER_CLIENT

#include <deque>
#include <string>
#include <vector>

#include <mlib/signals_holder.hpp>

#include "common.hpp"


/// Класс, через который клиентская часть управляет демоном.
/// Сделана для того чтобы:
/// - Отделить файлы демона от файлов клиента, ускорив тем самым компиляцию.
/// - Заложить возможность полного отделения демона от клиента, чтобы впоследствии
///   можно было без особых усилий поставить между ними сокеты или, например, DBus.
class Daemon_proxy
{
	public:
		Daemon_proxy(const std::string& daemon_config_path);
		~Daemon_proxy(void);


	public:
		/// Сигнал на получение сообщений от демона.
		sigc::signal<void, const Daemon_message&>	daemon_message_signal;

		/// Сигнал на получение notify-сообщений от демона.
		sigc::signal<void, const Notify_message&>	notify_message_signal;

	private:
		/// Демон, которым управляет proxy.
		Daemon*						daemon;

		m::Signals_holder			signals_holder;


	public:
		/// Добавляет торрент в сессию.
		/// @throw - m::Exception.
		void						add_torrent(const std::string& torrent_uri, const New_torrent_settings& torrent_settings);

		/// Возвращает текущее ограничение на скорость.
		/// @throw - m::Exception.
		Speed						get_rate_limit(Traffic_type traffic_type) const;

		/// Возвращает информацию о текущей сессии.
		/// @throw - m::Exception.
		Session_status				get_session_status(void) const;

		/// Возвращает текущие настройки демона.
		/// @throw - m::Exception.
		Daemon_settings				get_settings(void) const;

		/// Возвращает подробную информацию о торренте.
		/// @throw - m::Exception.
		Torrent_details				get_torrent_details(const Torrent_id& torrent_id) const;

		/// Возвращает директорию, в которую был скачан торрент.
		/// @throw - m::Exception.
		std::string					get_torrent_download_path(const Torrent_id& torrent_id) const;

		/// Записывает в files список файлов торрента, а в statuses текущий статус файлов.
		/// Если переданная ревизия равна текущей, то не пишет в files ничего и заполняет
		/// только statuses.
		/// @return - текущую ревизию.
		/// @throw - m::Exception.
		Revision					get_torrent_files_info(const Torrent_id& torrent_id, std::vector<Torrent_file> *files, std::vector<Torrent_file_status>* statuses, Revision revision) const;

		/// Возвращает информацию о пирах торрента.
		/// @throw - m::Exception.
		void						get_torrent_peers_info(const Torrent_id& torrent_id, std::vector<Torrent_peer_info>& peers_info) const;

		/// Записывает в download_settings текущие параметры скачивания
		/// торрента, а в revision текущую ревизию параметров скачивания,
		/// если revision не равна текущей ревизии пареметров скачивания
		/// торрента.
		/// @return - true, если данные были записаны (ревизии не равны).
		/// @throw - m::Exception.
		bool						get_torrent_new_download_settings(const Torrent_id& torrent_id, Revision* revision, Download_settings* download_settings) const;

		/// Записывает в trackers текущие трекеры торрента, а в revision
		/// текущую ревизию трекеров, если revision не равна текущей ревизии
		/// трекеров.
		/// @return - true, если данные были записаны (ревизии не равны).
		/// @throw - m::Exception.
		bool						get_torrent_new_trackers(const Torrent_id& torrent_id, Revision* revision, std::vector<std::string>* trackers) const;

		/// Возвращает список с информацией о торрентах текущей сессии.
		/// @throw - m::Exception.
		void						get_torrents(std::vector<Torrent_info>& torrents_info);

		/// Прерывает выполнение текущего "временного действия".
		/// @param complete - если false, то действие отменяется, если true
		/// - выполняется досрочно.
		/// @throw - m::Exception.
		void						interrupt_temporary_action(bool complete);

		/// Производит необходимые действия с переданными ей торрентами.
		/// @throw - m::Exception.
		void						process_torrents(const std::vector<Torrent_id>& torrents_ids, Torrent_process_action action);

		/// Производит действие над группой торрентов, ожидает time секунд и
		/// отменяет произведенные изменения (асинхронно).
		/// @throw - m::Exception.
		void						process_torrents_temporary(Temporary_action action, Torrents_group group, Time time);

		/// Сбрасывает все счетчики статистики в 0.
		/// @throw - m::Exception.
		void						reset_statistics(void);

		/// Устанавливает параметры копирования файлов торрента по завершении их скачивания.
		/// @throw - m::Exception.
		void						set_copy_when_finished(const Torrent_id& torrent_id, bool copy, const std::string& to);

		/// Устанавливает флаг скачивания для файлов с идентификаторами files_ids.
		/// @throw - m::Exception.
		void						set_files_download_status(const Torrent_id& torrent_id, const std::vector<int>& files_ids, bool download);

#if 0
		/// Устанавливает новые пути для файлов.
		/// @throw - m::Exception.
		void						set_files_new_paths(const Torrent_id& torrent_id, const std::vector<Torrent_file>& files_new_paths);
#endif

		/// Устанавливает приоритет для файлов, идентификторы которых содержатся в списке files_id.
		/// @throw - m::Exception.
		void						set_files_priority(const Torrent_id& torrent_id, const std::vector<int>& files_ids, const Torrent_file_settings::Priority priority);

		/// Устанавливает ограничение на скорость.
		/// @throw - m::Exception.
		void						set_rate_limit(Traffic_type traffic_type, Speed speed);

		/// Задает текущие настройки демона.
		/// @throw - m::Exception.
		void						set_settings(const Daemon_settings& settings);

		/// Устанавливает, режим скачивания торрента - последовательный или
		/// в зависимости от доступности частей торрента.
		/// @throw - m::Exception.
		void						set_sequential_download(const Torrent_id& torrent_id, bool value);

		/// Задает список трекеров торрента.
		/// @throw - m::Exception.
		void						set_torrent_trackers(const Torrent_id& torrent_id, const std::vector<std::string>& trackers);

		/// Запускает демон.
		/// @throw - m::Exception.
		void						start(void);

		/// Запускает торренты.
		/// @throw - m::Exception.
		void						start_torrents(const Torrents_group group);

		/// Инициирует остановку демона.
		/// @throw - m::Exception.
		void						stop(void);

		/// Останавливает торренты.
		/// @throw - m::Exception.
		void						stop_torrents(const Torrents_group group);
};

#endif

