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


#include <gtkmm/entry.h>

#include <mlib/gtk/builder.hpp>
#include <mlib/gtk/dialog.hpp>
#include <mlib/gtk/misc.hpp>
#include <mlib/fs.hpp>
#include <mlib/libtorrent.hpp>

#include "main.hpp"
#include "main_window.hpp"
#include "open_magnet_dialog.hpp"



namespace Open_magnet_dialog_aux
{

// Private -->
	class Private
	{
		public:
			Private(const m::gtk::Builder& builder);


		public:
			Gtk::Entry*	magnet_link;
	};



	Private::Private(const m::gtk::Builder& builder)
	{
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "magnet_link", this->magnet_link);
	}
// Private <--

}



Open_magnet_dialog::Open_magnet_dialog(BaseObjectType* cobject, const m::gtk::Builder& builder)
:
	m::gtk::Dialog(cobject),
	priv(new Private(builder))
{
	priv->magnet_link->signal_activate().connect(sigc::bind<int>(
		sigc::mem_fun(*this, &Open_magnet_dialog::response), Gtk::RESPONSE_OK ));
}



Open_magnet_dialog::~Open_magnet_dialog(void)
{
	MLIB_D("Destroying open magnet dialog...");
}



void Open_magnet_dialog::create(Gtk::Window& parent_window)
{
	MLIB_D("Creating open magnet dialog...");

	m::gtk::Builder builder = MLIB_GTK_BUILDER_CREATE(
		m::fs::Path(APP_UI_PATH) / "dialog.open_magnet_link.glade", "open_magnet_link");

	Open_magnet_dialog* dialog;
	MLIB_GTK_BUILDER_GET_WIDGET_DERIVED(builder, "open_magnet_link", dialog);
	dialog->init(parent_window);

	// Запрашиваем у пользователя magnet-ссылку -->
	{
		int response;

		while(true)
		{
			response = dialog->run();
			
			if(response != Gtk::RESPONSE_OK)
				break;

			std::string uri = dialog->get_uri();

			if(m::lt::is_magnet_uri(uri))
			{
				get_main_window().open_torrent(uri);
				break;
			}
			else
			{
				if(uri.empty())
				{
					if(!m::gtk::message(*dialog, _("Invalid magnet URI"),
						_("Please enter a magnet URI.") ))
						return;
				}
				else
				{
					if(!m::gtk::message(*dialog, _("Invalid magnet URI"),
						__("'%1' is not a magnet URI.", uri) ))
						return;
				}
			}
		}

		if(response != Gtk::RESPONSE_NONE)
			delete dialog;
	}
	// Запрашиваем у пользователя magnet-ссылку <--
}



std::string Open_magnet_dialog::get_uri(void)
{
	return priv->magnet_link->get_text();
}



void Open_magnet_dialog::on_hide(void)
{
	delete this;
}

