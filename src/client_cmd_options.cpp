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
#include <iostream>
#include <string>

#include <gtkmm/main.h>
#include <glibmm/optioncontext.h>

#include <mlib/fs.hpp>

#include "main.hpp"
#include "client_cmd_options.hpp"



int Client_cmd_options::invalid_number_value = -2;
Speed Client_cmd_options::invalid_speed_value = -2;



Client_cmd_options::Client_cmd_options(void)
:
	download_rate_limit(this->invalid_speed_value),
	upload_rate_limit(this->invalid_speed_value),

	max_uploads(this->invalid_number_value),
	max_connections(this->invalid_number_value),

	start(NONE),
	stop(NONE),

	only_pass(false)
{
	// Конфигурационный файл по умолчанию
	this->config_path = get_default_config_dir_path();
}



template<class T>
void Client_cmd_options::greater_or_equal_check(const std::string option_name, const T value, T target_value)
{
	if(target_value < value)
		M_THROW(__("Bad option '%1' value '%2'.", option_name, value));
}



void Client_cmd_options::parse(int argc, char *argv[])
{
	bool show_version = false;
	Glib::OptionContext cmd_parser(_("torrent_path"));

	if(is_gui_mode())
	{
		// Добавляем опции GTK
		Gtk::Main::add_gtk_option_group(cmd_parser);
	}

	// Добавляем свои опции -->
		Glib::OptionGroup entries_group("main", _("Main options"));

		// only_pass -->
			Glib::OptionEntry version_entry;
			version_entry.set_long_name("version");
			version_entry.set_flags( version_entry.get_flags() | Glib::OptionEntry::FLAG_NO_ARG );
			version_entry.set_description(_("Show program version"));
			entries_group.add_entry(version_entry, show_version);
		// only_pass <--

		// config_path -->
			// TODO:
			// Когда путь передается как --config ~/dir, то все нормально -
			// ~ раскрывается оболочкой. Но если путь передается как
			// --config=~/dir, то ~ не раскрывается, и не понятно, что с
			// ней делать - раскрывать самому - а вдруг пользователь
			// действительно захотел использовать директорию, содержащую ~
			// в имени. Вообщем на досуге надо подумать, как лучше это
			// обойти.
			Glib::OptionEntry config_path_entry;
			config_path_entry.set_long_name("config");
			config_path_entry.set_flags(Glib::OptionEntry::FLAG_FILENAME);
			config_path_entry.set_description(__("Configuration directory path (default: ~/%1)", DEFAULT_CONFIG_DIR_NAME));
			config_path_entry.set_arg_description(_("DIRECTORY"));
			entries_group.add_entry_filename(config_path_entry, this->config_path);
		// config_path <--

		// download_rate_limit -->
			Glib::OptionEntry download_rate_limit_entry;
			download_rate_limit_entry.set_long_name("download-rate-limit");
			download_rate_limit_entry.set_description(_("Set download rate limit (KB/s)"));
			download_rate_limit_entry.set_arg_description(_("SPEED"));
			entries_group.add_entry(download_rate_limit_entry, this->download_rate_limit);
		// download_rate_limit <--

		// upload_rate_limit -->
			Glib::OptionEntry upload_rate_limit_entry;
			upload_rate_limit_entry.set_long_name("upload-rate-limit");
			upload_rate_limit_entry.set_description(_("Set upload rate limit (KB/s)"));
			upload_rate_limit_entry.set_arg_description(_("SPEED"));
			entries_group.add_entry(upload_rate_limit_entry, this->upload_rate_limit);
		// upload_rate_limit <--

		// max_uploads -->
			Glib::OptionEntry max_uploads_entry;
			max_uploads_entry.set_long_name("max-uploads");
			max_uploads_entry.set_description(_("Set maximum uploads"));
			max_uploads_entry.set_arg_description(_("NUMBER"));
			entries_group.add_entry(max_uploads_entry, this->max_uploads);
		// max_uploads <--

		// max_connections -->
			Glib::OptionEntry max_connections_entry;
			max_connections_entry.set_long_name("max-connections");
			max_connections_entry.set_description(_("Set maximum connections"));
			max_connections_entry.set_arg_description(_("NUMBER"));
			entries_group.add_entry(max_connections_entry, this->max_connections);
		// max_connections <--

		// start -->
			Glib::ustring start_string;
			Glib::OptionEntry start_entry;
			start_entry.set_long_name("start");
			start_entry.set_description(_("Start torrents"));
			start_entry.set_arg_description(_("{all,downloads,uploads}"));
			entries_group.add_entry(start_entry, start_string);
		// start <--

		// stop -->
			Glib::ustring stop_string;
			Glib::OptionEntry stop_entry;
			stop_entry.set_long_name("stop");
			stop_entry.set_description(_("Stop torrents"));
			stop_entry.set_arg_description(_("{all,downloads,uploads}"));
			entries_group.add_entry(stop_entry, stop_string);
		// start <--

		// only_pass -->
			Glib::OptionEntry only_pass_entry;
			only_pass_entry.set_short_name('o');
			only_pass_entry.set_long_name("only-pass");
			only_pass_entry.set_flags( only_pass_entry.get_flags() | Glib::OptionEntry::FLAG_NO_ARG );
			only_pass_entry.set_description(__("Only pass commands to already running %1 instance. Does not start new instance if it is not running yet", APP_NAME));
			entries_group.add_entry(only_pass_entry, this->only_pass);
		// only_pass <--

		// Шаблон
		//  -->
			//Glib::OptionEntry _entry;
			//_entry.set_long_name("");
			//_entry.set_description(_(""));
			//entries_group.add_entry(_entry, this->);
		//  <--

		cmd_parser.set_main_group(entries_group);
	// Добавляем свои опции <--

	// Парсим полученные опции -->
		try
		{
			cmd_parser.parse(argc, argv);
		}
		catch(Glib::OptionError e)
		{
			M_THROW(EE(e));
		}
	// Парсим полученные опции <--

	// Если необходимо отобразить версию программы -->
		if(show_version)
		{
			std::cout << APP_NAME << " " << APP_VERSION_STRING << std::endl;
			std::cout << m::get_copyright_string(_("Konishchev Dmitry"), APP_YEAR) << std::endl;
			exit(EXIT_SUCCESS);
		}
	// Если необходимо отобразить версию программы <--

	// Проверяем полученные значения -->
		// download_rate_limit
		this->greater_or_equal_check("download_rate_limit", this->invalid_speed_value, this->download_rate_limit);

		// upload_rate_limit
		this->greater_or_equal_check("upload_rate_limit", this->invalid_speed_value, this->upload_rate_limit);


		// max_uploads
		this->greater_or_equal_check("max_uploads", this->invalid_number_value, this->max_uploads);

		// max_connections
		this->greater_or_equal_check("max_connections", this->invalid_number_value, this->max_connections);


		// start
		this->start_stop_check("start", start_string, &this->start);

		// stop
		this->start_stop_check("stop", stop_string, &this->stop);
	// Проверяем полученные значения <--

	// Все оставшиеся аргументы - файлы торрентов
	for(int i = 1; i < argc; i++)
	{
		try
		{
			this->torrents_paths.push_back(Path(L2U(argv[i])).absolute());
		}
		catch(m::Exception& e)
		{
			MLIB_W(
				__("Error while processing torrent '%1'", L2U(argv[i])),
				__("Can't get '%1' absolute path: %2.", L2U(argv[i]), EE(e))
			);
		}
	}
}



void Client_cmd_options::start_stop_check(const std::string option_name, const std::string& string_value, Torrents_group* target_value)
{
	if(string_value == "")
		*target_value = NONE;
	else if(string_value == "all")
		*target_value = ALL;
	else if(string_value == "downloads")
		*target_value = DOWNLOADS;
	else if(string_value == "uploads")
		*target_value = UPLOADS;
	else
		M_THROW(__("Bad option '%1' value '%2'. Allowed values: all, downloads, uploads.", option_name, string_value));
}



std::vector<std::string> Client_cmd_options::to_strings(void)
{
	std::vector<std::string> argv;

	if(this->download_rate_limit > this->invalid_speed_value)
		argv.push_back("--download-rate-limit=" + m::to_string(this->download_rate_limit ));

	if(this->upload_rate_limit > this->invalid_speed_value)
		argv.push_back("--upload-rate-limit=" + m::to_string(this->upload_rate_limit ));

	if(this->max_uploads > this->invalid_number_value)
		argv.push_back("--max-uploads=" + m::to_string(this->max_uploads));

	if(this->max_connections > this->invalid_number_value)
		argv.push_back("--max-connections=" + m::to_string(this->max_connections));

	switch(this->start)
	{
		case NONE:
			break;

		case ALL:
			argv.push_back("--start=all");
			break;

		case DOWNLOADS:
			argv.push_back("--start=downloads");
			break;

		case UPLOADS:
			argv.push_back("--start=uploads");
			break;

		default:
			break;
	}

	switch(this->stop)
	{
		case NONE:
			break;

		case ALL:
			argv.push_back("--stop=all");
			break;

		case DOWNLOADS:
			argv.push_back("--stop=downloads");
			break;

		case UPLOADS:
			argv.push_back("--stop=uploads");
			break;

		default:
			break;
	}
	
	for(size_t i = 0; i < this->torrents_paths.size(); i++)
		argv.push_back(this->torrents_paths[i]);
	
	return argv;
}

