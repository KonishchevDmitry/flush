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


#include <gdkmm/pixbuf.h>

#include <gtkmm/action.h>
#include <gtkmm/box.h>
#include <gtkmm/stock.h>
#include <gtkmm/toolbutton.h>

#include <mlib/gtk/action.hpp>
#include <mlib/gtk/misc.hpp>

#include "app_icons.hpp"



namespace app_icons
{

namespace
{
	class Invalid_icon_id {};

	/// Возвращает StockID, который соответствует иконке id. Если id не
	/// соответствует ни одна Stock'овая иконка, генерирует Invalid_icon_id.
	/// @throw - Invalid_icon_id.
	Gtk::StockID	get_stock_id(Id id);

	/// Возвращает имя иконки, которое соответствует иконке id внутри текущей
	/// темы. Если id не соответствует ни одна иконка в текущей теме, генерирует
	/// Invalid_icon_id.
	/// @throw - Invalid_icon_id.
	std::string		get_theme_name(Id id);



	Gtk::StockID get_stock_id(Id id)
	{
		// В данный момент не используется ни одна Stock'овая иконка.

		switch(id)
		{
			default:
				throw Invalid_icon_id();
				break;
		}
	}



	std::string get_theme_name(Id id)
	{
		std::string name;

		switch(id & ~ICON_FLAG_PAUSE & ~ICON_FLAG_TRACKER_ERROR)
		{
			case ICON_DOWNLOAD:
				name = "download";
				break;

			case ICON_DOWNLOAD_AND_UPLOAD:
				name = "download-and-upload";
				break;

			case ICON_STATISTICS:
				name = "statistics";
				break;

			case ICON_TORRENT_ALLOCATING:
				name = "torrent-allocating";
				break;

			case ICON_TORRENT_CHECKING:
				name = "torrent-checking";
				break;

			case ICON_TORRENT_DOWNLOADING:
				name = "torrent-downloading";
				break;

			case ICON_TORRENT_SEEDING:
				name = "torrent-seeding";
				break;

			case ICON_TORRENT_UPLOADING:
				name = "torrent-uploading";
				break;

			case ICON_TORRENT_WAITING_FOR_DOWNLOAD:
				name = "torrent-waiting-for-download";
				break;

			case ICON_TORRENTS_ALL:
				name = "torrents-all";
				break;

			case ICON_TORRENTS_CHECKING:
				name = "torrents-checking";
				break;

			case ICON_TORRENTS_DOWNLOADING:
				name = "torrents-downloading";
				break;

			case ICON_TORRENTS_PAUSED:
				name = "torrents-paused";
				break;

			case ICON_TORRENTS_SEEDING:
				name = "torrents-seeding";
				break;

			case ICON_TORRENTS_TRACKER_ERROR:
				name = "torrents-tracker-error";
				break;

			case ICON_TORRENTS_UPLOADING:
				name = "torrents-uploading";
				break;

			case ICON_TORRENTS_WAITING_FOR_DOWNLOAD:
				name = "torrents-waiting-for-download";
				break;

			case ICON_UPLOAD:
				name = "upload";
				break;

			default:
				throw Invalid_icon_id();
				break;
		}

		if(id & ICON_FLAG_PAUSE)
			name += "-paused";

		if(id & ICON_FLAG_TRACKER_ERROR)
			name += "-tracker-error";

		return name;
	}
}



Glib::RefPtr<Gtk::Action> create_action(const Glib::ustring& name, Id id, const Glib::ustring& label, const Glib::ustring& tooltip)
{
	try
	{
		return m::gtk::create_action_with_icon_name(name, get_theme_name(id), label, tooltip);
	}
	catch(Invalid_icon_id&)
	{
		try
		{
			return Gtk::Action::create(name, get_stock_id(id), label, tooltip);
		}
		catch(Invalid_icon_id&)
		{
			MLIB_LE();
		}
	}
}



Glib::RefPtr<Gdk::Pixbuf> get_pixbuf(Id id, const Gtk::IconSize& size)
{
	try
	{
		return m::gtk::get_theme_icon(get_theme_name(id), size);
	}
	catch(Invalid_icon_id&)
	{
		try
		{
			return m::gtk::get_stock_icon(get_stock_id(id), size);
		}
		catch(Invalid_icon_id&)
		{
			MLIB_LE();
		}
	}
}



void set_for_tool_button(Gtk::ToolButton& button, Id id)
{
	try
	{
		return button.set_icon_name(get_theme_name(id));
	}
	catch(Invalid_icon_id&)
	{
		try
		{
			return button.set_stock_id(get_stock_id(id));
		}
		catch(Invalid_icon_id&)
		{
			MLIB_LE();
		}
	}
}

}

