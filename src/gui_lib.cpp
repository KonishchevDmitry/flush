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


#include <mlib/gtk/misc.hpp>

#include "gui_lib.hpp"
#include "main.hpp"
#include "main_window.hpp"



Glib::ustring format_window_title(const Glib::ustring& title)
{
	if(title == "")
		return APP_NAME;
	else
		return title + " - " + std::string(APP_NAME);
}



m::gtk::Dialog_response ok_cancel_dialog(const std::string& title, const std::string& message)
{
	return m::gtk::ok_cancel_dialog(get_main_window(), title, message);
}

