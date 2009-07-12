/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
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


#include <mlib/gtk/main.hpp>
#include <mlib/gtk/window_settings.hpp>

#include "window.hpp"


namespace m { namespace gtk {

Window::Window(const std::string& title, const Settings& settings, int width, int height, int border_width)
{
	this->init(NULL, title, settings, width, height, border_width);
}



Window::Window(Gtk::Window& parent_window, const std::string& title, const Settings& settings, int width, int height, int border_width)
{
	this->init(&parent_window, title, settings, width, height, border_width);
}



void Window::init(Gtk::Window* parent_window, const std::string& title, const Settings& settings, int width, int height, int border_width)
{
	this->set_border_width(border_width);
	this->set_title(title);

	if(parent_window)
	{
		this->set_transient_for(*parent_window);

		if(parent_window->is_visible())
			this->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
		else
			this->set_position(Gtk::WIN_POS_CENTER);
	}
	else
		this->set_position(Gtk::WIN_POS_CENTER);

	if(width > 0 && height > 0)
		this->set_default_size(width, height);

	if(settings.width > 0 && settings.height > 0)
		this->resize(settings.width, settings.height);
}



void Window::save_settings(Settings& settings) const
{
	this->get_size(settings.width, settings.height);
}

}}

