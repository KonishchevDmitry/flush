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


#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/separator.h>
#include <gtkmm/stock.h>

#include <mlib/gtk/misc.hpp>
#include <mlib/gtk/vbox.hpp>

#include "daemon_proxy.hpp"
#include "gui_lib.hpp"
#include "main.hpp"
#include "main_window.hpp"
#include "torrent_options_view.hpp"
#include "trackers_view.hpp"



class Torrent_options_view::Gui
{
	public:
		Gtk::CheckButton		sequential_download_check_button;

		Gtk::CheckButton		copy_when_finished_check_button;
		Gtk::Entry				copy_when_finished_to_entry;

		Trackers_view			trackers_view;
};



Torrent_options_view::Torrent_options_view(void)
:
	download_settings_revision(INIT_REVISION),
	trackers_revision(INIT_REVISION),
	gui( new Gui )
{
	this->set_border_width(m::gtk::BOX_BORDER_WIDTH);
	this->set_sensitive(false);

	Gtk::HBox* main_hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
	this->pack_start(*main_hbox, true, true);

	// Скачивание -->
	{
		//Gtk::Frame* frame = Gtk::manage( new Gtk::Frame(_("Downloading")) );
		//main_hbox->pack_start(*frame, false, false);

		Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
		main_hbox->pack_start(*vbox, false, false);
		//frame->add(*vbox);

		m::gtk::vbox::add_header(*vbox, _("Downloading"));

		// Sequential download -->
			this->gui->sequential_download_check_button.set_label(_("Sequential download"));
			this->gui->sequential_download_check_button.signal_toggled().connect(
				sigc::mem_fun(*this, &Torrent_options_view::on_sequential_download_toggled_callback)
			);
			vbox->pack_start(this->gui->sequential_download_check_button, false, false);
		// Sequential download <--

		// Copy when finished to -->
			this->gui->copy_when_finished_check_button.set_label(_("Copy when finished to:"));
			this->gui->copy_when_finished_check_button.signal_toggled().connect(
				sigc::mem_fun(*this, &Torrent_options_view::on_copy_when_finished_toggled_callback)
			);
			vbox->pack_start(this->gui->copy_when_finished_check_button, false, false);

			{
				Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::VBOX_SPACING));
				vbox->pack_start(*hbox, false, false);

				this->gui->copy_when_finished_to_entry.set_editable(false);
				hbox->pack_start(this->gui->copy_when_finished_to_entry, false, true);

				Gtk::Button* button;

				button = Gtk::manage(new Gtk::Button());
				button->set_image( *Gtk::manage( new Gtk::Image(Gtk::Stock::DIRECTORY, Gtk::ICON_SIZE_MENU) ));
				button->signal_clicked().connect(
					sigc::mem_fun(*this, &Torrent_options_view::on_select_copy_when_finished_to_callback)
				);
				hbox->pack_start(*button, false, false);
			}
		// Copy when finished to <--
	}
	// Скачивание <--

main_hbox->pack_start(*Gtk::manage(new Gtk::VSeparator), false, false);
	// Трекеры -->
	{
		//Gtk::Frame* frame = Gtk::manage( new Gtk::Frame(_("Trackers")) );
		//main_hbox->pack_start(*frame, true, true);

		Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
		main_hbox->pack_start(*vbox, true, true);
		//frame->add(*vbox);

		m::gtk::vbox::add_header(*vbox, _("Trackers"));

		vbox->pack_start(this->gui->trackers_view, true, true);
		//frame->add(this->gui->trackers_view);
	}
	// Трекеры <--

	this->gui->trackers_view.signal_changed().connect(sigc::mem_fun(
		*this, &Torrent_options_view::on_trackers_changed_callback
	));
}



Torrent_options_view::~Torrent_options_view(void)
{
	delete this->gui;
}



void Torrent_options_view::clear(void)
{
	this->gui_updating = true;

		this->torrent_id = Torrent_id();

		this->set_sensitive(false);

		this->gui->sequential_download_check_button.set_active(false);
		this->gui->copy_when_finished_check_button.set_active(false);
		this->gui->copy_when_finished_to_entry.set_text("");
		this->gui->trackers_view.set( std::vector<std::string>() );

	this->gui_updating = false;
}



void Torrent_options_view::custom_update(bool force)
{
	if(this->torrent_id)
	{
		Download_settings download_settings;
		std::vector<std::string> trackers;

		if(force)
		{
			this->download_settings_revision = INIT_REVISION;
			this->trackers_revision = INIT_REVISION;
		}

		this->set_sensitive(true);

		try
		{
			if(get_daemon_proxy().get_torrent_new_download_settings(this->torrent_id, &this->download_settings_revision, &download_settings))
			{
				this->gui_updating = true;

					this->gui->sequential_download_check_button.set_active(
						download_settings.sequential_download
					);

					this->gui->copy_when_finished_check_button.set_active(
						download_settings.copy_when_finished
					);

					this->gui->copy_when_finished_to_entry.set_text(
						download_settings.copy_when_finished_to
					);

				this->gui_updating = false;
			}

			if(get_daemon_proxy().get_torrent_new_trackers(this->torrent_id, &this->trackers_revision, &trackers))
			{
				this->gui_updating = true;
					this->gui->trackers_view.set(trackers);
				this->gui_updating = false;
			}
		}
		catch(m::Exception& e)
		{
			MLIB_W(__("Error while getting torrent download settings. %1", EE(e)));
			this->clear();
		}
	}
	else
	{
		if(force)
			this->clear();
	}
}



void Torrent_options_view::on_copy_when_finished_toggled_callback(void)
{
	if(this->gui_updating)
		return;

	try
	{
		get_daemon_proxy().set_copy_when_finished(
			this->torrent_id, this->gui->copy_when_finished_check_button.get_active(),
			this->gui->copy_when_finished_to_entry.get_text()
		);
	}
	catch(m::Exception& e)
	{
		MLIB_W(__("Error while setting torrent download settings. %1", EE(e)));
	}

	// Чтобы в случае ошибки отобразить то, что действительно хранится в
	// настройках демона.
	this->custom_update(true);
}



void Torrent_options_view::on_select_copy_when_finished_to_callback(void)
{
	Gtk::FileChooserDialog dialog(
		get_main_window(), format_window_title(_("Please choose a directory for torrent finished files copying")),
		Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER
	);

	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
	dialog.set_default_response(Gtk::RESPONSE_OK);

	if(this->gui->copy_when_finished_to_entry.get_text() != "")
		dialog.set_filename(U2L(this->gui->copy_when_finished_to_entry.get_text()));

	if(dialog.run() == Gtk::RESPONSE_OK)
	{
		this->gui->copy_when_finished_to_entry.set_text(L2U(dialog.get_filename()));
		this->on_copy_when_finished_toggled_callback();
	}
}



void Torrent_options_view::on_sequential_download_toggled_callback(void)
{
	if(this->gui_updating)
		return;

	try
	{
		get_daemon_proxy().set_sequential_download(
			this->torrent_id, this->gui->sequential_download_check_button.get_active()
		);
	}
	catch(m::Exception& e)
	{
		MLIB_W(__("Error while setting torrent download settings. %1", EE(e)));
	}

	// Чтобы в случае ошибки отобразить то, что действительно хранится в
	// настройках демона.
	this->custom_update(true);
}



void Torrent_options_view::on_trackers_changed_callback(void)
{
	try
	{
		get_daemon_proxy().set_torrent_trackers(this->torrent_id, this->gui->trackers_view.get());
	}
	catch(m::Exception& e)
	{
		MLIB_W(__("Can't set torrent trackers. %1", EE(e)));
	}

	// Чтобы в случае ошибки отобразить то, что действительно хранится в
	// настройках демона.
	this->custom_update(true);
}



void Torrent_options_view::update(const Torrent_id& torrent_id)
{
	if(torrent_id)
	{
		if(this->torrent_id != torrent_id)
		{
			this->torrent_id = torrent_id;
			this->custom_update(true);
		}
		else
			this->custom_update();
	}
	else
	{
		if(this->torrent_id)
			this->clear();
	}
}

