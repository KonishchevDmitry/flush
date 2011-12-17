/**************************************************************************
*                                                                         *
*   Flush - GTK-based BitTorrent client                                   *
*   http://sourceforge.net/projects/flush                                 *
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


#include <gdk/gdk.h>

#include <glibmm/markup.h>

#include <gdkmm/cursor.h>
#include <gdkmm/window.h>

#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>

#include <mlib/fs.hpp>
#include <mlib/string.hpp>

#include <mlib/gtk/builder.hpp>
#include <mlib/gtk/misc.hpp>

#include "application.hpp"
#include "client_settings.hpp"
#include "common.hpp"
#include "daemon_proxy.hpp"
#include "main.hpp"
#include "torrent_details_view.hpp"



// Torrent_details_view -->
	Torrent_details_view::Torrent_details_view(void)
	:
		compact(get_client_settings().gui.compact_details_tab),

		size(NULL),
		requested_size(NULL),
		downloaded_requested_size(NULL),

		total_download(NULL),
		total_payload_download(NULL),
		total_upload(NULL),
		total_payload_upload(NULL),
		total_failed(NULL),
		total_redundant(NULL),

		download_speed(NULL),
		download_payload_speed(NULL),
		upload_speed(NULL),
		upload_payload_speed(NULL),

		share_ratio(NULL),

		peers(NULL),
		seeds(NULL),

		next_announce(NULL),
		announce_interval(NULL),

		time_added(NULL),
		time_left(NULL),
		time_seeding(NULL)
	{
		this->update_layout();
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
		{
		#if GTK_CHECK_VERSION(3, 0, 0)
			gdk_window->set_cursor(Gdk::Cursor::create(Gdk::HAND2));
		#else
			gdk_window->set_cursor(Gdk::Cursor(Gdk::HAND2));
		#endif
		}

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
		if(label)
			label->set_label(string);
	}



	void Torrent_details_view::set_time(Gtk::Label* label, Time time)
	{
		this->set_string(label, m::time_to_string_with_date(time));
	}



	void Torrent_details_view::update(const Torrent_id& torrent_id)
	{
		// Переключаем режим, если это необходимо
		if(this->compact != get_client_settings().gui.compact_details_tab)
		{
			this->compact = get_client_settings().gui.compact_details_tab;
			this->update_layout();
		}


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



	void Torrent_details_view::update_layout(void)
	{
		// Получает виджет, который отображается в обоих режимах
		#define this_get(name) MLIB_GTK_BUILDER_GET_WIDGET(builder, #name, this->name)

		// Получает виджет, который отображается только в полном режиме
		#define this_GET(name) { if(!compact) MLIB_GTK_BUILDER_GET_WIDGET(builder, #name, this->name); else this->name = NULL; }


		// Удаляем предыдущий контейнер с виджетами -->
			BOOST_FOREACH(Gtk::Widget* widget, this->get_children())
			{
				this->remove(*widget);
				delete widget;
			}
		// Удаляем предыдущий контейнер с виджетами <--


		std::string builder_path;

		if(compact)
			builder_path = Path(APP_UI_PATH) / "torrent_info_tabs.details_compact.glade";
		else
			builder_path = Path(APP_UI_PATH) / "torrent_info_tabs.details.glade";

		m::gtk::Builder builder = MLIB_GTK_BUILDER_CREATE(builder_path, "details");

		this->pack_start(*MLIB_GTK_BUILDER_GET_WIDGET(builder, "details"), false, false);


		this_get(status);

		this_get(size);
		this_get(requested_size);
		this_get(downloaded_requested_size);

		this_GET(total_download);
		this_get(total_payload_download);
		this_GET(total_upload);
		this_get(total_payload_upload);
		this_GET(total_failed);
		this_GET(total_redundant);

		this_GET(download_speed);
		this_get(download_payload_speed);
		this_GET(upload_speed);
		this_get(upload_payload_speed);

		this_get(share_ratio);

		this_GET(peers);
		this_GET(seeds);

		this_GET(next_announce);
		this_GET(announce_interval);

		this_GET(time_added);
		this_get(time_left);
		this_GET(time_seeding);

		this_get(tracker_status);
		this_get(publisher_url);
		this_get(publisher_url_event_box);


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

		#undef this_get
		#undef this_GET
	}
// Torrent_details_view

