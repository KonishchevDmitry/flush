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


#ifdef MLIB_ENABLE_GTK

#include <mlib/main.hpp>

#include <mlib/gtk/misc.hpp>

#include "dialog.hpp"


namespace m { namespace gtk {

	Dialog::Dialog(BaseObjectType* cobject)
	:
		Gtk::Dialog(cobject)
	{
		this->property_destroy_with_parent() = true;
		this->set_title(this->get_title());
	}



	Dialog::Dialog(Gtk::Window& parent_window, const std::string& title, const Settings& settings, int width, int height, int border_width)
	:
		Gtk::Dialog(title, parent_window, true)
	{
		this->property_destroy_with_parent() = true;
		this->set_border_width(border_width);
		this->set_title(title);

		if(width > 0 && height > 0)
			this->set_default_size(width, height);

		this->init(parent_window, settings);
	}



	void Dialog::init(Gtk::Window& parent_window, const Settings& settings)
	{
		this->set_transient_for(parent_window);

		if(parent_window.is_visible())
			this->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
		else
			this->set_position(Gtk::WIN_POS_CENTER);

		if(settings.width > 0 && settings.height > 0)
			this->resize(settings.width, settings.height);
	}



	void Dialog::set_title(const Glib::ustring& title)
	{
		Gtk::Dialog::set_title(m::gtk::format_window_title(title));
	}



	void Dialog::save_settings(Settings& settings) const
	{
		this->get_size(settings.width, settings.height);
	}

}}

#endif

