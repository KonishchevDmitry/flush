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


#ifndef HEADER_APPLICATION
	#define HEADER_APPLICATION

	#include <memory>
	#include <queue>

	#include <boost/thread.hpp>

	#include <dbus-c++/dbus.h>
	#include "adaptor_glue.hpp"
	#include "proxy_glue.hpp"

	#ifndef MLIB_ENABLE_LIBS_FORWARDS
		#include <gtkmm/dialog.h>
	#endif

	#include <glibmm/dispatcher.h>

	#include "client_cmd_options.hpp"
	#include "client_settings.hpp"
	#include "common.hpp"
	#include "daemon_proxy.hpp"



	/// Сообщение для отображения пользователю.
	class Message
	{
		public:
			typedef Message_type Type;


		public:
			inline
			Message(void);

			inline
			Message(Type type, const std::string& title, const std::string& message);


		public:
			/// Тип сообщения.
			Type			type;

			/// Заголовок сообщения.
			std::string		title;

			/// Текст сообщения.
			std::string		message;
	};



	namespace Application_aux {
		class Private;
	}


	/// Класс, представляющий все приложение в целом и хранящий
	/// все основные его классы.
	class Application
	:
		public com::blogspot::konishchevdmitry::flush_adaptor,
		public DBus::IntrospectableAdaptor,
		public DBus::ObjectAdaptor
	{
		private:
			typedef Application_aux::Private Private;


		public:
			Application(const Client_cmd_options& cmd_options, DBus::Connection& dbus_connection, const std::string& dbus_path, const std::string& dbus_name, int close_signal_fd);
			~Application(void);


		private:
			/// Указатель на текущий экземпляр Application.
			static Application*			ptr;

			boost::scoped_ptr<Private>	priv;


			/// Опции командной строки, с которыми было запущено приложение.
			const Client_cmd_options	cmd_options;

			/// Путь к конфигу.
			std::string					config_dir_path;

			/// Текущие настройки клиента.
			Client_settings				client_settings;


			/// Мьютекс для синхронизации потоков при добавлении
			/// сообщений.
			boost::mutex				mutex;

			/// Очередь сообщений для отображения пользователю.
			std::queue<Message>			messages;

			/// Сигнал, генерируемый при добавлении нового сообщения
			/// для отображения его пользователю.
			static Glib::Dispatcher		message_signal;

			/// Окно с сообщением, отображаемым в данный момент
			/// или NULL, если в данный момент сообщение не отображается.
			Gtk::Dialog*				message_dialog;


			/// Объект, через который осуществляется взаимодействие
			/// с демоном.
			std::auto_ptr<Daemon_proxy>	daemon_proxy;

			/// Главное окно приложения.
			Main_window*				main_window;


		public:
			/// Добавляет сообщение в очередь сообщений для отображения пользователю.
			void				add_message(const Message& message);

			/// Добавляет торрент в текущую сессию.
			/// @throw - m::Exception.
			void				add_torrent(const std::string& torrent_path, const New_torrent_settings& torrent_settings);

			/// Инициирует завершение работы с приложением.
			void				close(void);

			/// Возвращает текущий экземпляр Application.
			static inline
			Application*		get(void);

			/// Возвращает текущие настройки клиента.
			inline
			Client_settings&	get_client_settings(void);

			/// Возвращает путь к конфигурационным файлам.
			inline
			const std::string&	get_config_dir_path(void);

			/// Возвращает proxy объект демона.
			inline
			Daemon_proxy&		get_daemon_proxy(void);

			/// Возвращает главное окно приложения.
			inline
			Main_window&		get_main_window(void);

			/// Вызывается, когда пользователь активирует какой-нибудь файл,
			/// папку или URL.
			///
			/// Производит всю необходимую работу по открытию ресурса в той
			/// среде, которую предпочитает пользователь.
			void				open_uri(const std::string& uri);

			/// Cохраняет текущие настройки клиента
			/// в конфигурационный файл.
			void				save_settings(void);

			/// Инициирует начало работы приложения.
			/// @throw - m::Exception.
			void				start(void);

			/// Эту функцию вызывает подсистема приложения, ответственная за
			/// завершение его работы, когда все демоны и пр. уже остановлены в
			/// результате вызова метода close().
			void				stop(void);

		private:
			/// Получает опции командной строки и производит все необходимые действия
			/// по их обработке.
			void				dbus_cmd_options(const std::vector<std::string>& cmd_options_strings);

			/// Обработчик сигнала на получение сигнала (системного) на
			/// завершение приложения.
			bool				on_close_signal_cb(Glib::IOCondition condition);

			/// Обработчик сигнала на получение сообщений от демона.
			void				on_daemon_message_cb(const Daemon_message& message);

			/// Обработчик сигнала на получение нового сообщения для отображения пользователю.
			void				on_message_callback(void);

			/// Обработчик сигнала на закрытие пользователем окна с сообщением.
			void				on_message_response_callback(int response);

			/// Производит обработку переданных опций командной строки.
			void				process_cmd_options(const Client_cmd_options& cmd_options);

			/// Отображает очередное сообщение.
			void				show_next_message(void);
	};



	/// Класс для передачи опций командной строки уже запущенной копии GUI клиента.
	class Application_proxy
	:
		public com::blogspot::konishchevdmitry::flush_proxy,
		public DBus::IntrospectableProxy,
		public DBus::ObjectProxy
	{
		public:
			Application_proxy(DBus::Connection& dbus_connection, const std::string& dbus_path, const std::string& dbus_name);
	};


	#include "application.hh"

#endif

