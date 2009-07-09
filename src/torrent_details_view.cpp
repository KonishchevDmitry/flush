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


#include <gdk/gdkevents.h>

#include <glibmm/markup.h>

#include <gdkmm/cursor.h>
#include <gdkmm/window.h>

#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>

#include <mlib/gtk/glade.hpp>
#include <mlib/gtk/misc.hpp>

#include "application.hpp"
#include "daemon_proxy.hpp"
#include "main.hpp"
#include "torrent_details_view.hpp"



// Torrent_details_view -->
	Torrent_details_view::Torrent_details_view(void)
	{
		Glib::RefPtr<Gnome::Glade::Xml> glade = MLIB_GLADE_CREATE(
			std::string(APP_UI_PATH) + "/torrent_info_tabs.details.glade", "details"
		);

		this->pack_start(*MLIB_GLADE_GET_WIDGET(glade, "details"), false, false);

		MLIB_GLADE_GET_WIDGET(glade, "status",						this->status);

		MLIB_GLADE_GET_WIDGET(glade, "size",						this->size);
		MLIB_GLADE_GET_WIDGET(glade, "requested_size",				this->requested_size);
		MLIB_GLADE_GET_WIDGET(glade, "downloaded_requested_size",	this->downloaded_requested_size);

		MLIB_GLADE_GET_WIDGET(glade, "total_download",				this->total_download);
		MLIB_GLADE_GET_WIDGET(glade, "total_payload_download",		this->total_payload_download);
		MLIB_GLADE_GET_WIDGET(glade, "total_upload",				this->total_upload);
		MLIB_GLADE_GET_WIDGET(glade, "total_payload_upload",		this->total_payload_upload);
		MLIB_GLADE_GET_WIDGET(glade, "total_failed",				this->total_failed);
		MLIB_GLADE_GET_WIDGET(glade, "total_redundant",				this->total_redundant);

		MLIB_GLADE_GET_WIDGET(glade, "download_speed",				this->download_speed);
		MLIB_GLADE_GET_WIDGET(glade, "download_payload_speed",		this->download_payload_speed);
		MLIB_GLADE_GET_WIDGET(glade, "upload_speed",				this->upload_speed);
		MLIB_GLADE_GET_WIDGET(glade, "upload_payload_speed",		this->upload_payload_speed);

		MLIB_GLADE_GET_WIDGET(glade, "share_ratio",					this->share_ratio);

		MLIB_GLADE_GET_WIDGET(glade, "peers",						this->peers);
		MLIB_GLADE_GET_WIDGET(glade, "seeds",						this->seeds);

		MLIB_GLADE_GET_WIDGET(glade, "next_announce",				this->next_announce);
		MLIB_GLADE_GET_WIDGET(glade, "announce_interval",			this->announce_interval);

		MLIB_GLADE_GET_WIDGET(glade, "time_added",					this->time_added);
		MLIB_GLADE_GET_WIDGET(glade, "time_left",					this->time_left);
		MLIB_GLADE_GET_WIDGET(glade, "time_seeding",				this->time_seeding);

		MLIB_GLADE_GET_WIDGET(glade, "tracker_status",				this->tracker_status);
		MLIB_GLADE_GET_WIDGET(glade, "publisher_url",				this->publisher_url);
		MLIB_GLADE_GET_WIDGET(glade, "publisher_url_event_box",		this->publisher_url_event_box);

		// Publisher URL -->
			this->publisher_url_event_box->signal_button_press_event().connect(
				sigc::mem_fun(*this, &Torrent_details_view::on_publisher_url_button_press_event_cb)
			);
			this->publisher_url_event_box->signal_enter_notify_event().connect(
				sigc::mem_fun(*this, &Torrent_details_view::on_publisher_url_enter_notify_event_cb)
			);
			this->publisher_url_event_box->signal_leave_notify_event().connect(
				sigc::mem_fun(*this, &Torrent_details_view::on_publisher_url_leave_notify_event_cb)
			);
		// Publisher URL

		this->show_all();
	}



	void Torrent_details_view::clear(void)
	{
		this->status->set_text("");
		this->status->set_fraction(0);

		this->set_string(	this->size,							""	);
		this->set_string(	this->requested_size,				""	);
		this->set_string(	this->downloaded_requested_size,	""	);

		this->set_string(	this->total_download,				""	);
		this->set_string(	this->total_payload_download,		""	);
		this->set_string(	this->total_upload,					""	);
		this->set_string(	this->total_payload_upload,			""	);
		this->set_string(	this->total_failed,					""	);
		this->set_string(	this->total_redundant,				""	);

		this->set_string(	this->download_speed,				""	);
		this->set_string(	this->download_payload_speed,		""	);
		this->set_string(	this->upload_speed,					""	);
		this->set_string(	this->upload_payload_speed,			""	);

		this->set_string(	this->share_ratio,					""	);

		this->set_string(	this->peers,						""	);
		this->set_string(	this->seeds,						""	);

		this->set_string(	this->time_added,					""	);
		this->set_string(	this->time_left,					""	);
		this->set_string(	this->time_seeding,					""	);


		this->set_string(	this->tracker_status,				""	);
		this->publisher_url_string = "";
		this->set_string(	this->publisher_url,				""	);
	}



	bool Torrent_details_view::on_publisher_url_button_press_event_cb(GdkEventButton* event)
	{
		if(event->type == GDK_BUTTON_PRESS && m::is_url_string(this->publisher_url_string))
			get_application().open_uri(this->publisher_url_string);

		return FALSE;
	}



	bool Torrent_details_view::on_publisher_url_enter_notify_event_cb(GdkEventCrossing* event)
	{
		Glib::RefPtr<Gdk::Window> gdk_window = this->publisher_url_event_box->get_window();

		if(gdk_window)
			gdk_window->set_cursor(Gdk::Cursor(Gdk::HAND2));

		return FALSE;
	}



	bool Torrent_details_view::on_publisher_url_leave_notify_event_cb(GdkEventCrossing* event)
	{
		Glib::RefPtr<Gdk::Window> gdk_window = this->publisher_url_event_box->get_window();

		if(gdk_window)
			gdk_window->set_cursor();

		return FALSE;
	}



	void Torrent_details_view::set_size(Gtk::Label* label, Size size)
	{
		this->set_string(label, m::size_to_string(size));
	}



	void Torrent_details_view::set_speed(Gtk::Label* label, Speed speed)
	{
		this->set_string(label, m::speed_to_string(speed));
	}



	void Torrent_details_view::set_string(Gtk::Label* label, const std::string& string)
	{
		label->set_label(string);
	}



	void Torrent_details_view::set_time(Gtk::Label* label, Time time)
	{
		this->set_string(label, m::time_to_string_with_date(time));
	}



	void Torrent_details_view::update(const Torrent_id& torrent_id)
	{
		if(torrent_id)
		{
			try
			{
				// m::Exception
				Torrent_details details = get_daemon_proxy().get_torrent_details(torrent_id);


				this->status->set_text(
					"  " + details.name + ": " +
					details.get_status_string() + " " +
					m::to_string(details.progress) + "%  "
				);
				this->status->set_fraction( static_cast<double>(details.progress) / 100 );

				this->set_size(		this->size,							details.size);
				this->set_size(		this->requested_size,				details.requested_size);
				this->set_size(		this->downloaded_requested_size,	details.downloaded_requested_size);

				this->set_size(		this->total_download,				details.total_download);
				this->set_size(		this->total_payload_download,		details.total_payload_download);
				this->set_size(		this->total_upload,					details.total_upload);
				this->set_size(		this->total_payload_upload,			details.total_payload_upload);
				this->set_size(		this->total_failed,					details.total_failed);
				this->set_size(		this->total_redundant,				details.total_redundant);

				this->set_speed(	this->download_speed,				details.download_speed);
				this->set_speed(	this->download_payload_speed,		details.payload_download_speed);
				this->set_speed(	this->upload_speed,					details.upload_speed);
				this->set_speed(	this->upload_payload_speed,			details.payload_upload_speed);

				this->set_string(	this->share_ratio,					get_share_ratio_string(details.total_payload_upload, details.total_payload_download));

				this->set_string(	this->peers,						m::to_string(details.peers_num));
				this->set_string(	this->seeds,						m::to_string(details.seeds_num));


				this->set_string(	this->next_announce,				m::get_within_hour_time_left_string(details.next_announce));
				this->set_string(	this->announce_interval,			m::get_time_left_string(details.announce_interval));


				this->set_time(		this->time_added,					details.time_added);
				this->set_string(	this->time_left,					m::get_time_left_string(details.get_time_left()));
				this->set_string(	this->time_seeding,					m::get_time_duration_string(details.time_seeding));


				this->set_string(	this->tracker_status,				details.tracker_status);
				this->publisher_url_string = details.publisher_url;
				this->set_string(	this->publisher_url,				"<u><span color='#2828ED'>" +
																			Glib::Markup::escape_text(details.publisher_url) +
																		"</span></u>");
			}
			catch(m::Exception& e)
			{
				// Этого торрента уже вполне может не оказаться, т. к. между
				// обновлением списка торрентов и этим запросом торрент мог
				// быть удален, например, функцией автоматического удаления
				// старых торрентов.
				MLIB_D(_C("Updating torrent details view failed. %1", EE(e)));
				this->clear();
			}
		}
		else
			this->clear();
	}
// Torrent_details_view

