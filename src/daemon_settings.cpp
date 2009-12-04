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
#define _STREAM_COMPAT


#include <fstream>

#include <libconfig.h++>

#include <libtorrent/bencode.hpp>
#include <libtorrent/entry.hpp>

#include <mlib/fs.hpp>
#include <mlib/libconfig.hpp>
#include <mlib/libtorrent.hpp>
#include <mlib/misc.hpp>
#include <mlib/string.hpp>

#include "common.hpp"
#include "daemon_settings.hpp"
#include "daemon_types.hpp"


#define CHECK_OPTION_TYPE(setting, type, action)																		\
{																														\
	if(setting.getType() != type)																						\
	{																													\
		MLIB_SW(__("Daemon config: Bad option '%1' type at line %2.", setting.getName(), setting.getSourceLine()));		\
		action;																											\
	}																													\
}



namespace
{
	/// Выводит отладочное сообщение о неверном значении опции.
	template<class T>
	void bad_option_value(const libconfig::Setting& setting, T value);

	/// Извлекает строку пути из конфига.
	void get_path_value(const libconfig::Setting& setting, std::string* place_to);

	/// Выводит отладочное сообщение о неверной кодировке, в которой
	/// представлено значение опции.
	void invalid_option_utf_value(const libconfig::Setting& setting);

	/// Выводит отладочное сообщение о неизвестной опции.
	void unknown_option(const libconfig::Setting& setting);



	template<class T>
	void bad_option_value(const libconfig::Setting& setting, T value)
	{
		MLIB_SW(__(
			"Daemon config: Bad option '%1' value '%2' at line %3.",
			setting.getName(), value, setting.getSourceLine()
		));
	}



	void get_path_value(const libconfig::Setting& setting, std::string* place_to)
	{
		if(m::is_valid_utf(static_cast<const char *>(setting)))
		{
			std::string path = static_cast<const char *>(setting);

			if(path != "")
			{
				if(!Path(path).is_absolute())
					bad_option_value(setting, path);
				else
					*place_to = path;
			}
		}
		else
			invalid_option_utf_value(setting);
	}



	void invalid_option_utf_value(const libconfig::Setting& setting)
	{
		MLIB_SW(__(
			"Daemon config: Invalid option '%1' UTF-8 value at line %2.",
			setting.getName(), setting.getSourceLine()
		));
	}



	void unknown_option(const libconfig::Setting& setting)
	{
		MLIB_SW(__(
			"Daemon config: Unknown option '%1' at line %2.",
			setting.getName(), setting.getSourceLine()
		));
	}
}



// Daemon_settings_light -->
	// Torrents_auto_load -->
		Daemon_settings_light::Torrents_auto_load::Torrents_auto_load(void)
		:
			is(false),
			copy(false),
			delete_loaded(false)
		{
		}



		bool Daemon_settings_light::Torrents_auto_load::equal(const Torrents_auto_load& auto_load) const
		{
			return this->is == auto_load.is && this->from == auto_load.from;
		}
	// Torrents_auto_load <--



	// Auto_clean -->
		Daemon_settings_light::Auto_clean::Auto_clean(void)
		:
			max_seeding_time(0),
			max_ratio(0),
			max_seeding_torrents(0)
		{
		}



		bool Daemon_settings_light::Auto_clean::operator!=(const Auto_clean& clean) const
		{
			return
				this->max_seeding_time			!= clean.max_seeding_time ||
				this->max_seeding_time_type		!= clean.max_seeding_time_type ||
				this->max_ratio					!= clean.max_ratio ||
				this->max_ratio_type			!= clean.max_ratio_type ||
				this->max_seeding_torrents		!= clean.max_seeding_torrents ||
				this->max_seeding_torrents_type	!= clean.max_seeding_torrents_type;
		}
	// Auto_clean <--



	Daemon_settings_light::Daemon_settings_light(void)
	:
		listen_random_port(true),
		listen_ports_range(6000, 8000),

		lsd(false),
		upnp(false),
		natpmp(false),
		smart_ban(true),
		pex(true),

		max_uploads(-1),
		max_connections(-1),

		use_max_announce_interval(false),
		max_announce_interval(30 * 60),

		ip_filter_enabled(false)
	{
	}
// Daemon_settings_light <--



// Daemon_settings -->
	const std::string Daemon_settings::config_file_name = "daemon.conf";
	const std::string Daemon_settings::dht_state_file_name = "dht.state";



	Daemon_settings::Daemon_settings(void)
	:
		dht(true),

		download_rate_limit(-1),
		upload_rate_limit(-1)
	{
	}



	Daemon_settings::Daemon_settings(const Daemon_settings_light& settings)
	{
		this->assign(settings);
	}



	void Daemon_settings::assign(const Daemon_settings_light &settings)
	{
		*static_cast<Daemon_settings_light*>(this) = settings;
	}



	void Daemon_settings::read(const std::string& config_dir_path, Daemon_statistics* statistics)
	{
		Errors_pool errors;

		try
		{
			this->read_config( Path(config_dir_path) / this->config_file_name, statistics );
		}
		catch(m::Exception& e)
		{
			errors += EE(e);
		}

		try
		{
			this->read_dht_state( Path(config_dir_path) / this->dht_state_file_name );
		}
		catch(m::Exception& e)
		{
			errors += EE(e);
		}

		errors.throw_if_exists();
	}



	void Daemon_settings::read_auto_clean_settings(const libconfig::Setting& setting)
	{
		Auto_clean& clean = this->torrents_auto_clean;

		int int_val;
		float float_val;
		std::string string_val;


		if(setting.lookupValue("max_seeding_time", int_val))
		{
			if(int_val >= 0)
			{
				clean.max_seeding_time = int_val;

				if(setting.lookupValue("max_seeding_time_type", string_val))
				{
					try
					{
						clean.max_seeding_time_type = Auto_clean_type::from_string(string_val);
					}
					catch(m::Exception& e)
					{
						bad_option_value(setting, EE(e));
					}
				}
			}
			else
				bad_option_value(setting, int_val);
		}

		if(setting.lookupValue("max_share_ratio", float_val))
		{
			if(float_val >= 0)
			{
				clean.max_ratio = float_val;

				if(setting.lookupValue("max_share_ratio_type", string_val))
				{
					try
					{
						clean.max_ratio_type = Auto_clean_type::from_string(string_val);
					}
					catch(m::Exception& e)
					{
						bad_option_value(setting, EE(e));
					}
				}
			}
			else
				bad_option_value(setting, float_val);
		}

		if(setting.lookupValue("max_seeding_torrents", int_val))
		{
			if(int_val >= 0)
			{
				clean.max_seeding_torrents = int_val;

				if(setting.lookupValue("max_seeding_torrents_type", string_val))
				{
					try
					{
						clean.max_seeding_torrents_type = Auto_clean_type::from_string(string_val);
					}
					catch(m::Exception& e)
					{
						bad_option_value(setting, EE(e));
					}
				}
			}
			else
				bad_option_value(setting, int_val);
		}
	}



	// Для совместимости с версиями < 0.5
	COMPATIBILITY
	void Daemon_settings::read_auto_delete_settings(const libconfig::Setting& root)
	{
		Auto_clean_type type;

		bool	auto_delete = false;
		bool	auto_delete_with_data = false;

		int		max_seeding_time;
		float	max_ratio;
		int		max_seeding_torrents;


		root.lookupValue("auto_delete_torrents", auto_delete);
		root.lookupValue("auto_delete_torrents_with_data", auto_delete_with_data);

		if(auto_delete)
		{
			if(auto_delete_with_data)
				type = Auto_clean_type::REMOVE_WITH_DATA;
			else
				type = Auto_clean_type::REMOVE;
		}

		if(root.lookupValue("auto_delete_torrents_max_seed_time", max_seeding_time))
		{
			if(max_seeding_time >= 0)
			{
				this->torrents_auto_clean.max_seeding_time_type = type;
				this->torrents_auto_clean.max_seeding_time = max_seeding_time;
			}
		}

		if(root.lookupValue("auto_delete_torrents_max_share_ratio", max_ratio))
		{
			if(max_ratio > 0)
			{
				this->torrents_auto_clean.max_ratio_type = type;
				this->torrents_auto_clean.max_ratio = max_ratio;
			}
		}

		if(root.lookupValue("auto_delete_torrents_max_seeds", max_seeding_torrents))
		{
			if(max_seeding_torrents >= 0)
			{
				this->torrents_auto_clean.max_seeding_torrents_type = type;
				this->torrents_auto_clean.max_seeding_torrents = max_seeding_torrents;
			}
		}
	}



	void Daemon_settings::read_auto_load_settings(const libconfig::Setting& group_setting)
	{
		Torrents_auto_load& auto_load = this->torrents_auto_load;

		for(int i = 0; i < group_setting.getLength(); i++)
		{
			const libconfig::Setting& setting = group_setting[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "is"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				auto_load.is = setting;
			}
			else if(m::is_eq(setting_name, "from"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)
				get_path_value(setting, &auto_load.from);
			}
			else if(m::is_eq(setting_name, "to"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)
				get_path_value(setting, &auto_load.to);
			}
			else if(m::is_eq(setting_name, "copy"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				auto_load.copy = setting;
			}
			else if(m::is_eq(setting_name, "copy_to"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)
				get_path_value(setting, &auto_load.copy_to);
			}
			else if(m::is_eq(setting_name, "delete_loaded"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				auto_load.delete_loaded = setting;
			}
			else
				unknown_option(setting);
		}

		// Проверяем полученные значения -->
			if(auto_load.is && (auto_load.from == "" || auto_load.to == ""))
				auto_load.is = false;

			if(auto_load.copy && auto_load.copy_to == "")
				auto_load.copy = false;
		// Проверяем полученные значения <--
	}



	void Daemon_settings::read_config(const std::string& config_path, Daemon_statistics* statistics)
	{
		libconfig::Config config;
		std::string real_config_path = config_path;
		Version config_version = M_GET_VERSION(0, 1, 0);

		try
		{
			real_config_path = m::fs::config::start_reading(config_path);

			if(m::fs::is_exists(real_config_path))
				config.readFile(U2L(real_config_path).c_str());
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Reading configuration file '%1' failed. %2", m::fs::get_abs_path_lazy(config_path), EE(e)));
		}
		catch(libconfig::FileIOException& e)
		{
			M_THROW(__("Error while reading configuration file '%1': %2.", m::fs::get_abs_path_lazy(real_config_path), EE(e)));
		}
		catch(libconfig::ParseException& e)
		{
			M_THROW(__("Error while parsing configuration file '%1': %2.", m::fs::get_abs_path_lazy(real_config_path), EE(e)));
		}

		const libconfig::Setting& config_root = config.getRoot();

		try
		{
			config_version = static_cast<m::libconfig::Version>(config_root["version"]);
		}
		catch(libconfig::SettingNotFoundException)
		{
		}

		// Автоматическое удаление торрентов -->
		{
			// Для совместимости с версиями < 0.5
			COMPATIBILITY
			if(config_version < M_GET_VERSION(0, 5, 0))
				this->read_auto_delete_settings(config_root);
		}
		// Автоматическое удаление торрентов <--

		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "version"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Version_type, continue)
			}
			else if(m::is_eq(setting_name, "libtorrent_version"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Version_type, continue)
			}
			else if(m::is_eq(setting_name, "listen_random_port"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->listen_random_port = setting;
			}
			else if(m::is_eq(setting_name, "listen_ports_range"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeArray, continue)

				if(setting.getLength() != 2)
				{
					bad_option_value(setting, _C("size = %1", setting.getLength()));
					continue;
				}

				CHECK_OPTION_TYPE(setting[0], libconfig::Setting::TypeInt, continue)

				// Для совместимости с версиями < 0.7
				COMPATIBILITY
				if(config_version < M_GET_VERSION(0, 7, 0) && static_cast<int>(setting[0]) == 0)
					this->listen_random_port = true;
				else if
				(
					static_cast<int>(setting[0]) < m::PORT_MIN || static_cast<int>(setting[0]) > m::PORT_MAX ||
					static_cast<int>(setting[1]) < m::PORT_MIN || static_cast<int>(setting[1]) > m::PORT_MAX ||
					static_cast<int>(setting[0]) > static_cast<int>(setting[1])
				)
				{
					this->listen_random_port = true;

					bad_option_value(
						setting,
						_C(
							"start_port = %1, end_port = %2",
							static_cast<int>(setting[0]), static_cast<int>(setting[1])
						)
					);
				}
				else
					this->listen_ports_range = std::pair<int, int>(setting[0], setting[1]);
			}
			else if(
				// Для совместимости с версиями < 0.5
				COMPATIBILITY
				config_version < M_GET_VERSION(0, 5, 0) && (
					m::is_eq(setting_name, "auto_delete_torrents") ||
					m::is_eq(setting_name, "auto_delete_torrents_with_data") ||
					m::is_eq(setting_name, "auto_delete_torrents_max_seed_time") ||
					m::is_eq(setting_name, "auto_delete_torrents_max_share_ratio") ||
					m::is_eq(setting_name, "auto_delete_torrents_max_seeds")
				)
			)
				;
			else if(m::is_eq(setting_name, "dht"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->dht = setting;
			}
			else if(m::is_eq(setting_name, "lsd"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->lsd = setting;
			}
			else if(m::is_eq(setting_name, "upnp"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->upnp = setting;
			}
			else if(m::is_eq(setting_name, "natpmp"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->natpmp = setting;
			}
			else if(m::is_eq(setting_name, "smart_ban"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->smart_ban = setting;
			}
			else if(m::is_eq(setting_name, "pex"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->pex = setting;
			}
			else if(m::is_eq(setting_name, "download_rate_limit"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Speed_type, continue)

				if(static_cast<Speed>( static_cast<m::libconfig::Speed>(setting) ) < -1)
					bad_option_value(setting, static_cast<m::libconfig::Speed>(setting));
				else
					this->download_rate_limit = get_rate_limit_from_lt(static_cast<m::libconfig::Speed>(setting));
			}
			else if(m::is_eq(setting_name, "upload_rate_limit"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Speed_type, continue)

				if(static_cast<Speed>( static_cast<m::libconfig::Speed>(setting) ) < -1)
					bad_option_value(setting, static_cast<m::libconfig::Speed>(setting));
				else
					this->upload_rate_limit = get_rate_limit_from_lt(static_cast<m::libconfig::Speed>(setting));
			}
			else if(m::is_eq(setting_name, "max_uploads"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeInt, continue)

				if(static_cast<int>(setting) < -1)
					bad_option_value(setting, static_cast<int>(setting));
				else
					this->max_uploads = setting;
			}
			else if(m::is_eq(setting_name, "max_connections"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeInt, continue)

				if(static_cast<int>(setting) < -1)
					bad_option_value(setting, static_cast<int>(setting));
				else
					this->max_connections = setting;
			}
			else if(m::is_eq(setting_name, "enable_max_announce_interval"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->use_max_announce_interval = setting;
			}
			else if(m::is_eq(setting_name, "max_announce_interval"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Time_type, continue)

				const Time min_interval = 5 * 60;
				m::libconfig::Time interval = static_cast<m::libconfig::Time>(setting);

				if(interval < min_interval)
				{
					bad_option_value(setting, interval);
					interval = min_interval;
				}

				this->max_announce_interval = interval;
			}
			else if(m::is_eq(setting_name, "ip_filter_enabled"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->ip_filter_enabled = setting;
			}
			else if(m::is_eq(setting_name, "ip_filter"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeList, continue)
				this->read_ip_filter_settings(setting);
			}
			else if(m::is_eq(setting_name, "auto_load_torrents"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->read_auto_load_settings(setting);
			}
			else if(m::is_eq(setting_name, "auto_clean_torrents"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->read_auto_clean_settings(setting);
			}
			else if(m::is_eq(setting_name, "statistics"))
			{
				const libconfig::Setting& parent_setting = setting;

				for(int i = 0; i < parent_setting.getLength(); i++)
				{
					const libconfig::Setting& setting = parent_setting[i];
					const char* setting_name = setting.getName();

					if(m::is_eq(setting_name, "statistics_start_time"))
					{
						CHECK_OPTION_TYPE(setting, m::libconfig::Time_type, continue)

						statistics->statistics_start_time = static_cast<m::libconfig::Time>(setting);

						if(statistics->statistics_start_time < 0 || statistics->statistics_start_time > statistics->session_start_time)
						{
							bad_option_value(setting, static_cast<m::libconfig::Time>(setting));
							statistics->statistics_start_time = statistics->session_start_time;
						}
					}
					else if(m::is_eq(setting_name, "total_download"))
					{
						CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)

						if(static_cast<Size>( static_cast<m::libconfig::Size>(setting) ) < -1)
							bad_option_value(setting, static_cast<m::libconfig::Size>(setting));
						else
							statistics->total_download = static_cast<m::libconfig::Size>(setting);
					}
					else if(m::is_eq(setting_name, "total_payload_download"))
					{
						CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)

						if(static_cast<Size>( static_cast<m::libconfig::Size>(setting) ) < -1)
							bad_option_value(setting, static_cast<m::libconfig::Size>(setting));
						else
							statistics->total_payload_download = static_cast<m::libconfig::Size>(setting);
					}
					else if(m::is_eq(setting_name, "total_upload"))
					{
						CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)

						if(static_cast<Size>( static_cast<m::libconfig::Size>(setting) ) < -1)
							bad_option_value(setting, static_cast<m::libconfig::Size>(setting));
						else
							statistics->total_upload = static_cast<m::libconfig::Size>(setting);
					}
					else if(m::is_eq(setting_name, "total_payload_upload"))
					{
						CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)

						if(static_cast<Size>( static_cast<m::libconfig::Size>(setting) ) < -1)
							bad_option_value(setting, static_cast<m::libconfig::Size>(setting));
						else
							statistics->total_payload_upload = static_cast<m::libconfig::Size>(setting);
					}
					else if(m::is_eq(setting_name, "total_failed"))
					{
						CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)

						if(static_cast<Size>( static_cast<m::libconfig::Size>(setting) ) < -1)
							bad_option_value(setting, static_cast<m::libconfig::Size>(setting));
						else
							statistics->total_failed = static_cast<m::libconfig::Size>(setting);
					}
					else if(m::is_eq(setting_name, "total_redundant"))
					{
						CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)

						if(static_cast<Size>( static_cast<m::libconfig::Size>(setting) ) < -1)
							bad_option_value(setting, static_cast<m::libconfig::Size>(setting));
						else
							statistics->total_redundant = static_cast<m::libconfig::Size>(setting);
					}
					else
						unknown_option(setting);
				}
			}
			else
				unknown_option(setting);
		}

		// Для совместимости с версиями < 0.7
		COMPATIBILITY
		if(config_version < M_GET_VERSION(0, 7, 0))
			this->listen_random_port = false;

		// Для совместимости с версиями < 0.9
		COMPATIBILITY
		if(config_version < M_GET_VERSION(0, 9, 0))
			if(!this->ip_filter.empty())
				this->ip_filter_enabled = true;
	}



	void Daemon_settings::read_dht_state(std::string dht_state_path)
	{
		try
		{
			dht_state_path = m::fs::config::start_reading(dht_state_path);

			if(m::fs::is_exists(dht_state_path))
			{
				m::Buffer file_buf;

				file_buf.load_file(dht_state_path);

				try
				{
					this->dht_state = lt::bdecode(file_buf.get_data(), file_buf.get_cur_ptr());
				}
				catch(lt::invalid_encoding)
				{
					M_THROW(_("invalid DHT state data"));
				}
			}
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Error while reading DHT state from file '%1': %2.", dht_state_path, EE(e)));
		}
	}



	void Daemon_settings::read_ip_filter_settings(const libconfig::Setting& filter_setting)
	{
		// При любой ошибке прекращаем парсинг, т. к. лучше вообще не загружать
		// фильтр, если он битый.

		Ip_filter_rule rule;
		std::vector<Ip_filter_rule> ip_filter;

		for(int i = 0; i < filter_setting.getLength(); i++)
		{
			const libconfig::Setting& setting = filter_setting[i];
			CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, return)

			if(
				setting.lookupValue("from", rule.from) &&
				setting.lookupValue("to", rule.to) &&
				setting.lookupValue("block", rule.block)
			)
			{
				try
				{
					rule.check();
					ip_filter.push_back(rule);
				}
				catch(m::Exception& e)
				{
					MLIB_SW(__("Daemon config: Invalid IP filter rule at line %1: %2.",
						setting.getSourceLine(), EE(e) ));
				}
			}
			else
			{
				MLIB_SW(__("Daemon config: Invalid IP filter rule at line %1.", setting.getSourceLine()));
				return;
			}
		}

		this->ip_filter.swap(ip_filter);
	}



	void Daemon_settings::write(const std::string& config_dir_path, const Session_status& session_status) const
	{
		Errors_pool errors;

		try
		{
			this->write_config( Path(config_dir_path) / this->config_file_name, session_status );
		}
		catch(m::Exception& e)
		{
			errors += EE(e);
		}

		try
		{
			this->write_dht_state( Path(config_dir_path) / this->dht_state_file_name );
		}
		catch(m::Exception& e)
		{
			errors += EE(e);
		}

		errors.throw_if_exists();
	}



	void Daemon_settings::write_config(const std::string& config_path, const Session_status& session_status) const
	{
		libconfig::Config config;
		libconfig::Setting& config_root = config.getRoot();

		// Пишем все необходимые настройки -->
			config_root.add("version", m::libconfig::Version_type) = static_cast<m::libconfig::Version>(APP_VERSION);
			config_root.add("libtorrent_version", m::libconfig::Version_type) = static_cast<m::libconfig::Version>(M_LT_GET_VERSION());

			config_root.add("listen_random_port", libconfig::Setting::TypeBoolean) = this->listen_random_port;
			{
				libconfig::Setting& setting = config_root.add("listen_ports_range", libconfig::Setting::TypeArray);
				setting.add(libconfig::Setting::TypeInt) = this->listen_ports_range.first;
				setting.add(libconfig::Setting::TypeInt) = this->listen_ports_range.second;
			}

			config_root.add("dht", libconfig::Setting::TypeBoolean) = this->dht;
			config_root.add("lsd", libconfig::Setting::TypeBoolean) = this->lsd;
			config_root.add("upnp", libconfig::Setting::TypeBoolean) = this->upnp;
			config_root.add("natpmp", libconfig::Setting::TypeBoolean) = this->natpmp;
			config_root.add("smart_ban", libconfig::Setting::TypeBoolean) = this->smart_ban;
			config_root.add("pex", libconfig::Setting::TypeBoolean) = this->pex;

			config_root.add("download_rate_limit", m::libconfig::Speed_type) = static_cast<m::libconfig::Speed>(get_lt_rate_limit(this->download_rate_limit));
			config_root.add("upload_rate_limit", m::libconfig::Speed_type) = static_cast<m::libconfig::Speed>(get_lt_rate_limit(this->upload_rate_limit));

			config_root.add("max_uploads", libconfig::Setting::TypeInt) = this->max_uploads;
			config_root.add("max_connections", libconfig::Setting::TypeInt) = this->max_connections;

			config_root.add("enable_max_announce_interval", libconfig::Setting::TypeBoolean) =
				this->use_max_announce_interval;
			config_root.add("max_announce_interval", m::libconfig::Time_type) =
				static_cast<m::libconfig::Time>(this->max_announce_interval);

			// IP фильтр -->
				config_root.add("ip_filter_enabled", libconfig::Setting::TypeBoolean) = this->ip_filter_enabled;

				if(!this->ip_filter.empty())
				{
					libconfig::Setting& filter_setting = config_root.add("ip_filter", libconfig::Setting::TypeList);

					for(size_t i = 0; i < this->ip_filter.size(); i++)
					{
						const Ip_filter_rule& rule = ip_filter[i];
						libconfig::Setting& rule_setting = filter_setting.add(libconfig::Setting::TypeGroup);

						rule_setting.add("from", libconfig::Setting::TypeString) = rule.from;
						rule_setting.add("to", libconfig::Setting::TypeString) = rule.to;
						rule_setting.add("block", libconfig::Setting::TypeBoolean) = rule.block;
					}
				}
			// IP фильтр <--

			// Автоматическая загрузка торрентов из директории -->
			{
				const Torrents_auto_load& auto_load = this->torrents_auto_load;

				libconfig::Setting& setting = config_root.add("auto_load_torrents", libconfig::Setting::TypeGroup);

				setting.add("is", libconfig::Setting::TypeBoolean) = auto_load.is;
				setting.add("from", libconfig::Setting::TypeString) = auto_load.from;
				setting.add("to", libconfig::Setting::TypeString) = auto_load.to;

				setting.add("copy", libconfig::Setting::TypeBoolean) = auto_load.copy;
				setting.add("copy_to", libconfig::Setting::TypeString) = auto_load.copy_to;

				setting.add("delete_loaded", libconfig::Setting::TypeBoolean) = auto_load.delete_loaded;
			}
			// Автоматическая загрузка торрентов из директории <--

			// Автоматическая "очистка" от старых торрентов -->
			{
				const Auto_clean& clean = this->torrents_auto_clean;

				libconfig::Setting& setting = config_root.add("auto_clean_torrents", libconfig::Setting::TypeGroup);

				setting.add("max_seeding_time", libconfig::Setting::TypeInt) =
					clean.max_seeding_time;
				setting.add("max_seeding_time_type", libconfig::Setting::TypeString) =
					clean.max_seeding_time_type.to_string();

				setting.add("max_share_ratio", libconfig::Setting::TypeFloat) =
					static_cast<float>(clean.max_ratio);
				setting.add("max_share_ratio_type", libconfig::Setting::TypeString) =
					clean.max_ratio_type.to_string();

				setting.add("max_seeding_torrents", libconfig::Setting::TypeInt) =
					static_cast<int>(clean.max_seeding_torrents);
				setting.add("max_seeding_torrents_type", libconfig::Setting::TypeString) =
					clean.max_seeding_torrents_type.to_string();
			}
			// Автоматическая "очистка" от старых торрентов <--

			// statistics -->
			{
				libconfig::Setting& setting = config_root.add("statistics", libconfig::Setting::TypeGroup);

				setting.add("statistics_start_time", m::libconfig::Size_type) = session_status.statistics_start_time;
				setting.add("total_download", m::libconfig::Size_type) = session_status.total_download;
				setting.add("total_payload_download", m::libconfig::Size_type) = session_status.total_payload_download;
				setting.add("total_upload", m::libconfig::Size_type) = session_status.total_upload;
				setting.add("total_payload_upload", m::libconfig::Size_type) = session_status.total_payload_upload;
				setting.add("total_failed", m::libconfig::Size_type) = session_status.total_failed;
				setting.add("total_redundant", m::libconfig::Size_type) = session_status.total_redundant;
			}
			// statistics <--
		// Пишем все необходимые настройки <--

		// Сохраняем полученные настройки в файл -->
		{
			std::string real_config_path = config_path;

			try
			{
				real_config_path = m::fs::config::start_writing(config_path);
				config.writeFile(U2L(real_config_path).c_str());
				m::fs::sync_file(real_config_path);
				m::fs::config::end_writing(config_path);
			}
			catch(m::Exception& e)
			{
				M_THROW(__("Writing configuration file '%1' failed. %2", m::fs::get_abs_path_lazy(config_path), EE(e)));
			}
			catch(libconfig::FileIOException e)
			{
				M_THROW(__("Can't write configuration file %1: %2.", m::fs::get_abs_path_lazy(real_config_path), EE(e)));
			}
		}
		// Сохраняем полученные настройки в файл <--
	}



	void Daemon_settings::write_dht_state(const std::string& dht_state_path) const
	{
		try
		{
			// Генерирует m::Exception.
			std::string real_dht_state_path = m::fs::config::start_writing(dht_state_path);

			// Не вносим в try-блок, чтобы при ошибке деструктор не
			// перезаписал значение errno.
			std::ofstream dht_state_file;

			try
			{
				dht_state_file.exceptions(std::ofstream::failbit | std::ofstream::badbit);

				dht_state_file.open(
					U2L(real_dht_state_path).c_str(),
					std::ios::out | std::ios::binary | std::ios::trunc
				);

				lt::bencode(
					std::ostream_iterator<char>(dht_state_file),
					this->dht_state
				);

				dht_state_file.close();
			}
			catch(std::ofstream::failure& e)
			{
				M_THROW(__("Can't write DHT state to file '%1': %2.", real_dht_state_path, EE()));
			}

			// Генерирует m::Exception.
			m::fs::sync_file(real_dht_state_path);

			// Генерирует m::Exception.
			m::fs::config::end_writing(dht_state_path);
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Writing DHT state to file '%1' failed. %2", dht_state_path, EE(e)));
		}
	}



	Daemon_settings& Daemon_settings::operator=(const Daemon_settings_light &settings)
	{
		if(this != &settings)
			this->assign(settings);

		return *this;
	}
// Daemon_settings <--



// Torrent_settings -->
	const std::string Torrent_settings::config_file_name = "torrent.conf";
	const std::string Torrent_settings::resume_data_file_name = "resume_data";



	Torrent_settings::Torrent_settings(const std::string& download_path)
	:
		time_added(time(NULL)),
		time_seeding(0),

		is_paused(true),
		download_path(download_path),

		download_settings(""),
		encoding(MLIB_UTF_CHARSET_NAME),

		total_download(0),
		total_payload_download(0),
		total_upload(0),
		total_payload_upload(0),
		total_failed(0),
		total_redundant(0),

		bytes_done(0),
		// Чтобы мы почувствовали разницу даже когда торрент имеет размер 0 байт
		bytes_done_on_last_torrent_finish(-1)
	{
	}



	Torrent_settings::Torrent_settings(
		const std::string& magnet,
		const std::string& name,
		bool paused,
		const std::string& download_path,
		const Download_settings& download_settings,
		const std::string& encoding,
		const std::vector<Torrent_file_settings>& files_settings,
		const std::vector<std::string>& trackers
	)
	:
		magnet(magnet),
		name(name),

		time_added(time(NULL)),
		time_seeding(0),

		is_paused(paused),
		download_path(download_path),

		download_settings(download_settings),
		encoding(encoding),
		files_settings(files_settings),
		trackers(trackers),

		total_download(0),
		total_payload_download(0),
		total_upload(0),
		total_payload_upload(0),
		total_failed(0),
		total_redundant(0),

		bytes_done(0),
		// Чтобы мы почувствовали разницу даже когда торрент имеет размер 0 байт
		bytes_done_on_last_torrent_finish(-1)
	{
	}



	Torrent_settings::Torrent_settings(const Torrent& torrent, const lt::entry& resume_data)
	:
		magnet(torrent.magnet),
		name(torrent.name),

		time_added(torrent.time_added),
		time_seeding(torrent.time_seeding),

		is_paused(torrent.is_paused()),
		download_path(torrent.get_download_path()),

		download_settings(torrent.get_download_settings()),

		encoding(torrent.encoding),
		files_settings(torrent.files_settings),
		resume_data(resume_data),

		bytes_done_on_last_torrent_finish(torrent.bytes_done_on_last_torrent_finish)
	{
		lt::torrent_status status = torrent.handle.status();
		this->trackers = m::lt::get_torrent_trackers(torrent.handle);

		// Получаем текущую статистику торрента -->
			this->total_download         = torrent.total_download + status.total_download;
			this->total_payload_download = torrent.total_payload_download + status.total_payload_download;
			this->total_upload           = torrent.total_upload + status.total_upload;
			this->total_payload_upload   = torrent.total_payload_upload + status.total_payload_upload;
			this->total_failed           = torrent.total_failed + status.total_failed_bytes;
			this->total_redundant        = torrent.total_redundant + status.total_redundant_bytes;
		// Получаем текущую статистику торрента <--

		this->bytes_done = status.total_done;
	}



	void Torrent_settings::read(const std::string& settings_dir_path, Read_flags* flags)
	{
		Errors_pool errors;

		try
		{
			this->read_config(Path(settings_dir_path) / this->config_file_name, flags);
		}
		catch(m::Exception& e)
		{
			errors += EE(e);
		}

		try
		{
			this->read_resume_data(Path(settings_dir_path) / this->resume_data_file_name);
		}
		catch(m::Exception& e)
		{
			errors += EE(e);
		}

		errors.throw_if_exists();
	}



	void Torrent_settings::read_config(const std::string& config_path, Read_flags* flags)
	{
		Version daemon_version = M_GET_VERSION(0, 0, 0);
		libconfig::Config config;

		// Генерирует m::Exception
		this->read_config_data(&config, config_path);

		const libconfig::Setting& config_root = config.getRoot();

		// Получаем версию демона, который производил запись конфига -->
			try
			{
				const libconfig::Setting& setting = config_root["version"];

				CHECK_OPTION_TYPE(setting, m::libconfig::Version_type, M_THROW_EMPTY())
				daemon_version = static_cast<m::libconfig::Version>(setting);
			}
			catch(m::Exception)
			{
			}
			catch(libconfig::SettingNotFoundException)
			{
			}
		// Получаем версию демона, который производил запись конфига <--

		// Устанавливаем специальное значение, чтобы при переходе на новую
		// версию Flush при первом запуске не выдал кучу нотификаций.
		// -->
			COMPATIBILITY
			if(daemon_version < M_GET_VERSION(0, 9, 0))
				this->bytes_done_on_last_torrent_finish = -2;
		// <--

		for(int setting_id = 0; setting_id < config_root.getLength(); setting_id++)
		{
			const libconfig::Setting& setting = config_root[setting_id];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "version"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Version_type, continue)
			}
			else if(m::is_eq(setting_name, "libtorrent_version"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Version_type, continue)
			}
			else if(m::is_eq(setting_name, "magnet"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				std::string magnet = static_cast<const char *>(setting);

				try
				{
					if(!m::lt::is_magnet_uri(magnet))
						M_THROW_EMPTY();

					m::lt::get_torrent_metadata(magnet);

					this->magnet = magnet;
				}
				catch(m::Exception& e)
				{
					bad_option_value(setting, magnet);
				}
			}
			else if(m::is_eq(setting_name, "name"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				Glib::ustring name = static_cast<const char *>(setting);

				if(m::is_valid_utf(name))
				{
					if(!m::is_empty_string(name))
						this->name = name;
				}
				else
					invalid_option_utf_value(setting);
			}
			else if(m::is_eq(setting_name, "time_added"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Time_type, continue)
				this->time_added = static_cast<m::libconfig::Time>(setting);
			}
			else if(m::is_eq(setting_name, "time_seeding"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Time_type, continue)
				this->time_seeding = static_cast<m::libconfig::Time>(setting);
			}
			else if(m::is_eq(setting_name, "download_path"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)
				this->download_path = static_cast<const char*>(setting);
			}
			else if(m::is_eq(setting_name, "sequential_download"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->download_settings.sequential_download = setting;
			}
			// Для совместимости с версиями < 0.4
			COMPATIBILITY
			else if(m::is_eq(setting_name, "copy_on_finished_path"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				std::string to_path = L2U(static_cast<const char*>(setting));

				if(to_path != "")
				{
					if(Path(to_path).is_absolute())
					{
						this->download_settings.copy_when_finished = true;
						this->download_settings.copy_when_finished_to = to_path;
					}
					else
						bad_option_value(setting, to_path);
				}
			}
			else if(m::is_eq(setting_name, "copy_when_finished_to"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				std::string to_path = static_cast<const char*>(setting);

				if(m::is_valid_utf(to_path))
				{
					if(to_path != "")
					{
						if(Path(to_path).is_absolute())
						{
							this->download_settings.copy_when_finished = false;
							this->download_settings.copy_when_finished_to = to_path;

							try
							{
								libconfig::Setting& is_setting = config_root["copy_when_finished"];

								CHECK_OPTION_TYPE(is_setting, libconfig::Setting::TypeBoolean, continue)
								this->download_settings.copy_when_finished = is_setting;
							}
							catch(libconfig::SettingNotFoundException)
							{
							}
						}
						else
							bad_option_value(setting, to_path);
					}
				}
				else
					invalid_option_utf_value(setting);
			}
			else if(m::is_eq(setting_name, "copy_when_finished"))
			{
				// Пропускаем данную опцию. Ее мы обработаем в другой ветке.
			}
			// Для совместимости с версиями < 0.2
			COMPATIBILITY
			else if(m::is_eq(setting_name, "files_priorities"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeArray, continue)

				std::vector<Torrent_file_settings> files_settings;
				files_settings.resize(setting.getLength());

				if(setting.getLength())
				{
					CHECK_OPTION_TYPE(setting[0], libconfig::Setting::TypeInt, continue)

					for(int file_id = 0; file_id < setting.getLength(); file_id++)
						files_settings[file_id].download = bool(static_cast<int>(setting[file_id]));
				}

				this->files_settings.swap(files_settings);
			}
			else if(m::is_eq(setting_name, "encoding"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				std::string encoding = static_cast<const char*>(setting);

				if(m::is_valid_encoding_name(encoding))
					this->encoding = encoding;
				else
					bad_option_value(setting, encoding);
			}
			else if(m::is_eq(setting_name, "files"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeList, continue)

				std::vector<Torrent_file_settings> files_settings;
				files_settings.resize(setting.getLength());

				for(int file_id = 0; file_id < setting.getLength(); file_id++)
				{
					const libconfig::Setting& file_setting = setting[file_id];

					CHECK_OPTION_TYPE(file_setting, libconfig::Setting::TypeGroup, continue)

					for(int i = 0; i < file_setting.getLength(); i++)
					{
						const libconfig::Setting& setting = file_setting[i];
						const char* setting_name = setting.getName();

						if(m::is_eq(setting_name, "path"))
						{
							CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

							// Т. к. раньше конфигурационный файл писался в кодировке локали.
							COMPATIBILITY
							if(daemon_version < M_GET_VERSION(0, 4, 0))
							{
								std::string path = L2U(static_cast<const char*>(setting));

								if(path != "")
									files_settings[file_id].path = path;
							}
							else
							{
								std::string path = static_cast<const char*>(setting);

								if(m::is_valid_utf(path))
								{
									if(Path(path).is_absolute())
										files_settings[file_id].path = path;
									else
										bad_option_value(setting, path);
								}
								else
									invalid_option_utf_value(setting);
							}
						}
						else if(m::is_eq(setting_name, "download"))
						{
							CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
							files_settings[file_id].download = setting;
						}
						else if(m::is_eq(setting_name, "priority"))
						{
							CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

							try
							{
								files_settings[file_id].set_priority_by_name(setting);
							}
							catch(m::Exception& e)
							{
								MLIB_SW(__("Daemon config: %1", EE(e)));
							}
						}
						else
							unknown_option(setting);
					}
				}

				this->files_settings.swap(files_settings);
			}
			else if(m::is_eq(setting_name, "trackers"))
			{
				int tracker_id;
				std::vector<std::string> trackers;

				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeArray, continue)

				trackers.reserve(setting.getLength());

				for(tracker_id = 0; tracker_id < setting.getLength(); tracker_id++)
				{
					const libconfig::Setting& tracker_setting = setting[tracker_id];

					if(!tracker_id)
						CHECK_OPTION_TYPE(tracker_setting, libconfig::Setting::TypeString, break)

					std::string tracker = static_cast<const char*>(tracker_setting);

					if(m::is_valid_utf(tracker))
					{
						if(!m::is_empty_string(tracker))
							trackers.push_back(tracker);
						else
							bad_option_value(setting, tracker);
					}
					else
						invalid_option_utf_value(tracker_setting);
				}

				if(!setting.getLength() || !trackers.empty())
				{
					this->trackers = trackers;
					*flags |= READ_FLAG_TRACKERS_GOTTEN;
				}
			}
			else if(m::is_eq(setting_name, "is_paused"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->is_paused = setting;
			}
			else if(m::is_eq(setting_name, "total_download"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)
				this->total_download = static_cast<m::libconfig::Size>(setting);
			}
			else if(m::is_eq(setting_name, "total_payload_download"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)
				this->total_payload_download = static_cast<m::libconfig::Size>(setting);
			}
			else if(m::is_eq(setting_name, "total_upload"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)
				this->total_upload = static_cast<m::libconfig::Size>(setting);
			}
			else if(m::is_eq(setting_name, "total_payload_upload"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)
				this->total_payload_upload = static_cast<m::libconfig::Size>(setting);
			}
			else if(m::is_eq(setting_name, "total_failed"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)
				this->total_failed = static_cast<m::libconfig::Size>(setting);
			}
			else if(m::is_eq(setting_name, "total_redundant"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)
				this->total_redundant = static_cast<m::libconfig::Size>(setting);
			}
			else if(m::is_eq(setting_name, "bytes_done"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)
				this->bytes_done = static_cast<m::libconfig::Size>(setting);
			}
			else if(m::is_eq(setting_name, "bytes_done_on_last_torrent_finish"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Size_type, continue)
				this->bytes_done_on_last_torrent_finish = static_cast<m::libconfig::Size>(setting);
			}
			else
				unknown_option(setting);
		}
	}



	void Torrent_settings::read_config_data(libconfig::Config* config, const std::string& config_path)
	{
		std::string real_config_path = config_path;

		try
		{
			real_config_path = m::fs::config::start_reading(config_path);
			config->readFile(U2L(real_config_path).c_str());
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Reading torrent configuration file '%1' failed. %2", m::fs::get_abs_path_lazy(config_path), EE(e)));
		}
		catch(libconfig::FileIOException& e)
		{
			M_THROW(__("Error while reading torrent configuration file '%1': %2.", m::fs::get_abs_path_lazy(real_config_path), EE(e)));
		}
		catch(libconfig::ParseException& e)
		{
			M_THROW(__("Error while parsing torrent configuration file '%1': %2.", m::fs::get_abs_path_lazy(real_config_path), EE(e)));
		}
	}



	void Torrent_settings::read_resume_data(std::string resume_data_path)
	{
		try
		{
			resume_data_path = m::fs::config::start_reading(resume_data_path);

			if(!m::fs::is_exists(resume_data_path))
				return;
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Can't read torrent resume data from file '%1': %2.", resume_data_path, EE(e)));
		}

		m::Buffer file_buf;

		try
		{
			file_buf.load_file(resume_data_path);
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Can't read torrent resume data from file '%1': %2.", resume_data_path, EE(e)));
		}

		{
			Version version;
			const char* data_start = file_buf.get_data();
			const char* data_end = file_buf.get_cur_ptr();

			// Получаем номер версии libtorrent, с которой была записана resume data -->
				if(sizeof version > size_t(data_end - data_start))
					M_THROW(__("Error while reading torrent resume data from file '%1': file broken.", resume_data_path));

				memcpy(&version, data_start, sizeof version);
				data_start += sizeof version;

				if(!m::is_valid_version(version))
					M_THROW(__("Error while reading torrent resume data from file '%1': file broken.", resume_data_path));
			// Получаем номер версии libtorrent, с которой была записана resume data <--

			// При переходе от libtorrent 0.13 к libtorrent 0.14, похоже,
			// изменился формат resume data, причем при чтении resume data,
			// записанного при помощи libtorrent 0.13, libtorrent 0.14 не
			// генерировал исключение и возвращал libtorrent::entry, в которой
			// было указано, что в торренте скачано 0 байт.
			//
			// Поэтому, при переходе от 0.13 к 0.14 просто не читаем resume
			// data, позволяя libtorrent проверить файлы.
			// -->
				if
				(
					m::get_major_minor_version(version) == m::get_major_minor_version(m::get_version(0, 13, 0)) &&
					m::get_major_minor_version(m::lt::get_version()) == m::get_major_minor_version(m::get_version(0, 14, 0))
				)
					MLIB_D(_C("Skipping loading resume data '%1': it has been written by another version of libtorrent (%2).", resume_data_path, version));
				else
				{
					try
					{
						this->resume_data = lt::bdecode(data_start, data_end);
					}
					catch(lt::invalid_encoding)
					{
						M_THROW(__("Can't read torrent resume data from file '%1': bad resume data.", resume_data_path));
					}
				}
			// <--
		}
	}



	void Torrent_settings::write(const std::string& settings_dir_path) const
	{
		Errors_pool errors;

		try
		{
			this->write_config(Path(settings_dir_path) / this->config_file_name);
		}
		catch(m::Exception& e)
		{
			errors += EE(e);
		}

		try
		{
			this->write_resume_data(Path(settings_dir_path) / this->resume_data_file_name);
		}
		catch(m::Exception& e)
		{
			errors += EE(e);
		}

		errors.throw_if_exists();
	}



	void Torrent_settings::write_config(const std::string& config_path) const
	{
		libconfig::Config config;
		libconfig::Setting& config_root = config.getRoot();

		config_root.add("version", m::libconfig::Version_type) = static_cast<m::libconfig::Version>(APP_VERSION);
		config_root.add("libtorrent_version", m::libconfig::Version_type) = static_cast<m::libconfig::Version>(M_LT_GET_VERSION());

		if(!this->magnet.empty())
			config_root.add("magnet", libconfig::Setting::TypeString) = this->magnet;
		config_root.add("name", libconfig::Setting::TypeString) = this->name;

		config_root.add("time_added", m::libconfig::Time_type) = static_cast<m::libconfig::Time>(this->time_added);
		config_root.add("time_seeding", m::libconfig::Time_type) = static_cast<m::libconfig::Time>(this->time_seeding);

		config_root.add("download_path", libconfig::Setting::TypeString) = this->download_path;

		config_root.add("sequential_download", libconfig::Setting::TypeBoolean) = this->download_settings.sequential_download;
		config_root.add("copy_when_finished", libconfig::Setting::TypeBoolean) = this->download_settings.copy_when_finished;

		if(!this->download_settings.copy_when_finished_to.empty())
			config_root.add("copy_when_finished_to", libconfig::Setting::TypeString) = this->download_settings.copy_when_finished_to;

		config_root.add("encoding", libconfig::Setting::TypeString) = this->encoding;

		// files -->
		{
			libconfig::Setting& files_setting = config_root.add("files", libconfig::Setting::TypeList);

			for(size_t i = 0; i < this->files_settings.size(); i++)
			{
				libconfig::Setting& file_setting = files_setting.add(libconfig::Setting::TypeGroup);

				if(!this->files_settings[i].path.empty())
					file_setting.add("path", libconfig::Setting::TypeString) = this->files_settings[i].path;

				file_setting.add("download", libconfig::Setting::TypeBoolean) = this->files_settings[i].download;
				file_setting.add("priority", libconfig::Setting::TypeString) = this->files_settings[i].get_priority_name();
			}
		}
		// files <--

		// trackers -->
		{
			libconfig::Setting& trackers_setting = config_root.add("trackers", libconfig::Setting::TypeArray);

			for(size_t i = 0; i < this->trackers.size(); i++)
				trackers_setting.add(libconfig::Setting::TypeString) = this->trackers[i];
		}
		// trackers <--

		config_root.add("is_paused", libconfig::Setting::TypeBoolean) = this->is_paused;
		config_root.add("total_download", m::libconfig::Size_type) = static_cast<m::libconfig::Size>(this->total_download);
		config_root.add("total_payload_download", m::libconfig::Size_type) = static_cast<m::libconfig::Size>(this->total_payload_download);
		config_root.add("total_upload", m::libconfig::Size_type) = static_cast<m::libconfig::Size>(this->total_upload);
		config_root.add("total_payload_upload", m::libconfig::Size_type) = static_cast<m::libconfig::Size>(this->total_payload_upload);
		config_root.add("total_failed", m::libconfig::Size_type) = static_cast<m::libconfig::Size>(this->total_failed);
		config_root.add("total_redundant", m::libconfig::Size_type) = static_cast<m::libconfig::Size>(this->total_redundant);

		config_root.add("bytes_done", m::libconfig::Size_type) = static_cast<m::libconfig::Size>(this->bytes_done);
		config_root.add("bytes_done_on_last_torrent_finish", m::libconfig::Size_type) =
			static_cast<m::libconfig::Size>(this->bytes_done_on_last_torrent_finish);

		// Сохраняем полученные настройки в файл -->
		{
			std::string real_config_path = config_path;

			try
			{
				real_config_path = m::fs::config::start_writing(config_path);
				config.writeFile(U2L(real_config_path).c_str());
				m::fs::sync_file(real_config_path);
				m::fs::config::end_writing(config_path);
			}
			catch(m::Exception& e)
			{
				M_THROW(__("Writing torrent configuration file '%1' failed. %2", m::fs::get_abs_path_lazy(config_path), EE(e)));
			}
			catch(libconfig::FileIOException e)
			{
				M_THROW(__("Can't write torrent configuration file %1: %2.", m::fs::get_abs_path_lazy(real_config_path), EE(e)));
			}
		}
		// Сохраняем полученные настройки в файл <--
	}



	void Torrent_settings::write_resume_data(const std::string& resume_data_path) const
	{
		if(this->resume_data == lt::entry())
		{
			try
			{
				m::fs::rm_if_exists(resume_data_path);
			}
			catch(m::Exception& e)
			{
				M_THROW(__("Removing old resume data '%1' failed. %2", resume_data_path, EE(e)));
			}

			return;
		}

		try
		{
			// Генерирует m::Exception.
			std::string real_resume_data_path = m::fs::config::start_writing(resume_data_path);

			// Не вносим в try-блок, чтобы при ошибке деструктор не
			// перезаписал значение errno.
			std::ofstream resume_data_file;

			try
			{
				resume_data_file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
				resume_data_file.open(U2L(real_resume_data_path).c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

				// Пишем версию libtorrent, которая производила запись resume data -->
				{
					Version cur_lt_version = m::lt::get_version();
					resume_data_file.write(reinterpret_cast<const char*>(&cur_lt_version), sizeof cur_lt_version);
				}
				// Пишем версию libtorrent, которая производила запись resume data <--

				std::ostream_iterator<char> file_iterator(resume_data_file);
				lt::bencode(file_iterator, this->resume_data);

				resume_data_file.close();
			}
			catch(std::ofstream::failure& e)
			{
				M_THROW(__("Can't write torrent resume data to file '%1': %2.", real_resume_data_path, EE()));
			}

			// Генерирует m::Exception.
			m::fs::sync_file(real_resume_data_path);

			// Генерирует m::Exception.
			m::fs::config::end_writing(resume_data_path);
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Writing torrent resume data to file '%1' failed. %2", resume_data_path, EE(e)));
		}
	}
// Torrent_settings <--

