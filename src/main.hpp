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


#ifndef HEADER_MAIN
	#define HEADER_MAIN

	/// Создает диалог для отображения сообщения.
	Gtk::Dialog*		create_message_dialog(Gtk::Window& parent_window, Message_type type, std::string title, Glib::ustring message);

	/// Возвращает объект application.
	Application&		get_application(void);

	/// Возвращает текущие настройки клиента.
	Client_settings&	get_client_settings(void);

	/// Возвращает proxy объект демона.
	/// Сделана для того, чтобы не подключать заголовочный
	/// файл Application, когда необходим только он.
	Daemon_proxy&		get_daemon_proxy(void);

	/// Возвращает путь к директории с конфигурационными файлами
	/// по умолчанию.
	std::string			get_default_config_dir_path(void);

	/// Возвращает главное окно приложения.
	/// Сделана для того, чтобы не подключать заголовочный
	/// файл Application, когда необходимо только окно.
	Main_window&		get_main_window(void);

	/// Определяет, в каком режиме запущена программа.
	bool				is_gui_mode(void);

	/// Выводит на консоль информационное сообщение.
	void 				print_info(const char* file, const int line, const std::string& title, const std::string& message);

	/// Выводит на консоль silent warning сообщение.
	void 				print_silent_warning(const char* file, const int line, const std::string& message);

	/// Выводит на консоль warning сообщение.
	void 				print_warning(const char* file, const int line, const std::string& title, const std::string& message);

	/// Отображает Info-сообщение.
	void				show_info_message(Gtk::Window& parent_window, const std::string& message);

	/// Отображает Info-сообщение.
	void				show_info_message(Gtk::Window& parent_window, const std::string& title, const std::string& message);

	/// Отображает Warning-сообщение.
	void				show_warning_message(Gtk::Window& parent_window, const std::string& message);

	/// Отображает Warning-сообщение.
	void				show_warning_message(Gtk::Window& parent_window, const std::string& title, const std::string& message);

	/// Отображает Warning-сообщение либо поверх того окна, в котором находится
	/// виджет, либо, если виджет не находится ни в одном окне отображает его
	/// поверх главного окна.
	void				show_warning_message(Gtk::Widget& parent_widget, const std::string& message);

	/// Отображает Warning-сообщение либо поверх того окна, в котором находится
	/// виджет, либо, если виджет не находится ни в одном окне отображает его
	/// поверх главного окна.
	void				show_warning_message(Gtk::Widget& parent_widget, const std::string& title, const std::string& message);

	/// Инициирует обновление графического интерфейса.
	void				update_gui(void);

#endif

