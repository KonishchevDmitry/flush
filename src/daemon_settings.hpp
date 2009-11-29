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

	#ifndef MLIB_ENABLE_LIBS_FORWARDS
		#include <libconfig.h++>
	#endif

	#include <libtorrent/entry.hpp>

	#include "common.hpp"


	/// Облегченная структура для хранения тех настроек, которые невозможно
	/// (или не хочется) получать посредством libtorrent.
	class Daemon_settings_light
	{
		public:
			/// Настройки автоматической загрузки торрентов из директории.
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


			/// Настройки автоматической очистки от старых торрентов.
			class Auto_clean
			{
				public:
					Auto_clean(void);


				public:
					/// Максимальное время жизни торрента (c).
					time_t				max_seeding_time;
					Auto_clean_type		max_seeding_time_type;

					/// Максимальный рейтинг торрента.
					Share_ratio			max_ratio;
					Auto_clean_type		max_ratio_type;

					/// Максимальное количество раздающих торрентов.
					ssize_t				max_seeding_torrents;
					Auto_clean_type		max_seeding_torrents_type;


				public:
					bool	operator!=(const Auto_clean& clean) const;
			};


		public:
			Daemon_settings_light(void);


		public:
			/// Используется ли случайный порт, или диапазон портов, заданный
			/// пользователем.
			bool							listen_random_port;

			/// Диапазон портов для прослушивания.
			std::pair<int, int>				listen_ports_range;


			// Включен ли LSD.
			bool							lsd;

			// Включен ли UPnP.
			bool							upnp;

			// Включен ли NAT-PMP.
			bool							natpmp;

			// Включен ли Smart Ban.
			bool							smart_ban;

			// Включен ли Peer Exchange.
			bool							pex;


			/// Максимальное количество соединений для раздачи,
			/// которое может быть открыто.
			int								max_uploads;

			/// Максимальное количество соединений, которое может
			/// быть открыто.
			int								max_connections;


			/// Нужно ли использовать максимальный интервал между запросами
			/// пиров у трекера.
			bool							use_max_announce_interval;

			/// Максимальный интервал между запросами пиров у трекера.
			Time							max_announce_interval;


			/// IP фильтр.
			std::vector<Ip_filter_rule>		ip_filter;


			/// Настройки автоматической загрузки торрентов из директории.
			Torrents_auto_load				torrents_auto_load;

			/// Настройки автоматической очистки от старых торрентов.
			Auto_clean						torrents_auto_clean;
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
			/// @throw - m::Exception.
			void				read(const std::string& config_dir_path, Daemon_statistics* statistics);

			/// Записывает все настройки в конфиг.
			/// @throw - m::Exception.
			void				write(const std::string& config_dir_path, const Session_status& session_status) const;

		private:
			void				assign(const Daemon_settings_light &settings);

			/// Считывает настройки автоматической "очистки" от старых торрентов.
			void				read_auto_clean_settings(const libconfig::Setting& setting);

			/// Считывает настройки автоматического удаления торрентов.
			void				read_auto_delete_settings(const libconfig::Setting& root);

			/// Считывает настройки автоматической подгрузки торрентов из директории.
			void				read_auto_load_settings(const libconfig::Setting& group_setting);

			/// Считывает настройки из конфигурационного файла.
			/// @throw - m::Exception.
			void				read_config(const std::string& config_path, Daemon_statistics* statistics);

			/// Считывает из конфика сессию DHT.
			/// @throw - m::Exception.
			void				read_dht_state(std::string dht_state_path);

			/// Считывает настройки IP фильтра.
			void				read_ip_filter_settings(const libconfig::Setting& filter_setting);

			/// Сохраняет настройки в конфигурационный файл.
			/// @throw - m::Exception.
			void				write_config(const std::string& config_path, const Session_status& session_status) const;

			/// Записывает DHT сессию в конфиг.
			/// @throw - m::Exception.
			void				write_dht_state(const std::string& dht_state_path) const;


		public:
			Daemon_settings&	operator=(const Daemon_settings_light &settings);
	};



	class Torrent_settings
	{
		public:
			typedef int Read_flags;

			enum Read_flag {
				/// Список трекеров прочитан из конфига
				READ_FLAG_TRACKERS_GOTTEN
			};


		public:
			/// Конструктор для новых торрентов.
			Torrent_settings(const std::string& download_path);

			/// Конструктор для новых торрентов.
			Torrent_settings(
				const std::string& magnet,
				const std::string& name,
				bool paused,
				const std::string& download_path,
				const Download_settings& download_settings,
				const std::string& encoding,
				const std::vector<Torrent_file_settings>& files_settings,
				const std::vector<std::string>& trackers
			);

			/// Конструктор для получения настроек торрента, присутствующего
			/// в текущей сессии.
			Torrent_settings(const Torrent& torrent, const lt::entry& resume_data);


		public:
			/// Magnet-ссылка или "", если у торрента ее нет.
			std::string							magnet;

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

			/// Кодировка *.torrent файла.
			std::string							encoding;

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


			/// Размер скачанных данных.
			Size								bytes_done;

			/// Размер скачанных данных, который был в последний раз, когда
			/// libtorrent сгенерировал для торрента torrent_finished_alert.
			Size								bytes_done_on_last_torrent_finish;


		private:
			/// Имя конфигурационного файла.
			static
			const std::string					config_file_name;

			/// Имя файла c resume data.
			static
			const std::string					resume_data_file_name;


		public:
			/// Читает все необходимые настройки и по мере чтения записывает в
			/// flags информацию о прочитанных данных.
			/// @throw - m::Exception.
			void		read(const std::string& settings_dir_path, Read_flags* flags);

			/// Записывает все необходимые настройки.
			/// @throw - m::Exception.
			void		write(const std::string& settings_dir_path) const;

		private:
			/// Читает настройки из конфигурационного файла торрента и по мере
			/// чтения записывает в flags информацию о прочитанных данных.
			/// @throw - m::Exception.
			void		read_config(const std::string& config_path, Read_flags* flags);

			/// Считывает конфигурационный файл, возвращая описывающую его
			/// структуру.
			/// @throw - m::Exception.
			static void	read_config_data(libconfig::Config* config, const std::string& config_path);

			/// Читает resume data торрента.
			/// @throw - m::Exception.
			void		read_resume_data(std::string resume_data_path);

			/// Записывает настройки в конфигурационный файл торрента.
			/// @throw - m::Exception.
			void		write_config(const std::string& config_path) const;

			/// Записывает resume data торрента.
			/// @throw - m::Exception.
			void		write_resume_data(const std::string& resume_data_path) const;
	};

#endif

