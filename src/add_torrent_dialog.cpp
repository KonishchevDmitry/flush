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
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>

#include <mlib/gtk/vbox.hpp>
#include <mlib/libtorrent.hpp>
#include <mlib/string.hpp>

#include "add_torrent_dialog.hpp"
#include "common.hpp"
#include "application.hpp"
#include "client_settings.hpp"
#include "gui_lib.hpp"
#include "main.hpp"
#include "torrent_files_view.hpp"



Add_torrent_dialog::Add_torrent_dialog(Gtk::Window& parent_window, const std::string& torrent_path, const std::string& torrent_encoding)
:
	m::gtk::Window(
		parent_window, format_window_title(_("Adding torrent")),
		get_client_settings().gui.add_torrent_dialog.window,
		640, 480
	),

	download_to_dialog(
		*this,
		format_window_title(_("Please select torrent download directory")),
		Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER
	),
	download_to_button(download_to_dialog),

	copy_when_finished_to_dialog(
		*this,
		format_window_title(_("Please select directory for finished downloads copying")),
		Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER
	),
	copy_when_finished_to_button(copy_when_finished_to_dialog),

	torrent_path(torrent_path),

	// Генерирует m::Exception
	torrent_info(m::lt::get_torrent_metadata(torrent_path, torrent_encoding).info),

	torrent_encoding(torrent_encoding),

	// Генерирует m::Exception
	torrent_files_view(torrent_info, get_client_settings().gui.add_torrent_dialog.torrent_files_view)
{
	Client_settings& client_settings = get_client_settings();

	Gtk::VBox* main_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
	this->add(*main_vbox);

	// Torrent -->
	{
		m::gtk::vbox::add_header(*main_vbox, _("Torrent"));

		Gtk::Entry* entry = Gtk::manage(new Gtk::Entry());
		entry->set_editable(false);
		entry->set_text(this->torrent_path);
		m::gtk::vbox::add_widget_with_label(*main_vbox, _("Path:"), *entry, true, true);

		this->torrent_name_entry.set_text(this->torrent_info.name());
		m::gtk::vbox::add_widget_with_label(*main_vbox, _("Name:"), this->torrent_name_entry, true, true);

		this->start_torrent_check_button.set_label(_("Start torrent"));
		this->start_torrent_check_button.set_active(client_settings.user.start_torrent_on_adding);
		main_vbox->pack_start(this->start_torrent_check_button, false, false);
	}
	// Torrent <--

	m::gtk::vbox::add_space(*main_vbox);

	// Paths -->
	{
		m::gtk::vbox::add_header(*main_vbox, _("Paths"));

		// downloads path -->
			this->download_to_dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			this->download_to_dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
			this->download_to_dialog.set_default_response(Gtk::RESPONSE_OK);
			this->download_to_button.set_filename(U2L(client_settings.user.download_to));
			m::gtk::vbox::add_widget_with_label(*main_vbox, _("Downloads path:"), this->download_to_button, true, true);
		// downloads path <--

		// copy_when_finished_to -->
			this->copy_when_finished_to_check_button.set_label( _("Copy when finished to:") ),
			this->copy_when_finished_to_check_button.set_active();
			this->copy_when_finished_to_check_button.signal_toggled().connect(sigc::mem_fun(*this, &Add_torrent_dialog::on_copy_when_finished_to_toggled_callback));

			this->copy_when_finished_to_dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			this->copy_when_finished_to_dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
			this->copy_when_finished_to_dialog.set_default_response(Gtk::RESPONSE_OK);

			if(client_settings.user.copy_finished_to != "")
			{
				this->copy_when_finished_to_check_button.set_active();
				this->copy_when_finished_to_button.set_filename(U2L(client_settings.user.copy_finished_to));
			}
			else
				this->copy_when_finished_to_check_button.set_active(false);

			m::gtk::vbox::add_widget_with_labeled_widget(
				*main_vbox, this->copy_when_finished_to_check_button,
				this->copy_when_finished_to_button, true, true
			);
		// copy_when_finished_to <--
	}
	// Paths <--

	m::gtk::vbox::add_space(*main_vbox);

	// Torrent_files_view -->
	{
		Gtk::ScrolledWindow* torrent_files_view_scrolled_window = Gtk::manage(new Gtk::ScrolledWindow());
		torrent_files_view_scrolled_window->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		torrent_files_view_scrolled_window->add(this->torrent_files_view);
		main_vbox->pack_start(*torrent_files_view_scrolled_window, true, true);
	}
	// Torrent_files_view <--

	// OK, Cancel -->
	{
		Gtk::ButtonBox* button_box = Gtk::manage(new Gtk::HButtonBox());
		button_box->set_layout(Gtk::BUTTONBOX_END);
		button_box->set_spacing(m::gtk::HBOX_SPACING);
		main_vbox->pack_start(*button_box, false, false);


		Gtk::Button* button;

		button = Gtk::manage(new Gtk::Button(Gtk::Stock::CANCEL));
		button->signal_clicked().connect(sigc::mem_fun(*this, &Add_torrent_dialog::on_cancel_button_callback));
		button_box->add(*button);

		button = Gtk::manage(new Gtk::Button(Gtk::Stock::OK));
		button->signal_clicked().connect(sigc::mem_fun(*this, &Add_torrent_dialog::on_ok_button_callback));
		button_box->add(*button);
	}
	// OK, Cancel <--

	// Закрытие окна
	this->signal_delete_event().connect(sigc::mem_fun(*this, &Add_torrent_dialog::on_close_callback));

	this->show_all();
}



void Add_torrent_dialog::close(void)
{
	this->save_settings(get_client_settings().gui.add_torrent_dialog);
	delete this;
}



void Add_torrent_dialog::on_cancel_button_callback(void)
{
	this->close();
}



bool Add_torrent_dialog::on_close_callback(GdkEventAny* event)
{
	this->close();
	return true;
}



void Add_torrent_dialog::on_copy_when_finished_to_toggled_callback(void)
{
	this->copy_when_finished_to_button.set_sensitive( this->copy_when_finished_to_check_button.get_active() );
}



void Add_torrent_dialog::on_ok_button_callback(void)
{
	if(m::is_empty_string(this->torrent_name_entry.get_text()))
	{
		show_warning_message(
			*this, _("Invalid torrent name"),
			_("You have entered invalid torrent name. Please enter non-empty torrent name.")
		);

		return;
	}

	try
	{
		get_application().add_torrent(
			this->torrent_path,
			New_torrent_settings(
				this->torrent_name_entry.get_text(),
				this->start_torrent_check_button.get_active(),
				L2U(this->download_to_button.get_filename()),
				(
					this->copy_when_finished_to_check_button.get_active()
					?
						L2U(this->copy_when_finished_to_button.get_filename())
					:
						""
				),
				torrent_encoding,
				this->torrent_files_view.get_files_settings()
			)
		);
	}
	catch(m::Exception& e)
	{
		MLIB_W(
			_("Opening torrent failed"),
			__(
				"Opening torrent '%1' failed. %2",
				this->torrent_path, EE(e)
			)
		);
	}

	this->close();
}



void Add_torrent_dialog::save_settings(Add_torrent_dialog_settings& settings)
{
	m::gtk::Window::save_settings(settings.window);
	this->torrent_files_view.save_settings(settings.torrent_files_view);
}

