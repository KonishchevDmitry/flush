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


#include <cstdlib>

#include <algorithm>
#include <iterator>
#include <queue>

#include <boost/thread.hpp>

#include <dbus-c++/dbus.h>

#include <sigc++/signal.h>

#include <gtkmm/dialog.h>
#include <gtkmm/main.h>
#include <gtkmm/window.h>

#include <mlib/gtk/main.hpp>
#include <mlib/misc.hpp>
#include <mlib/signals_holder.hpp>

#include "application.hpp"
#include "client_cmd_options.hpp"
#include "common.hpp"
#include "daemon_proxy.hpp"
#include "daemon_settings.hpp"
#include "main.hpp"
#include "main_window.hpp"


#define CLIENT_CONFIG_FILE_NAME "client.conf"


namespace
{
	#ifdef DEVELOP_MODE
		/// Функция для отображения silent предупреждений разработчику.
		void silent_warning_function(const char* file, const int line, const std::string& message);
	#endif

	/// Функция для отображения информационных сообщений пользователю.
	void info_function(const char* file, size_t line, const std::string& title, const std::string& message);

	/// Функция для отображения предупреждений пользователю.
	void warning_function(const char* file, size_t line, const std::string& title, const std::string& message);



	#ifdef DEVELOP_MODE
		void silent_warning_function(const char* file, size_t line, const std::string& message)
		{
			print_silent_warning(file, line, message);
			get_application().add_message( Message(WARNING, _("Silent warning"), message) );
		}
	#endif



	void info_function(const char* file, size_t line, const std::string& title, const std::string& message)
	{
		print_info(file, line, title, message);
		get_application().add_message( Message(INFO, title, message) );
	}



	void warning_function(const char* file, size_t line, const std::string& title, const std::string& message)
	{
		print_warning(file, line, title, message);
		get_application().add_message( Message(WARNING, title, message) );
	}
}



namespace Application_aux
{

	class Private
	{
		public:
			Private(void);


		public:
			/// Сигнализирует о том, что в данный момент производится
			/// завершение работы приложения.
			bool							stopping;

			m::Signals_holder				sholder;
	};



	Private::Private(void)
	:
		stopping(false)
	{
	}
}



Application* Application::ptr = NULL;

// TODO:
//
// В принципе, ничего плохого в этом нет, т. к. Application - singleton, но не
// красиво. :)
//
// static он сделан потому, что Glib::Dispatcher необходимо удалять из потока
// main loop, что довольно проблемматично.
//
// Использовать m::gtk::Dispatcher в данном случае нельзя, т. к. он требует
// входа в критическую секцию GTK.
Glib::Dispatcher Application::message_signal;



Application::Application(const Client_cmd_options& cmd_options, DBus::Connection& dbus_connection, const std::string& dbus_path, const std::string& dbus_name, int close_signal_fd)
:
	DBus::ObjectAdaptor(dbus_connection, dbus_path.c_str()),
	priv(new Private),
	cmd_options(cmd_options),
	message_dialog(NULL),
	main_window(NULL)
{
	// Может быть только один экземпляр Application
	MLIB_A(!this->ptr);
	this->ptr = this;

	// Устанавливаем функции для отображения сообщений пользователю -->
		m::set_info_function(info_function);
		m::set_warning_function(warning_function);

		#ifdef DEVELOP_MODE
			m::set_silent_warning_function(silent_warning_function);
		#endif
	// Устанавливаем функции для отображения сообщений пользователю <--

	// Необходимо создавать после задания функций для отображения сообщений,
	// чтобы пользователь мог увидить ошибки, которые генерирует конструктор
	// daemon_proxy.
	this->daemon_proxy = std::auto_ptr<Daemon_proxy>(
		new Daemon_proxy(cmd_options.config_path)
	);

	// Путь к конфигурационным файлам приложения
	this->config_dir_path = cmd_options.config_path;

	// Читаем конфиг -->
		try
		{
			this->client_settings.read_config(CLIENT_CONFIG_FILE_NAME);
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
		}
	// Читаем конфиг <--

	this->main_window = new Main_window(this->client_settings.gui.main_window);

	/// Сигнал на отображение сообщения пользователю.
	priv->sholder.push(this->message_signal.connect(
		sigc::mem_fun(*this, &Application::on_message_callback)));

	/// Сигнал на получение сообщений от демона.
	priv->sholder.push(this->daemon_proxy->daemon_message_signal.connect(
		sigc::mem_fun(*this, &Application::on_daemon_message_cb)));

	// В glibmm есть бага, из-за которой приложение падает, если где-нибудь у
	// себя хранить указатель на Glib::IOSource (вроде как - глубоко не копал).
	// Поэтому создаем его тут, благо больше он нам нигде не понадобится.
	// -->
	{
		Glib::RefPtr<Glib::IOSource> close_io_source = Glib::IOSource::create(close_signal_fd, Glib::IO_IN);
		/// Сигнал на завершение работы приложения.
		close_io_source->connect( sigc::mem_fun(*this, &Application::on_close_signal_cb) );
		close_io_source->attach();
	}
	// <--
}



Application::~Application(void)
{
	MLIB_A(this->ptr);

	priv->sholder.disconnect();

	// Устанавливаем функцию по умолчанию для отображения сообщений
	// пользователю.
	m::set_warning_function(NULL);

	delete this->main_window;

	this->ptr = NULL;

	MLIB_D("Application has been destroyed.");
}



void Application::add_message(const Message& message)
{
	bool generate_signal;

	{
		boost::mutex::scoped_lock lock(this->mutex);
		generate_signal = this->messages.empty();
		this->messages.push(message);
	}

	// Лишние сигналы не генерируем
	if(generate_signal)
		this->message_signal();
}



void Application::add_torrent(const std::string& torrent_path, const New_torrent_settings& torrent_settings)
{
	get_daemon_proxy().add_torrent(torrent_path, torrent_settings);
}



void Application::close(void)
{
	if(!priv->stopping)
	{
		MLIB_D("Closing the application...");

		priv->stopping = true;

		this->main_window->close();

		// Скрываем все открытые окна, тем самым создавая иллюзию быстрого
		// завершения работы.
		BOOST_FOREACH(Gtk::Window* window, Gtk::Window::list_toplevels())
			window->hide();

		// Инициируем остановку демона
		this->daemon_proxy->stop();
	}
}



void Application::dbus_cmd_options(const std::vector<std::string>& cmd_options_strings)
{
	m::gtk::Scoped_enter gtk_lock;

	if(priv->stopping)
		return;

	// Переводим строки из UTF-8 в кодировку локали -->
		std::vector<std::string> cmd_options_locale_strings;
		cmd_options_locale_strings.reserve(cmd_options_strings.size());

		for(size_t i = 0; i < cmd_options_strings.size(); i++)
			cmd_options_locale_strings.push_back( U2L(cmd_options_strings[i]) );
	// Переводим строки из UTF-8 в кодировку локали <--

	// Формируем argc и argv -->
		int argc = cmd_options_locale_strings.size() + 1;
		char** argv = new char*[argc + 1];

		argv[0] = const_cast<char *>( APP_BIN_PATH );
		argv[argc] = NULL;

		for(size_t i = 0; i < cmd_options_locale_strings.size(); i++)
			argv[i + 1] = const_cast<char *>( cmd_options_locale_strings[i].c_str() );
	// Формируем argc и argv <--

	// Парсим полученные опции -->
		Client_cmd_options cmd_options;

		try
		{
			cmd_options.parse(argc, argv);
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
			delete [] argv;
			return;
		}

		delete [] argv;
	// Парсим полученные опции <--

	// Передаем на обработку -->
		try
		{
			this->process_cmd_options(cmd_options);
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
		}
	// Передаем на обработку <--
}



bool Application::on_close_signal_cb(Glib::IOCondition condition)
{
	m::gtk::Scoped_enter gtk_lock;

	if(this->ptr)
		this->close();

	return false;
}



void Application::on_daemon_message_cb(const Daemon_message& message)
{
	this->main_window->add_daemon_message(message);

	if(message.get_type() == Daemon_message::WARNING)
		MLIB_W(message);
}



void Application::on_message_callback(void)
{
	m::gtk::Scoped_enter lock;

	// Если уже отображается сообщение, то откладываем
	// отображение текущего сообщения.
	if(!this->message_dialog)
		this->show_next_message();
}



void Application::on_message_response_callback(int response)
{
	MLIB_A(this->message_dialog);

	delete this->message_dialog;
	this->message_dialog = NULL;

	// Отображаем следующее сообщение в очереди.
	this->show_next_message();
}



void Application::open_uri(const std::string& uri)
{
	if(this->client_settings.user.open_command != "")
	{
		try
		{
			std::vector<std::string> args;
			args.push_back(uri);

			m::run(this->client_settings.user.open_command, args);
		}
		catch(m::Exception& e)
		{
			MLIB_SW(__("Running open command failed. %1.", EE(e)));
		}
	}
}



void Application::process_cmd_options(const Client_cmd_options& cmd_options)
{
	// torrents paths
	for(size_t i = 0; i < cmd_options.torrents_paths.size(); i++)
		this->main_window->open_torrent(cmd_options.torrents_paths[i]);

	// daemon settings -->
		if(
			cmd_options.download_rate_limit != cmd_options.invalid_speed_value ||
			cmd_options.upload_rate_limit != cmd_options.invalid_speed_value ||
			cmd_options.max_uploads != cmd_options.invalid_number_value ||
			cmd_options.max_connections != cmd_options.invalid_number_value
		)
		{
			try
			{
				Daemon_settings daemon_settings = this->daemon_proxy->get_settings();

				if(cmd_options.download_rate_limit != cmd_options.invalid_speed_value)
					daemon_settings.download_rate_limit = cmd_options.download_rate_limit;

				if(cmd_options.upload_rate_limit != cmd_options.invalid_speed_value)
					daemon_settings.upload_rate_limit = cmd_options.upload_rate_limit;

				if(cmd_options.max_uploads != cmd_options.invalid_number_value)
					daemon_settings.max_uploads = cmd_options.max_uploads;

				if(cmd_options.max_connections != cmd_options.invalid_number_value)
					daemon_settings.max_connections = cmd_options.max_connections;

				this->daemon_proxy->set_settings(daemon_settings);
			}
			catch(m::Exception& e)
			{
				MLIB_W(EE(e));
			}
		}
	// daemon settings <--

	// start/stop -->
		try
		{
			if(cmd_options.start != NONE)
				this->daemon_proxy->start_torrents(cmd_options.start);

			if(cmd_options.stop != NONE)
				this->daemon_proxy->stop_torrents(cmd_options.stop);
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
		}
	// start/stop <--
}



void Application::save_settings(void)
{
	MLIB_D("Saving settings...");

	// Записываем настройки в конфиг -->
		try
		{
			this->client_settings.write_config(CLIENT_CONFIG_FILE_NAME);
		}
		catch(m::Exception& e)
		{
			MLIB_W(_("Error while saving settings"), EE(e));
		}
	// Записываем настройки в конфиг <--
}



void Application::show_next_message(void)
{
	Message message;

	// Если в очереди есть сообщения, то извлекаем
	// из нее самое старое, иначе возвращаем управление.
	// -->
	{
		boost::mutex::scoped_lock lock(this->mutex);

		if(this->messages.empty())
			return;
		else
		{
			message = this->messages.front();
			this->messages.pop();
		}
	}
	// <--

	// Отображаем сообщение -->
		this->message_dialog = create_message_dialog(
			*this->main_window, message.type, message.title, message.message
		);

		this->message_dialog->signal_response().connect(
			sigc::mem_fun(*this, &Application::on_message_response_callback)
		);

		this->message_dialog->show();
	// Отображаем сообщение <--
}



void Application::start(void)
{
	// Запускаем демон
	this->daemon_proxy->start();

	// Обрабатываем опции командной строки
	this->process_cmd_options(this->cmd_options);

	update_gui();
}



void Application::stop(void)
{
	MLIB_D("Application stopped. Destroying it...");
	delete this;

	MLIB_D("Stopping main loop...");
	Gtk::Main::quit();
}




Application_proxy::Application_proxy(DBus::Connection& dbus_connection, const std::string& dbus_path, const std::string& dbus_name)
:
	DBus::ObjectProxy(dbus_connection, dbus_path.c_str(), dbus_name.c_str())
{
}

