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


#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/separator.h>
#include <gtkmm/stock.h>
#include <gtkmm/table.h>

#include <mlib/string.hpp>

#include <mlib/gtk/misc.hpp>
#include <mlib/gtk/vbox.hpp>

#include "common.hpp"
#include "daemon_proxy.hpp"
#include "gui_lib.hpp"
#include "main.hpp"
#include "statistics_window.hpp"



namespace
{
	enum { RESPONSE_RESET };
}



Statistics_window::Statistics_window(Gtk::Window& parent_window)
:
	m::gtk::Dialog(parent_window, format_window_title(_("Statistics"))),

	rows_num(0),
	columns_num(2),
	table(1, columns_num)
{
}



void Statistics_window::add_row(void)
{
	this->table.resize(++this->rows_num, this->columns_num);
}



void Statistics_window::attach_separator(void)
{
	this->add_row();
	this->table.attach(
		*Gtk::manage( new Gtk::HSeparator() ),
		0, this->columns_num,
		this->rows_num - 1, this->rows_num
	);
}



void Statistics_window::attach_share_ratio(const std::string& name, Share_ratio ratio)
{
	this->attach_value(name, get_share_ratio_string(ratio));
}



void Statistics_window::attach_size(const std::string& name, Size size)
{
	this->attach_value(name, m::size_to_string(size));
}



void Statistics_window::attach_time(const std::string& name, Time time)
{
	this->attach_value(name, m::time_to_string_with_date(time));
}



void Statistics_window::attach_value(const std::string& name, const std::string& value)
{
	Gtk::Label* label;

	this->add_row();

	label = Gtk::manage( new Gtk::Label(name + ":  ") );
	label->set_alignment(Gtk::ALIGN_LEFT);
	this->table.attach(*label, 0, 1, this->rows_num - 1, this->rows_num);

	label = Gtk::manage( new Gtk::Label(value) );
	label->set_alignment(Gtk::ALIGN_RIGHT);
	this->table.attach(*label, 1, 2, this->rows_num - 1, this->rows_num);
}



void Statistics_window::on_reset_callback(void)
{
	#warning
//	if(m::gtk::yes_no_dialog(*this, _("Reset statistics?"), _("Are you sure want to reset statistics?")))
//		this->response(RESPONSE_RESET);
	if(m::gtk::yes_no_dialog(*this, _("Reset statistics?"), _("Are you sure want to reset statistics?")));
		if(m::gtk::yes_no_dialog(*this, _("GGG?"), _("HHH?")))
		this->response(RESPONSE_RESET);
}



void Statistics_window::run(void)
{
	try
	{
		// m::Exception
		Session_status session_status = get_daemon_proxy().get_session_status();

		Gtk::VBox* main_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		main_vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
		this->get_vbox()->pack_start(*main_vbox, false, false);

		m::gtk::vbox::add_big_header(*main_vbox, _("Statistics"), false, false, true);

		// Упаковываем таблицу во фрейм -->
		{
			Gtk::Frame* frame = Gtk::manage( new Gtk::Frame() );
			main_vbox->pack_start(*frame, false, false);

			Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
			vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
			frame->add(*vbox);

			this->table.set_row_spacings(m::gtk::VBOX_SPACING);
			vbox->pack_start(this->table, false, false);
		}
		// Упаковываем таблицу во фрейм <--

		// Переносим в таблицу все необходимые данные -->
		{
			Share_ratio share_ratio = get_share_ratio(session_status.payload_upload, session_status.payload_download);
			Share_ratio total_share_ratio = get_share_ratio(session_status.total_payload_upload, session_status.total_payload_download);

			this->attach_time(			_("Session start time"),		session_status.session_start_time);
			this->attach_time(			_("Statistics start time"),		session_status.statistics_start_time);
			this->attach_separator();
			this->attach_size(			_("Downloaded"),				session_status.download);
			this->attach_size(			_("Payload download"),			session_status.payload_download);
			this->attach_size(			_("Total download"),			session_status.total_download);
			this->attach_size(			_("Total payload download"),	session_status.total_payload_download);
			this->attach_separator();
			this->attach_size(			_("Uploaded"),					session_status.upload);
			this->attach_size(			_("Payload upload"),			session_status.payload_upload);
			this->attach_size(			_("Total upload"),				session_status.total_upload);
			this->attach_size(			_("Total payload upload"),		session_status.total_payload_upload);
			this->attach_separator();
			this->attach_share_ratio(	_("Share ratio"),				share_ratio);
			this->attach_share_ratio(	_("Total share ratio"),			total_share_ratio);
			this->attach_separator();
			this->attach_size(			_("Failed"),					session_status.failed);
			this->attach_size(			_("Total failed"),				session_status.total_failed);
			this->attach_size(			_("Redundant"),					session_status.redundant);
			this->attach_size(			_("Total redundant"),			session_status.total_redundant);
		}
		// Переносим в таблицу все необходимые данные <--

		// Добавляем кнопки -->
		{
			this->get_action_area()->property_layout_style() = Gtk::BUTTONBOX_CENTER;

			// Reset -->
				Gtk::Button* reset_button = Gtk::manage( new Gtk::Button(_("Reset"), Gtk::RESPONSE_CANCEL) );
				reset_button->set_image( *Gtk::manage( new Gtk::Image(Gtk::Stock::REFRESH, Gtk::ICON_SIZE_BUTTON) ));
				reset_button->signal_clicked().connect(sigc::mem_fun(*this, &Statistics_window::on_reset_callback));
				this->get_action_area()->add(*reset_button);
			// Reset <--

			// OK
			this->add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

			this->set_default_response(Gtk::RESPONSE_OK);
		}
		// Добавляем кнопки <--

		this->show_all_children();
		this->set_resizable(false);

		if(m::gtk::Dialog::run() == RESPONSE_RESET)
		{
			// Пользователь дал команду на сброс статистики
			// m::Exception
			get_daemon_proxy().reset_statistics();
		}
	}
	catch(m::Exception& e)
	{
		MLIB_W(EE(e));
	}
}



