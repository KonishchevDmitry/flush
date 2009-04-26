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


#ifndef HEADER_DAEMON_SETTINGS
	#define HEADER_DAEMON_SETTINGS

	#include <string>
	#include <deque>

	#include <libtorrent/entry.hpp>



	/// Облегченная структура для хранения тех настроек, которые невозможно
	/// получить посредством libtorrent.
	class Daemon_settings_light
	{
		public:
			class Torrents_auto_load
			{
				public:
					Torrents_auto_load(void);


				public:
					/// Нужно ли автоматически загружать торренты.
					bool				is;

					/// Директория, из которой требуется автоматически загружать
					/// торренты.
					std::string			from;

					/// Директория, в которую необходимо скачивать торренты.
					std::string			to;


					/// Нужно ли копировать торренты по завершении скачивания.
					bool				copy;

					/// Директория, в которую необходимо копировать торренты по
					/// завершении скачивания.
					std::string			copy_to;


					/// Удалять обработанные *.torrent файлы из директории.
					bool				delete_loaded;


				public:
					/// Проверяет настройки на эквивалентность по отношению к
					/// fs_watcher.
					bool	equal(const Torrents_auto_load& auto_load) const;
			};


		public:
			Daemon_settings_light(void);


		public:
			/// Диапазон портов для прослушивания.
			std::pair<int, int>		listen_ports_range;


			// Включен ли LSD.
			bool					lsd;

			// Включен ли UPnP.
			bool					upnp;

			// Включен ли NAT-PMP.
			bool					natpmp;

			// Включен ли Smart Ban.
			bool					smart_ban;

			// Включен ли Peer Exchange.
			bool					pex;


			/// Максимальное количество соединений для раздачи,
			/// которое может быть открыто.
			int						max_uploads;

			/// Максимальное количество соединений, которое может
			/// быть открыто.
			int						max_connections;


			/// Настройки автоматической загрузки торрентов из директории.
			Torrents_auto_load		torrents_auto_load;


			/// Удалять старые торренты или нет.
			bool					auto_delete_torrents;

			/// Удалять старые торренты вместе с данными или нет.
			bool					auto_delete_torrents_with_data;

			/// Максимальное время жизни торрента (c).
			time_t					auto_delete_torrents_max_seed_time;

			/// Максимальный рейтинг торрента.
			Share_ratio				auto_delete_torrents_max_share_ratio;

			/// Максимальное количество раздающих торрентов.
			int						auto_delete_torrents_max_seeds;

		private:
			/// Начальный порт по умолчанию для диапазона прослушиваемых
			/// портов.
			static
			const int				default_listen_start_port;

			/// Конечный порт по умолчанию для диапазона прослушиваемых
			/// портов.
			static
			const int				default_listen_end_port;
	};



	/// Хранит все текущие настройки демона. Используется для передачи
	/// настроек от демона к клиенту и обратно.
	class Daemon_settings: public Daemon_settings_light
	{
		public:
			Daemon_settings(void);
			Daemon_settings(const Daemon_settings_light& settings);


		public:
			/// Прослушиваемый в данный момент порт.
			int					listen_port;


			/// Включен ли DHT.
			bool				dht;

			/// DHT сессия.
			lt::entry			dht_state;


			/// Ограничение на скорость скачивания (КБ/с).
			Speed				download_rate_limit;

			/// Ограничение на скорость отдачи (КБ/с).
			Speed				upload_rate_limit;

		private:
			/// Имя конфигурационного файла.
			static
			const std::string	config_file_name;

			/// Имя файла c сессией DHT.
			static
			const std::string	dht_state_file_name;


		public:
			/// Считывает все настройки из конфига.
			void				read(const std::string& config_dir_path, Daemon_statistics* statistics) throw(m::Exception);

			/// Записывает все настройки в конфиг.
			void				write(const std::string& config_dir_path, const Session_status& session_status) const throw(m::Exception);

		private:
			void				assign(const Daemon_settings_light &settings);

			/// Считывает настройки автоматической подгрузки торрентов из директории.
			void				read_auto_load_settings(const libconfig::Setting& group_setting);

			/// Считывает настройки из конфигурационного файла.
			void				read_config(const std::string& config_path, Daemon_statistics* statistics) throw(m::Exception);

			/// Считывает из конфика сессию DHT.
			void				read_dht_state(std::string dht_state_path) throw(m::Exception);

			/// Сохраняет настройки в конфигурационный файл.
			void				write_config(const std::string& config_path, const Session_status& session_status) const throw(m::Exception);

			/// Записывает DHT сессию в конфиг.
			void				write_dht_state(const std::string& dht_state_path) const throw(m::Exception);


		public:
			Daemon_settings&	operator=(const Daemon_settings_light &settings);
	};



	class Torrent_settings
	{
		public:
			/// Конструктор для новых торрентов.
			Torrent_settings(
				const std::string& name,
				bool paused,
				const std::string& download_path,
				const Download_settings& download_settings,
				const std::vector<Torrent_file_settings>& files_settings,
				const std::vector<std::string>& trackers
			);

			/// Конструктор для получения настроек торрента, присутствующего
			/// в текущей сессии.
			Torrent_settings(const Torrent& torrent, const lt::entry& resume_data);


		public:
			/// Имя торрента или "", если необходимо использовать имя по
			/// умолчанию.
			std::string							name;

			/// Время, когда торрент был добавлен в сессию.
			time_t								time_added;

			/// Время в секундах, в течении которого торрент
			/// находился в состоянии seeding.
			time_t								time_seeding;

			/// Определяет, приостановлен торрент или нет.
			bool								is_paused;

			/// Директория, в которую будут скачиваться файлы торрента.
			std::string							download_path;

			/// Параметры скачивания торрента.
			Download_settings					download_settings;

			/// Настройки файлов торрента.
			std::vector<Torrent_file_settings>	files_settings;

			/// Данные для восстановления закачки торрента с того места,
			/// на котором она была прервана ранее.
			lt::entry							resume_data;

			/// Трекеры, с которыми необходимо осуществлять работу.
			std::vector<std::string>			trackers;

			/// Общий объем полученной информации.
			Size								total_download;

			/// Общий объем скачанных данных (полезная нагрузка).
			Size								total_payload_download;

			/// Общий объем отправленной информации.
			Size								total_upload;

			/// Общий объем отправленных данных (полезная нагрузка).
			Size								total_payload_upload;

			/// Общий объем скачанных данных, которые оказались поврежденными (мусор).
			Size								total_failed;

			/// Общий объем данных, которые были приняты от пиров, несмотря на то, что
			/// уже были скачаны. Это может возникнуть из-за задержек в работе сети.
			Size								total_redundant;

		private:
			/// Имя конфигурационного файла.
			static
			const std::string					config_file_name;

			/// Имя файла c resume data.
			static
			const std::string					resume_data_file_name;


		public:
			/// Читает все необходимые настройки.
			void read(const std::string& settings_dir_path) throw(m::Exception);

			/// Записывает все необходимые настройки.
			void write(const std::string& settings_dir_path) const throw(m::Exception);

		private:
			/// Читает настройки из конфигурационного файла торрента.
			void read_config(const std::string& config_path) throw(m::Exception);

			/// Читает resume data торрента.
			void read_resume_data(std::string resume_data_path) throw(m::Exception);

			/// Записывает настройки в конфигурационный файл торрента.
			void write_config(const std::string& config_path) const throw(m::Exception);

			/// Записывает resume data торрента.
			void write_resume_data(const std::string& resume_data_path) const throw(m::Exception);
	};

#endif

