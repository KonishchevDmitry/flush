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


#include <deque>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/ref.hpp>
#include <boost/thread.hpp>

#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/alert.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/version.hpp>

#include <mlib/async_fs.hpp>
#include <mlib/fs.hpp>
#include <mlib/libtorrent.hpp>

#include "daemon_session.hpp"
#include "daemon_types.hpp"


#define TORRENT_FILE_NAME "torrent.torrent"
#define RESUME_DATA_FILE_NAME "resume"
#define SAVE_SESSION_TIMEOUT ( 5 * 60 * 1000 ) // ms
#define THREAD_CANCEL_RESPONSE_TIMEOUT ( 500 ) // ms
#define UPDATE_TORRENTS_STATISTICS_TIMEOUT ( 60 ) // s



namespace
{
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



	/// Функция сравнения двух торрентов по времени раздачи.
	inline
	bool Torrent_seeding_time_compare(const Seeding_torrent& a, const Seeding_torrent& b);



	bool Torrent_seeding_time_compare(const Seeding_torrent& a, const Seeding_torrent& b)
	{
		return a.seed_time < b.seed_time;
	}
}



Daemon_session::Daemon_session(const std::string& config_dir_path)
:
	Daemon_fs(),
	is_stop(false),
	messages_thread(NULL),
	pending_torrent_resume_data(0),
	last_update_torrent_statistics_time(time(NULL)),
	session(new lt::session(lt::fingerprint("LT", LIBTORRENT_VERSION_MAJOR, LIBTORRENT_VERSION_MINOR, 0, 0), 0))
{
	// Устанавливаем уровень сообщений, которые мы будем принимать от libtorrent -->
		this->session->set_alert_mask(
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
		lt::session_settings lt_settings = this->session->settings();
		lt_settings.user_agent = _C("%1 %2", APP_NAME, APP_VERSION_STRING);
		lt_settings.ignore_limits_on_local_network = false;
		this->session->set_settings(lt_settings);
	}
	// Настройки libtorrent <--

	// Задаем начальные настройки демона -->
	{
		Daemon_settings daemon_settings;

		// Читаем конфиг -->
			try
			{
				daemon_settings.read(this->get_config_dir_path(), &this->statistics);
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
	this->save_session_connection = Glib::signal_timeout().connect(
		sigc::mem_fun(*this, &Daemon_session::on_save_session_callback),
		SAVE_SESSION_TIMEOUT
	);

	// Обработчик сигнала на обновление статистической информации о торрентах
	this->update_torrents_statistics_connection = Glib::signal_timeout().connect(
		sigc::mem_fun(*this, &Daemon_session::on_update_torrents_statistics_callback),
		UPDATE_TORRENTS_STATISTICS_TIMEOUT * 1000 // раз в минуту
	);

	// Обработчик сигнала на получение очередной resume data от libtorrent.
	this->torrent_resume_data_signal.connect(
		sigc::mem_fun(*this, &Daemon_session::on_torrent_resume_data_callback)
	);

	// Обработчик сигнала на завершение скачивания торрента.
	this->torrent_finished_signal.connect(
		sigc::mem_fun(*this, &Daemon_session::on_torrent_finished_callback)
	);

	// Поток для получения сообщений от libtorrent.
	this->messages_thread = new boost::thread(boost::ref(*this));
}



Daemon_session::~Daemon_session(void)
{
	// Сигнал на автоматическое сохранение текущей сессии
	this->save_session_connection.disconnect();

	// Сигнал на обновление статистической информации о торрентах
	this->update_torrents_statistics_connection.disconnect();

	// Сигнал на появление новых торрентов для автоматического
	// добавления.
	this->fs_watcher_connection.disconnect();

	// Останавливаем поток для получения сообщений от libtorrent -->
		if(this->messages_thread)
		{
			{
				boost::mutex::scoped_lock lock(this->mutex);
				this->is_stop = true;
			}

			this->messages_thread->join();
			delete this->messages_thread;
		}
	// Останавливаем поток для получения сообщений от libtorrent <--

	// Старые resume data нас уже не интересуют - просто отбрасываем их и
	// генерируем новые.
	this->pending_torrent_resume_data -= this->torrents_resume_data.size();

	// Останавливаем сессию, чтобы прекратить скачивание торрентов.
	this->session->pause();

	// Сохраняем текущую сессию -->
	{
		try
		{
			this->save_session();
		}
		catch(m::Exception& e)
		{
			MLIB_W(_("Saving session failed"), __("Saving session failed. %1", EE(e)));
		}

		// Ожидаем получения запрошенных resume data -->
			MLIB_D("Waiting for pending resume data...");

			while(this->pending_torrent_resume_data)
			{
				if(this->session->wait_for_alert(lt::time_duration(INT_MAX)))
				{
					std::auto_ptr<lt::alert> alert;

					while( this->pending_torrent_resume_data && (alert = this->session->pop_alert()).get() )
					{
						if( dynamic_cast<lt::save_resume_data_alert*>(alert.get()) )
						{
							pending_torrent_resume_data--;

							lt::save_resume_data_alert* resume_data_alert = dynamic_cast<lt::save_resume_data_alert*>(alert.get());
							Torrent_id torrent_id = Torrent_id(resume_data_alert->handle);

							MLIB_D(_C("Gotten resume data for torrent '%1'.", torrent_id));
							this->save_torrent_settings_if_exists(torrent_id, *resume_data_alert->resume_data);
						}
						else if( dynamic_cast<lt::save_resume_data_failed_alert*>(alert.get()) )
						{
							// Насколько я понял, получение
							// lt::save_resume_data_failed_alert не означает
							// какую-то внутреннюю ошибку. Данный alert
							// возвращается в случае, если к моменту генерации
							// resume data торрент был уже удален или он
							// находится в таком состоянии, в котором resume
							// data получить не возможно, к примеру, когда
							// данные только проверяются.

							pending_torrent_resume_data--;

							lt::save_resume_data_failed_alert* failed_alert = dynamic_cast<lt::save_resume_data_failed_alert*>(alert.get());
							Torrent_id torrent_id = Torrent_id(failed_alert->handle);

							MLIB_D(_C("Gotten failed resume data for torrent '%1': %2.", torrent_id, failed_alert->msg));
							this->save_torrent_settings_if_exists(torrent_id, lt::entry());
						}
					}
				}
			}
		// Ожидаем получения запрошенных resume data <--
	}
	// Сохраняем текущую сессию <--

	delete this->session;
}



void Daemon_session::add_torrent(const std::string& torrent_path, const New_torrent_settings& new_torrent_settings, bool error_if_not_exists) throw(m::Exception)
{
	Torrent_id torrent_id;

	torrent_id = this->add_torrent_to_config(torrent_path, new_torrent_settings, error_if_not_exists);

	if(torrent_id)
		this->load_torrent(torrent_id);
}



Torrent_id Daemon_session::add_torrent_to_config(const std::string& torrent_path, const New_torrent_settings& new_torrent_settings, bool error_if_not_exists) const throw(m::Exception)
{
	MLIB_D(_C("Adding torrent '%1' to config...", torrent_path));

	m::Buffer torrent_data;
	std::auto_ptr<lt::torrent_info> torrent_info;

	// Получаем данные торрента -->
		try
		{
			torrent_data.load_file(torrent_path);
		}
		catch(m::Sys_exception& e)
		{
			// No such file or directory
			if(e.errno_val == ENOENT && !error_if_not_exists)
			{
				MLIB_D(_C("Torrent file '%1' is already not exists. Skiping it...", torrent_path));
				return Torrent_id();
			}
			else
				M_THROW(__("Error while reading torrent file '%1': %2.", torrent_path, EE(e)));
		}
	// Получаем данные торрента <--

	// Получаем информацию о торренте -->
		try
		{
			torrent_info = std::auto_ptr<lt::torrent_info>(
				new lt::torrent_info(m::lt::get_torrent_info(torrent_data))
			);
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Error while reading torrent file '%1': %2.", torrent_path, EE(e)));
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
			MLIB_D(_C("Torrent '%1' is already exists in the current session.", torrent_path));
			return Torrent_id();
		}
	}

	// Создаем директорию, в которой будет храниться
	// информация о торренте.
	// -->
		try
		{
			m::fs::unix_mkdir(torrent_dir_path);
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
			trackers = m::lt::get_torrent_trackers(*torrent_info);

			// Массив либо должен быть пуст, либо в нем должно быть такое
			// количество файлов, как и в открываемом торренте.
			if(!new_torrent_settings.files_settings.empty() && new_torrent_settings.files_settings.size() != files_num)
				M_THROW(__("Torrent file '%1' has been changed while it's been processed.", torrent_path));
		// Получаем информацию о торренте <--

		// Сохраняем *.torrent файл в его папку -->
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
					torrent_path, m::fs::get_abs_path_lazy(torrent_dest_path), EE(e)
				));
			}
		}
		// Сохраняем *.torrent файл в его папку <--

		// Сохраняем конфигурационный файл торрента -->
			Torrent_settings(
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



void Daemon_session::add_torrent_to_session(lt::torrent_info torrent_info, const Torrent_settings& torrent_settings)
{
	Torrent_id torrent_id(torrent_info.info_hash());
	lt::torrent_handle torrent_handle;

	// Меняем информацию о торренте в соответствии с требуемыми настройками
	// -->
		if(torrent_settings.name != "")
			torrent_info.files().set_name(torrent_settings.name);

		for(size_t i = 0; i < torrent_settings.files_settings.size(); i++)
		{
			const std::string& path = torrent_settings.files_settings[i].path;

			if(!path.empty())
			{
				// Лучше не делать лишних переименований - в какой-то версии
				// libtorrent эти пути не проверялись на соответствие.
				if(torrent_info.file_at(i).path.string() != U2LT(path))
				#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
					torrent_info.files().rename_file(i, U2LT(path));
				#else
					torrent_info.rename_file(i, U2LT(path));
				#endif
			}
		}
	// <--

	// Добавляем торрент в сессию libtorrent -->
		try
		{
			std::vector<char> resume_data;
			lt::entry resume_data_entry = torrent_settings.resume_data;

			lt::add_torrent_params params;

			params.ti = boost::intrusive_ptr<lt::torrent_info>(new lt::torrent_info(torrent_info));
			params.save_path = U2LT(torrent_settings.download_path);

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

			params.paused = true;
			params.auto_managed = false;
			params.duplicate_is_error = true;

			torrent_handle = this->session->add_torrent(params);
		}
		catch(lt::duplicate_torrent e)
		{
			MLIB_LE();
		}
	// Добавляем торрент в сессию libtorrent <--

	Torrent torrent(torrent_id, torrent_handle, torrent_settings);

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
	this->torrents.insert(std::pair<Torrent_id, Torrent>(torrent_id, torrent));
}



void Daemon_session::auto_clean_torrent(const Torrent_id& id, const std::string& name, const Auto_clean_type& clean_type) throw(m::Exception)
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



void Daemon_session::auto_load_if_torrent(const std::string& torrent_path) throw(m::Exception)
{
	bool is_torrent_file = false;

	MLIB_D(_C("Checking torrent '%1' for auto loading...", torrent_path));

	// Проверяем, действительно ли это *.torrent файл -->
		if(m::fs::check_extension(torrent_path, "torrent"))
		{
			try
			{
				m::fs::Stat file_stat = m::fs::unix_stat(torrent_path);

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
	// Проверяем, действительно ли это *.torrent файл <--

	if(is_torrent_file)
	{
		MLIB_D(_C("Auto loading torrent '%1'...", torrent_path));

		// Добавляем торрент в сессию -->
			this->add_torrent(
				torrent_path,
				New_torrent_settings(
					true, this->settings.torrents_auto_load.to,
					(
						this->settings.torrents_auto_load.copy
						?
							this->settings.torrents_auto_load.copy_to
						:
							""
					),
					std::vector<Torrent_file_settings>(), false
				),
				false
			);
		// Добавляем торрент в сессию <--

		// Удаляем загруженный *.torrent файл -->
			if(this->settings.torrents_auto_load.delete_loaded)
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

	while(this->fs_watcher.get(&torrent_path))
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
				MLIB_W(
					_("Automatic torrents loading failed"),
					__("Automatic torrents loading failed. %1", EE(errors))
				);

				errors = Errors_pool();
			}

			MLIB_W(
				_("Automatic torrents loading failed"),
				__(
					"Automatic torrents loading failed: directory '%1' has been deleted or moved.",
					this->settings.torrents_auto_load.from
				)
			);

			// "Сбрасываем" мониторинг
			this->fs_watcher.unset_watching_directory();

			// Выкидываем все, что осталось в очереди
			this->fs_watcher.clear();

			// Оключаем мониторинг в настройках
			this->settings.torrents_auto_load.is = false;
		}
	}

	if(errors)
	{
		MLIB_W(
			_("Automatic torrents loading failed"),
			__("Automatic torrents loading failed. %1", EE(errors))
		);
	}
}



void Daemon_session::automate(void)
{
	Errors_pool errors;
	Daemon_settings::Auto_clean& clean = this->settings.torrents_auto_clean;

	MLIB_D("Checking for automation needs...");

	// Ограничение на максимальное время раздачи и максимальный рейтинг -->
		if(clean.max_seeding_time_type || clean.max_ratio_type)
		{
			MLIB_D("Checking for max seeding time and max ratio...");

			std::vector<Cleaned_torrent> cleaned_torrents;

			M_FOR_CONST_IT(this->torrents, it)
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
		if(clean.max_seeding_torrents_type && this->torrents.size() > static_cast<size_t>(clean.max_seeding_torrents))
		{
			MLIB_D("Checking for max seeding torrents...");

			std::vector<Seeding_torrent> seeding_torrents;

			M_FOR_CONST_IT(this->torrents, it)
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
	// Если по завершении скачивания необходимо
	// скопировать файлы данного торрента.
	if(torrent.download_settings.copy_when_finished)
	{
		std::string src_path = torrent.get_download_path();
		std::string dest_path = torrent.download_settings.copy_when_finished_to;

		// Получаем список скачанных файлов торрента -->
			std::vector<std::string> files_paths;

			try
			{
				files_paths = m::lt::get_torrent_downloaded_files_paths(torrent.handle);
			}
			catch(lt::invalid_handle)
			{
				MLIB_LE();
			}
		// Получаем список скачанных файлов торрента <--


		m::async_fs::copy_files(
			torrent.id,
			src_path, dest_path, files_paths,
			_("Copying finished torrent failed"),
			__(
				"Copying finished torrent '%1' from '%2' to '%3' failed.",
				torrent.name, src_path, dest_path
			)
		);

		MLIB_D(_C(
			"Copying finished torrent '%1' from '%2' to '%3' has been scheduled.",
			torrent.name, src_path, dest_path
		));

		// Данное сообщение будет возникать в каждой новой сессии
		// libtorrent, поэтому снимаем флаг.
		torrent.download_settings.copy_when_finished = false;
		torrent.download_settings_revision++;

		// Планируем сохранение настроек торрента
		this->schedule_torrent_settings_saving(torrent);
	}
}



void Daemon_session::get_messages(std::deque<Daemon_message>& messages)
{
	messages.clear();

	boost::mutex::scoped_lock lock(this->mutex);
	messages.swap(this->messages);
}



Speed Daemon_session::get_rate_limit(Traffic_type traffic_type) const
{
	switch(traffic_type)
	{
		case DOWNLOAD:
			return get_rate_limit_from_lt(this->session->download_rate_limit());
			break;

		case UPLOAD:
			return get_rate_limit_from_lt(this->session->upload_rate_limit());
			break;

		default:
			MLIB_LE();
			break;
	}
}



Session_status Daemon_session::get_session_status(void) const throw(m::Exception)
{
	return Session_status(
		this->statistics, this->session->status(),
		this->get_rate_limit(DOWNLOAD), this->get_rate_limit(UPLOAD)
	);
}



Daemon_settings Daemon_session::get_settings(void) const
{
	Daemon_settings settings = this->settings;

	settings.listen_port = this->session->is_listening() ? this->session->listen_port() : -1;

	settings.dht = this->is_dht_started();

	if(settings.dht)
		settings.dht_state = this->session->dht_state();

	settings.download_rate_limit = this->get_rate_limit(DOWNLOAD);
	settings.upload_rate_limit = this->get_rate_limit(UPLOAD);

	return settings;
}



const Torrent& Daemon_session::get_torrent(const Torrent_id& torrent_id) const throw(m::Exception)
{
	Torrents_container::const_iterator torrent_iter = this->torrents.find(torrent_id);

	// Торрента с таким ID уже нет
	if(torrent_iter == this->torrents.end())
		M_THROW(__("Bad torrent id '%1'.", torrent_id));

	return torrent_iter->second;
}



Torrent& Daemon_session::get_torrent(const Torrent_id& torrent_id) throw(m::Exception)
{
	Torrents_container::iterator torrent_iter = this->torrents.find(torrent_id);

	// Торрента с таким ID уже нет
	if(torrent_iter == this->torrents.end())
		M_THROW(__("Bad torrent id '%1'.", torrent_id));

	return torrent_iter->second;
}



const Torrent& Daemon_session::get_torrent(const lt::torrent_handle& torrent_handle) const throw(m::Exception)
{
	return this->get_torrent(Torrent_id(torrent_handle.info_hash()));
}



Torrent& Daemon_session::get_torrent(const lt::torrent_handle& torrent_handle) throw(m::Exception)
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
				*files = m::lt::get_torrent_files(torrent.handle.get_torrent_info());
			}
			catch(m::Exception)
			{
				MLIB_LE();
			}
			catch(lt::invalid_handle)
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

		if(downloaded.size() != files_settings.size())
			MLIB_LE();

		for(size_t i = 0; i < downloaded.size(); i++)
			statuses->push_back( Torrent_file_status(files_settings[i], downloaded[i]) );
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
	torrents_info.reserve(this->torrents.size());

	M_FOR_CONST_IT(this->torrents, it)
		torrents_info.push_back(it->second.get_info());
}



bool Daemon_session::is_dht_started(void) const
{
	return !(this->session->dht_state() == lt::entry());
}



bool Daemon_session::is_torrent_exists(const Torrent_id& torrent_id) const
{
	return this->torrents.find(torrent_id) != this->torrents.end();
}



void Daemon_session::load_torrent(const Torrent_id& torrent_id) throw(m::Exception)
{
	MLIB_D(_C("Loading torrent '%1'...", torrent_id));

	lt::torrent_handle torrent_handle;

	// Генерирует m::Exception
	lt::torrent_info torrent_info = m::lt::get_torrent_info(Path(this->get_torrent_dir_path(torrent_id)) / TORRENT_FILE_NAME);

	// Получаем настройки торрента -->
		Torrent_settings torrent_settings(
			"",
			true,
			this->get_torrents_download_path(),
			Download_settings(""),
			std::vector<Torrent_file_settings>(torrent_info.num_files(), Torrent_file_settings()),
			m::lt::get_torrent_trackers(torrent_info)
		);

		// Если прочитать не получится, то торрент будет
		// добавлен с настройками по умолчанию.
		// -->
			try
			{
				torrent_settings.read(this->get_torrent_dir_path(torrent_id));
			}
			catch(m::Exception& e)
			{
				MLIB_W(__(
					"Restoring torrent '%1' [%2] previous session settings failed. %3",
					torrent_info.name(), torrent_id, EE(e)
				));
			}
		// <--
	// Получаем настройки торрента <--

	// Добавляем торрент к сессии
	add_torrent_to_session(torrent_info, torrent_settings);

	MLIB_D(_C("Torrent '%1' has been loaded.", torrent_id));
}



void Daemon_session::load_torrents_from_auto_load_dir(void) throw(m::Exception)
{
	if(this->settings.torrents_auto_load.is)
	{
		Path torrents_dir_path = this->settings.torrents_auto_load.from;

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



void Daemon_session::load_torrents_from_config(void) throw(m::Exception)
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



void Daemon_session::on_torrent_finished_callback(void)
{
	std::deque<lt::torrent_handle> finished_torrents;

	{
		boost::mutex::scoped_lock lock(this->mutex);
		finished_torrents.swap(this->finished_torrents);
	}

	for(size_t i = 0; i < finished_torrents.size(); i++)
	{
		try
		{
			this->finish_torrent(this->get_torrent(finished_torrents[i]));
		}
		catch(m::Exception)
		{
			// Такого торрента уже не существует, следовательно,
			// это сообщение уже не актуально.
		}
	}
}



void Daemon_session::on_torrent_resume_data_callback(void)
{
	Torrent_id torrent_id;
	lt::entry resume_data;

	{
		boost::mutex::scoped_lock lock(this->mutex);
		torrent_id = this->torrents_resume_data.front().first;
		resume_data = this->torrents_resume_data.front().second;
		this->torrents_resume_data.pop();
	}

	this->pending_torrent_resume_data--;
	save_torrent_settings_if_exists(torrent_id, resume_data);
}



bool Daemon_session::on_update_torrents_statistics_callback(void)
{
	time_t current_time = time(NULL);
	time_t time_diff = current_time - this->last_update_torrent_statistics_time;
	this->last_update_torrent_statistics_time = current_time;

	// Если, к примеру, были переведены часы.
	if(time_diff < 0)
		time_diff = UPDATE_TORRENTS_STATISTICS_TIMEOUT;

	M_FOR_IT(this->torrents, it)
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

	// Планируем сохранение настроек торрента
	this->schedule_torrent_settings_saving(torrent);
}



void Daemon_session::remove_torrent(const Torrent_id& torrent_id) throw(m::Exception)
{
	// Удаляем из самой сессии
	this->remove_torrent_from_session(torrent_id);

	// Удаляем конфигурационные файлы торрента
	this->remove_torrent_from_config(torrent_id);
}



void Daemon_session::remove_torrent_from_config(const Torrent_id& torrent_id) const throw(m::Exception)
{
	m::fs::rm(this->get_torrent_dir_path(torrent_id));
}



void Daemon_session::remove_torrent_from_session(const Torrent_id& torrent_id) throw(m::Exception)
{
	Torrent& torrent = this->get_torrent(torrent_id);

	try
	{
		this->session->remove_torrent(torrent.handle);
	}
	catch(lt::invalid_handle e)
	{
		MLIB_W(__("Bad torrent id: '%1'.", torrent.id));
	}

	this->torrents.erase(torrent_id);
}



void Daemon_session::remove_torrent_with_data(const Torrent_id& torrent_id) throw(m::Exception)
{
	Torrent& torrent = this->get_torrent(torrent_id);

	std::string torrent_name = torrent.name;
	std::string download_path = Path(torrent.get_download_path());


	// Получаем список файлов торрента -->
		std::vector<std::string> files_paths;

		try
		{
			files_paths = m::lt::get_torrent_files_paths(torrent.handle.get_torrent_info());
		}
		catch(lt::invalid_handle)
		{
			MLIB_LE();
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
	this->statistics.reset(this->session->status());
}



void Daemon_session::resume_torrent(Torrent& torrent) throw(m::Exception)
{
	bool paused = false;

	try
	{
		const lt::torrent_status torrent_status = torrent.handle.status();
		paused = torrent_status.paused;

		if(paused)
		{
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

	// Планируем сохранение настроек торрента
	if(paused)
		this->schedule_torrent_settings_saving(torrent);
}



void Daemon_session::save_session(void) throw(m::Exception)
{
	MLIB_D("Saving session...");

	// Планируем сохранение настроек торрентов
	M_FOR_CONST_IT(this->torrents, it)
		this->schedule_torrent_settings_saving(it->second);

	// Пишем конфиг демона -->
		try
		{
			Daemon_settings settings = this->get_settings();
			settings.write(
				this->get_config_dir_path(),
				Session_status(
					this->statistics, this->session->status(),
					this->get_rate_limit(DOWNLOAD), this->get_rate_limit(UPLOAD)
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
	Torrents_container::const_iterator torrent_it = this->torrents.find(torrent_id);

	if(torrent_it != this->torrents.end())
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

	this->pending_torrent_resume_data++;
}



void Daemon_session::set_copy_when_finished(Torrent& torrent, bool copy, const std::string& to) throw(m::Exception)
{
	if(copy && !Path(to).is_absolute())
		M_THROW(__("Invalid copy when finished to path '%1'.", to));

	torrent.download_settings.copy_when_finished = copy;
	torrent.download_settings.copy_when_finished_to = to;

	torrent.download_settings_revision++;
}



void Daemon_session::set_files_download_status(Torrent& torrent, const std::vector<int>& files_ids, bool download) throw(m::Exception)
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



void Daemon_session::set_files_priority(Torrent& torrent, const std::vector<int>& files_ids, const Torrent_file_settings::Priority priority) throw(m::Exception)
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
			this->session->set_download_rate_limit(get_lt_rate_limit(speed));
			break;

		case UPLOAD:
			this->session->set_upload_rate_limit(get_lt_rate_limit(speed));
			break;

		default:
			MLIB_LE();
			break;
	}
}



void Daemon_session::set_sequential_download(Torrent& torrent, bool value) throw(m::Exception)
{
	torrent.handle.set_sequential_download(value);
	torrent.download_settings_revision++;
}



void Daemon_session::set_settings(const Daemon_settings& settings, const bool init_settings)
{
	// Диапазон портов для прослушивания
	if(this->settings.listen_ports_range != settings.listen_ports_range || init_settings)
	{
		this->settings.listen_ports_range = settings.listen_ports_range;

		// Порт и 0.0.0.0 присваивать обязательно - иначе из Интернета качаться не будет.
		this->session->listen_on(this->settings.listen_ports_range, "0.0.0.0");
	}


	// DHT -->
		if(this->is_dht_started() != settings.dht || init_settings)
		{
			if(settings.dht)
			{
				if(init_settings)
					this->session->start_dht(settings.dht_state);
				else
					this->session->start_dht();
			}
			else
				this->session->stop_dht();
		}
	// DHT <--

	// LSD -->
		if(this->settings.lsd != settings.lsd || init_settings)
		{
			if( (this->settings.lsd = settings.lsd) )
				this->session->start_lsd();
			else
				this->session->stop_lsd();
		}
	// LSD <--

	// UPnP -->
		if(this->settings.upnp != settings.upnp || init_settings)
		{
			if( (this->settings.upnp = settings.upnp) )
				this->session->start_upnp();
			else
				this->session->stop_upnp();
		}
	// UPnP <--

	// NAT-PMP -->
		if(this->settings.natpmp != settings.natpmp || init_settings)
		{
			if( (this->settings.natpmp = settings.natpmp) )
				this->session->start_natpmp();
			else
				this->session->stop_natpmp();
		}
	// NAT-PMP <--

	// smart_ban, pex -->
	{
		// Расширения можно только подключать, отключить их уже нельзя
		// -->
			if( settings.smart_ban && ( !this->settings.smart_ban || init_settings) )
				this->session->add_extension(&lt::create_smart_ban_plugin);

			if( settings.pex && ( !this->settings.pex || init_settings) )
				this->session->add_extension(&lt::create_ut_pex_plugin);
		// <--

		this->settings.smart_ban = settings.smart_ban;
		this->settings.pex = settings.pex;
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
	if(this->settings.max_uploads != settings.max_uploads || init_settings)
	{
		this->settings.max_uploads = settings.max_uploads;
		this->session->set_max_uploads(settings.max_uploads);
	}

	// Максимальное количество соединений, которое может
	// быть открыто.
	if(this->settings.max_connections != settings.max_connections || init_settings)
	{
		this->settings.max_connections = settings.max_connections;
		this->session->set_max_connections(settings.max_connections);
	}

	// IP фильтр -->
		if(this->settings.ip_filter != settings.ip_filter)
		{
			lt::ip_filter ip_filter;

			for(size_t i = 0; i < settings.ip_filter.size(); i++)
			{
				const Ip_filter_rule& rule = settings.ip_filter[i];

				ip_filter.add_rule(
					lt::address_v4::from_string(rule.from),
					lt::address_v4::from_string(rule.to),
					rule.block ? lt::ip_filter::blocked : 0
				);
			}

			this->session->set_ip_filter(ip_filter);
			this->settings.ip_filter = settings.ip_filter;
		}
	// IP фильтр <--

	// Настройки автоматической "подгрузки" *.torrent файлов. -->
	{
		Daemon_settings::Torrents_auto_load& auto_load = this->settings.torrents_auto_load;

		// Если настройки не эквивалентны
		if(!auto_load.equal(settings.torrents_auto_load))
		{
			// "Сбрасываем" мониторинг
			this->fs_watcher.unset_watching_directory();

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
						this->fs_watcher.set_watching_directory(auto_load.from);
					}
					catch(m::Exception& e)
					{
						auto_load.is = false;

						MLIB_W(
							_("Setting directory for automatic torrents loading failed"),
							__(
								"Can't set directory '%1' for automatic torrents loading. %2",
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
							MLIB_W(
								_("Automatic torrents loading failed"), EE(e)
							);
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
		if(this->settings.torrents_auto_clean != settings.torrents_auto_clean)
		{
			this->settings.torrents_auto_clean = settings.torrents_auto_clean;

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
			MLIB_W(
				_("Automatic torrents loading failed"), EE(e)
			);
		}

		// Обработчик сигнала на появление новых торрентов для автоматического
		// добавления.
		this->fs_watcher_connection = this->fs_watcher.connect(
			sigc::mem_fun(*this, &Daemon_session::auto_load_torrents)
		);
	// <--
}



void Daemon_session::start_torrents(const Torrents_group group)
{
	MLIB_D(_C("Starting torrents [%1].", static_cast<int>(group)));

	M_FOR_IT(this->torrents, it)
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



void Daemon_session::stop_torrents(const Torrents_group group)
{
	MLIB_D(_C("Stopping torrents [%1].", static_cast<int>(group)));

	M_FOR_IT(this->torrents, it)
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
		{
			boost::mutex::scoped_lock lock(this->mutex);

			if(this->is_stop)
			{
				MLIB_D("Daemon messages thread ending it's work.");
				return;
			}
		}

		if(this->session->wait_for_alert(lt::time_duration(static_cast<int64_t>(THREAD_CANCEL_RESPONSE_TIMEOUT * 1000))))
		{
			int added_messages = 0;
			std::auto_ptr<lt::alert> alert;

			while( (alert = this->session->pop_alert()).get() )
			{
				Daemon_message message = Daemon_message(*alert);

				{
					// Resume data
					if( dynamic_cast<lt::save_resume_data_alert*>(alert.get()) )
					{
						lt::save_resume_data_alert* resume_data_alert = dynamic_cast<lt::save_resume_data_alert*>(alert.get());
						Torrent_id torrent_id = Torrent_id(resume_data_alert->handle);

						MLIB_D(_C("Gotten resume data for torrent '%1'.", torrent_id));

						{
							boost::mutex::scoped_lock lock(this->mutex);
							this->torrents_resume_data.push(Torrent_resume_data(torrent_id, *resume_data_alert->resume_data));
						}

						// Извещаем демон о получении новой resume data.
						this->torrent_resume_data_signal();
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
						// data получить не возможно, к примеру, когда
						// данные только проверяются.

						lt::save_resume_data_failed_alert* failed_alert = dynamic_cast<lt::save_resume_data_failed_alert*>(alert.get());
						Torrent_id torrent_id = Torrent_id(failed_alert->handle);

						MLIB_D(_C("Gotten failed resume data for torrent '%1': %2.", torrent_id, failed_alert->msg));

						{
							boost::mutex::scoped_lock lock(this->mutex);
							this->torrents_resume_data.push(Torrent_resume_data(torrent_id, lt::entry()));
						}

						// Извещаем демон о получении новой resume data.
						this->torrent_resume_data_signal();
					}
					else
					{
						// Скачивание торрента завершено.
						// Данное сообщение необходимо обработать по особому.
						if( dynamic_cast<lt::torrent_finished_alert*>(alert.get()) )
						{
							{
								boost::mutex::scoped_lock lock(this->mutex);
								this->finished_torrents.push_back( dynamic_cast<lt::torrent_alert*>( alert.get() )->handle );
							}

							// Извещаем демон о завершении скачивания торрента
							this->torrent_finished_signal();
						}

						// Передаем сообщение пользователю -->
							{
								boost::mutex::scoped_lock lock(this->mutex);
								this->messages.push_back(message);
							}

							added_messages++;
						// Передаем сообщение пользователю <--
					}
				}

				MLIB_D(_C("libtorrent alert: %1", message.get()));
			}

			// Информируем клиент о поступлении новых сообщений.
			if(added_messages)
				this->messages_signal();
		}
	}
}

