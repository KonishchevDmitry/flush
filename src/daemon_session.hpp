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

	#include <deque>
	#include <string>
	#include <queue>
	#include <vector>

	#include <boost/thread.hpp>

	#include <libtorrent/entry.hpp>
	#include <libtorrent/torrent_handle.hpp>
	#include <libtorrent/torrent_info.hpp>

	#include <glibmm/dispatcher.h>

	#include <mlib/fs_watcher.hpp>
	#include <mlib/libtorrent.hpp>

	#include "daemon_fs.hpp"
	#include "daemon_types.hpp"



	class Daemon_session: public Daemon_fs
	{
		private:
			typedef std::pair<Torrent_id, lt::entry> Torrent_resume_data;
			typedef std::map<Torrent_id, Torrent> Torrents_container;
			
			#if 0
				/// Задача на переименование файлов торрента.
				struct Rename_files_task
				{
					Rename_files_task(const Torrent_id& torrent_id, const std::vector<Torrent_file>& new_paths)
					:
						torrent_id(torrent_id),
						new_paths(new_paths)
					{
					}


					Torrent_id					torrent_id;
					std::vector<Torrent_file>	new_paths;
				};

				/// Результат выполнения операции по переименованию файла торрента.
				struct Rename_file_reply
				{
					enum Type { OK, FAILED};


					Rename_file_reply(void)
					{
					}

					Rename_file_reply(Type type, const Torrent_id& torrent_id, int file_index, const std::string& error = "")
					:
						type(type),
						torrent_id(torrent_id),
						file_index(file_index),
						error(error)
					{
					}


					Type				type;
					Torrent_id			torrent_id;
					int					file_index;
					std::string			error;
				};
			#endif


		public:
			Daemon_session(const std::string& config_dir_path);
			~Daemon_session(void);


		public:
			/// Сигнал, генерируемый при получении сообщений,
			/// приходящих от libtorrent.
			Glib::Dispatcher				messages_signal;

		private:
			/// Сигнал для остановки потоков.
			bool							is_stop;

			/// Мьютекс для синхронизации потоков.
			///
			/// Блокирует доступ к:
			/// * torrents_resume_data
			/// * finished_torrents
			/// * messages
			boost::mutex					mutex;


			/// Очередь сообщений libtorrent для выдачи клиенту.
			std::deque<Daemon_message>		messages;

			/// Поток для получения сообщений от libtorrent.
			boost::thread*					messages_thread;


			#if 0
				/// Сигнал на приостановку асинхронной файловой системы.
				sigc::connection				async_fs_paused_connection;

				/// Очередь задач на переименование файлов торрентов.
				std::queue<Rename_files_task>	rename_files_tasks;

				/// Сигнал на завершение обработки задачи по переименованию файла
				/// торрента libtorrent'ом.
				Glib::Dispatcher				torrent_file_renamed_signal;

				/// Результаты выполнения операций по переименованию файлов
				/// торрентов.
				std::queue<Rename_file_reply>	renamed_files_replies;
			#endif


			/// Количество resume data, которые были запрошены у libtorrent, но
			/// еще не были получены и обработаны.
			int								pending_torrent_resume_data;

			/// resume data, которые были получены от libtorrent, но еще не
			/// обработаны.
			std::queue<Torrent_resume_data>	torrents_resume_data;

			/// Сигнал на получение очередной resume data от libtorrent.
			Glib::Dispatcher				torrent_resume_data_signal;


			/// Сигнал на завершение скачивания торрента.
			Glib::Dispatcher				torrent_finished_signal;

			/// Очередь завершенных торрентов, которую необходимо
			/// обработать.
			std::deque<lt::torrent_handle>	finished_torrents;


			/// Сигнал на автоматическое сохранение текущей сессии.
			sigc::connection				save_session_connection;


			/// Время последнего обновления статистики.
			time_t							last_update_torrent_statistics_time;

			/// Сигнал на обновление статистической информации о торрентах.
			sigc::connection				update_torrents_statistics_connection;


			/// Объект, с помощью которого производятся наблюдения за
			/// изменениями в файловой системе.
			m::Fs_watcher					fs_watcher;

			/// Сигнал на появление новых торрентов для автоматического
			/// добавления.
			sigc::connection				fs_watcher_connection;


			/// Текущая libtorrent сессия.
			lt::session*					session;

			/// Торренты текущей сессии.
			Torrents_container				torrents;

			/// Настройки текущей сессии.
			Daemon_settings_light			settings;

			/// Статистика по прошлым и текущей сессиям.
			Daemon_statistics				statistics;


		public:
			/// Добавляет в сессию новый торрент файл.
			///
			/// @param torrent_path - путь к файлу торрента.
			/// @param error_if_not_exists - определяет, является ли ошибкой,
			/// если файл торрента уже не существует.
			void						add_torrent(const std::string& torrent_path, const New_torrent_settings& torrent_settings, bool error_if_not_exists = true) throw(m::Exception);

			/// Записывает в messages сообщения демона, если они есть.
			void						get_messages(std::deque<Daemon_message>& messages);

			/// Возвращает текущее ограничение на скорость.
			Speed						get_rate_limit(Traffic_type traffic_type) const;

			/// Возвращает информацию о текущей сессии.
			Session_status				get_session_status(void) const throw(m::Exception);

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
			/// @return - валидный Torrent_id, если торрент был добавлен в
			/// конфиг. Если торрент не был добавлен в конфиг, и это не является
			/// ошибкой, то возвращает невалидный Torrent_id. Это может
			/// произойти, к примеру, когда добавляется торрент, который уже
			/// присутствует в сессии, но в настройках было указано, что
			/// добавление торрента-дубликата не является ошибкой.
			Torrent_id					add_torrent_to_config(const std::string& torrent_path, const New_torrent_settings& new_torrent_settings, bool error_if_not_exists) const throw(m::Exception);

			/// Добавляет торрент к сессии.
			void						add_torrent_to_session(lt::torrent_info torrent_info, const Torrent_settings& torrent_settings);

			/// Производит все необходимые действия по завершению скачивания
			/// торрента.
			void						finish_torrent(Torrent& torrent);

			/// Возвращает ссылку на торрент по его идентификатору.
			const Torrent&				get_torrent(const Torrent_id& torrent_id) const throw(m::Exception);

			/// Возвращает ссылку на торрент по его идентификатору.
			Torrent&					get_torrent(const Torrent_id& torrent_id) throw(m::Exception);

			/// Возвращает ссылку на торрент по его хэндлу.
			const Torrent&				get_torrent(const lt::torrent_handle& torrent_handle) const throw(m::Exception);

			/// Возвращает ссылку на торрент по его хэндлу.
			Torrent&					get_torrent(const lt::torrent_handle& torrent_handle) throw(m::Exception);

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

			/// Проверяет, существует ли в текущей сессии торрент с таким идентификатором.
			bool						is_torrent_exists(const Torrent_id& torrent_id) const;

			/// Приостанавливает работу с торрентом.
			void						pause_torrent(Torrent& torrent);

			/// Удаляет торрент.
			void						remove_torrent(const Torrent_id& torrent_id) throw(m::Exception);

			/// Удаляет конфигурационные файлы торрента.
			void						remove_torrent_from_config(const Torrent_id& torrent_id) const throw(m::Exception);

			/// Удаляет торрент из текущей сессии.
			void						remove_torrent_from_session(const Torrent_id& torrent_id) throw(m::Exception);

			/// Удаляет торрент вместе с данными (скачанными файлами).
			void						remove_torrent_with_data(const Torrent_id& torrent_id) throw(m::Exception);

			/// Возобновляет работу с торрентом.
			void						resume_torrent(Torrent& torrent) throw(m::Exception);

			/// Сохраняет настройки всех торрентов текущей сессии.
			void						save_session(void) throw(m::Exception);

			/// Устанавливает параметры копирования файлов торрента по завершении их скачивания.
			void						set_copy_when_finished(Torrent& torrent, bool copy, const std::string& to) throw(m::Exception);

			/// Устанавливает флаг скачивания для файлов с идентификаторами files_ids.
			void						set_files_download_status(Torrent& torrent, const std::vector<int>& files_ids, bool download) throw(m::Exception);

#if 0
			/// Устанавливает новые пути для файлов.
			void						set_files_new_paths(const Torrent& torrent, const std::vector<Torrent_file>& files_new_paths) throw(m::Exception);
#endif

			/// Устанавливает приоритет для файлов, идентификторы которых содержатся в списке files_id.
			void						set_files_priority(Torrent& torrent, const std::vector<int>& files_ids, const Torrent_file_settings::Priority priority) throw(m::Exception);

			/// Устанавливает, режим скачивания торрента - последовательный или
			/// в зависимости от доступности частей торрента.
			void						set_sequential_download(Torrent& torrent, bool value) throw(m::Exception);

			/// Задает список трекеров торрента.
			void						set_torrent_trackers(Torrent& torrent, const std::vector<std::string>& trackers);

			/// Запускает сессию.
			void						start(void);

		private:
			/// Выполняет все необходимые действия по автоматизации некоторых
			/// операций. К примеру, если пришло время удалить старые торренты,
			/// то удаляет их.
			void						automate(void);

			/// Выполняет все необходимые действия для автоматической загрузки
			/// торрента torrent_path, если он является торрентом.
			void						auto_load_if_torrent(const std::string& torrent_path) throw(m::Exception);

			/// Загружает торренты, которые находятся в данный момент в очереди
			/// на автоматическую подгрузку.
			void						auto_load_torrents(void);

			/// Возвращает список трекеров торрента.
			std::vector<std::string>	get_torrent_trackers(const Torrent& torrent) const;

			/// Определяет, включен ли DHT или нет.
			bool						is_dht_started(void) const;

			/// Загружает торрент в сессию по его идентификатору.
			void						load_torrent(const Torrent_id& torrent_id) throw(m::Exception);

			/// Загружает торренты из директории для автоматической загрузки
			/// торрентов, если это необходимо.
			void						load_torrents_from_auto_load_dir(void) throw(m::Exception);

			/// Загружает все сохраненные торренты.
			/// Запускается при старте демона, чтобы добавить
			/// все торренты из прошлой сессии.
			void						load_torrents_from_config(void) throw(m::Exception);

#if 0
			// Обработчик сигнала на приостановку асинхронной файловой системы.
			void						on_async_fs_paused_callback(void);
#endif

			/// Обработчик сигнала на автоматическое сохранение текущей сессии.
			bool						on_save_session_callback(void);

#if 0
			/// Обработчик сигнала на получение ответа от libtorrent на
			/// переименование файла торрента.
			void						on_torrent_file_rename_reply_callback(void);
#endif

			/// Обработчик сигнала на завершение скачивания торрента.
			void						on_torrent_finished_callback(void);

			/// Обработчик сигнала на получение очередной resume data от libtorrent.
			void						on_torrent_resume_data_callback(void);

			/// Обработчик сигнала на обновления статистической информации
			/// о торрентах.
			bool						on_update_torrents_statistics_callback(void);

#if 0
			/// Произоводит всю необходимую работу по обработке задачи на
			/// переименование файлов торрента.
			void						process_rename_files_task(const Rename_files_task& task);
#endif

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

