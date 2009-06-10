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


#include <fcntl.h>
#include <locale.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef ENABLE_NLS
	#include <libintl.h>
#endif

#include <cerrno>
#include <iomanip>
#include <iostream>
#include <memory>

#include <dbus/dbus.h>

#include <dbus-c++/dbus.h>
#include <dbus-c++/glib-integration.h>

#include <gdk/gdk.h>

#include <gtk/gtkicontheme.h>

#include <gtkmm/box.h>
#include <gtkmm/expander.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/linkbutton.h>
#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>

#include <mlib/gtk/misc.hpp>
#include <mlib/gtk/vbox.hpp>
#include <mlib/gtk/window.hpp>
#include <mlib/fs.hpp>
#include <mlib/misc.hpp>

#include "application.hpp"
#include "main.hpp"
#include "main_window.hpp"
#include "client_cmd_options.hpp"


#ifdef DEVELOP_MODE
	// Для ускорения сборки в процессе разработки
	// используются функции для доступа к различным
	// часто используемым объектам.
	//
	// В режиме разработки они реализованы как обычные
	// функции, а в обычном режиме, в котором увеличение
	// времени компиляции за счет необходимости
	// подключения лишних заголовочных файлов практически
	// не мешает - как inline функции.
	#include "main.hh"
#endif


#define DBUS_SESSION_LINK_NAME "dbus_session"
#define DBUS_SESSION_ENV_NAME "DBUS_SESSION_BUS_ADDRESS"



const std::string APP_CUSTOM_ICON_TORRENT_WAITING_FOR_DOWNLOAD					= "torrent-waiting-for-download";
const std::string APP_CUSTOM_ICON_TORRENT_WAITING_FOR_DOWNLOAD_BROCKEN_TRACKER	= "torrent-waiting-for-download-brocken-tracker";
const std::string APP_CUSTOM_ICON_TORRENT_DOWNLOADING							= "torrent-downloading";
const std::string APP_CUSTOM_ICON_TORRENT_DOWNLOADING_BROCKEN_TRACKER			= "torrent-downloading-brocken-tracker";
const std::string APP_CUSTOM_ICON_TORRENT_SEEDING								= "torrent-seeding";
const std::string APP_CUSTOM_ICON_TORRENT_SEEDING_BROCKEN_TRACKER				= "torrent-seeding-brocken-tracker";
const std::string APP_CUSTOM_ICON_TORRENT_UPLOADING								= "torrent-uploading";
const std::string APP_CUSTOM_ICON_TORRENT_UPLOADING_BROCKEN_TRACKER				= "torrent-uploading-brocken-tracker";

const std::string APP_CUSTOM_ICON_UPLOAD										= "upload";
const std::string APP_CUSTOM_ICON_DOWNLOAD_AND_UPLOAD							= "download-and-upload";
const std::string APP_CUSTOM_ICON_DOWNLOAD										= "download";
const std::string APP_CUSTOM_ICON_STATISTICS									= "statistics";



namespace
{
	/// Warning-функция для стадии инициализации программы.
	/// В отличии от обычной, завершает программу и,
	/// если программа запущена в графической среде,
	/// отображает графическое сообщение.
	void app_init_warning_function(const char* file, const int line, const std::string& title, const std::string& message);

	/// Error-функция. Отличается от стандартной тем, что в
	/// графическом режиме отображает окно с ошибкой.
	void error_function(const char* file, const int line, const std::string& message);

	/// Проверяет, запущена ли программа в режиме отображения
	/// ошибки. Если да - осуществляет работу в режиме
	/// отображения ошибки.
	void error_mode_check(int argc, char *argv[]);

	/// Обработчик нажатия на кнопку OK в окне отображения ошибки.
	void on_error_window_ok_button_callback(void);

	/// Наш собственный обработчик сигнала на нажатие по кнопке-ссылке.
	void on_linkbutton_uri_callback(Gtk::LinkButton* button, const Glib::ustring& uri);

	/// Выводит на консоль Error сообщение.
	void print_error(const char* file, const int line, const std::string& message, const std::string& debug_info);

	/// Обработчик сигнала SIGCHLD.
	void sigchld_handler(int signal_no);



	void app_init_warning_function(const char* file, const int line, const std::string& title, const std::string& message)
	{
		print_warning(file, line, title, message);

		if(is_gui_mode())
		{
			Gtk::MessageDialog dialog("", false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK);
			dialog.set_title(__("%1: Error", APP_NAME));
			if(title.empty())
				dialog.set_message(__("Starting %1 failed:", APP_NAME));
			else
				dialog.set_message(title);
			dialog.set_secondary_text(message);
			dialog.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
			dialog.run();
		}

		exit(EXIT_FAILURE);
	}



	void error_function(const char* file, const int line, const std::string& message)
	{
		const char* locale_settings[] = {
			"LANG",
			"LC_CTYPE",
			"LC_NUMERIC",
			"LC_TIME",
			"LC_COLLATE",
			"LC_MONETARY",
			"LC_MESSAGES",
			"LC_PAPER",
			"LC_NAME",
			"LC_ADDRESS",
			"LC_TELEPHONE",
			"LC_MEASUREMENT",
			"LC_IDENTIFICATION",
			"LC_ALL",
			NULL
		};
		const char** locale_setting = locale_settings;

		// debug info -->
			std::string debug_info;

			debug_info +=
				std::string("") +
				APP_NAME + " " + APP_VERSION_STRING + ".\n" +
				_("Error at") + " " + file + ":" + m::to_string(line) + ".\n" +
				_("\nLocale settings:\n");

			while(*locale_setting)
			{
				const char* locale_setting_value = getenv(*locale_setting);
				if(!locale_setting_value)
					locale_setting_value = "";

				debug_info += std::string("") + *locale_setting + "=" + locale_setting_value + "\n";
				locale_setting++;
			}

			#ifdef ENABLE_NLS
				debug_info += _("\nCompiled with native language support.\n");
			#else
				debug_info += _("\nCompiled without native language support.\n");
			#endif
		// debug info <--

		print_error(file, line, message, debug_info);

		if(is_gui_mode())
			execl(APP_BIN_PATH, APP_BIN_PATH, "--error-mode", message.c_str(), debug_info.c_str(), NULL);

		abort();
	}



	void error_mode_check(int argc, char *argv[])
	{
		if(argc >= 2 && m::is_eq(argv[1], "--error-mode"))
		{
			if(is_gui_mode())
			{
				std::string error_message;
				std::string debug_info;

				if(argc >= 3)
					error_message = L2U(argv[2]);
				else
					error_message = _("Logical error.");

				if(argc >= 4)
					debug_info = L2U(argv[3]);


				m::gtk::Window error_window(__("%1: Critical error", APP_NAME));
				error_window.set_border_width(m::gtk::WINDOW_BORDER_WIDTH * 2);
				error_window.set_resizable(false);

				Gtk::HBox main_hbox(false, m::gtk::WINDOW_BORDER_WIDTH * 2);
				error_window.add(main_hbox);

				// Error image -->
					Gtk::Image error_image(Gtk::Stock::DIALOG_ERROR, Gtk::ICON_SIZE_DIALOG);
					error_image.set_alignment(0, 0);
					main_hbox.pack_start(error_image, false, false);
				// Error image <--

				Gtk::VBox main_vbox(false, m::gtk::VBOX_SPACING * 2);
				main_hbox.pack_start(main_vbox, false, false);

				// Title -->
					Gtk::Label title_label;
					title_label.set_markup(std::string("<b>") + _("Critical error") + "</b>");
					title_label.set_selectable();
					title_label.set_line_wrap();
					title_label.set_alignment(0, 0);
					main_vbox.pack_start(title_label, false, false);
				// Title <--

				// Message -->
					Gtk::Label message_label(error_message);
					message_label.set_line_wrap();
					message_label.set_selectable();
					message_label.set_alignment(0, 0);
					main_vbox.pack_start(message_label, false, false);
				// Message <--

				// Debug info -->
					if(debug_info != "")
					{
						Gtk::Expander* expander = Gtk::manage( new Gtk::Expander(_("Debug info:")) );
						main_vbox.pack_start(*expander, false, false);

						Gtk::Label* debug_info_label = Gtk::manage( new Gtk::Label(debug_info) );
						debug_info_label->set_line_wrap();
						debug_info_label->set_selectable();
						debug_info_label->set_alignment(0, 0);
						expander->add(*debug_info_label);
					}
				// Debug info <--

				// OK button -->
					Gtk::HBox ok_button_hbox(false, m::gtk::HBOX_SPACING);
					main_vbox.pack_start(ok_button_hbox, false, false);

					Gtk::Button ok_button(Gtk::Stock::OK);
					ok_button.set_flags(ok_button.get_flags() | Gtk::CAN_DEFAULT);
					error_window.set_default(ok_button);
					ok_button.signal_clicked().connect(sigc::ptr_fun(on_error_window_ok_button_callback));
					ok_button_hbox.pack_end(ok_button, false, false);
				// OK button <--

				error_window.show_all();

				Gtk::Main::run(error_window);
			}

			abort();
		}
	}



	void on_error_window_ok_button_callback(void)
	{
		Gtk::Main::quit();
	}



	void on_linkbutton_uri_callback(Gtk::LinkButton* button, const Glib::ustring& uri)
	{
	}



	void print_error(const char* file, const int line, const std::string& message, const std::string& debug_info)
	{
		std::cerr
			#ifdef DEBUG_MODE
				<< U2L(m::get_log_debug_prefix(file, line))
			#endif
			<< "E: " << U2L(message) << std::endl
			<< std::endl
			<< U2L(_("Debug info:")) << std::endl
			<< U2L(debug_info) << std::endl;

		std::cerr.flush();
	}



	void sigchld_handler(int signal_no)
	{
		int child_exit_status;

		wait(&child_exit_status);
	}
}



Gtk::Dialog* create_message_dialog(Gtk::Window& parent_window, Message_type type, std::string title, Glib::ustring message)
{
	Gtk::StockID dialog_stock_id;
	Gtk::Dialog* message_dialog = NULL;

	std::string short_message;
	std::string long_message;
	const int labels_width = 300;

	MLIB_D(_C("GUI message: [%1] %2", title, message));

	// Определяем все переменные, которые зависят от типа сообщения -->
		switch(type)
		{
			case WARNING:
				if(title == "")
					title = _("Warning");
				dialog_stock_id = Gtk::Stock::DIALOG_WARNING;
				break;

			case INFO:
				if(title == "")
					title = _("Information");
				dialog_stock_id = Gtk::Stock::DIALOG_INFO;
				break;

			default:
				break;
		}
	// Определяем все переменные, которые зависят от типа сообщения <--

	// Когда выводится содержимое Errors_pool, сообщение всегда будет содержать
	// пустую строку, выводить которую не имеет смысла.
	if(!message.empty() && message[0] == '\n')
		message = message.substr(1);

	// Разбиваем сообщение на два - короткое и длинное.
	// В итоге должно получиться одно из двух:
	// * будет короткое сообщение в несколько строк и не будет длинного.
	// * будет короткое сообщение в одну строку и длинное, в которое будут
	//   помещены остальные строки.
	// -->
	{
		const size_t short_message_max_lines = 3;
		size_t lines_num = 0;
		size_t new_string_pos = 0;

		while(new_string_pos != message.npos && lines_num <= short_message_max_lines)
		{
			new_string_pos = message.find('\n', lines_num ? new_string_pos + 1 : 0);
			lines_num++;
		}

		if(lines_num > short_message_max_lines)
		{
			new_string_pos = message.find('\n');
			short_message = message.substr(0, new_string_pos);
			long_message = message.substr(new_string_pos + 1);
		}
		else
			short_message = message;
	}
	// <--

	message_dialog = new Gtk::Dialog(
		_C("%1: %2", APP_NAME, title),
		parent_window, true
	);
	message_dialog->set_border_width(m::gtk::WINDOW_BORDER_WIDTH);
	message_dialog->set_resizable(false);

	Gtk::HBox* main_hbox = Gtk::manage( new Gtk::HBox(false, m::gtk::HBOX_SPACING * 3) );
	message_dialog->get_vbox()->add(*main_hbox);

	// Image -->
	{
		Gtk::Image* error_image = Gtk::manage( new Gtk::Image(dialog_stock_id, Gtk::ICON_SIZE_DIALOG) );
		error_image->set_alignment(0, 0);
		main_hbox->pack_start(*error_image, false, false);
	}
	// Image <--

	Gtk::VBox* main_vbox = Gtk::manage( new Gtk::VBox(false, m::gtk::VBOX_SPACING * 3) );
	main_hbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
	main_hbox->pack_start(*main_vbox, false, false);

	// Title -->
	{
		Gtk::Label* title_label = Gtk::manage( new Gtk::Label() );
		title_label->set_markup(std::string("<b>") + title + "</b>");
		title_label->set_line_wrap();
		title_label->set_selectable();
		title_label->set_alignment(0, 0);
		if(!long_message.empty())
			title_label->set_size_request(labels_width, -1);
		main_vbox->pack_start(*title_label, false, false);
	}
	// Title <--

	// Short message -->
	{
		Gtk::Label* message_label = Gtk::manage( new Gtk::Label(short_message) );
		message_label->set_line_wrap();
		message_label->set_selectable();
		message_label->set_alignment(0, 0);
		if(!long_message.empty())
			message_label->set_size_request(labels_width, -1);
		main_vbox->pack_start(*message_label, false, false);
	}
	// Short message <--

	// Long message -->
		if(!long_message.empty())
		{
			Gtk::ScrolledWindow* scrolled_window = Gtk::manage( new Gtk::ScrolledWindow() );
			scrolled_window->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
			scrolled_window->set_size_request(-1, 100);
			main_vbox->add(*scrolled_window);

			Gtk::VBox* vbox = Gtk::manage( new Gtk::VBox() );
			vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
			scrolled_window->add(*vbox);

			Gtk::Label* message_label = Gtk::manage( new Gtk::Label(long_message) );
			message_label->set_alignment(0, 0);
			message_label->set_selectable();
			vbox->add(*message_label);
		}
	// Long message <--

	message_dialog->add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	message_dialog->show_all_children();

	return message_dialog;
}



std::string get_default_config_dir_path(void)
{
	return Path(m::fs::get_user_home_path()) / DEFAULT_CONFIG_DIR_NAME;
}



bool is_gui_mode(void)
{
	bool gui_mode = getenv("DISPLAY");
	return gui_mode;
}



void print_info(const char* file, const int line, const std::string& title, const std::string& message)
{
	std::cout
		#ifdef DEBUG_MODE
			<< U2L(m::get_log_debug_prefix(file, line))
		#endif
		<< "I: ";

	if(!title.empty())
		std::cout << "[" << U2L(title) << "] ";

	std::cout << U2L(message) << std::endl;

	std::cout.flush();
}



void print_silent_warning(const char* file, const int line, const std::string& message)
{
	std::cerr
		#ifdef DEBUG_MODE
			<< U2L(m::get_log_debug_prefix(file, line))
		#endif
		<< "SW: " << U2L(message) << std::endl;

	std::cerr.flush();
}



void print_warning(const char* file, const int line, const std::string& title, const std::string& message)
{
	std::cerr
		#ifdef DEBUG_MODE
			<< U2L(m::get_log_debug_prefix(file, line))
		#endif
		<< "W: ";

	if(!title.empty())
		std::cerr << "[" << U2L(title) << "] ";

	std::cerr << U2L(message) << std::endl;

	std::cerr.flush();
}



void show_info_message(Gtk::Window& parent_window, const std::string& message)
{
	show_info_message(parent_window, "", message);
}



void show_info_message(Gtk::Window& parent_window, const std::string& title, const std::string& message)
{
	Gtk::Dialog* message_dialog = NULL;
	message_dialog = create_message_dialog(parent_window, INFO, title, message);
	message_dialog->run();
	delete message_dialog;
}



void show_warning_message(Gtk::Window& parent_window, const std::string& message)
{
	show_warning_message(parent_window, "", message);
}



void show_warning_message(Gtk::Window& parent_window, const std::string& title, const std::string& message)
{
	Gtk::Dialog* message_dialog = NULL;
	message_dialog = create_message_dialog(parent_window, WARNING, title, message);
	message_dialog->run();
	delete message_dialog;
}



void show_warning_message(Gtk::Widget& parent_widget, const std::string& message)
{
	Gtk::Window* window = dynamic_cast<Gtk::Window*>(parent_widget.get_toplevel());

	if(window)
		show_warning_message(*window, message);
	else
		show_warning_message(get_main_window(), message);
}



void show_warning_message(Gtk::Widget& parent_widget, const std::string& title, const std::string& message)
{
	Gtk::Window* window = dynamic_cast<Gtk::Window*>(parent_widget.get_toplevel());

	if(window)
		show_warning_message(*window, title, message);
	else
		show_warning_message(get_main_window(), title, message);
}



int main(int argc, char *argv[])
{
	// Устанавливаем обработчики сигналов -->
	{
		struct sigaction sig_action;

		// SIGPIPE -->
			sig_action.sa_handler = SIG_IGN;
			sigemptyset(&sig_action.sa_mask);
			sig_action.sa_flags = 0;

			sigaction(SIGPIPE, &sig_action, NULL);
		// SIGPIPE <--

		// SIGCHLD -->
			sig_action.sa_handler = &sigchld_handler;
			sigemptyset(&sig_action.sa_mask);
			sig_action.sa_flags = 0;

			sigaction(SIGCHLD, &sig_action, NULL);
		// SIGCHLD <--
	}
	// Устанавливаем обработчики сигналов <--
	// gettext -->
		setlocale(LC_ALL, "");

		#ifdef ENABLE_NLS
			if(!bindtextdomain(APP_UNIX_NAME, APP_LOCALE_PATH))
				MLIB_D(_C("Unable to bind text domain: %1.", EE(errno)));
			else if(!bind_textdomain_codeset(APP_UNIX_NAME, "UTF-8"))
				MLIB_D(_C("Unable to bind text domain codeset: %1.", EE(errno)));
			else if(!textdomain(APP_UNIX_NAME))
				MLIB_D(_C("Unable to set text domain: %1.", EE(errno)));
		#endif
	// gettext <--

	std::auto_ptr<Gtk::Main> gtk_main;

	// Если программа запущена из консоли, то Gtk::Main
	// просто завершит ее, поэтому необходимо инициализировать
	// Main loop только тогда, когда доступен графический режим.
	if(is_gui_mode())
	{
		Glib::thread_init();
		gdk_threads_init();
		gdk_threads_enter();

		gtk_main = std::auto_ptr<Gtk::Main>(new Gtk::Main(argc, argv));

		Gtk::Window::set_default_icon_name(APP_UNIX_NAME);
		Gtk::IconTheme::get_default()->append_search_path(APP_CUSTOM_ICONS_PATH);
	}

	// Входим в режим отображения ошибки, если это
	// необходимо.
	error_mode_check(argc, argv);

	// В отличие от стандартной функции выводит графическое сообщение.
	m::set_error_function(error_function);

	// Теперь в случае любого Warning'а программа завершится с
	// ошибкой.
	m::set_warning_function(app_init_warning_function);

	// Устанавливаем имя команды программы - оно будет отображаться,
	// например, если выполнить APP_CMD_NAME --help.
	Glib::set_prgname(APP_UNIX_NAME);

	// Устанавливаем реальное имя программы.
	Glib::set_application_name(APP_NAME);

	// Парсим опции командной строки -->
		Client_cmd_options cmd_options;

		try
		{
			cmd_options.parse(argc, argv);
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
		}
	// Парсим опции командной строки <--

	// Создаем директорию с конфигурационными файлами,
	// если она еще не существует.
	// -->
		try
		{
			m::fs::mkdir_if_not_exists_with_race_conditions(cmd_options.config_path);
		}
		catch(m::Exception& e)
		{
			MLIB_W(__("Can't create config directory '%1': %2.", cmd_options.config_path, EE(e)));
		}
	// <--

	// Получаем уникальный идентификатор конфига и делаем директорию
	// конфига текущей директорией, чтобы в случае создания ссылок
	// или монтирований выше по дереву директорий мы оставались все
	// равно в своей директории с конфигом.
	// -->
		std::string config_unique_id;

		{
			int config_dir_fd = -1;

			try
			{
				config_dir_fd = m::fs::unix_open(cmd_options.config_path, O_RDONLY);
			}
			catch(m::Exception& e)
			{
				MLIB_W(__("Error while opening config directory '%1': %2.", cmd_options.config_path, EE(e)));
			}


			m::fs::Stat config_dir_stat;

			try
			{
				config_dir_stat = m::fs::unix_fstat(config_dir_fd);
			}
			catch(m::Exception& e)
			{
				MLIB_W(__("Error! Can't stat config directory '%1': %2.", cmd_options.config_path, EE(e)));
				close(config_dir_fd);
			}

			if(!config_dir_stat.is_dir())
			{
				MLIB_W(__("Error! Config path '%1' is not a directory.", cmd_options.config_path));
				close(config_dir_fd);
			}

			if(fchdir(config_dir_fd))
			{
				MLIB_W(__("Error! Can't change current directory to config directory '%1': %2.", cmd_options.config_path, EE(errno)));
				close(config_dir_fd);
			}

			close(config_dir_fd);

			config_unique_id = m::to_string(config_dir_stat.dev) + "_" + m::to_string(config_dir_stat.ino);
		}
	// <--

	// Если программа запущена из консоли, то, скорее всего,
	// DBus сессия X'ов нам недоступна, или доступна другая
	// DBus сессия. Поэтому пытаемся ее получить.
	// -->
		if(!is_gui_mode())
		{
			bool link_exists = false;

			try
			{
				link_exists = m::fs::is_lexists(DBUS_SESSION_LINK_NAME);
			}
			catch(m::Exception& e)
			{
				MLIB_W(__(
					"Can't communicate with running %1 instance. Error while reading link '%2' target path: %3.",
					APP_NAME, m::fs::get_abs_path_lazy(DBUS_SESSION_LINK_NAME), EE(e)
				));
			}

			if(link_exists)
			{
				std::string dbus_address;

				try
				{
					dbus_address = m::fs::unix_readlink(DBUS_SESSION_LINK_NAME);
				}
				catch(m::Exception& e)
				{
					MLIB_W(__(
						"Can't communicate with running %1 instance. Error while reading link '%2' target path: %3.",
						APP_NAME, m::fs::get_abs_path_lazy(DBUS_SESSION_LINK_NAME), EE(e)
					));
				}

				try
				{
					m::setenv(DBUS_SESSION_ENV_NAME, dbus_address, true);
				}
				catch(m::Exception& e)
				{
					MLIB_W(__(
						"Can't set environment variable '%1'='%2': %3.",
						DBUS_SESSION_ENV_NAME, dbus_address
					));
				}
			}
			else
				MLIB_W(__("Can't communicate with running %1 instance. It is not running.", APP_NAME));
		}
	// <--

	// Генерируем имя DBus, уникальное для данного конфига
	std::string dbus_name = std::string(DBUS_NAME_PREFIX) + "config_" + config_unique_id;

	// DBus будет использовать Glib'овский диспетчер -->
		DBus::Glib::BusDispatcher dispatcher;
		DBus::default_dispatcher = &dispatcher;
		dispatcher.attach(NULL);
	// DBus будет использовать Glib'овский диспетчер <--

	// Являемся ли мы владельцем DBus имени.
	bool is_owner = false;

	// Есть ли в сессии владелец DBus имени.
	bool has_owner = true;

	std::auto_ptr<DBus::Connection> dbus_connection;

	try
	{
		// Создаем соединение DBus
		dbus_connection = std::auto_ptr<DBus::Connection>(new DBus::Connection(DBus::Connection::SessionBus()));

		if(is_gui_mode() && !cmd_options.only_pass)
		{
			// Определяем, являемся ли мы хозяином сессии -->
				switch(dbus_connection->request_name(dbus_name.c_str(), DBUS_NAME_FLAG_DO_NOT_QUEUE))
				{
					case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER:
						is_owner = true;
						break;

					case DBUS_REQUEST_NAME_REPLY_EXISTS:
						break;

					case DBUS_REQUEST_NAME_REPLY_IN_QUEUE:
						MLIB_W(
							_("Unexpected DBus behavior"),
							_("Name request has been added to queue, but queuing has been forbidden.")
						);
						break;

					case DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER:
						MLIB_W(
							_("Unexpected DBus behavior while requesting name"),
							_("DBus saying that application is already name owner, but name request has not been sent yet.")
						);
						break;

					default:
						MLIB_W(
							_("Unexpected DBus behavior while requesting name"),
							_("Unknown reply has been gotten.")
						);
						break;
				}
			// Определяем, являемся ли мы хозяином сессии <--
		}
		else
			has_owner = dbus_connection->has_name(dbus_name.c_str());
	}
	catch(DBus::Error e)
	{
		MLIB_W(_("DBus error"), EE(e));
	}


	// Мы захватили имя и находимся в графическом режиме
	if(is_owner)
	{
		// Создаем ссылку, которая будет указывать на DBus сессию.
		//
		// TODO:
		// Вообще говоря, не лучшее решение, т. к. если программа
		// упадет, то ссылка останется и будет указывать на устаревшую
		// сессию. Также могут возникнуть проблемы, если запускать
		// программу с разных дисплеев.
		// Пока реализовывать что-либо более сложное не охота. :)
		// -->
		{
			try
			{
				if(m::fs::is_lexists(DBUS_SESSION_LINK_NAME))
				{
					try
					{
						m::fs::rm(DBUS_SESSION_LINK_NAME);
					}
					catch(m::Exception& e)
					{
						MLIB_W(__(
							"Creating %1 session failed. Can't remove DBus session link '%2'. %3",
							APP_NAME, m::fs::get_abs_path_lazy(DBUS_SESSION_LINK_NAME), EE(e)
						));
					}
				}
			}
			catch(m::Exception& e)
			{
				MLIB_W(__(
					"Creating %1 session failed. Error while reading link '%2' target path: %3.",
					APP_NAME, m::fs::get_abs_path_lazy(DBUS_SESSION_LINK_NAME), EE(e)
				));
			}

			const char *dbus_session_address = getenv(DBUS_SESSION_ENV_NAME);
			if(!dbus_session_address)
				MLIB_W(__("Creating %1 session failed. Can't get DBus session bus address.", APP_NAME));

			try
			{
				m::fs::unix_symlink(dbus_session_address, DBUS_SESSION_LINK_NAME);
			}
			catch(m::Exception& e)
			{
				MLIB_W(__(
					"Creating %1 session failed. Can't create DBus session link '%2': %3.",
					APP_NAME, m::fs::get_abs_path_lazy(DBUS_SESSION_LINK_NAME), EE(e)
				));
			}
		}
		// <--


		/// Устанавливаем собственный обработчик сигнала для кнопок-ссылок,
		/// чтобы мы могли назначать им какой-угодно URL, и GTK при этом не сыпал
		/// Warning'ами.
		Gtk::LinkButton::set_uri_hook(sigc::ptr_fun(on_linkbutton_uri_callback));

		/// Активизируем "режим дерева" для GtkCellRendererToggle.
		m::gtk::activate_cell_renderer_toggle_tree_mode();

		// Теперь уже нет необходимости завершать работу в случае любого
		// Warning'а.
		m::set_warning_function(NULL);


		Application app(cmd_options, *dbus_connection, DBUS_PATH, dbus_name);

		try
		{
			app.start();
			Gtk::Main::run();

			// Удаляем созданную ссылку -->
				try
				{
					m::fs::rm(DBUS_SESSION_LINK_NAME);
				}
				catch(m::Exception& e)
				{
					MLIB_W(__(
						"Closing %1 session failed. Can't remove DBus session link '%2': %3.",
						APP_NAME, m::fs::get_abs_path_lazy(DBUS_SESSION_LINK_NAME), EE(e)
					));
				}
			// Удаляем созданную ссылку <--
		}
		catch(m::Exception& e)
		{
			// Удаляем созданную ссылку -->
				try
				{
					m::fs::rm(DBUS_SESSION_LINK_NAME);
				}
				catch(m::Exception& e)
				{
					MLIB_W(__(
						"Closing %1 session failed. Can't remove DBus session link '%2': %3.",
						APP_NAME, m::fs::get_abs_path_lazy(DBUS_SESSION_LINK_NAME), EE(e)
					));
				}
			// Удаляем созданную ссылку <--

			m::set_warning_function(app_init_warning_function);
			MLIB_W(EE(e));
		}
	}
	// Мы находимся в консоли или было указание просто передать
	// опции командной строки.
	else
	{
		if(has_owner)
		{
			std::vector<std::string> cmd_options_strings = cmd_options.to_strings();

			if(cmd_options_strings.empty())
				MLIB_I(_("Nothing to process. Exiting..."));
			else
			{
				MLIB_I(_("Delivering command line options to working copy..."));

				Application_proxy remote_app(*dbus_connection, DBUS_PATH, dbus_name);

				try
				{
					remote_app.dbus_cmd_options(cmd_options_strings);
				}
				catch(DBus::Error e)
				{
					MLIB_W(__("Can't communicate with running %1 instance via DBus: %2.", APP_NAME, EE(e)));
				}
			}
		}
		else
			MLIB_W(__("Can't communicate with running %1 instance. It is not running.", APP_NAME));
	}

	MLIB_D("Exiting...");

    return EXIT_SUCCESS;
}

