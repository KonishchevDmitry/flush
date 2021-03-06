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


#include <cstdlib>

#include <algorithm>
#include <iterator>
#include <queue>

#include <boost/thread.hpp>

#include <dbus-c++/dbus.h>

#include <sigc++/signal.h>

#include <libnotify/notify.h>
#include <libnotify/notification.h>

#include <gtkmm/dialog.h>
#include <gtkmm/main.h>
#include <gtkmm/window.h>

#include <mlib/misc.hpp>
#include <mlib/signals_holder.hpp>

#include <mlib/gtk/dispatcher.hpp>
#include <mlib/gtk/main.hpp>

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
			bool								stopping;

			m::Signals_holder					sholder;
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

	/// Включаем или отключаем поддержку оповещений через libnotify
	this->update_notifications_support();

	this->main_window = new Main_window(this->client_settings.gui.main_window);

	/// Сигнал на отображение сообщения пользователю.
	priv->sholder.push(this->message_signal.connect(
		sigc::mem_fun(*this, &Application::on_message_callback)));

	/// Сигнал на получение сообщений от демона.
	priv->sholder.push(this->daemon_proxy->daemon_message_signal.connect(
		sigc::mem_fun(*this, &Application::on_daemon_message_cb)));

	/// Сигнал на получение notify-сообщений от демона.
	priv->sholder.push(this->daemon_proxy->notify_message_signal.connect(
		sigc::mem_fun(*this, &Application::on_daemon_notify_message_cb)));

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
	MLIB_D("Destroying application...");

	MLIB_A(this->ptr);

	priv->sholder.disconnect();

	// Устанавливаем функцию по умолчанию для отображения сообщений
	// пользователю.
	m::set_warning_function(NULL);

	MLIB_D("Destroying main window...");
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



void Application::add_torrent(const std::string& torrent_uri, const New_torrent_settings& torrent_settings)
{
	get_daemon_proxy().add_torrent(torrent_uri, torrent_settings);

	// Чтобы торрент появился моментально
	update_gui();
}



void Application::close(void)
{
	if(!priv->stopping)
	{
		MLIB_D("Closing the application...");

		priv->stopping = true;

		this->main_window->close();

		// Инициируем остановку демона
		this->daemon_proxy->stop();
	}
}



std::string Application::dbus_cmd_options(const std::vector<std::string>& cmd_options_strings)
{
	m::gtk::Scoped_enter gtk_lock;

	if(priv->stopping)
		return _("application is now ending it's work");

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
			return EE(e);
		}

		delete [] argv;
	// Парсим полученные опции <--

	// Передаем на обработку, отделяя ее от данного потока, чтобы как можно
	// быстрее вернуть управление DBus-обработчику. Это необходимо хотя бы
	// потому, что при обработке могут открываться диалоговые окна, которые
	// могут приостановить выполнение текущего потока на неопределенное
	// количество времени, в результате чего клиент DBus отвалится по таймауту
	// и отобразит об этом ошибку пользователю.
	this->process_cmd_options(cmd_options);

	return "";
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



void Application::on_daemon_notify_message_cb(const Notify_message& message)
{
	MLIB_D(_C("Notify message: [%1] %2", message.get_title(), message.get_message()));

	if(notify_is_initted())
	{
		// Отфильтровываем не интересующие нас сообщения -->
		{
			const Gui_settings& gui = get_client_settings().gui;

			switch(message.get_type())
			{
				case Notify_message::TORRENT_FINISHED:
					if(!gui.download_completed_notification)
						return;
					break;

				case Notify_message::TORRENT_FINISHED_AND_ALL:
					if(!gui.download_completed_notification || gui.all_downloads_completed_notification)
						return;
					break;

				case Notify_message::ALL_TORRENTS_FINISHED:
					if(!gui.all_downloads_completed_notification)
						return;
					break;
			}
		}
		// Отфильтровываем не интересующие нас сообщения <--

		// Отображаем полученное уведомление -->
		{
			GError *gerror = NULL;

			// Демон libnotify ищет иконки в стандартных путях, а наши могут лежать
			// в нестандартных. Поэтому прописываем полный путь к файлу.
			const char *icon_path = APP_ICONS_PATH "/hicolor/48x48/apps/" APP_UNIX_NAME ".png";

			NotifyNotification *notify = notify_notification_new(
				message.get_title().c_str(), message.get_message().c_str(), icon_path
				#if defined(NOTIFY_CHECK_VERSION)
					#if !NOTIFY_CHECK_VERSION(0, 7, 0)
						, NULL
					#endif
				#else
					, NULL
				#endif
			);

			if(!notify_notification_show(notify, &gerror))
			{
				MLIB_W(_("Unable to show libnotify message"),
					__("Unable to show libnotify message: %1. You can disable notifications in the Preferences dialog.\nMessage text:\n%2", gerror->message, message.get_message()) );

				g_error_free(gerror);
			}

			g_object_unref(G_OBJECT(notify));
		}
		// Отображаем полученное уведомление <--
	}
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

	MLIB_D("Stopping the main loop...");
	Gtk::Main::quit();
}



void Application::update_notifications_support(void)
{
	Gui_settings& gui = this->client_settings.gui;

	if(gui.download_completed_notification || gui.all_downloads_completed_notification)
	{
		if(!notify_is_initted())
			if(!notify_init(APP_NAME))
				MLIB_W(_("Unable to initialize a libnotify instance. Notification's displaying is not possible."));
	}
	else
	{
		if(notify_is_initted())
			notify_uninit();
	}
}




Application_proxy::Application_proxy(DBus::Connection& dbus_connection, const std::string& dbus_path, const std::string& dbus_name)
:
	DBus::ObjectProxy(dbus_connection, dbus_path.c_str(), dbus_name.c_str())
{
}

