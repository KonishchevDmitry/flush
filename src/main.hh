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


#include "application.hpp"
#include "client_settings.hpp"
#include "daemon_proxy.hpp"
#include "main_window.hpp"



Application& get_application(void)
{
	return *Application::get();
}



Client_settings& get_client_settings(void)
{
	return Application::get()->get_client_settings();
}



Daemon_proxy& get_daemon_proxy(void)
{
	return Application::get()->get_daemon_proxy();
}



Main_window& get_main_window(void)
{
	return Application::get()->get_main_window();
}



void update_gui(void)
{
	get_main_window().update_gui();
}

