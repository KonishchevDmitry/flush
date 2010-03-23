/**************************************************************************
*                                                                         *
*   Flush - GTK-based BitTorrent client                                   *
*   http://sourceforge.net/projects/flush                                 *
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


#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/ref.hpp>
#include <boost/thread.hpp>

#include <glibmm/main.h>

#include <libtorrent/extensions/metadata_transfer.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/alert.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/version.hpp>

#include <mlib/gtk/dispatcher.hpp>
#include <mlib/gtk/main.hpp>
#include <mlib/async_fs.hpp>
#include <mlib/fs.hpp>
#include <mlib/fs_watcher.hpp>
#include <mlib/libtorrent.hpp>
#include <mlib/main.hpp>
#include <mlib/misc.hpp>
#include <mlib/signals_holder.hpp>
#include <mlib/sys.hpp>

#include "common.hpp"
#include "daemon_session.hpp"
#include "daemon_types.hpp"
#include "main.hpp"

#if M_BOOST_GET_VERSION() >= M_GET_VERSION(1, 36, 0)
	#include <boost/unordered_map.hpp>
#else
	#include <map>
#endif


#define RESUME_DATA_FILE_NAME "resume"
#define SAVE_SESSION_TIMEOUT ( 5 * 60 * 1000 ) // ms
#define THREAD_CANCEL_RESPONSE_TIMEOUT ( 500 ) // ms
#define UPDATE_TORRENTS_STATISTICS_TIMEOUT ( 60 ) // s



namespace
{
	#if M_BOOST_GET_VERSION() >= M_GET_VERSION(1, 36, 0)
		typedef boost::unordered_map<Torrent_id, Torrent> Torrents_container;
	#else
		typedef std::map<Torrent_id, Torrent> Torrents_container;
	#endif



	class Cleaned_torrent
	{
		public:
			Cleaned_torrent(
				const Torrent_id&		id,
				const std::string&		name,
				const Auto_clean_type&	clean_type
			) : id(id), name(name), clean_type(clean_type) { }


		public:
			Torrent_id			id;
			std::string			name;
			Auto_clean_type		clean_type;
	};



	class Seeding_torrent
	{
		public:
			Seeding_torrent(
				const Torrent_id&	id,
				const std::string&	name,
				const time_t		seed_time
			) : id(id), name(name), seed_time(seed_time) {}


		public:
			Torrent_id		id;
			std::string		name;
			time_t			seed_time;
	};



	/// Инициирует получение пиров от трекера заранее, если установленный в
	/// данный момент интервал получения пиров превышает max_announce_interval.
	void	force_torrent_reannounce_if_needed(lt::torrent_handle& handle, Time max_announce_interval);

	/// Функция сравнения двух торрентов по времени раздачи.
	inline
	bool	Torrent_seeding_time_compare(const Seeding_torrent& a, const Seeding_torrent& b);



	void force_torrent_reannounce_if_needed(lt::torrent_handle& handle, Time max_announce_interval)
	{
		try
		{
			if(!handle.is_paused())
			{
				lt::torrent_status status = handle.status();

				if(status.next_announce.total_seconds() > max_announce_interval)
					handle.force_reannounce( boost::posix_time::seconds(max_announce_interval) );
			}
		}
		catch(lt::invalid_handle&)
		{
		}
	}



	bool Torrent_seeding_time_compare(const Seeding_torrent& a, const Seeding_torrent& b)
	{
		return a.seed_time < b.seed_time;
	}
}



// Tracker_error -->
	/// Класс, описывающий ошибку соединения с трекером.
	class Tracker_error
	{
		public:
			Tracker_error(const lt::tracker_error_alert& alert);


		public:
			Torrent_id	torrent_id;
			std::string	error_string;
	};



	Tracker_error::Tracker_error(const lt::tracker_error_alert& alert)
	:
		torrent_id(alert.handle),
		error_string(alert.message())
	{
		class Invalid_message {};

		const size_t max_msg_size = 100;
		const size_t msg_size = this->error_string.size();

		std::string tracker_url = alert.url;
		std::string first_part =
			" (" + tracker_url + ") (" + m::to_string(alert.status_code) + ") ";

		size_t first_part_pos = this->error_string.rfind(first_part);

		try
		{
			if(first_part_pos != std::string::npos)
			{
				std::string second_part = " (" + m::to_string(alert.times_in_row) + ")";

				if(first_part_pos + first_part.size() + second_part.size() < msg_size)
				{
					if(!this->error_string.compare(msg_size - second_part.size(), second_part.size(), second_part))
					{
						std::string message = this->error_string.substr(
							first_part_pos + first_part.size(),
							msg_size - (first_part_pos + first_part.size() + second_part.size())
						);

						if(message.size() > max_msg_size)
							message = message.substr(0, max_msg_size) + "...";

						this->error_string = __("%1 (%2). Errors: %3.", message, tracker_url, alert.times_in_row);
					}
					else
					{
						std::string message = this->error_string.substr(first_part_pos + first_part.size());

						if(message.size() > max_msg_size)
							message = message.substr(0, max_msg_size) + "...";

						this->error_string = message + " (" + tracker_url + ").";
					}
				}
				else
					throw Invalid_message();
			}
			else
				throw Invalid_message();
		}
		catch(Invalid_message&)
		{
			if(this->error_string.size() > max_msg_size)
				this->error_string = this->error_string.substr(0, max_msg_size) + "...";
		}
	}
// Tracker_error <--



// Private -->
namespace Daemon_session_aux
{

	class Private
	{
		public:
			Private(void);


		public:
			/// Сигнализирует о том, что в данный момент запущен процесс
			/// завершения сессии.
			bool								session_stopping;

			/// Текущая libtorrent сессия.
			lt::session							session;


			/// Настройки текущей сессии.
			Daemon_settings_light				settings;

			/// За время существования сессии в нее могут добавляться и
			/// удаляться торренты с одинаковыми идентификаторами. Данный
			/// контейнер хранит текущие порядковые номера для каждого
			/// идентификатора торрента, когда-либо добавленного в эту сессию.
		#if M_BOOST_GET_VERSION() >= M_GET_VERSION(1, 36, 0)
			boost::unordered_map<Torrent_id, size_t>	torrents_serial_numbers;
		#else
			std::map<Torrent_id, size_t>				torrents_serial_numbers;
		#endif

			/// Торренты текущей сессии.
			Torrents_container					torrents;

			/// Время последнего обновления статистики.
			time_t								last_update_torrent_statistics_time;

			/// Статистика по прошлым и текущей сессиям.
			Daemon_statistics					statistics;


			/// Удерживает сигналы, которые необходимо отсоединить перед
			/// остановкой сессии.
			m::Signals_holder					before_stopping_sholder;

			/// Удерживает сигналы, которые необходимо отсоединить после
			/// остановки сессии.
			m::Signals_holder					after_stopping_sholder;


			/// Сигнал для остановки messages_thread.
			M_LIBRARY_COMPATIBILITY
			/// Boost 1.34 не предоставляет возможности завершить выполение
			/// потока, поэтому используем данный флаг.
			volatile bool						stop_messages_thread;

			/// Поток для получения сообщений от libtorrent.
			std::auto_ptr<boost::thread>		messages_thread;


			/// Сигнал на получение очередной resume data от libtorrent.
			m::gtk::Dispatcher<
			void (const Torrent_id&,
			boost::shared_ptr<lt::entry>)>		torrent_resume_data_signal;

			/// Количество resume data, которые были запрошены у libtorrent, но
			/// еще не были получены и обработаны.
			size_t								pending_torrent_resume_data;


			/// Сигнал на получение ошибки соединения с трекером.
			m::gtk::Dispatcher<
			void (const Tracker_error&)>		tracker_error_signal;

			/// Сигнал на завершение скачивания торрента.
			m::gtk::Dispatcher<
			void (const lt::torrent_handle&)>	torrent_finished_signal;


			/// Активное в данный момент "временное действие".
			Temporary_action					temporary_action;

			/// Сигнал на истечение срока "временного действия".
			sigc::connection					temporary_action_expired_connection;

			/// Торренты, к которым было применено "временное действие".
			std::list<Torrent_full_id>			temporary_action_torrents;


			/// Объект, с помощью которого производятся наблюдения за
			/// изменениями в файловой системе.
			m::Fs_watcher						fs_watcher;


		public:
			/// Переводит сессию в остановленное состояние, если для этого
			/// пришло время.
			void	set_stopped_state_if_needed(void);
	};



	Private::Private(void)
	:
		session_stopping(false),
		session(lt::fingerprint("LT", LIBTORRENT_VERSION_MAJOR, LIBTORRENT_VERSION_MINOR, 0, 0), 0),

		last_update_torrent_statistics_time(time(NULL)),

		stop_messages_thread(false),

		pending_torrent_resume_data(0),

		temporary_action(TEMPORARY_ACTION_NONE)
	{
	}



	void Private::set_stopped_state_if_needed(void)
	{
		if(this->session_stopping && !pending_torrent_resume_data)
			this->stop_messages_thread = true;
	}

}
// Private <--



Daemon_session::Daemon_session(const std::string& config_dir_path)
:
	priv(new Private)
{
	// Устанавливаем уровень сообщений, которые мы будем принимать от libtorrent -->
		priv->session.set_alert_mask(
			lt::alert::error_notification |
			lt::alert::port_mapping_notification |
			lt::alert::storage_notification |
			lt::alert::tracker_notification |
			lt::alert::status_notification |
			lt::alert::ip_block_notification |
			lt::alert::tracker_notification
		);
	// Устанавливаем уровень сообщений, которые мы будем принимать от libtorrent <--

	// Настройки libtorrent -->
	{
		lt::session_settings lt_settings = priv->session.settings();

		lt_settings.user_agent = _C("%1 %2", APP_NAME, APP_VERSION_STRING);
		lt_settings.ignore_limits_on_local_network = false;

		// Увеличиваем количество пиров, которые клиент запрашивает у трекера.
		// Если говорить о версии 0.14.3, то libtorrent все равно ограничивает
		// их количество до 999.
		lt_settings.num_want = 10000;

	#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 15, 0)
		lt_settings.announce_to_all_trackers = true;
		lt_settings.announce_to_all_tiers = true;
	#endif

		priv->session.set_settings(lt_settings);
	}
	// Настройки libtorrent <--

	// Расширения -->
		// Поддержка magnet-ссылок
		priv->session.add_extension(&lt::create_metadata_plugin);
		priv->session.add_extension(&lt::create_ut_metadata_plugin);
	// Расширения <--

	// Задаем начальные настройки демона -->
	{
		Daemon_settings daemon_settings;

		// Читаем конфиг -->
			try
			{
				daemon_settings.read(this->get_config_dir_path(), &priv->statistics);
			}
			catch(m::Exception& e)
			{
				MLIB_W(EE(e));
			}
		// Читаем конфиг <--

		this->set_settings(daemon_settings, true);
	}
	// Задаем начальные настройки демона <--

	// Обработчик сигнала на автоматическое сохранение текущей сессии
	priv->before_stopping_sholder.push(Glib::signal_timeout().connect(
		sigc::mem_fun(*this, &Daemon_session::on_save_session_callback),
		SAVE_SESSION_TIMEOUT
	));

	// Обработчик сигнала на обновление статистической информации о торрентах
	priv->before_stopping_sholder.push(Glib::signal_timeout().connect(
		sigc::mem_fun(*this, &Daemon_session::on_update_torrents_statistics_callback),
		UPDATE_TORRENTS_STATISTICS_TIMEOUT * 1000 // раз в минуту
	));

	// Обработчик сигнала на получение очередной resume data от libtorrent.
	priv->after_stopping_sholder.push(priv->torrent_resume_data_signal.connect(
		sigc::mem_fun(*this, &Daemon_session::on_torrent_resume_data_callback)
	));

	// Обработчик сигнала на завершение скачивания торрента.
	priv->before_stopping_sholder.push(priv->torrent_finished_signal.connect(
		sigc::mem_fun(*this, &Daemon_session::on_torrent_finished_callback)
	));

	// Обработчик сигнала на получение сообщения об ошибке соединения с
	// трекером.
	priv->before_stopping_sholder.push(priv->tracker_error_signal.connect(
		sigc::mem_fun(*this, &Daemon_session::on_tracker_error_cb)
	));

	// Поток для получения сообщений от libtorrent.
	priv->messages_thread = std::auto_ptr<boost::thread>(
		new boost::thread(boost::ref(*this)) );
}



Daemon_session::~Daemon_session(void)
{
	// Для работы умных указателей
}



void Daemon_session::add_torrent(const std::string& torrent_uri, const New_torrent_settings& new_torrent_settings, bool error_if_not_exists)
{
	Torrent_id torrent_id;

	torrent_id = this->add_torrent_to_config(torrent_uri, new_torrent_settings, error_if_not_exists);

	if(torrent_id)
		this->load_torrent(torrent_id);
}



Torrent_id Daemon_session::add_torrent_to_config(const std::string& torrent_uri, const New_torrent_settings& new_torrent_settings, bool error_if_not_exists) const
{
	MLIB_D(_C("Adding torrent '%1' to config...", torrent_uri));

	m::Buffer torrent_data;
	std::auto_ptr<lt::torrent_info> torrent_info;
	bool magnet = m::lt::is_magnet_uri(torrent_uri);

	// Получаем информацию о торренте -->
		if(magnet)
		{
			// Генерирует m::Exception
			torrent_info = std::auto_ptr<lt::torrent_info>(
				new lt::torrent_info(m::lt::get_torrent_metadata(
					torrent_uri, new_torrent_settings.encoding
				).info)
			);
		}
		else
		{
			// Получаем данные торрента -->
				try
				{
					torrent_data.load_file(torrent_uri);
				}
				catch(m::Sys_exception& e)
				{
					// No such file or directory
					if(e.errno_val == ENOENT && !error_if_not_exists)
					{
						MLIB_D(_C("Torrent file '%1' is already not exists. Skiping it...", torrent_uri));
						return Torrent_id();
					}
					else
						M_THROW(__("Error while reading torrent file '%1': %2.", torrent_uri, EE(e)));
				}
			// Получаем данные торрента <--

			// Получаем информацию о торренте -->
				try
				{
					torrent_info = std::auto_ptr<lt::torrent_info>(
						new lt::torrent_info(m::lt::get_torrent_metadata(
							torrent_data, new_torrent_settings.encoding
						).info)
					);
				}
				catch(m::Exception& e)
				{
					M_THROW(__("Error while reading torrent file '%1': %2.", torrent_uri, EE(e)));
				}
			// Получаем информацию о торренте <--
		}
	// Получаем информацию о торренте <--

	Torrent_id torrent_id = torrent_info->info_hash();
	std::string torrent_dir_path = this->get_torrent_dir_path(torrent_id);

	// Проверяем, нет ли уже торрента с таким идентификатором в текущей сессии
	if(this->is_torrent_exists(torrent_id))
	{
		if(new_torrent_settings.duplicate_is_error)
			M_THROW(_("This torrent is already exists in the current session."));
		else
		{
			MLIB_D(_C("Torrent '%1' is already exists in the current session.", torrent_uri));
			return Torrent_id();
		}
	}

	// Создаем директорию, в которой будет храниться
	// информация о торренте.
	// -->
		try
		{
			m::sys::unix_mkdir(torrent_dir_path);
		}
		catch(m::Exception& e)
		{
			M_THROW(__(
				"Can't create torrent configuration directory '%1': %2.",
				m::fs::get_abs_path_lazy(torrent_dir_path), EE(e)
			));
		}
	// <--

	try
	{
		size_t files_num;
		std::string torrent_name;
		std::vector<std::string> trackers;

		// Получаем информацию о торренте -->
			files_num = torrent_info->num_files();
			torrent_name = torrent_info->name();

			if(new_torrent_settings.trackers.get())
				trackers = *new_torrent_settings.trackers;
			else
				trackers = m::lt::get_torrent_trackers(*torrent_info);

			// Массив либо должен быть пуст, либо в нем должно быть такое
			// количество файлов, как и в открываемом торренте.
			if(!new_torrent_settings.files_settings.empty() && new_torrent_settings.files_settings.size() != files_num)
				M_THROW(__("Torrent '%1' has been changed while it's been processed.", torrent_uri));
		// Получаем информацию о торренте <--

		// Сохраняем *.torrent файл в его папку -->
			if(!magnet)
			{
				std::string torrent_dest_path = Path(torrent_dir_path) / TORRENT_FILE_NAME;

				try
				{
					torrent_data.write_file(torrent_dest_path);
				}
				catch(m::Exception& e)
				{
					M_THROW(__(
						"Can't copy torrent file '%1' to '%2': %3.",
						torrent_uri, m::fs::get_abs_path_lazy(torrent_dest_path), EE(e)
					));
				}
			}
		// Сохраняем *.torrent файл в его папку <--

		// Сохраняем конфигурационный файл торрента -->
			Torrent_settings(
				magnet ? torrent_uri : "",
				(
					new_torrent_settings.name == ""
					?
						torrent_name
					:
						new_torrent_settings.name
				),
				!new_torrent_settings.start,
				new_torrent_settings.download_path,
				Download_settings(new_torrent_settings.copy_on_finished_path),
				new_torrent_settings.encoding,
				(
					new_torrent_settings.files_settings.empty()
					?
						std::vector<Torrent_file_settings>(files_num, Torrent_file_settings())
					:
						new_torrent_settings.files_settings
				),
				trackers
			).write(torrent_dir_path);
		// Сохраняем конфигурационный файл торрента <--
	}
	catch(m::Exception& e)
	{
		try
		{
			// Удаляем все, что мы только что создали
			this->remove_torrent_from_config(torrent_id);
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
		}

		throw;
	}

	return torrent_id;
}



void Daemon_session::add_torrent_to_session(m::lt::Torrent_metadata torrent_metadata, const Torrent_settings& torrent_settings)
{
	Torrent_id torrent_id(torrent_metadata.info.info_hash());
	lt::torrent_handle torrent_handle;

	// Меняем информацию о торренте в соответствии с требуемыми настройками
	// -->
		if(torrent_settings.name != "")
		{
			#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 15, 0)
			{
				lt::file_storage files = torrent_metadata.info.files();
				files.set_name(torrent_settings.name);
				torrent_metadata.info.remap_files(files);
			}
			#else
				torrent_metadata.info.files().set_name(torrent_settings.name);
			#endif
		}

		for(size_t i = 0; i < torrent_settings.files_settings.size(); i++)
		{
			const std::string& path = torrent_settings.files_settings[i].path;

			if(!path.empty())
			{
				std::string new_path = U2LT(path);

				// Лучше не делать лишних переименований - в какой-то версии
				// libtorrent эти пути не проверялись на соответствие.
				if(torrent_metadata.info.file_at(i).path.string() != new_path)
				#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
					torrent_metadata.info.files().rename_file(i, new_path);
				#else
					torrent_metadata.info.rename_file(i, new_path);
				#endif
			}
		}
	// <--

	// Добавляем торрент в сессию libtorrent -->
		try
		{
			lt::add_torrent_params params;
			params.save_path = U2LT(torrent_settings.download_path);
			params.paused = true;
			params.auto_managed = false;
			params.duplicate_is_error = true;

			// Если мы добавляем *.torrent-файл
			if(torrent_metadata.info.metadata_size())
			{
				std::vector<char> resume_data;
				lt::entry resume_data_entry = torrent_settings.resume_data;

				params.ti = boost::intrusive_ptr<lt::torrent_info>(new lt::torrent_info(torrent_metadata.info));

				// Если у нас есть resume data
				if(resume_data_entry.type() != lt::entry::undefined_t)
				{
					#if M_LT_GET_VERSION() >= M_GET_VERSION(0, 14, 3)
						// Наши настройки, которые мы задали только что, будут
						// конфликтовать с теми, которые есть в resume_data (они
						// идентичны). В результате libtorrent сгенерирует кучу
						// lt::file_rename_failed_alert'ов вида:
						// ${torrent_name}: failed to rename file 197: ${file_name}.
						//
						// Доверять эту информацию resume_data мы не можем, т. к.
						// resume_data можно сгенерировать не всегда: если торрент
						// только добавился, и началась проверка файлов, то
						// resume_data сгенерировать не удастся. Если в данный
						// момент прервать работу клиента, то все настройки будут
						// утеряны.
						//
						// Поэтому просто вырезаем эту информацию из resume_data.
						{
							lt::entry* entry = resume_data_entry.find_key("mapped_files");

							if(entry)
								*entry = lt::entry(entry->type());
						}
					#endif

					// Добавляем торрент в приостановленном состоянии ("paused" в
					// resume data, которая хранит состояние торрента в момент
					// генерации resume_data, имеет приоритет над params.paused).
					if(resume_data_entry.type() == lt::entry::dictionary_t)
						resume_data_entry["paused"] = true;

					lt::bencode(std::back_inserter(resume_data), resume_data_entry);
					params.resume_data = &resume_data;
				}

				torrent_handle = priv->session.add_torrent(params);
			}
			// Если мы добавляем magnet-ссылку
			else
				torrent_handle = lt::add_magnet_uri(priv->session, torrent_settings.magnet, params);
		}
		catch(lt::duplicate_torrent e)
		{
			MLIB_LE();
		}
	// Добавляем торрент в сессию libtorrent <--

	Torrent torrent(
		Torrent_full_id(torrent_id, ++priv->torrents_serial_numbers[torrent_id]),
		torrent_handle, torrent_metadata, torrent_settings
	);

	// Передаем libtorrent настройки файлов торрента.
	torrent.sync_files_settings();

	// Передаем libtorrent настройки трекеров торрента.
	this->set_torrent_trackers(torrent, torrent_settings.trackers);

	try
	{
		// Теперь, когда мы передали libtorrent всю необходимую информацию о
		// торренте, можно его запустить.
		if(!torrent_settings.is_paused)
			torrent.handle.resume();
	}
	catch(lt::invalid_handle e)
	{
		MLIB_LE();
	}

	// Добавляем торрент в список торрентов сессии
	priv->torrents.insert(std::pair<Torrent_id, Torrent>(torrent_id, torrent));
}



void Daemon_session::auto_clean_torrent(const Torrent_id& id, const std::string& name, const Auto_clean_type& clean_type)
{
	MLIB_D(_C("Auto cleaning torrent '%1' (%2) [%3]...", name, id, clean_type.type));

	switch(clean_type.type)
	{
		case Auto_clean_type::PAUSE:
		{
			try
			{
				this->pause_torrent(this->get_torrent(id));
			}
			catch(m::Exception& e)
			{
				M_THROW(__("Can't pause torrent '%1'. %2", name, EE(e)));
			}
		}
		break;

		case Auto_clean_type::REMOVE:
		{
			try
			{
				this->remove_torrent(id);
			}
			catch(m::Exception& e)
			{
				M_THROW(__("Can't remove torrent '%1'. %2", name, EE(e)));
			}
		}
		break;

		case Auto_clean_type::REMOVE_WITH_DATA:
		{
			try
			{
				this->remove_torrent_with_data(id);
			}
			catch(m::Exception& e)
			{
				M_THROW(__("Can't remove with data torrent '%1'. %2", name, EE(e)));
			}
		}
		break;

		default:
			MLIB_LE();
			break;
	}
}



void Daemon_session::auto_load_if_torrent(const std::string& torrent_path)
{
	bool is_torrent_file = false;

	MLIB_D(_C("Checking torrent '%1' for auto loading...", torrent_path));

	// Проверяем, действительно ли это *.torrent файл -->
		if(m::fs::check_extension(torrent_path, "torrent"))
		{
			try
			{
				m::sys::Stat file_stat = m::sys::unix_stat(torrent_path);

				// К примеру, Firefox сначала создает пустой файл
				// *.torrent и файл *.torrent.part, закрывает *.torrent и
				// начинает писать данные в *.torrent.part. Т. к. при закрытии
				// *.torrent сгенерируется событие inotify, то мы просто
				// пропускаем файлы нулевого размера. Да и вообще, это применимо
				// не только к Firefox - в любом случае загрузка файла нулевого
				// размера вызовет ошибку. Конечно, может быть, стоило бы ее
				// показать пользователю, но уж больно велика вероятность того,
				// что если файл нулевого размера - то это не ошибка, просто
				// программа закрыла его по какой-либо причине, но в самое
				// ближайшее время в нем появятся реальные данные торрента.
				if(file_stat.is_reg() && file_stat.size)
					is_torrent_file = true;
			}
			catch(m::Sys_exception& e)
			{
				if(e.errno_val == ENOENT)
					MLIB_D("File already is not exists.");
				else
					M_THROW(__("Can't stat torrent file: %1.", EE(e)));
			}
		}
		else
			MLIB_D(_C("'%1' is not a torrent file.", torrent_path));
	// Проверяем, действительно ли это *.torrent файл <--

	if(is_torrent_file)
	{
		MLIB_D(_C("Auto loading torrent '%1'...", torrent_path));

		// Добавляем торрент в сессию -->
		{
			New_torrent_settings new_torrent_settings(
				true, priv->settings.torrents_auto_load.to,
				(
					priv->settings.torrents_auto_load.copy
					?
						priv->settings.torrents_auto_load.copy_to
					:
						""
				),
				MLIB_UTF_CHARSET_NAME, std::vector<Torrent_file_settings>(),
				std::auto_ptr<String_vector>(), false
			);

			this->add_torrent(torrent_path, new_torrent_settings, false);
		}
		// Добавляем торрент в сессию <--

		// Удаляем загруженный *.torrent файл -->
			if(priv->settings.torrents_auto_load.delete_loaded)
			{
				try
				{
					m::fs::rm_if_exists(torrent_path);
				}
				catch(m::Exception& e)
				{
					MLIB_W(
						_("Deleting automatically loaded *.torrent file failed"),
						__(
							"Can't delete automatically loaded *.torrent file '%1': %2.",
							torrent_path, EE(e)
						)
					);
				}
			}
		// Удаляем загруженный *.torrent файл <--
	}
}



void Daemon_session::auto_load_torrents(void)
{
	Errors_pool errors;
	std::string torrent_path;

	while(priv->fs_watcher.get(&torrent_path))
	{
		// Появился новый файл
		if(torrent_path != "")
		{
			try
			{
				this->auto_load_if_torrent(torrent_path);
			}
			catch(m::Exception& e)
			{
				errors += __(
					"Automatic loading the torrent '%1' failed. %2",
					torrent_path, EE(e)
				);
			}
		}
		// Директория была перемещена/удалена
		else
		{
			if(errors)
			{
				MLIB_W(_("Automatic torrents loading failed"), EE(errors));
				errors = Errors_pool();
			}

			MLIB_W(
				_("Automatic torrents loading failed"),
				__(
					"Directory '%1' has been deleted or moved.",
					priv->settings.torrents_auto_load.from
				)
			);

			// "Сбрасываем" мониторинг
			priv->fs_watcher.unset_watching_directory();

			// Выкидываем все, что осталось в очереди
			priv->fs_watcher.clear();

			// Отключаем мониторинг в настройках
			priv->settings.torrents_auto_load.is = false;
		}
	}

	if(errors)
		MLIB_W(_("Automatic torrents loading failed"), EE(errors));
}



void Daemon_session::automate(void)
{
	Errors_pool errors;
	Daemon_settings::Auto_clean& clean = priv->settings.torrents_auto_clean;

	MLIB_D("Checking for automation needs...");

	// Ограничение на максимальное время раздачи и максимальный рейтинг -->
		if(clean.max_seeding_time_type || clean.max_ratio_type)
		{
			MLIB_D("Checking for max seeding time and max ratio...");

			std::vector<Cleaned_torrent> cleaned_torrents;

			M_FOR_CONST_IT(priv->torrents, it)
			{
				const Torrent& torrent = it->second;

				if(torrent.seeding)
				{
					Auto_clean_type clean_type;

					if(clean.max_seeding_time_type && torrent.time_seeding >= clean.max_seeding_time)
					{
						MLIB_D(_C("Setting auto clean type [%1] for torrent '%2'.",
							clean.max_seeding_time_type, torrent.name));
						clean_type.set_if_stricter(clean.max_seeding_time_type);
					}

					if(clean.max_ratio_type && Torrent_info(torrent).get_share_ratio() >= clean.max_ratio)
					{
						MLIB_D(_C("Setting auto clean type [%1] for torrent '%2'.",
							clean.max_ratio_type, torrent.name));
						clean_type.set_if_stricter(clean.max_ratio_type);
					}

					if(clean_type)
						cleaned_torrents.push_back( Cleaned_torrent(torrent.id, torrent.name, clean_type) );
				}
			}

			M_FOR_CONST_IT(cleaned_torrents, it)
			{
				const Cleaned_torrent& torrent = *it;

				try
				{
					this->auto_clean_torrent(
						torrent.id, torrent.name, torrent.clean_type
					);
				}
				catch(m::Exception& e)
				{
					errors += EE(e);
				}
			}
		}
	// Ограничение на максимальное время раздачи и максимальный рейтинг <--

	// Ограничение на количество раздаваемых торрентов -->
		if(clean.max_seeding_torrents_type && priv->torrents.size() > static_cast<size_t>(clean.max_seeding_torrents))
		{
			MLIB_D("Checking for max seeding torrents...");

			std::vector<Seeding_torrent> seeding_torrents;

			M_FOR_CONST_IT(priv->torrents, it)
			{
				const Torrent& torrent = it->second;

				if(torrent.seeding)
				{
					seeding_torrents.push_back(
						Seeding_torrent(torrent.id, torrent.name, torrent.time_seeding)
					);
				}
			}

			sort(seeding_torrents.begin(), seeding_torrents.end(), Torrent_seeding_time_compare);

			for(size_t i = clean.max_seeding_torrents; i < seeding_torrents.size(); i++)
			{
				const Seeding_torrent& torrent = seeding_torrents[i];

				try
				{
					this->auto_clean_torrent(
						torrent.id, torrent.name, clean.max_seeding_torrents_type
					);
				}
				catch(m::Exception& e)
				{
					errors += EE(e);
				}
			}
		}
	// Ограничение на количество раздаваемых торрентов <--

	if(errors)
		MLIB_W(_("Automatic torrents cleaning failed"), errors);
}



void Daemon_session::finish_torrent(Torrent& torrent)
{
	bool schedule_torrent_settings_saving = false;

	// Информируем пользователя, если это необходимо -->
	{
		try
		{
			// Генерирует lt::invalid_handle
			lt::torrent_status status = torrent.handle.status();

			if(torrent.bytes_done_on_last_torrent_finish != status.total_done)
			{
				COMPATIBILITY
				// Совместимость с версиями < 0.9.
				// Специальное значение, чтобы при переходе на новую версию
				// Flush при первом запуске не выдал кучу нотификаций.
				if(torrent.bytes_done_on_last_torrent_finish != -2)
				{
					bool all_completed = true;

					// Проверяем, есть ли еще скачивающиеся торренты -->
					{
						Torrents_container::const_iterator begin = priv->torrents.begin();
						Torrents_container::const_iterator end = priv->torrents.end();
						Torrents_container::const_iterator it = begin;

						while(it != end)
						{
							const Torrent& torrent = it->second;
							Torrent_info info = torrent.get_info();

							// Если торрент не на паузе и у него скачаны не все данные
							if(info.downloaded_requested_size != info.requested_size && !info.paused)
							{
								// Если торрент проверяется
								if(info.status <= Torrent_info::CHECKING_FILES)
								{
									// Либо этот торрент не скачал еще ни
									// одного байта, либо он поставлен на
									// проверку пользователем.
									if(torrent.bytes_done_on_last_torrent_finish < 0)
									{
										all_completed = false;
										break;
									}
									// В данный момент у торрента проверяется resume data.
									else
									{
										// Тут очень сложно что-либо сказать
										// наверняка, поэтому попытаемся
										// сделать хотя бы какие-то
										// предположения.

										// Если в прошлой сессии торрент был
										// скачанным, то, скорее всего, таким
										// окажется и в этой сессии.
										if(torrent.bytes_done == torrent.bytes_done_on_last_torrent_finish)
											;
										// В противном случае он продолжит
										// скачивание.
										else
										{
											all_completed = false;
											break;
										}
									}
								}
								// Торрент скачивается
								else
								{
									all_completed = false;
									break;
								}
							}

							it++;
						}
					}
					// Проверяем, есть ли еще скачивающиеся торренты <--

					// Уведомляем о завершении скачивания торрента
					this->notify_message_signal(Notify_message(
						all_completed ? Notify_message::TORRENT_FINISHED_AND_ALL : Notify_message::TORRENT_FINISHED,
						_C("Torrent '%1' downloaded.", torrent.name)
					));

					// Уведомляем о завершении скачивания всех торрентов
					if(all_completed)
						this->notify_message_signal(Notify_message(
							Notify_message::ALL_TORRENTS_FINISHED, _("All torrents downloaded.") ));
				}

				torrent.bytes_done_on_last_torrent_finish = status.total_done;
				schedule_torrent_settings_saving = true;
			}
		}
		catch(lt::invalid_handle&)
		{
			MLIB_LE();
		}
	}
	// Информируем пользователя, если это необходимо <--

	// Если по завершении скачивания необходимо
	// скопировать файлы данного торрента.
	if(torrent.download_settings.copy_when_finished)
	{
		std::vector<bool> interested_files;

		// Формируем список интересующих нас файлов -->
		{
			size_t i = 0;
			interested_files.reserve(torrent.files_settings.size());

			BOOST_FOREACH(const Torrent_file_settings& settings, torrent.files_settings)
				interested_files[i++] = settings.download;
		}
		// Формируем список интересующих нас файлов <--

		// Получаем список скачанных файлов торрента -->
			std::vector<std::string> files_paths;

			try
			{
				files_paths = m::lt::get_torrent_downloaded_files_paths(torrent.handle, interested_files);
			}
			catch(lt::invalid_handle)
			{
				MLIB_LE();
			}
		// Получаем список скачанных файлов торрента <--


		std::string src_path = torrent.get_download_path();
		std::string dest_path = torrent.download_settings.copy_when_finished_to;

		m::async_fs::copy_files(
			torrent.id,
			src_path, dest_path, files_paths,
			_("Copying finished torrent's files failed"),
			__(
				"Copying finished torrent's '%1' files from '%2' to '%3' failed.",
				torrent.name, src_path, dest_path
			)
		);

		MLIB_D(_C(
			"Copying finished torrent's files '%1' from '%2' to '%3' has been scheduled.",
			torrent.name, src_path, dest_path
		));

		// Данное сообщение будет возникать в каждой новой сессии
		// libtorrent, поэтому снимаем флаг.
		torrent.download_settings.copy_when_finished = false;
		torrent.download_settings_revision++;
		schedule_torrent_settings_saving = true;
	}

	// Планируем сохранение настроек торрента
	if(schedule_torrent_settings_saving)
		this->schedule_torrent_settings_saving(torrent);
}



Speed Daemon_session::get_rate_limit(Traffic_type traffic_type) const
{
	switch(traffic_type)
	{
		case DOWNLOAD:
			return get_rate_limit_from_lt(priv->session.download_rate_limit());
			break;

		case UPLOAD:
			return get_rate_limit_from_lt(priv->session.upload_rate_limit());
			break;

		default:
			MLIB_LE();
			break;
	}
}



Session_status Daemon_session::get_session_status(void) const
{
	return Session_status(
		priv->statistics, priv->session.status(),
		this->get_rate_limit(DOWNLOAD), this->get_rate_limit(UPLOAD),
		priv->temporary_action != TEMPORARY_ACTION_NONE
	);
}



Daemon_settings Daemon_session::get_settings(void) const
{
	Daemon_settings settings = priv->settings;

	settings.listen_port = priv->session.is_listening() ? priv->session.listen_port() : -1;

	settings.dht = this->is_dht_started();

	if(settings.dht)
		settings.dht_state = priv->session.dht_state();

	settings.download_rate_limit = this->get_rate_limit(DOWNLOAD);
	settings.upload_rate_limit = this->get_rate_limit(UPLOAD);

	return settings;
}



const Torrent& Daemon_session::get_torrent(const Torrent_full_id& full_id) const
{
	const Torrent& torrent = this->get_torrent(full_id.id);

	if(torrent.serial_number == full_id.serial_number)
		return torrent;
	else
		M_THROW(__("Bad torrent id '%1'.", torrent.id));
}



Torrent& Daemon_session::get_torrent(const Torrent_full_id& full_id)
{
	Torrent& torrent = this->get_torrent(full_id.id);

	if(torrent.serial_number == full_id.serial_number)
		return torrent;
	else
		M_THROW(__("Bad torrent id '%1'.", torrent.id));
}



const Torrent& Daemon_session::get_torrent(const Torrent_id& torrent_id) const
{
	Torrents_container::const_iterator torrent_iter = priv->torrents.find(torrent_id);

	// Торрента с таким ID уже нет
	if(torrent_iter == priv->torrents.end())
		M_THROW(__("Bad torrent id '%1'.", torrent_id));

	return torrent_iter->second;
}



Torrent& Daemon_session::get_torrent(const Torrent_id& torrent_id)
{
	Torrents_container::iterator torrent_iter = priv->torrents.find(torrent_id);

	// Торрента с таким ID уже нет
	if(torrent_iter == priv->torrents.end())
		M_THROW(__("Bad torrent id '%1'.", torrent_id));

	return torrent_iter->second;
}



const Torrent& Daemon_session::get_torrent(const lt::torrent_handle& torrent_handle) const
{
	return this->get_torrent(Torrent_id(torrent_handle.info_hash()));
}



Torrent& Daemon_session::get_torrent(const lt::torrent_handle& torrent_handle)
{
	return this->get_torrent(Torrent_id(torrent_handle.info_hash()));
}



Torrent_details Daemon_session::get_torrent_details(const Torrent& torrent) const
{
	return Torrent_details(torrent);
}



Revision Daemon_session::get_torrent_files_info(const Torrent& torrent, std::vector<Torrent_file> *files, std::vector<Torrent_file_status>* statuses, Revision revision) const
{
	// files -->
		if(revision == torrent.files_revision)
			files->clear();
		else
		{
			try
			{
				if(torrent.handle.has_metadata())
					*files = m::lt::get_torrent_files(torrent.handle.get_torrent_info());
				else
					files->clear();
			}
			catch(lt::invalid_handle)
			{
				MLIB_LE();
			}
			catch(m::Exception)
			{
				MLIB_LE();
			}
		}

	// files <--

	// statuses -->
	{
		const std::vector<Torrent_file_settings>& files_settings = torrent.files_settings;

		statuses->clear();
		statuses->reserve(files_settings.size());

		std::vector<lt::size_type> downloaded;

		try
		{
			torrent.handle.file_progress(downloaded);
		}
		catch(lt::invalid_handle)
		{
			MLIB_LE();
		}

		if(downloaded.size() == files_settings.size())
			for(size_t i = 0; i < downloaded.size(); i++)
				statuses->push_back( Torrent_file_status(files_settings[i], downloaded[i]) );
		else
		{
			if(files_settings.empty())
			{
				// Видимо, данный торрент скачивался через magnet-ссылку,
				// метаданные уже скачались, но мы еще не успели на это
				// прореагировать. Поэтому пока что ведем себя так, как будто
				// мы еще не получили метаданные торрента.
				files->clear();
			}
			else
				MLIB_LE();
		}
	}
	// statuses <--

	return torrent.files_revision;
}



void Daemon_session::get_torrent_peers_info(const Torrent torrent, std::vector<Torrent_peer_info>& peers_info) const
{
	std::vector<lt::peer_info> lt_peers_info;

	try
	{
		torrent.handle.get_peer_info(lt_peers_info);
	}
	catch(lt::invalid_handle)
	{
		MLIB_LE();
	}

	peers_info.clear();
	peers_info.reserve(lt_peers_info.size());

	M_FOR_CONST_IT(lt_peers_info, it)
		peers_info.push_back(Torrent_peer_info(*it));
}



bool Daemon_session::get_torrent_new_download_settings(const Torrent& torrent, Revision* revision, Download_settings* download_settings) const
{
	if(torrent.download_settings_revision != *revision)
	{
		*revision = torrent.download_settings_revision ;
		*download_settings = torrent.get_download_settings();

		return true;
	}
	else
		return false;
}



bool Daemon_session::get_torrent_new_trackers(const Torrent& torrent, Revision* revision, std::vector<std::string>* trackers) const
{
	if(torrent.trackers_revision != *revision)
	{
		*revision = torrent.trackers_revision;
		*trackers = this->get_torrent_trackers(torrent);

		return true;
	}
	else
		return false;
}



std::vector<std::string> Daemon_session::get_torrent_trackers(const Torrent& torrent) const
{
	try
	{
		return m::lt::get_torrent_trackers(torrent.handle);
	}
	catch(lt::invalid_handle)
	{
		MLIB_LE();
	}
}



void Daemon_session::get_torrents_info(std::vector<Torrent_info>& torrents_info) const
{
	torrents_info.clear();
	torrents_info.reserve(priv->torrents.size());

	M_FOR_CONST_IT(priv->torrents, it)
		torrents_info.push_back(it->second.get_info());
}



void Daemon_session::interrupt_temporary_action(bool complete)
{
	MLIB_D(_C("Interrupting temporary action by completing(%1) it...", complete));

	if(priv->temporary_action != TEMPORARY_ACTION_NONE)
	{
		if(complete)
			this->rollback_temporary_action();

		priv->temporary_action = TEMPORARY_ACTION_NONE;
		priv->temporary_action_expired_connection.disconnect();
		priv->temporary_action_torrents.clear();

		MLIB_D("Temporary action is canceled.");
	}
	else
		MLIB_D("Temporary action is not active.");
}



bool Daemon_session::is_dht_started(void) const
{
	return !(priv->session.dht_state() == lt::entry());
}



bool Daemon_session::is_torrent_exists(const Torrent_id& torrent_id) const
{
	return priv->torrents.find(torrent_id) != priv->torrents.end();
}



void Daemon_session::load_torrent(const Torrent_id& torrent_id)
{
	MLIB_D(_C("Loading torrent '%1'...", torrent_id));

	Errors_pool errors;
	Torrent_settings::Read_flags settings_flags = 0;
	Torrent_settings torrent_settings( this->get_torrents_download_path() );

	// Получаем настройки торрента.
	// Если прочитать не получится, то торрент будет
	// добавлен с настройками по умолчанию.
	// -->
		try
		{
			torrent_settings.read(this->get_torrent_dir_path(torrent_id), &settings_flags);
		}
		catch(m::Exception& e)
		{
			errors += __("Restoring torrent's previous session settings failed. %1", EE(e));
		}
	// <--

	std::auto_ptr<m::lt::Torrent_metadata> torrent_metadata;

	// Получаем информацию о торренте -->
	{
		bool torrent_not_exists_error = false;
		std::string torrent_path = Path(this->get_torrent_dir_path(torrent_id)) / TORRENT_FILE_NAME;

		try
		{
			m::Buffer torrent_data;

			try
			{
				torrent_data.load_file(torrent_path);
			}
			catch(m::Sys_exception& e)
			{
				if(e.errno_val == ENOENT)
					torrent_not_exists_error = true;

				throw;
			}

			// Генерирует m::Exception
			torrent_metadata = std::auto_ptr<m::lt::Torrent_metadata>(
				new m::lt::Torrent_metadata( m::lt::get_torrent_metadata(torrent_data, torrent_settings.encoding) ) );
		}
		catch(m::Exception& e)
		{
			// Magnet-ссылки у нас нет
			if(torrent_settings.magnet.empty())
			{
				errors += __("Error while reading torrent file '%1': %2.", torrent_path, EE(e));
				errors.throw_if_exists();
			}
			// У нас есть еще magnet-ссылка
			else
			{
				// Чтобы, если потеряется *.torrent-файл, по окончании проверки
				// скачанных файлов пользователь увидел уведомление о
				// завершении скачивания торрента.
				torrent_settings.bytes_done_on_last_torrent_finish = -1;

				if(torrent_not_exists_error)
				{
					// Это не ошибка. Просто на данный момент мы пока что
					// не имеем *.torrent-файл.
				}
				else
					errors += __("Error while reading torrent file '%1': %2.", torrent_path, EE(e));

				// Получаем информацию по magnet-ссылке -->
					try
					{
						torrent_metadata = std::auto_ptr<m::lt::Torrent_metadata>(
							new m::lt::Torrent_metadata( m::lt::get_magnet_metadata(torrent_settings.magnet) ) );
					}
					catch(m::Exception& e)
					{
						errors += EE(e);
						errors.throw_if_exists();
					}
				// Получаем информацию по magnet-ссылке <--
			}
		}
	}
	// Получаем информацию о торренте <--

	std::string torrent_name = torrent_settings.name.empty()
		? torrent_metadata->info.name()
		: torrent_settings.name;

	// Проверяем информацию, полученную из конфига -->
		if(size_t(torrent_metadata->info.num_files()) != torrent_settings.files_settings.size())
		{
			// Если торрент добавляется по magnet-ссылке, то у него еще нет
			// информации о файлах, но в конфиге, эта информация при
			// определенных обстоятельствах может появиться.
			if(torrent_metadata->info.metadata_size())
				MLIB_SW(__("Invalid torrent '%1' files' settings. Rejecting them...", torrent_name));

			torrent_settings.files_settings.clear();
			torrent_settings.files_settings.resize(torrent_metadata->info.num_files());
		}

		if(!(settings_flags & Torrent_settings::READ_FLAG_TRACKERS_GOTTEN))
			torrent_settings.trackers = m::lt::get_torrent_trackers(torrent_metadata->info);
	// Проверяем информацию, полученную из конфига <--

	if(errors)
		MLIB_W(_("Torrent added with errors"),
			__("Torrent '%1' has been added with errors.%2", torrent_name, EE(errors)) );

	// Добавляем торрент к сессии
	add_torrent_to_session(*torrent_metadata, torrent_settings);

	MLIB_D(_C("Torrent '%1' has been loaded.", torrent_id));
}



void Daemon_session::load_torrents_from_auto_load_dir(void)
{
	if(priv->settings.torrents_auto_load.is)
	{
		Path torrents_dir_path = priv->settings.torrents_auto_load.from;

		// Загружаем каждый торрент в директории -->
			try
			{
				Errors_pool errors;

				for
				(
					fs::directory_iterator dir_it(U2L(torrents_dir_path.string()));
					dir_it != fs::directory_iterator();
					dir_it++
				)
				{
					std::string torrent_path = L2U( dir_it->path().string() );

					try
					{
						auto_load_if_torrent(torrent_path);
					}
					catch(m::Exception& e)
					{
						errors += __(
							"Automatic loading the torrent '%1' failed. %2",
							torrent_path, EE(e)
						);
					}
				}

				errors.throw_if_exists();
			}
			catch(fs::filesystem_error& e)
			{
				M_THROW(
					__(
						"Can't read directory '%1' for automatic torrents loading: %2.",
						torrents_dir_path, EE(e)
					)
				);
			}
		// Загружаем каждый торрент в директории <--
	}
}



void Daemon_session::load_torrents_from_config(void)
{
	try
	{
		Path torrents_dir_path = this->get_torrents_dir_path();

		// Загружаем каждый торрент в директории -->
			try
			{
				for
				(
					fs::directory_iterator directory_iterator(U2L(torrents_dir_path.string()));
					directory_iterator != fs::directory_iterator();
					directory_iterator++
				)
				{
					Torrent_id torrent_id = L2U(Path(directory_iterator->path()).basename());

					try
					{
						this->load_torrent(torrent_id);
					}
					catch(m::Exception& e)
					{
						MLIB_W(_("Loading torrent failed"), __("Loading of the torrent '%1' failed. %2", torrent_id, EE(e)));
					}
				}
			}
			catch(fs::filesystem_error e)
			{
				M_THROW(__("Can't read directory with torrents configs '%1': %2.", torrents_dir_path, EE(e)));
			}
		// Загружаем каждый торрент в директории <--
	}
	catch(m::Exception& e)
	{
		M_THROW(__("Loading torrents from previous session failed. %1", EE(e)));
	}
}



bool Daemon_session::on_save_session_callback(void)
{
	m::gtk::Scoped_enter gtk_lock;

	try
	{
		this->save_session();
	}
	catch(m::Exception& e)
	{
		MLIB_W(_("Saving session failed"), __("Saving session failed. %1", EE(e)));
	}

	return true;
}



bool Daemon_session::on_temporary_action_expired_cb(void)
{
	m::gtk::Scoped_enter gtk_lock;
	this->interrupt_temporary_action(true);
	return false;
}



void Daemon_session::on_torrent_finished_callback(const lt::torrent_handle& torrent_handle)
{
	try
	{
		this->finish_torrent(this->get_torrent(torrent_handle));
	}
	catch(m::Exception&)
	{
		// Такого торрента уже не существует, следовательно,
		// это сообщение уже не актуально.
	}
}



void Daemon_session::on_torrent_resume_data_callback(const Torrent_id& torrent_id, boost::shared_ptr<lt::entry> resume_data)
{
	this->save_torrent_settings_if_exists(torrent_id, *resume_data);
	priv->pending_torrent_resume_data--;
	priv->set_stopped_state_if_needed();
}



void Daemon_session::on_tracker_error_cb(const Tracker_error& error)
{
	try
	{
		Torrent& torrent = this->get_torrent(error.torrent_id);
		torrent.tracker_error = error.error_string;
	}
	catch(m::Exception&)
	{
		// Такого торрента уже не существует, следовательно,
		// это сообщение уже не актуально.
	}
}



bool Daemon_session::on_update_torrents_statistics_callback(void)
{
	m::gtk::Scoped_enter gtk_lock;

	time_t current_time = time(NULL);
	time_t time_diff = current_time - priv->last_update_torrent_statistics_time;
	priv->last_update_torrent_statistics_time = current_time;

	// Если, к примеру, были переведены часы.
	if(time_diff < 0)
		time_diff = UPDATE_TORRENTS_STATISTICS_TIMEOUT;

	M_FOR_IT(priv->torrents, it)
	{
		Torrent& torrent = it->second;
		Torrent_info torrent_info = torrent.get_info();

		if(
			torrent.seeding && (
				torrent_info.status == Torrent_info::SEEDING ||
				torrent_info.status == Torrent_info::UPLOADING
			) && !torrent_info.paused
		)
			torrent.time_seeding += time_diff;

		torrent.seeding = (
			torrent_info.status == Torrent_info::SEEDING ||
			torrent_info.status == Torrent_info::UPLOADING
		) && !torrent_info.paused;
	}

	this->automate();

	return true;
}



void Daemon_session::pause_torrent(Torrent& torrent)
{
	try
	{
		torrent.handle.pause();
	}
	catch(lt::invalid_handle)
	{
		MLIB_LE();
	}

	torrent.seeding = false;
}



void Daemon_session::process_torrents_temporary(Temporary_action action, Torrents_group group, Time time)
{
	bool pause_negate;
	void (Daemon_session::* action_fuction)(Torrent& torrent);

	this->interrupt_temporary_action(true);

	switch(action)
	{
		case TEMPORARY_ACTION_RESUME:
			pause_negate = false;
			action_fuction = &Daemon_session::resume_torrent;
			break;

		case TEMPORARY_ACTION_PAUSE:
			pause_negate = true;
			action_fuction = &Daemon_session::pause_torrent;
			break;

		default:
			MLIB_LE();
			break;
	}

	M_FOR_IT(priv->torrents, it)
	{
		Torrent& torrent = it->second;

		if(torrent.is_paused() ^ pause_negate && torrent.is_belong_to(group))
		{
			priv->temporary_action_torrents.push_back(torrent.get_full_id());
			(this->*action_fuction)(torrent);
		}
	}

	if(!priv->temporary_action_torrents.empty())
	{
		MLIB_D(_C("Temporary processing action %1.", action));

		priv->temporary_action = action;
		priv->temporary_action_expired_connection = Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &Daemon_session::on_temporary_action_expired_cb),
			time * 1000
		);
	}
	else
		MLIB_D(_C("Skiping temporary processing action %1: there is no such torrents.", action));
}



void Daemon_session::recheck_torrent(const Torrent_id& torrent_id)
{
	try
	{
		Torrent& torrent = this->get_torrent(torrent_id);

		// Если торрент еще не получил метаданных, libtorrent падает с
		// Segmentation fault.
		if(torrent.handle.has_metadata())
		{
			// Чтобы оповещение сгенерировалось даже в том случае, когда все файлы
			// на месте.
			torrent.bytes_done_on_last_torrent_finish = -1;
			torrent.handle.force_recheck();
		}
	}
	catch(lt::invalid_handle)
	{
		MLIB_LE();
	}
}



void Daemon_session::remove_torrent(const Torrent_id& torrent_id)
{
	// Удаляем из самой сессии
	this->remove_torrent_from_session(torrent_id);

	// Удаляем конфигурационные файлы торрента
	this->remove_torrent_from_config(torrent_id);
}



void Daemon_session::remove_torrent_from_config(const Torrent_id& torrent_id) const
{
	m::fs::rm(this->get_torrent_dir_path(torrent_id));
}



void Daemon_session::remove_torrent_from_session(const Torrent_id& torrent_id)
{
	Torrent& torrent = this->get_torrent(torrent_id);

	try
	{
		priv->session.remove_torrent(torrent.handle);
	}
	catch(lt::invalid_handle e)
	{
		MLIB_W(__("Bad torrent id: '%1'.", torrent.id));
	}

	priv->torrents.erase(torrent_id);
}



void Daemon_session::remove_torrent_with_data(const Torrent_id& torrent_id)
{
	Torrent& torrent = this->get_torrent(torrent_id);

	std::string torrent_name = torrent.name;
	std::string download_path = Path(torrent.get_download_path());

	// Чтобы гарантировать, что далее список файлов торрента меняться не будет
	// (актуально для magnet-ссылок).
	this->pause_torrent(torrent);

	// Получаем список файлов торрента -->
		std::vector<std::string> files_paths;

		try
		{
			files_paths = m::lt::get_torrent_files_paths(torrent.handle.get_torrent_info());
		}
		catch(lt::invalid_handle)
		{
			// В случае magnet-ссылки все время будет генерироваться
			// исключение, пока не будут получены данные торрента.
		}
	// Получаем список файлов торрента <--

	try
	{
		// Удаляем торрент из сессии
		this->remove_torrent(torrent_id);
	}
	catch(m::Exception& e)
	{
		try
		{
			// Асинхронно удаляем данные торрента.
			// Файла может и не существовать, если торрент был добавлен в
			// приостановленном состоянии и ни разу не запускался.
			m::async_fs::rm_files_with_empty_dirs(
				torrent_id,
				download_path, files_paths,
				_("Deleting torrent data failed"),
				__("Deleting torrent '%1' data failed.", torrent_name)
			);
		}
		catch(m::Exception& ee)
		{
			M_THROW(EE(e) + "\n" + EE(ee));
		}

		throw;
	}

	// Асинхронно удаляем данные торрента
	m::async_fs::rm_files_with_empty_dirs(
		torrent_id,
		download_path, files_paths,
		_("Deleting torrent data failed"),
		__("Deleting torrent '%1' data failed.", torrent_name)
	);
}



void Daemon_session::reset_statistics(void)
{
	priv->statistics.reset(priv->session.status());
}



void Daemon_session::resume_torrent(Torrent& torrent)
{
	bool paused = false;

	try
	{
		const lt::torrent_status torrent_status = torrent.handle.status();
		paused = torrent_status.paused;

		if(paused)
		{
			torrent.tracker_error.clear();
			torrent.handle.resume();

			// libtorrent::torrent_handle::resume сбрасывает эти счетчики в 0
			// (но только в том случае, если торрент в данный момент находится
			// на паузе). Поэтому перед каждым resume'ом сохраняем их значение.
			torrent.total_download         += torrent_status.total_download;
			torrent.total_payload_download += torrent_status.total_payload_download;
			torrent.total_upload           += torrent_status.total_upload;
			torrent.total_payload_upload   += torrent_status.total_payload_upload;
		}
	}
	catch(lt::invalid_torrent_file)
	{
		MLIB_LE();
	}
}



void Daemon_session::rollback_temporary_action(void)
{
	void (Daemon_session::* action_fuction)(Torrent& torrent);

	MLIB_D(_C("Rolling back temporary action %1...", priv->temporary_action));

	switch(priv->temporary_action)
	{
		case TEMPORARY_ACTION_RESUME:
			action_fuction = &Daemon_session::pause_torrent;
			break;

		case TEMPORARY_ACTION_PAUSE:
			action_fuction = &Daemon_session::resume_torrent;
			break;

		default:
			MLIB_LE();
			break;
	}

	M_FOR_CONST_IT(priv->temporary_action_torrents, it)
	{
		try
		{
			Torrent& torrent = this->get_torrent(*it);
			(this->*action_fuction)(torrent);
		}
		catch(m::Exception&)
		{
			// Этого торрента уже просто нет в сессии.
		}
	}
}



void Daemon_session::save_session(void)
{
	MLIB_D("Saving session...");

	// Планируем сохранение настроек торрентов
	M_FOR_CONST_IT(priv->torrents, it)
		this->schedule_torrent_settings_saving(it->second);

	// Пишем конфиг демона -->
		try
		{
			Daemon_settings settings = this->get_settings();
			settings.write(
				this->get_config_dir_path(),
				Session_status(
					priv->statistics, priv->session.status(),
					this->get_rate_limit(DOWNLOAD), this->get_rate_limit(UPLOAD),
					priv->temporary_action != TEMPORARY_ACTION_NONE
				)
			);
		}
		catch(m::Exception)
		{
			throw;
		}
	// Пишем конфиг демона <--
}



void Daemon_session::save_torrent_settings_if_exists(const Torrent_id& torrent_id, const lt::entry& resume_data)
{
	Torrents_container::const_iterator torrent_it = priv->torrents.find(torrent_id);

	if(torrent_it != priv->torrents.end())
	{
		try
		{
			// Сохраняем настройки торрента
			Torrent_settings(torrent_it->second, resume_data).write(this->get_torrent_dir_path(torrent_id));
		}
		catch(m::Exception& e)
		{
			MLIB_W(
				_("Saving torrent settings failed"),
				__(
					"Saving torrent '%1' settings failed. %2",
					torrent_it->second.name, EE(e)
				)
			);
		}
	}
	else
		MLIB_D(_C("Saving settings has been requested for non-existent torrent '%1'.", torrent_id));
}



void Daemon_session::schedule_torrent_settings_saving(const Torrent& torrent)
{
	try
	{
		torrent.handle.save_resume_data();
	}
	catch(lt::invalid_handle)
	{
		MLIB_LE();
	}

	priv->pending_torrent_resume_data++;
}



void Daemon_session::set_copy_when_finished(Torrent& torrent, bool copy, const std::string& to)
{
	if(copy && !Path(to).is_absolute())
		M_THROW(__("Invalid copy when finished to path '%1'.", to));

	torrent.download_settings.copy_when_finished = copy;
	torrent.download_settings.copy_when_finished_to = to;

	torrent.download_settings_revision++;
}



void Daemon_session::set_files_download_status(Torrent& torrent, const std::vector<int>& files_ids, bool download)
{
	Errors_pool errors;
	std::vector<Torrent_file_settings>& files_settings = torrent.files_settings;

	for(size_t i = 0; i < files_ids.size(); i++)
	{
		int file_id = files_ids[i];

		if(file_id >= 0 && file_id < int(files_settings.size()))
			files_settings[file_id].download = download;
		else
			errors += __("Bad file id: %1.", file_id);
	}

	torrent.sync_files_settings();

	errors.throw_if_exists();
}



void Daemon_session::set_files_priority(Torrent& torrent, const std::vector<int>& files_ids, const Torrent_file_settings::Priority priority)
{
	Errors_pool errors;
	std::vector<Torrent_file_settings>& files_settings = torrent.files_settings;

	for(size_t i = 0; i < files_ids.size(); i++)
	{
		int file_id = files_ids[i];

		if(file_id >= 0 && file_id < int(files_settings.size()))
			files_settings[file_id].priority = priority;
		else
			errors += __("Bad file id: %1.", file_id);
	}

	torrent.sync_files_settings();

	errors.throw_if_exists();
}



void Daemon_session::set_rate_limit(Traffic_type traffic_type, Speed speed)
{
	switch(traffic_type)
	{
		case DOWNLOAD:
			priv->session.set_download_rate_limit(get_lt_rate_limit(speed));
			break;

		case UPLOAD:
			priv->session.set_upload_rate_limit(get_lt_rate_limit(speed));
			break;

		default:
			MLIB_LE();
			break;
	}
}



void Daemon_session::set_sequential_download(Torrent& torrent, bool value)
{
	torrent.handle.set_sequential_download(value);
	torrent.download_settings_revision++;
}



void Daemon_session::set_settings(const Daemon_settings& settings, const bool init_settings)
{
	// Диапазон портов для прослушивания -->
		if(
			priv->settings.listen_random_port != settings.listen_random_port ||
			priv->settings.listen_ports_range != settings.listen_ports_range || init_settings
		)
		{
			priv->settings.listen_random_port = settings.listen_random_port;
			priv->settings.listen_ports_range = settings.listen_ports_range;

			priv->session.listen_on(
				settings.listen_random_port ? std::make_pair(0, 0) : priv->settings.listen_ports_range,
				// "0.0.0.0" присваивать обязательно - иначе из Интернета качаться не будет.
				"0.0.0.0"
			);
		}
	// Диапазон портов для прослушивания <--

	// DHT -->
		if(this->is_dht_started() != settings.dht || init_settings)
		{
			if(settings.dht)
			{
				if(init_settings)
					priv->session.start_dht(settings.dht_state);
				else
					priv->session.start_dht();
			}
			else
				priv->session.stop_dht();
		}
	// DHT <--

	// LSD -->
		if(priv->settings.lsd != settings.lsd || init_settings)
		{
			if( (priv->settings.lsd = settings.lsd) )
				priv->session.start_lsd();
			else
				priv->session.stop_lsd();
		}
	// LSD <--

	// UPnP -->
		if(priv->settings.upnp != settings.upnp || init_settings)
		{
			if( (priv->settings.upnp = settings.upnp) )
				priv->session.start_upnp();
			else
				priv->session.stop_upnp();
		}
	// UPnP <--

	// NAT-PMP -->
		if(priv->settings.natpmp != settings.natpmp || init_settings)
		{
			if( (priv->settings.natpmp = settings.natpmp) )
				priv->session.start_natpmp();
			else
				priv->session.stop_natpmp();
		}
	// NAT-PMP <--

	// smart_ban, pex -->
	{
		// Расширения можно только подключать, отключить их уже нельзя
		// -->
			if( settings.smart_ban && ( !priv->settings.smart_ban || init_settings) )
				priv->session.add_extension(&lt::create_smart_ban_plugin);

			if( settings.pex && ( !priv->settings.pex || init_settings) )
				priv->session.add_extension(&lt::create_ut_pex_plugin);
		// <--

		priv->settings.smart_ban = settings.smart_ban;
		priv->settings.pex = settings.pex;
	}
	// smart_ban, pex <--

	// Ограничение на скорость скачивания.
	if(this->get_rate_limit(DOWNLOAD) != settings.download_rate_limit || init_settings)
		this->set_rate_limit(DOWNLOAD, settings.download_rate_limit);

	// Ограничение на скорость отдачи.
	if(this->get_rate_limit(UPLOAD) != settings.upload_rate_limit || init_settings)
		this->set_rate_limit(UPLOAD, settings.upload_rate_limit);

	// Максимальное количество соединений для раздачи,
	// которое может быть открыто.
	if(priv->settings.max_uploads != settings.max_uploads || init_settings)
	{
		priv->settings.max_uploads = settings.max_uploads;
		priv->session.set_max_uploads(settings.max_uploads);
	}

	// Максимальное количество соединений, которое может
	// быть открыто.
	if(priv->settings.max_connections != settings.max_connections || init_settings)
	{
		priv->settings.max_connections = settings.max_connections;
		priv->session.set_max_connections(settings.max_connections);
	}

	// Максимальный интервал между запросами пиров у трекера -->
		if(
			settings.use_max_announce_interval && (
				priv->settings.use_max_announce_interval != settings.use_max_announce_interval ||
				priv->settings.max_announce_interval != settings.max_announce_interval
			) && !init_settings
		)
		{
			// Вносим изменения для всех активных в данный момент торрентов
			M_FOR_IT(priv->torrents, it)
				force_torrent_reannounce_if_needed(it->second.handle, settings.max_announce_interval);
		}

		priv->settings.use_max_announce_interval = settings.use_max_announce_interval;
		priv->settings.max_announce_interval = settings.max_announce_interval;
	// Максимальный интервал между запросами пиров у трекера <--

	// IP фильтр -->
		if(
			priv->settings.ip_filter_enabled != settings.ip_filter_enabled ||
			priv->settings.ip_filter != settings.ip_filter
		)
		{
			lt::ip_filter ip_filter;

			if(settings.ip_filter_enabled)
			{
				for(size_t i = 0; i < settings.ip_filter.size(); i++)
				{
					const Ip_filter_rule& rule = settings.ip_filter[i];

					ip_filter.add_rule(
						lt::address_v4::from_string(rule.from),
						lt::address_v4::from_string(rule.to),
						rule.block ? lt::ip_filter::blocked : 0
					);
				}
			}

			priv->session.set_ip_filter(ip_filter);

			priv->settings.ip_filter_enabled = settings.ip_filter_enabled;
			priv->settings.ip_filter = settings.ip_filter;
		}
	// IP фильтр <--

	// Настройки автоматической "подгрузки" *.torrent файлов. -->
	{
		Daemon_settings::Torrents_auto_load& auto_load = priv->settings.torrents_auto_load;

		// Если настройки не эквивалентны
		if(!auto_load.equal(settings.torrents_auto_load))
		{
			// "Сбрасываем" мониторинг
			priv->fs_watcher.unset_watching_directory();

			// Загружаем оставшиеся в очереди торренты со старыми
			// настройками.
			this->auto_load_torrents();

			// Получаем новые настройки
			auto_load = settings.torrents_auto_load;

			if(auto_load.is)
			{
				// Вносим изменения в мониторинг -->
					try
					{
						priv->fs_watcher.set_watching_directory(auto_load.from);
					}
					catch(m::Exception& e)
					{
						auto_load.is = false;

						MLIB_W(
							_("Setting directory for automatic torrents loading failed"),
							__(
								"Setting directory '%1' for automatic torrents loading failed. %2",
								auto_load.from, EE(e)
							)
						);
					}
				// Вносим изменения в мониторинг <--

				// Также загружаем все торренты, которые в данный момент
				// присутствуют в директории, но только если эти настройки
				// задаются не на этапе инициализации. В противном случае
				// это необходимо сделать потом - после того, как будут
				// загружены все торренты из прошлой сессии.
				// -->
					if(!init_settings)
					{
						try
						{
							this->load_torrents_from_auto_load_dir();
						}
						catch(m::Exception& e)
						{
							MLIB_W(_("Automatic torrents loading failed"), EE(e));
						}
					}
				// <--
			}
		}
		else
			auto_load = settings.torrents_auto_load;
	}
	// Настройки автоматической "подгрузки" *.torrent файлов. <--

	// Настройки автоматической "очистки" от старых торрентов -->
		if(priv->settings.torrents_auto_clean != settings.torrents_auto_clean)
		{
			priv->settings.torrents_auto_clean = settings.torrents_auto_clean;

			// Чтобы изменения сразу же вступили в силу
			this->automate();
		}
	// Настройки автоматической "очистки" от старых торрентов <--
}



void Daemon_session::set_torrent_trackers(Torrent& torrent, const std::vector<std::string>& trackers)
{
	std::vector<lt::announce_entry> announces;

	announces.reserve(trackers.size());
	for(size_t i = 0; i < trackers.size(); i++)
		announces.push_back(lt::announce_entry(trackers[i]));

	try
	{
		// В libtorrent <= 0.14.2 есть ошибка, которая приводит к Segmentation
		// fault, если сначала задать пустой список торрентов, а потом задать
		// непустой.
		// Это простейшая защита от такой ошибки.
		#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
			if(!torrent.handle.trackers().empty())
			{
				if(!announces.empty())
					torrent.handle.replace_trackers(announces);
			}
			else
				MLIB_W(_("Setting torrent trackers failed: libtorrent internal error."));
		#else
			torrent.handle.replace_trackers(announces);
		#endif

		torrent.trackers_revision++;
	}
	catch(lt::invalid_handle)
	{
		MLIB_LE();
	}
}



void Daemon_session::start(void)
{
	// Загружаем все торренты из прошлой сессии -->
		try
		{
			this->load_torrents_from_config();
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
		}
	// Загружаем все торренты из прошлой сессии <--

	// После того, как торренты из прошлой сессии загружены, можно запустить
	// подсистему автоматической подгрузки новых торрентов.
	// -->
		try
		{
			this->load_torrents_from_auto_load_dir();
		}
		catch(m::Exception& e)
		{
			MLIB_W(_("Automatic torrents loading failed"), EE(e));
		}

		// Обработчик сигнала на появление новых торрентов для автоматического
		// добавления.
		priv->before_stopping_sholder.push(priv->fs_watcher.connect(
			sigc::mem_fun(*this, &Daemon_session::auto_load_torrents)
		));
	// <--
}



void Daemon_session::start_torrents(const Torrents_group group)
{
	MLIB_D(_C("Starting torrents [%1].", static_cast<int>(group)));

	M_FOR_IT(priv->torrents, it)
	{
		Torrent& torrent = it->second;
		Torrent_info torrent_info(torrent);

		switch(group)
		{
			case ALL:
				this->resume_torrent(torrent);
				break;

			case DOWNLOADS:
				if(torrent_info.requested_size != torrent_info.downloaded_requested_size)
					this->resume_torrent(torrent);
				break;

			case UPLOADS:
				if(torrent_info.requested_size == torrent_info.downloaded_requested_size)
					this->resume_torrent(torrent);
				break;

			default:
				MLIB_LE();
				break;
		}
	}
}



void Daemon_session::stop(void)
{
	MLIB_D("Gotten stop session signal.");

	if(!priv->session_stopping)
	{
		MLIB_D("Stopping the session...");

		priv->session_stopping = true;

		// Останавливаем сессию, чтобы прекратить скачивание торрентов
		priv->session.pause();

		// Т. к. работа завершается, то можно считать, что время "как бы истекло".
		this->interrupt_temporary_action(true);

		// Отсоединяем все сигналы, которые могут изменить состояние сессии
		priv->before_stopping_sholder.disconnect();

		// Сохраняем текущую сессию -->
			try
			{
				this->save_session();
			}
			catch(m::Exception& e)
			{
				MLIB_W(_("Saving session failed"), __("Saving session failed. %1", EE(e)));
			}
		// Сохраняем текущую сессию <--

		// Если нет необходимости сохранять настройки торрентов, то сразу
		// переходим на стадию завершения работы сессии.
		priv->set_stopped_state_if_needed();
	}
}



void Daemon_session::stop_torrents(const Torrents_group group)
{
	MLIB_D(_C("Stopping torrents [%1].", static_cast<int>(group)));

	M_FOR_IT(priv->torrents, it)
	{
		Torrent& torrent = it->second;
		Torrent_info torrent_info(torrent);

		switch(group)
		{
			case ALL:
				this->pause_torrent(torrent);
				break;

			case DOWNLOADS:
				if(torrent_info.requested_size != torrent_info.downloaded_requested_size)
					this->pause_torrent(torrent);
				break;

			case UPLOADS:
				if(torrent_info.requested_size == torrent_info.downloaded_requested_size)
					this->pause_torrent(torrent);
				break;

			default:
				MLIB_LE();
				break;
		}
	}
}



void Daemon_session::operator()(void)
{
	while(1)
	{
		if(priv->stop_messages_thread)
		{
			MLIB_D("Session is ready for destroying. Sending signal to the application...");

			m::gtk::Scoped_enter lock;
			stop_application();

			return;
		}


		if(priv->session.wait_for_alert(lt::time_duration(static_cast<int64_t>(THREAD_CANCEL_RESPONSE_TIMEOUT * 1000))))
		{
			std::auto_ptr<lt::alert> alert;

			while( (alert = priv->session.pop_alert()).get() )
			{
				Daemon_message message(*alert);

				// Resume data
				if( dynamic_cast<lt::save_resume_data_alert*>(alert.get()) )
				{
					lt::save_resume_data_alert* resume_data_alert = dynamic_cast<lt::save_resume_data_alert*>(alert.get());
					Torrent_id torrent_id = Torrent_id(resume_data_alert->handle);

					MLIB_D(_C("Gotten resume data for torrent '%1'.", torrent_id));

					// Извещаем демон о получении новой resume data -->
					{
						m::gtk::Scoped_enter lock;

						boost::shared_ptr<lt::entry> resume_data = resume_data_alert->resume_data;
						// Чтобы вне критической секции GTK не было обращений к
						// данным resume_data из разных потоков.
						resume_data_alert->resume_data.reset();

						priv->torrent_resume_data_signal(torrent_id, resume_data);
					}
					// Извещаем демон о получении новой resume data <--
				}
				// Invalid resume data
				else if( dynamic_cast<lt::save_resume_data_failed_alert*>(alert.get()) )
				{
					// Насколько я понял, получение
					// lt::save_resume_data_failed_alert не означает
					// какую-то внутреннюю ошибку. Данный alert
					// возвращается в случае, если к моменту генерации
					// resume data торрент был уже удален или он
					// находится в таком состоянии, в котором resume
					// data получить невозможно, к примеру, когда
					// данные только проверяются.

					lt::save_resume_data_failed_alert* failed_alert = dynamic_cast<lt::save_resume_data_failed_alert*>(alert.get());
					Torrent_id torrent_id = Torrent_id(failed_alert->handle);

					MLIB_D(_C("Gotten failed resume data for torrent '%1': %2.", torrent_id, failed_alert->msg));

					// Извещаем демон о получении новой resume data -->
					{
						m::gtk::Scoped_enter lock;
						boost::shared_ptr<lt::entry> resume_data(new lt::entry);
						priv->torrent_resume_data_signal(torrent_id, resume_data);
					}
					// Извещаем демон о получении новой resume data <--
				}
				else
				{
					// Получен список пиров от трекера
					if( dynamic_cast<lt::tracker_reply_alert*>(alert.get()) )
					{
						// Изменяем интервал запроса списка пиров от
						// трекера, если это необходимо.

						Time max_announce_interval = 0;

						{
							m::gtk::Scoped_enter gtk_lock;

							if(priv->settings.use_max_announce_interval)
								max_announce_interval = priv->settings.max_announce_interval;
						}

						if(max_announce_interval)
						{
							lt::torrent_handle& handle = static_cast<lt::torrent_alert*>( alert.get() )->handle;
							force_torrent_reannounce_if_needed(handle, max_announce_interval);
						}
					}
					// Не удалось соединиться с трекером.
					// Данное сообщение необходимо обработать по особому.
					else if( dynamic_cast<lt::tracker_error_alert*>(alert.get()) )
					{
						m::gtk::Scoped_enter lock;

						// Извещаем демон о получении сообщения об ошибке
						// соединения с трекером.
						priv->tracker_error_signal(
							Tracker_error( *static_cast<lt::tracker_error_alert*>(alert.get()) )
						);
					}
					// Получены данные торрента (при скачивании через magnet-ссылку)
					else if( dynamic_cast<lt::metadata_received_alert*>(alert.get()) )
					{
						m::gtk::Scoped_enter lock;

						try
						{
							Torrent& torrent = this->get_torrent(
								static_cast<lt::metadata_received_alert*>(alert.get())->handle );
							torrent.on_metadata_received(this->get_torrent_dir_path(torrent.id));
						}
						catch(m::Exception&)
						{
							MLIB_LE();
						}
					}
					// Скачивание торрента завершено.
					// Данное сообщение необходимо обработать по особому.
					else if( dynamic_cast<lt::torrent_finished_alert*>(alert.get()) )
					{
						// Извещаем демон о завершении скачивания торрента
						m::gtk::Scoped_enter lock;
						priv->torrent_finished_signal( static_cast<lt::torrent_alert*>( alert.get() )->handle );
					}
					// resume data устарела или испорчена.
					else if( dynamic_cast<lt::fastresume_rejected_alert*>(alert.get()) )
					{
						// Это значит, что торрент нам придется качать заново.

						lt::torrent_handle handle = static_cast<lt::torrent_alert*>( alert.get() )->handle;

						try
						{
							m::gtk::Scoped_enter lock;
							this->get_torrent(handle).bytes_done_on_last_torrent_finish = -1;
						}
						catch(m::Exception&)
						{
						}
					}

					// Передаем сообщение пользователю -->
					{
						m::gtk::Scoped_enter lock;
						this->messages_signal(message);
					}
					// Передаем сообщение пользователю <--
				}

				MLIB_D(_C("libtorrent alert: %1", message.get()));
			}
		}
	}
}

