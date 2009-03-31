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


Message::Message(void)
:
	type(ERROR)
{
}



Message::Message(Type type, const std::string& title, const std::string& message)
:
	type(type),
	title(title),
	message(message)
{
}



Application* Application::get(void)
{
	MLIB_A(Application::ptr);
	return Application::ptr;
}



Client_settings& Application::get_client_settings(void)
{
	return this->client_settings;
}



const std::string& Application::get_config_dir_path(void)
{
	return this->config_dir_path;
}



Daemon_proxy& Application::get_daemon_proxy(void)
{
	return *this->daemon_proxy;
}



Main_window& Application::get_main_window(void)
{
	MLIB_A(this->main_window);
	return *this->main_window;
}

