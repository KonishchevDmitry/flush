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

#ifndef HEADER_COMMON
	#define HEADER_COMMON

	#ifndef MLIB_ENABLE_LIBS_FORWARDS
		#include <gtkmm/stockid.h>

		#include <libtorrent/alert.hpp>
		#include <libtorrent/peer_id.hpp>
		#include <libtorrent/peer_info.hpp>
		#include <libtorrent/session_status.hpp>
		#include <libtorrent/torrent_handle.hpp>
	#endif

	#include <mlib/libtorrent.hxx>
	#include <mlib/main.hpp>


	/// Этим символом будет помечаться весь код, который написан
	/// для совместимости с предыдущими версиями - просто, чтобы
	/// его было легко найти.
	#define COMPATIBILITY

	/// Год, в котором было написано приложение.
	#define APP_YEAR 2009

	/// Числовое представление версии приложения.
	#define APP_VERSION M_GET_VERSION(APP_MAJOR_VERSION, APP_MINOR_VERSION, APP_SUB_MINOR_VERSION)

	// Пути, используемые в DBus.
	#define DBUS_NAME_PREFIX "com.blogspot.konishchevdmitry.flush."
	#define DBUS_PATH "/com/blogspot/konishchevdmitry/flush"

	/// Имя директории с конфигурационными файлами по умолчанию.
	#define DEFAULT_CONFIG_DIR_NAME ( "." APP_UNIX_NAME )


	using m::lt::Torrent_file;


	#if M_LT_GET_MAJOR_MINOR_VERSION() < M_GET_VERSION(0, 14, 0)
		#error You are using too old version of libtorrent-rasterbar. Please install libtorrent-rasterbar >= 0.14.
	#endif


	// Объявления типов для ускорения компиляции -->
		class Application;
		class Client;
		class Client_cmd_options;
		class Client_fs;
		class Client_settings;
		class Daemon;
		class Daemon_fs;
		class Daemon_proxy;
		class Daemon_settings;
		class Daemon_statistics;
		class Ip_filter_rule;
		class Ip_filter;
		class Log_view;
		class Main_window;
		class Settings_window;
		class Torrent;
		class Torrent_details_view;
		class Torrent_files_view;
		class Torrent_files_dynamic_view;
		class Torrent_files_static_view;
		class Torrent_options_view;
		class Torrent_peers_view;
		class Torrent_id;
		class Torrent_settings;
		class Torrents_view;
		class Torrents_viewport;
		class Trackers_view;
		class User_settings;
	// Объявления типов для ускорения компиляции <--


	/// Тип графического сообщения.
	enum Message_type { WARNING, INFO, ERROR };

	/// Хранит соотношение Upload/Download.
	/// Нельзя делать long double или Size_float, т. к. Share_ratio
	/// используется в GTK моделях, которые не поддерживают long double.
	typedef double Share_ratio;


	/// Хранит номер ревизии чего-либо.
	typedef unsigned int Revision;
	#define INIT_REVISION	0
	#define FIRST_REVISION	1



	/// "Временное действие" которое необходимо выполнить для торрента.
	enum Temporary_action
	{
		TEMPORARY_ACTION_NONE,
		TEMPORARY_ACTION_RESUME,
		TEMPORARY_ACTION_PAUSE
	};


	/// Определяет множество торрентов.
	enum Torrents_group { NONE, ALL, DOWNLOADS, UPLOADS };


	/// Тип трафика
	enum Traffic_type { DOWNLOAD, UPLOAD };


	/// Определяет действие, которое необходимо совершить над торрентом.
	enum Torrent_process_action
	{
		// Должны быть степенями двойки, чтобы можно было использовать, как
		// флаги.
		PAUSE				= 1 << 0,
		RESUME				= 1 << 1,
		RECHECK				= 1 << 2,
		REMOVE				= 1 << 3,
		REMOVE_WITH_DATA	= 1 << 4
	};

	/// Определяет множество действий, которые
	/// можно совершить над торрентом(ами).
	typedef int Torrent_process_actions;



	/// Определяет способ "очистки" старых торрентов.
	class Auto_clean_type
	{
		public:
			/// Методы очистки располагаются в порядке возрастания их
			/// "строгости".
			enum Type {
				NONE,
				PAUSE,
				REMOVE,
				REMOVE_WITH_DATA
			};


		public:
			Auto_clean_type(void);
			Auto_clean_type(const Type type);


		public:
			Type	type;


		public:
			/// Получает тип из строки.
			/// @throw - m::Exception.
			static Auto_clean_type	from_string(const std::string& string);

			/// Изменяет на тип type, если он является более строгим, чем
			/// текущий.
			void					set_if_stricter(const Auto_clean_type& type);

			/// Преобразовывает тип в StockID.
			Gtk::StockID			to_stock_id(void) const;

			/// Преобразовывает тип в строку.
			std::string				to_string(void) const;

		public:
			/// Переключает тип на следующий по списку.
			Auto_clean_type& 	operator++(void);

								operator bool(void) const;
	};



	/// Хранит сообщение, полученное от демона (libtorrent).
	class Daemon_message
	{
		public:
			/// Тип сообщения.
			enum Type { INFO, WARNING };


		public:
			Daemon_message(const lt::alert& alert);


		private:
			/// Тип сообщения.
			Type				type;

			/// Текст сообщения.
			std::string			message;


		public:
			/// Возвращает тип сообщения.
			inline
			Type				get_type(void) const;

			/// Возвращает текст сообщения.
			inline
			const std::string&	get(void) const;

			/// Задает всю необходимую информацию, которая должна храниться в объекте.
			void				set(const lt::alert& alert);

		public:
			inline
			operator 			const std::string&(void) const;
	};



	/// Параметры скачивания торрента. Облегченная версия. Хранит только те
	/// настройки, которые невозможно получить посредством libtorrent.
	class Download_settings_light
	{
		public:
			Download_settings_light(void);
			Download_settings_light(bool copy_when_finished, const std::string& copy_when_finished_to);


		public:
			/// Определяет, нужно ли по завершении скачивания торрента
			/// скопировать его файлы в директорию copy_when_finished_to.
			bool			copy_when_finished;

			/// Директория, в которую будут скопированы файлы торрента
			/// после того, как он скачается, если это необходимо.
			std::string		copy_when_finished_to;
	};



	/// Параметры скачивания торрента.
	class Download_settings: public Download_settings_light
	{
		public:
			Download_settings(void);
			Download_settings(const std::string& copy_when_finished_to);
			Download_settings(const Download_settings_light& light_settings, bool sequential_download);


		public:
			/// Определяет, нужно ли произоводить последовательное скачивание
			/// или нет.
			bool			sequential_download;
	};



	/// Используется для идентификации торрента.
	/// Хранит контрольную сумму торрента.
	class Torrent_id: public std::string
	{
		public:
			inline
			Torrent_id(void);

			inline
			Torrent_id(const std::string& hash);

			inline
			Torrent_id(const Glib::ustring& hash);

			Torrent_id(const lt::sha1_hash& hash);

			Torrent_id(const lt::torrent_handle& handle);


		public:
			/// Возвращает хэш торрента.
			lt::sha1_hash		hash(void) const;

			/// Возвращает true, если идентификатор является
			/// недействительным (равным Torrent_id()).
			bool				is_invalid(void) const;

		public:
			Torrent_id&		operator=(const lt::sha1_hash& hash);
			bool			operator==(const Torrent_id& torrent_id) const;
			bool			operator!=(const Torrent_id& torrent_id) const;

			/// Возвращает true, если идентификатор является
			/// недействительным (равным Torrent_id()).
							operator bool(void) const;
	};

	/// Для работы Glib::ustring::compose.
	std::wostream&	operator<<(std::wostream& stream, const Torrent_id& torrent_id);


	/// Используется для идентификации торрента.
	/// Однозначно идентифицирует торрент в пределах текущей сессии - если за
	/// время существования сессии в ней были торренты с одинаковыми Torrent_id,
	/// то Torrent_full_id у них всегда будут разными.
	class Torrent_full_id
	{
		public:
			inline
			Torrent_full_id(const Torrent_id& id, size_t serial_number);


		public:
			Torrent_id	id;
			size_t		serial_number;
	};


	/// Абстрактный класс, от которого наследуются все
	/// информационные виджеты.
	class Info_widget
	{
		public:
			virtual ~Info_widget(void) {}

		public:
			/// Вызывается родительским классом, когда пользователь
			/// выделяет торрент в Torrents_view.
			virtual void	torrent_changed(const Torrent_id& torrent_id) = 0;

			/// Вызывается родительским классом, когда наступает время
			/// обновления графического интерфейса.
			virtual void	update(const Torrent_id& torrent_id) = 0;
	};



	/// Представляет из себя одно правило IP-фильтра.
	class Ip_filter_rule
	{
		public:
			Ip_filter_rule(void);
			Ip_filter_rule(const std::string& from, const std::string& to, bool block);


		public:
			/// Начальный IP адрес.
			std::string	from;

			/// Конечный IP адрес.
			std::string	to;

			/// Блокировать, или нет.
			bool		block;


		public:
			/// Проверяет фильтр на ошибки.
			/// @throw - m::Exception.
			void	check(void) const;


		public:
			bool	operator==(const Ip_filter_rule& rule) const;
			bool	operator!=(const Ip_filter_rule& rule) const;
	};



	/// Абстрактный класс, от которого наследуются все информационные
	/// виджеты, отображающие информацию об отдельно взятом торренте.
	class Torrent_info_widget: public Info_widget
	{
		public:
			virtual ~Torrent_info_widget(void) {}


		public:
			/// Вызывается родительским классом, когда пользователь
			/// выделяет торрент в Torrents_view.
			virtual inline
			void	torrent_changed(const Torrent_id& torrent_id);
	};



	/// Абстрактный класс, от которого наследуются все информационные
	/// виджеты, отображающие информацию не об отдельно взятом торренте,
	/// а обо всей сессии в целом.
	class Session_info_widget: public Info_widget
	{
		public:
			virtual ~Session_info_widget(void) {}


		public:
			/// Вызывается родительским классом, когда пользователь
			/// выделяет торрент в Torrents_view.
			virtual inline
			void	torrent_changed(const Torrent_id& torrent_id);
	};



	/// Информация о текущей сессии.
	class Session_status
	{
		public:
			Session_status(
				const Daemon_statistics& daemon_statistics,
				const lt::session_status& libtorrent_session_status,
				Speed download_rate_limit, Speed upload_rate_limit,
				bool temporary_action_active
			);


		public:
			/// Время начала текущей сессии.
			Time	session_start_time;

			/// Время начала сбора статистической информации.
			Time	statistics_start_time;


			/// Ограничение на скорость скачивания (КБ/с).
			Speed	download_rate_limit;

			/// Ограничение на скорость отдачи (КБ/с).
			Speed	upload_rate_limit;


			/// Скорость скачивания (включая служебную информацию).
			Speed	download_speed;

			/// Скорость скачивания (только полезная нагрузка).
			Speed	payload_download_speed;

			/// Скорость отдачи (включая служебную информацию).
			Speed	upload_speed;

			/// Скорость отдачи (только полезная нагрузка).
			Speed	payload_upload_speed;

			/// Объем полученной информации за эту сессию.
			Size	download;

			/// Объем скачанных данных за эту сессию (полезная нагрузка).
			Size	payload_download;

			/// Общий объем полученной информации.
			Size	total_download;

			/// Общий объем скачанных данных (полезная нагрузка).
			Size	total_payload_download;

			/// Объем отправленной информации за эту сессию.
			Size	upload;

			/// Объем отправленных данных за эту сессию (полезная нагрузка).
			Size	payload_upload;

			/// Общий объем отправленной информации.
			Size	total_upload;

			/// Общий объем отправленных данных (полезная нагрузка).
			Size	total_payload_upload;

			/// Объем скачанных данных, которые оказались поврежденными (мусор).
			Size	failed;

			/// Общий объем скачанных данных, которые оказались поврежденными (мусор).
			Size	total_failed;

			/// Объем данных, которые были приняты от пиров, несмотря на то, что
			/// уже были скачаны. Это может возникнуть из-за задержек в работе сети.
			Size	redundant;

			/// Общий объем данных, которые были приняты от пиров, несмотря на то, что
			/// уже были скачаны. Это может возникнуть из-за задержек в работе сети.
			Size	total_redundant;


			/// Обрабатывается ли в данный момент какое-либо временное
			/// действие.
			bool	temporary_action_active;
	};



	struct Torrent_file_settings
	{
		/// Приоритет файла торрента.
		///
		/// Внимание!
		/// При добавлении новых приоритетов согласовать изменения с методами:
		///  * get_priority_localized_name
		///  * get_priority_name
		///  * set_priority_by_name
		enum Priority { NORMAL, HIGH };


		inline
		Torrent_file_settings(void);

		inline
		Torrent_file_settings(bool download, Priority priority);


		/// Путь к файлу, который задал пользователь, или пустая строка, если
		/// путь изменять не надо.
		std::string	path;

		/// Нужно скачивать файл или нет.
		bool		download;

		/// Приоритет скачивания файла.
		Priority	priority;


		/// Преобразовывает текущие настройки в приоритет файла для последующей
		/// передачи в libtorrent.
		int			convert_to_libtorrent_prioritry(void);

		/// Возвращает приоритет по умолчанию.
		static inline
		Priority	get_default_priority(void);

		/// Возвращает имя приоритета по умолчанию.
		static inline
		std::string	get_default_priority_localized_name(void);

		/// Возвращает имя приоритета по умолчанию.
		static inline
		std::string	get_default_priority_name(void);

		/// Возвращает приоритет по его имени.
		/// @throw - m::Exception.
		static
		Priority	get_priority_by_name(const std::string& name);

		/// Возвращает строковое представление приоритета в соответствии с
		/// текущей локалью.
		static
		std::string	get_priority_localized_name(Priority);

		/// Возвращает строковое представление приоритета в соответствии с
		/// текущей локалью.
		inline
		std::string	get_priority_localized_name(void) const;

		/// Возвращает строковое представление приоритета.
		static
		std::string	get_priority_name(Priority priority);

		/// Возвращает строковое представление приоритета.
		inline
		std::string	get_priority_name(void) const;

		/// Устанавливает приоритет по его имени.
		/// @throw - m::Exception.
		inline
		void		set_priority_by_name(const std::string& name);
	};



	/// Текущее состояние файла торрента.
	struct Torrent_file_status: public Torrent_file_settings
	{
		inline
		Torrent_file_status(bool download, Priority priority, Size downloaded);

		inline
		Torrent_file_status(const Torrent_file_settings& settings, Size downloaded);


		/// Количество скачанных байт.
		Size	downloaded;
	};



	/// Текущая статистическая информация о торренте.
	class Torrent_info
	{
		public:
			enum Status
			{
				ALLOCATING,
				QUEUED_FOR_CHECKING,
				CHECKING_FILES,
				WAITING_FOR_METADATA_DOWNLOAD,
				DOWNLOADING_METADATA,
				WAITING_FOR_DOWNLOAD,
				DOWNLOADING,
				SEEDING,
				UPLOADING,
				/// Если в новых версиях появится новый статус
				UNKNOWN
			};


		public:
			Torrent_info(const Torrent& torrent);

		protected:
			Torrent_info(void);


		public:
			/// Бит, который всегда устанавливается в false. Используется
			/// функциями отображения информации из Torrent_info, чтобы
			/// определить, был ли уже обработан данный экземпляр.
			bool			processed;


			/// Идентификатор торрента.
			Torrent_id		id;

			/// Имя торрента.
			std::string		name;


			/// Приостановлена ли в данный момент работа с торрентом или нет.
			bool			paused;

			/// Операция, которая выполняется в данный момент с торрентом.
			Status			status;

			/// Степень завершенности выполняемой в данный момент операции (0 - 100).
			int				progress;


			/// Размер всех файлов торрента.
			Size			size;

			/// Размер всех файлов торрента, выбранных для скачивания.
			Size			requested_size;

			/// Объем скачанных данных (считается только объем тех данных, которые мы
			/// хотели скачивать).
			Size			downloaded_requested_size;


			/// Скорость скачивания (включая служебную информацию).
			Speed			download_speed;

			/// Скорость скачивания (только полезная нагрузка).
			Speed			payload_download_speed;

			/// Скорость отдачи (включая служебную информацию).
			Speed			upload_speed;

			/// Скорость отдачи (только полезная нагрузка).
			Speed			payload_upload_speed;


			/// Общий объем полученной информации.
			Size			total_download;

			/// Общий объем скачанных данных (полезная нагрузка).
			Size			total_payload_download;

			/// Общий объем отправленной информации.
			Size			total_upload;

			/// Общий объем отправленных данных (полезная нагрузка).
			Size			total_payload_upload;

			/// Общий объем скачанных данных, которые оказались поврежденными (мусор).
			Size			total_failed;

			/// Общий объем данных, которые были приняты от пиров, несмотря на то, что
			/// уже были скачаны. Это может возникнуть из-за задержек в работе сети.
			Size			total_redundant;


			/// Количество пиров, подключенных в данный момент к этому торренту.
			int				peers_num;

			/// Количество сидеров, с которых в данный момент скачивается торрент.
			int				seeds_num;


			/// Время, когда торрент был добавлен в сессию.
			Time			time_added;

			/// Время в секундах, в течении которого торрент
			/// находился в состоянии seeding.
			Time			time_seeding;


			/// Определяет, задан ли для данного торрента хотя бы один трекер.
			bool			trackers_exists;

			/// true, если попытка соединиться с трекером завершилась неудачей.
			bool			tracker_broken;

			/// Текущий трекер, или "", если в данный момент нет установленного
			/// соединения с трекером.
			std::string		current_tracker;


		public:
			/// Возвращает процент завершенности скачивания данных.
			int				get_complete_percent(void) const;

			/// Возвращает текущий рейтинг торрента.
			Share_ratio		get_share_ratio(void) const;

			/// Возвращает строку статуса.
			std::string		get_status_string(void) const;

			/// Возвращает время время, которое осталось до завершения
			/// скачивания торрента.
			Time			get_time_left(void) const;

		protected:
			/// Производит всю необходимую работу по конструированию объекта.
			void			init(const Torrent& torrent, const lt::torrent_status& torrent_status);

		private:
			/// Возвращает Status, соответствующий lt::torrent_status.
			Status			get_status(const lt::torrent_status& torrent_status) const;


		public:
			bool	operator!=(const Torrent_info& torrent_info) const;
	};



	/// Подробная информация о торренте.
	class Torrent_details: public Torrent_info
	{
		public:
			Torrent_details(const Torrent& torrent);

		public:

			/// Время, через которое libtorrent отправит очередной запрос
			/// трекеру.
			Time		next_announce;

			/// Интервал между запросами, который рекомендует использовать
			/// трекер.
			Time		announce_interval;

			/// URL, с которого был скачан торрент.
			std::string	publisher_url;

			/// Текущий статус трекера.
			std::string	tracker_status;
	};



	/// Информация о пире торрент файла.
	class Torrent_peer_info
	{
		public:
			Torrent_peer_info(const lt::peer_info& peer_info);


		public:
			/// IP адрес пира.
			std::string				ip;

			/// Клиент пира.
			std::string				client;


			/// Скорость скачивания (включая служебную информацию).
			Speed					download_speed;

			/// Скорость скачивания (только полезная нагрузка).
			Speed					payload_download_speed;

			/// Скорость отдачи (включая служебную информацию).
			Speed					upload_speed;

			/// Скорость отдачи (только полезная нагрузка).
			Speed					payload_upload_speed;


			/// Общий объем скачанных данных (полезная нагрузка).
			Size					total_payload_download;

			/// Общий объем отправленных данных (полезная нагрузка).
			Size					total_payload_upload;


			/// Количество скачанных данных у клиента.
			int						availability;

			/// Количество ошибок при проверке скачанных данных.
			int						hash_fails;
	};



	/// Описывает параметры торрента, добавляемого
	/// извне в сессию.
	class New_torrent_settings
	{
		public:
			New_torrent_settings(
				bool start,
				const std::string& download_path,
				const std::string& copy_on_finished_path,
				const std::string& encoding = MLIB_UTF_CHARSET_NAME,
				const std::vector<Torrent_file_settings> files_settings = std::vector<Torrent_file_settings>(),
				bool duplicate_is_error = true
			);

			New_torrent_settings(
				const std::string& name,
				bool start,
				const std::string& download_path,
				const std::string& copy_on_finished_path,
				const std::string& encoding = MLIB_UTF_CHARSET_NAME,
				const std::vector<Torrent_file_settings> files_settings = std::vector<Torrent_file_settings>(),
				bool duplicate_is_error = true
			);


		public:
			/// Имя торрента или "", если необходимо использовать имя по
			/// умолчанию.
			const std::string							name;

			/// Запускать торрент или нет.
			const bool									start;

			/// Директория, в которую будет производится
			/// скачивание.
			const std::string							download_path;

			/// Директория, в которую необходимо скопировать
			/// файлы торрента по завершении скачивания
			/// или "", если копировать файлы не нужно.
			const std::string							copy_on_finished_path;

			/// Кодировка *.torrent файла.
			const std::string							encoding;

			/// Настройки файлов торрента или пустой массив,
			/// если необходимо задать настройки по умолчанию.
			const std::vector<Torrent_file_settings>	files_settings;

			/// Является ли ошибкой, если такой торрент уже присутствует в
			/// сессии.
			const bool									duplicate_is_error;
	};



	/// Возвращает максимальную скорость, которая в libtorrent представляется в
	/// Б/с из КБ/с.
	inline
	Speed		get_lt_rate_limit(Speed rate);

	/// Возвращает максимальную скорость в КБ/с по максимальной скорости,
	/// которая в libtorrent представляется в Б/с.
	inline
	Speed		get_rate_limit_from_lt(Speed rate);

	/// Подсчитывает share ratio.
	/// Внимание! В случае, если download == 0, возвращает бесконечность.
	Share_ratio	get_share_ratio(Size upload, Size download);

	/// Подсчитывает share ratio.
	/// В отличие от get_share_ratio(Size upload, Size download), в случае,
	/// когда download == 0, расчитывает рейтинг так, как будто download ==
	/// size.
	Share_ratio	get_share_ratio(Size upload, Size download, Size size);

	/// Возвращает строковое представление share ratio.
	std::string	get_share_ratio_string(Share_ratio ratio, bool show_zero_values = true);

	/// Возвращает строковое представление share ratio.
	std::string	get_share_ratio_string(Size upload, Size download, Size size = 0, bool show_zero_values = true);

	/// Возвращает имя трекера по его URL'у.
	/// Под именем в данный момент понимается домен второго уровня.
	std::string	get_tracker_name_by_url(const std::string& tracker_url);


	#include "common.hh"

#endif

