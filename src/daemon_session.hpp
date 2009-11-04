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


#ifndef HEADER_DAEMON_SESSION
#define HEADER_DAEMON_SESSION

#include <string>
#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <libtorrent/entry.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>

#include <mlib/gtk/dispatcher.hpp>

#include <mlib/libtorrent.hpp>

#include "common.hpp"
#include "daemon_fs.hpp"
#include "daemon_types.hpp"



namespace Daemon_session_aux {
	class Private;
}

class Tracker_error;


class Daemon_session: public Daemon_fs
{
	private:
		typedef Daemon_session_aux::Private Private;


	public:
		Daemon_session(const std::string& config_dir_path);
		~Daemon_session(void);


	public:
		/// Сигнал, генерируемый при получении сообщений,
		/// приходящих от libtorrent.
		m::gtk::Dispatcher<
		void (const Daemon_message&)>		messages_signal;

		m::gtk::Dispatcher<
		void (const Notify_message&)>		notify_message_signal;

	private:
		boost::scoped_ptr<Private>			priv;


	public:
		/// Добавляет в сессию новый торрент файл.
		///
		/// @param torrent_path - путь к файлу торрента.
		/// @param error_if_not_exists - определяет, является ли ошибкой,
		/// если файл торрента уже не существует.
		/// @throw - m::Exception.
		void						add_torrent(const std::string& torrent_path, const New_torrent_settings& torrent_settings, bool error_if_not_exists = true);

		/// Возвращает текущее ограничение на скорость.
		Speed						get_rate_limit(Traffic_type traffic_type) const;

		/// Возвращает информацию о текущей сессии.
		/// @throw - m::Exception.
		Session_status				get_session_status(void) const;

		/// Возвращает текущие настройки демона.
		Daemon_settings				get_settings(void) const;

		/// Возвращает информацию о пирах торрента.
		void						get_torrent_peers_info(const Torrent torrent, std::vector<Torrent_peer_info>& peers_info) const;

		/// Возвращает список с информацией о торрентах текущей сессии.
		void						get_torrents_info(std::vector<Torrent_info>& torrents_info) const;

		/// Сбрасывает все счетчики статистики в 0.
		void						reset_statistics(void);

		/// Устанавливает ограничение на скорость.
		void						set_rate_limit(Traffic_type traffic_type, Speed speed);

		/// Задает текущие настройки. Если init_settings == true, то не сверяет настройки
		/// с текущими, а сразу же применяет их (используется при инициализации демона).
		void						set_settings(const Daemon_settings& settings, const bool init_settings = false);

		/// Запускает торренты.
		void						start_torrents(const Torrents_group group);

		/// Останавливает торренты.
		void						stop_torrents(const Torrents_group group);

	protected:
		/// Сохраняет торрент torrent_path в папку, в которой
		/// хранятся все загруженные торренты.
		///
		/// @param torrent_path - путь к файлу торрента.
		/// @param error_if_not_exists - определяет, является ли ошибкой,
		/// если файл торрента уже не существует.
		///
		/// @throw - m::Exception.
		///
		/// @return - валидный Torrent_id, если торрент был добавлен в
		/// конфиг. Если торрент не был добавлен в конфиг, и это не является
		/// ошибкой, то возвращает невалидный Torrent_id. Это может
		/// произойти, к примеру, когда добавляется торрент, который уже
		/// присутствует в сессии, но в настройках было указано, что
		/// добавление торрента-дубликата не является ошибкой.
		Torrent_id					add_torrent_to_config(const std::string& torrent_path, const New_torrent_settings& new_torrent_settings, bool error_if_not_exists) const;

		/// Добавляет торрент к сессии.
		void						add_torrent_to_session(m::lt::Torrent_metadata torrent_metadata, const Torrent_settings& torrent_settings);

		/// Производит все необходимые действия по завершению скачивания
		/// торрента.
		void						finish_torrent(Torrent& torrent);

		/// Возвращает ссылку на торрент по его полному идентификатору.
		/// @throw - m::Exception.
		const Torrent&				get_torrent(const Torrent_full_id& full_id) const;

		/// Возвращает ссылку на торрент по его полному идентификатору.
		/// @throw - m::Exception.
		Torrent&					get_torrent(const Torrent_full_id& full_id);

		/// Возвращает ссылку на торрент по его идентификатору.
		/// @throw - m::Exception.
		const Torrent&				get_torrent(const Torrent_id& torrent_id) const;

		/// Возвращает ссылку на торрент по его идентификатору.
		/// @throw - m::Exception.
		Torrent&					get_torrent(const Torrent_id& torrent_id);

		/// Возвращает ссылку на торрент по его хэндлу.
		/// @throw - m::Exception.
		const Torrent&				get_torrent(const lt::torrent_handle& torrent_handle) const;

		/// Возвращает ссылку на торрент по его хэндлу.
		/// @throw - m::Exception.
		Torrent&					get_torrent(const lt::torrent_handle& torrent_handle);

		/// Возвращает подробную информацию о торренте.
		Torrent_details				get_torrent_details(const Torrent& torrent) const;

		/// Записывает в files список файлов торрента, а в statuses текущий статус файлов.
		/// Если переданная ревизия равна текущей, то не пишет в files ничего и заполняет
		/// только statuses.
		/// @return - текущую ревизию.
		Revision					get_torrent_files_info(const Torrent& torrent, std::vector<Torrent_file> *files, std::vector<Torrent_file_status>* statuses, Revision revision) const;

		/// Записывает в download_settings текущие параметры скачивания
		/// торрента, а в revision текущую ревизию параметров скачивания,
		/// если revision не равна текущей ревизии пареметров скачивания
		/// торрента.
		/// @return - true, если данные были записаны (ревизии не равны).
		bool						get_torrent_new_download_settings(const Torrent& torrent, Revision* revision, Download_settings* download_settings) const;

		/// Записывает в trackers текущие трекеры торрента, а в revision
		/// текущую ревизию трекеров, если revision не равна текущей ревизии
		/// трекеров.
		/// @return - true, если данные были записаны (ревизии не равны).
		bool						get_torrent_new_trackers(const Torrent& torrent, Revision* revision, std::vector<std::string>* trackers) const;

		/// Прерывает выполнение текущего "временного действия".
		/// @param complete - если false, то действие отменяется, если true
		/// - выполняется досрочно.
		void						interrupt_temporary_action(bool complete);

		/// Проверяет, существует ли в текущей сессии торрент с таким идентификатором.
		bool						is_torrent_exists(const Torrent_id& torrent_id) const;

		/// Приостанавливает работу с торрентом.
		void						pause_torrent(Torrent& torrent);

		/// Производит действие над группой торрентов, ожидает time секунд и
		/// отменяет произведенные изменения (асинхронно).
		void						process_torrents_temporary(Temporary_action action, Torrents_group group, Time time);

		/// Ставит торрент в очередь на перепроверку.
		/// @throw - m::Exception.
		void						recheck_torrent(const Torrent_id& torrent_id);

		/// Удаляет торрент.
		/// @throw - m::Exception.
		void						remove_torrent(const Torrent_id& torrent_id);

		/// Удаляет конфигурационные файлы торрента.
		/// @throw - m::Exception.
		void						remove_torrent_from_config(const Torrent_id& torrent_id) const;

		/// Удаляет торрент из текущей сессии.
		/// @throw - m::Exception.
		void						remove_torrent_from_session(const Torrent_id& torrent_id);

		/// Удаляет торрент вместе с данными (скачанными файлами).
		/// @throw - m::Exception.
		void						remove_torrent_with_data(const Torrent_id& torrent_id);

		/// Возобновляет работу с торрентом.
		/// @throw - m::Exception.
		void						resume_torrent(Torrent& torrent);

		/// Сохраняет настройки всех торрентов текущей сессии.
		/// @throw - m::Exception.
		void						save_session(void);

		/// Устанавливает параметры копирования файлов торрента по завершении их скачивания.
		/// @throw - m::Exception.
		void						set_copy_when_finished(Torrent& torrent, bool copy, const std::string& to);

		/// Устанавливает флаг скачивания для файлов с идентификаторами files_ids.
		/// @throw - m::Exception.
		void						set_files_download_status(Torrent& torrent, const std::vector<int>& files_ids, bool download);

		/// Устанавливает приоритет для файлов, идентификторы которых содержатся в списке files_id.
		/// @throw - m::Exception.
		void						set_files_priority(Torrent& torrent, const std::vector<int>& files_ids, const Torrent_file_settings::Priority priority);

		/// Устанавливает, режим скачивания торрента - последовательный или
		/// в зависимости от доступности частей торрента.
		/// @throw - m::Exception.
		void						set_sequential_download(Torrent& torrent, bool value);

		/// Задает список трекеров торрента.
		void						set_torrent_trackers(Torrent& torrent, const std::vector<std::string>& trackers);

		/// Запускает сессию.
		void						start(void);

		/// Инициирует остановку сессии.
		void						stop(void);

	private:
		/// "Избавляется" от торрента указанным методом.
		/// @throw - m::Exception.
		void						auto_clean_torrent(const Torrent_id& id, const std::string& name, const Auto_clean_type& clean_type);

		/// Выполняет все необходимые действия для автоматической загрузки
		/// торрента torrent_path, если он является торрентом.
		/// @throw - m::Exception.
		void						auto_load_if_torrent(const std::string& torrent_path);

		/// Загружает торренты, которые находятся в данный момент в очереди
		/// на автоматическую подгрузку.
		void						auto_load_torrents(void);

		/// Выполняет все необходимые действия по автоматизации некоторых
		/// операций. К примеру, если пришло время удалить старые торренты,
		/// то удаляет их.
		void						automate(void);

		/// Возвращает список трекеров торрента.
		std::vector<std::string>	get_torrent_trackers(const Torrent& torrent) const;

		/// Определяет, включен ли DHT или нет.
		bool						is_dht_started(void) const;

		/// Загружает торрент в сессию по его идентификатору.
		/// @throw - m::Exception.
		void						load_torrent(const Torrent_id& torrent_id);

		/// Загружает торренты из директории для автоматической загрузки
		/// торрентов, если это необходимо.
		/// @throw - m::Exception.
		void						load_torrents_from_auto_load_dir(void);

		/// Загружает все сохраненные торренты.
		/// Запускается при старте демона, чтобы добавить
		/// все торренты из прошлой сессии.
		/// @throw - m::Exception.
		void						load_torrents_from_config(void);

		/// Обработчик сигнала на автоматическое сохранение текущей сессии.
		bool						on_save_session_callback(void);

		/// Обработчик сигнала на истечение срока "временного действия".
		bool						on_temporary_action_expired_cb(void);

		/// Обработчик сигнала на завершение скачивания торрента.
		void						on_torrent_finished_callback(const lt::torrent_handle& torrent_handle);

		/// Обработчик сигнала на получение очередной resume data от libtorrent.
		void						on_torrent_resume_data_callback(const Torrent_id& torrent_id, boost::shared_ptr<lt::entry> resume_data);

		/// Обработчик сигнала на поступление сообщения об ошибке соединения с трекером.
		void						on_tracker_error_cb(const Tracker_error& error);

		/// Обработчик сигнала на обновления статистической информации
		/// о торрентах.
		bool						on_update_torrents_statistics_callback(void);

		/// Производит откат "временного действия" над торрентами.
		void						rollback_temporary_action(void);

		/// Сохраняет настройки торрента, если такой торрент еще существует.
		/// Вызывается функциями, которые получают resume data от libtorrent.
		/// Если такого торрента уже нет в сессии, то ничего страшного в этом нет.
		/// Это вполне нормальная ситуация, т. к. со времени генерации запроса очень
		/// многое могло измениться.
		void						save_torrent_settings_if_exists(const Torrent_id& torrent_id, const lt::entry& resume_data);

		/// Планирует сохранение настроек торрента.
		void						schedule_torrent_settings_saving(const Torrent& torrent);


	public:
		/// Поток для получения сообщений от libtorrent.
		void operator()(void);
};

#endif

