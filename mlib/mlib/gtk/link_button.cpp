/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
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


#include <gtk/gtk.h>

#include <gtkmm/linkbutton.h>

#include <mlib/gtk/main.hpp>

#include "link_button.hpp"


namespace m { namespace gtk {

Link_button::Link_button(const Glib::ustring& uri)
:
	link_button(NULL)
{
	this->recreate(uri);
}



Glib::ustring Link_button::get_uri(void) const
{
	return this->link_button->get_uri();
}



#if GTK_CHECK_VERSION(2, 14, 0)
	bool Link_button::get_visited(void) const
	{
		return this->link_button->get_visited();
	}
#endif



void Link_button::set_uri(const Glib::ustring& uri)
{
	if(this->link_button->get_uri() != uri)
		this->recreate(uri);
}



void Link_button::on_clicked_cb(void)
{
	this->clicked_signal();
}



void Link_button::recreate(const Glib::ustring& uri)
{
	if(this->link_button)
		this->remove(*this->link_button);

	this->link_button = Gtk::manage(new Gtk::LinkButton(uri));
	this->link_button->signal_clicked().connect(
		sigc::mem_fun(*this, &Link_button::on_clicked_cb));
	this->link_button->show();
	this->add(*this->link_button);
}



#if GTK_CHECK_VERSION(2, 14, 0)
	void Link_button::set_visited(bool visited)
	{
		this->link_button->set_visited(visited);
	}
#endif



sigc::signal<void>& Link_button::signal_clicked(void)
{
	return this->clicked_signal;
}

}}

