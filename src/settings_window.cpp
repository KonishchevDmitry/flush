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


#include <map>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/linkbutton.h>
#include <gtkmm/notebook.h>
#include <gtkmm/stock.h>
#include <gtkmm/table.h>
#include <gtkmm/treestore.h>
#include <gtkmm/window.h>

#include <mlib/gtk/tree_view.hpp>
#include <mlib/gtk/vbox.hpp>

#include "client_settings.hpp"
#include "daemon_settings.hpp"
#include "settings_window.hpp"



// Sections_view -->
	Settings_window::Sections_view_model_columns::Sections_view_model_columns(void)
	{
		add(this->id);
		add(this->name);
	}



	Settings_window::Sections_view_columns::Sections_view_columns(const Sections_view_model_columns& model_columns)
	:
		name(_("Name"), model_columns.name)
	{
		M_GTK_TREE_VIEW_ADD_STRING_COLUMN(name)
	}
// Sections_view <--



Settings_window::Settings_window(Gtk::Window& parent_window, Client_settings* client_settings, Daemon_settings* daemon_settings)
:
	m::gtk::Dialog(parent_window, std::string(APP_NAME) + ": " + _("Preferences")),

	client_settings(*client_settings),
	daemon_settings(*daemon_settings),

	download_to_dialog(
		*this,
		_("Please select torrents download directory"),
		Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER
	),
	download_to_button(download_to_dialog),

	copy_finished_to_dialog(
		*this,
		_("Please select directory for finished downloads copying"),
		Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER
	),
	copy_finished_to_button(copy_finished_to_dialog),

	auto_delete_torrents_vbox(false, m::gtk::VBOX_SPACING)
{
	this->set_resizable(false);

	Gtk::VBox* main_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
	this->add(*main_vbox);

	Gtk::HBox* main_hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
	main_vbox->pack_start(*main_hbox, true, true);

	std::map<Section, Gtk::TreePath> sections_paths;

	// Sections view -->
	{
		Gtk::Frame* sections_frame = Gtk::manage(new Gtk::Frame());
		main_hbox->pack_start(*sections_frame, true, true);

		sections_frame->add(this->sections_view);

		this->sections_view.set_headers_visible(false);
		this->sections_view.columns.name.property_sizing() = Gtk::TREE_VIEW_COLUMN_AUTOSIZE;
		this->sections_view.get_selection()->set_mode(Gtk::SELECTION_SINGLE);

		Gtk::TreeRow row;
		Gtk::TreeRow client_row;
		Gtk::TreeRow daemon_row;
		Gtk::TreeModel::iterator iter;

		// Добавляем разделы -->
			iter = this->sections_view.model->append();
			sections_paths[Settings_window::CLIENT] = this->sections_view.model->get_path(iter);
			client_row = *iter;
			client_row[this->sections_view.model_columns.id] = Settings_window::CLIENT;
			client_row[this->sections_view.model_columns.name] = _("Client");

			iter = this->sections_view.model->append(client_row.children());
			sections_paths[Settings_window::CLIENT_MAIN] = this->sections_view.model->get_path(iter);
			row = *iter;
			row[this->sections_view.model_columns.id] = Settings_window::CLIENT_MAIN;
			row[this->sections_view.model_columns.name] = _("Main");

			iter = this->sections_view.model->append(client_row.children());
			sections_paths[Settings_window::CLIENT_GUI] = this->sections_view.model->get_path(iter);
			row = *iter;
			row[this->sections_view.model_columns.id] = Settings_window::CLIENT_GUI;
			row[this->sections_view.model_columns.name] = _("GUI");

			iter = this->sections_view.model->append();
			sections_paths[Settings_window::DAEMON] = this->sections_view.model->get_path(iter);
			daemon_row = *iter;
			daemon_row[this->sections_view.model_columns.id] = Settings_window::DAEMON;
			daemon_row[this->sections_view.model_columns.name] = _("Daemon");

			iter = this->sections_view.model->append(daemon_row.children());
			sections_paths[Settings_window::DAEMON_NETWORK] = this->sections_view.model->get_path(iter);
			row = *iter;
			row[this->sections_view.model_columns.id] = Settings_window::DAEMON_NETWORK;
			row[this->sections_view.model_columns.name] = _("Network");

			iter = this->sections_view.model->append(daemon_row.children());
			sections_paths[Settings_window::DAEMON_AUTOMATION] = this->sections_view.model->get_path(iter);
			row = *iter;
			row[this->sections_view.model_columns.id] = Settings_window::DAEMON_AUTOMATION;
			row[this->sections_view.model_columns.name] = _("Automation");
		// Добавляем раздел <--

		this->sections_view.expand_all();

		// Устанавливаем обработчик сигнала на изменение выделенной секции
		this->sections_view.get_selection()->signal_changed().connect(
			sigc::mem_fun(*this, &Settings_window::on_section_changed_callback)
		);
	}
	// Sections view <--

	this->sections_notebook.set_show_tabs(false);
	main_hbox->pack_start(this->sections_notebook, true, true);

	// Client -->
	{
		Gtk::VBox* settings_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		settings_vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
		this->sections_notebook.append_page(*settings_vbox);

		Gtk::HBox* hbox;
		Gtk::VBox* vbox;
		Gtk::LinkButton* button;

		m::gtk::vbox::add_big_header(*settings_vbox, _("Client"));

		vbox = Gtk::manage(new Gtk::VBox(false, 0));
		settings_vbox->pack_start(*vbox, false, false);

		// Main -->
			hbox = Gtk::manage(new Gtk::HBox(false, 0));
			vbox->pack_start(*hbox, false, false);

			button = Gtk::manage(new Gtk::LinkButton("Client::Main", _("Main")));
			button->signal_clicked().connect(
				sigc::bind< std::pair<Gtk::LinkButton*, Gtk::TreePath> >(
					sigc::mem_fun(*this, &Settings_window::on_change_section_callback),
					std::pair<Gtk::LinkButton*, Gtk::TreePath>(
						button,
						sections_paths[Settings_window::CLIENT_MAIN]
					)
				)
			);
			hbox->pack_start(*button, false, false);
		// Main <--

		// GUI -->
			hbox = Gtk::manage(new Gtk::HBox(false, 0));
			vbox->pack_start(*hbox, false, false);

			button = Gtk::manage(new Gtk::LinkButton("Client::GUI", _("GUI")));
			button->signal_clicked().connect(
				sigc::bind< std::pair<Gtk::LinkButton*, Gtk::TreePath> >(
					sigc::mem_fun(*this, &Settings_window::on_change_section_callback),
					std::pair<Gtk::LinkButton*, Gtk::TreePath>(
						button,
						sections_paths[Settings_window::CLIENT_GUI]
					)
				)
			);
			hbox->pack_start(*button, false, false);
		// GUI <--
	}
	// Client <--

	// Client::Main -->
	{
		Gtk::VBox* settings_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		settings_vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
		this->sections_notebook.append_page(*settings_vbox);

		m::gtk::vbox::add_big_header(*settings_vbox, _("Client::Main"));

		// show_add_torrent_dialog -->
			this->show_add_torrent_dialog.set_label(_("Show add torrent dialog when opening torrent file"));
			settings_vbox->pack_start(this->show_add_torrent_dialog, false, false);
		// show_add_torrent_dialog <--

		// start torrent on adding -->
			this->start_torrent_on_adding_check_button.set_label(_("Start torrent on adding"));
			settings_vbox->pack_start(this->start_torrent_on_adding_check_button, false, false);
		// start torrent on adding <--

		// download_to -->
			this->download_to_dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			this->download_to_dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
			this->download_to_dialog.set_default_response(Gtk::RESPONSE_OK);
			m::gtk::vbox::add_widget_with_label(*settings_vbox, _("Download to:"), this->download_to_button);
		// download_to <--

		// copy_finished_to -->
		{
			Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
			settings_vbox->pack_start(*hbox, false, false);

			this->copy_finished_to_check_button.set_label( _("Copy finished to:") ),
			this->copy_finished_to_check_button.set_active();
			this->copy_finished_to_check_button.signal_toggled().connect(sigc::mem_fun(*this, &Settings_window::on_copy_finished_to_path_toggled_callback));
			hbox->pack_start(this->copy_finished_to_check_button, false, false);

			this->copy_finished_to_dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			this->copy_finished_to_dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
			this->copy_finished_to_dialog.set_default_response(Gtk::RESPONSE_OK);
			hbox->pack_end(this->copy_finished_to_button, false, false);
		}
		// copy_finished_to <--

		// run command
		m::gtk::vbox::add_widget_with_label(*settings_vbox, _("Open command:"), this->open_command, true, true);
	}
	// Client::Main <--

	// Client::GUI -->
	{
		Gtk::VBox* settings_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		settings_vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
		this->sections_notebook.append_page(*settings_vbox);

		m::gtk::vbox::add_big_header(*settings_vbox, _("Client::GUI"));

		// Miscellaneous -->
		{
			m::gtk::vbox::add_header(*settings_vbox, _("Miscellaneous"));

			// show_tray_icon -->
				this->show_tray_icon.set_label(_("Show tray icon"));
				settings_vbox->pack_start(this->show_tray_icon, false, false);
			// show_tray_icon <--

			{
				Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox(false, m::gtk::HBOX_SPACING) );
				settings_vbox->pack_start(*hbox, false, false);

				Gtk::VBox* vbox = Gtk::manage( new Gtk::VBox(false, m::gtk::VBOX_SPACING) );
				hbox->pack_start(*vbox, false, false);

				// gui_update_interval -->
					this->gui_update_interval.set_alignment(Gtk::ALIGN_RIGHT);

					add_spin_button(
						*vbox, _("GUI update interval (ms):"), this->gui_update_interval,
						std::pair<int,int>(GUI_MIN_UPDATE_INTERVAL, INT_MAX), std::pair<int,int>(1, 500)
					);
				// gui_update_interval <--

				// max_log_lines -->
					this->max_log_lines.set_alignment(Gtk::ALIGN_RIGHT);

					add_spin_button(
						*vbox, _("Max log lines:"), this->max_log_lines,
						std::pair<int,int>(-1, INT_MAX), std::pair<int,int>(1, 10)
					);
				// max_log_lines <--
			}
		}
		// Miscellaneous <--

		m::gtk::vbox::add_space(*settings_vbox);

		// Status bar -->
		{
			m::gtk::vbox::add_header(*settings_vbox, _("Status bar"));

			Gtk::Table* table = Gtk::manage(new Gtk::Table(2, 5));
			table->set_row_spacings(m::gtk::VBOX_SPACING);
			table->set_col_spacings(m::gtk::HBOX_SPACING);
			settings_vbox->pack_start(*table, false, false);


			this->status_bar_download_speed.set_label(_("Download speed"));
			table->attach(this->status_bar_download_speed, 0, 1, 0, 1);

			this->status_bar_payload_download_speed.set_label(_("Download speed (payload)"));
			table->attach(this->status_bar_payload_download_speed, 1, 2, 0, 1);


			this->status_bar_upload_speed.set_label(_("Upload speed"));
			table->attach(this->status_bar_upload_speed, 0, 1, 1, 2);

			this->status_bar_payload_upload_speed.set_label(_("Upload speed (payload)"));
			table->attach(this->status_bar_payload_upload_speed, 1, 2, 1, 2);


			this->status_bar_download.set_label(_("Downloaded"));
			table->attach(this->status_bar_download, 0, 1, 2, 3);

			this->status_bar_payload_download.set_label(_("Downloaded (payload)"));
			table->attach(this->status_bar_payload_download, 1, 2, 2, 3);


			this->status_bar_upload.set_label(_("Uploaded"));
			table->attach(this->status_bar_upload, 0, 1, 3, 4);

			this->status_bar_payload_upload.set_label(_("Uploaded (payload)"));
			table->attach(this->status_bar_payload_upload, 1, 2, 3, 4);


			this->status_bar_failed.set_label(_("Failed"));
			table->attach(this->status_bar_failed, 0, 1, 4, 5);

			this->status_bar_redundant.set_label(_("Redundant"));
			table->attach(this->status_bar_redundant, 1, 2, 4, 5);


			this->status_bar_share_ratio.set_label(_("Share ratio"));
			table->attach(this->status_bar_share_ratio, 0, 1, 5, 6);
		}
		// Status bar <--
	}
	// Client::GUI <--

	// Daemon -->
	{
		Gtk::VBox* settings_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		settings_vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
		this->sections_notebook.append_page(*settings_vbox);

		Gtk::HBox* hbox;
		Gtk::VBox* vbox;
		Gtk::LinkButton* button;

		m::gtk::vbox::add_big_header(*settings_vbox, _("Daemon"));

		vbox = Gtk::manage(new Gtk::VBox(false, 0));
		settings_vbox->pack_start(*vbox, false, false);

		// Network -->
			hbox = Gtk::manage(new Gtk::HBox(false, 0));
			vbox->pack_start(*hbox, false, false);

			button = Gtk::manage(new Gtk::LinkButton("Daemon::Network", _("Network")));
			button->signal_clicked().connect(
				sigc::bind< std::pair<Gtk::LinkButton*, Gtk::TreePath> >(
					sigc::mem_fun(*this, &Settings_window::on_change_section_callback),
					std::pair<Gtk::LinkButton*, Gtk::TreePath>(
						button,
						sections_paths[Settings_window::DAEMON_NETWORK]
					)
				)
			);
			hbox->pack_start(*button, false, false);
		// Network <--

		// Automation -->
			hbox = Gtk::manage(new Gtk::HBox(false, 0));
			vbox->pack_start(*hbox, false, false);

			button = Gtk::manage(new Gtk::LinkButton("Daemon::Automation", _("Automation")));
			button->signal_clicked().connect(
				sigc::bind< std::pair<Gtk::LinkButton*, Gtk::TreePath> >(
					sigc::mem_fun(*this, &Settings_window::on_change_section_callback),
					std::pair<Gtk::LinkButton*, Gtk::TreePath>(
						button,
						sections_paths[Settings_window::DAEMON_AUTOMATION]
					)
				)
			);
			hbox->pack_start(*button, false, false);
		// Automation <--
	}
	// Daemon <--

	// Daemon::Network -->
	{
		Gtk::VBox* settings_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		settings_vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
		this->sections_notebook.append_page(*settings_vbox);

		m::gtk::vbox::add_big_header(*settings_vbox, _("Daemon::Network"));

		// Ports -->
		{
			m::gtk::vbox::add_header(*settings_vbox, _("Listen port range"));

			// ports range -->
			{
				Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
				settings_vbox->pack_start(*hbox, false, false);

				Gtk::Label* label = Gtk::manage(new Gtk::Label(_("From")));
				hbox->pack_start(*label, false, false);

				this->listen_port_from.set_alignment(Gtk::ALIGN_RIGHT);
				this->listen_port_from.set_range(m::PORT_MIN, m::PORT_MAX);
				this->listen_port_from.set_increments(1, 1000);
				this->listen_port_from.signal_value_changed().connect(sigc::mem_fun(*this, &Settings_window::on_listen_port_range_change_callback));
				hbox->pack_start(this->listen_port_from, false, false);

				label = Gtk::manage(new Gtk::Label(_("to")));
				hbox->pack_start(*label, false, false);

				this->listen_port_to.set_alignment(Gtk::ALIGN_RIGHT);
				this->listen_port_to.set_range(m::PORT_MIN, m::PORT_MAX);
				this->listen_port_to.set_increments(1, 1000);
				this->listen_port_to.signal_value_changed().connect(sigc::mem_fun(*this, &Settings_window::on_listen_port_range_change_callback));
				hbox->pack_start(this->listen_port_to, false, false);
			}
			// ports range <--

			// listen port
			m::gtk::vbox::add_widget_with_label(*settings_vbox, _("Currently listen port:"), this->listen_port, false);
		}
		// Ports <--

		m::gtk::vbox::add_space(*settings_vbox);

		// Extras -->
		{
			m::gtk::vbox::add_header(*settings_vbox, _("Extras"));

			Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
			settings_vbox->pack_start(*hbox, false, false);

			Gtk::Table* table = Gtk::manage(new Gtk::Table(2, 3));
			table->set_row_spacings(m::gtk::VBOX_SPACING);
			table->set_col_spacings(m::gtk::HBOX_SPACING);
			hbox->pack_start(*table, false, false);

			this->dht.set_label("DHT");
			table->attach(this->dht, 0, 1, 0, 1);

			this->lsd.set_label("LSD");
			table->attach(this->lsd, 1, 2, 0, 1);

			this->pex.set_label("Peer exchange");
			table->attach(this->pex, 2, 3, 0, 1);

			this->upnp.set_label("UPnP");
			table->attach(this->upnp, 0, 1, 1, 2);

			this->natpmp.set_label("NAT-PMP");
			table->attach(this->natpmp, 1, 2, 1, 2);

			this->smart_ban.set_label("Smart ban");
			this->smart_ban.set_tooltip_text(_("Ban peers that sends bad data with very high accuracy"));
			table->attach(this->smart_ban, 2, 3, 1, 2);
		}
		// Extras <--

		m::gtk::vbox::add_space(*settings_vbox);

		// Bandwidth -->
		{
			Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox(false, m::gtk::HBOX_SPACING) );
			settings_vbox->pack_start(*hbox, false, false);

			Gtk::VBox* vbox = Gtk::manage( new Gtk::VBox(false, m::gtk::VBOX_SPACING) );
			hbox->pack_start(*vbox, false, false);

			m::gtk::vbox::add_header(*vbox, _("Bandwidth"));

			// download_rate_limit -->
				this->download_rate_limit.set_alignment(Gtk::ALIGN_RIGHT);

				add_spin_button(
					*vbox, _("Download rate limit (KB/s):"), this->download_rate_limit,
					std::pair<int,int>(-1, INT_MAX), std::pair<int,int>(1, 1000)
				);
			// download_rate_limit <--

			// upload_rate_limit -->
				this->upload_rate_limit.set_alignment(Gtk::ALIGN_RIGHT);

				add_spin_button(
					*vbox, _("Upload rate limit (KB/s):"), this->upload_rate_limit,
					std::pair<int,int>(-1, INT_MAX), std::pair<int,int>(1, 1000)
				);
			// upload_rate_limit <--

			// max_uploads -->
				this->max_uploads.set_alignment(Gtk::ALIGN_RIGHT);

				add_spin_button(
					*vbox, _("Max uploads:"), this->max_uploads,
					std::pair<int,int>(-1, INT_MAX), std::pair<int,int>(1, 10)
				);
			// max_uploads <--

			// max_connections -->
				this->max_connections.set_alignment(Gtk::ALIGN_RIGHT);

				add_spin_button(
					*vbox, _("Max connections:"), this->max_connections,
					std::pair<int,int>(-1, INT_MAX), std::pair<int,int>(1, 10)
				);
			// max_connections <--
		}
		// Bandwidth <--
	}
	// Daemon::Network <--

	// Daemon::Automation -->
	{
		Gtk::VBox* settings_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		settings_vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
		this->sections_notebook.append_page(*settings_vbox);

		m::gtk::vbox::add_big_header(*settings_vbox, _("Daemon::Automation"));

		// Delete torrents -->
			this->auto_delete_torrents.set_label( _("Delete torrents:") ),
			this->auto_delete_torrents.set_active();
			this->auto_delete_torrents.signal_toggled().connect(sigc::mem_fun(*this, &Settings_window::on_auto_delete_torrents_toggled_callback));
			settings_vbox->pack_start(this->auto_delete_torrents, false, false);

			Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
			settings_vbox->pack_start(*hbox, false, false);

			// Табуляция шириной в 4 пробела :)
			Gtk::Label* label = Gtk::manage(new Gtk::Label("    "));
			hbox->pack_start(*label, false, false);

			hbox->pack_start(this->auto_delete_torrents_vbox, true, true);

			// Delete with data -->
				this->auto_delete_torrents_with_data.set_label( _("Delete with data") ),
				this->auto_delete_torrents_with_data.set_active();
				this->auto_delete_torrents_vbox.pack_start(this->auto_delete_torrents_with_data, false, false);
			// Delete with data <--

			{
				Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox(false, m::gtk::HBOX_SPACING) );
				this->auto_delete_torrents_vbox.pack_start(*hbox, false, false);

				Gtk::VBox* vbox = Gtk::manage( new Gtk::VBox(false, m::gtk::VBOX_SPACING) );
				hbox->pack_start(*vbox, false, false);

				// Max seeding time -->
					this->auto_delete_torrents_max_seed_time.set_alignment(Gtk::ALIGN_RIGHT);

					add_spin_button(
						*vbox, _("Max seeding time (m):"), this->auto_delete_torrents_max_seed_time,
						std::pair<int,int>(-1, INT_MAX), std::pair<int,int>(1, 60)
					);
				// Max seeding time <--

				// Max seeding torrents num -->
					this->auto_delete_torrents_max_seeds.set_alignment(Gtk::ALIGN_RIGHT);

					add_spin_button(
						*vbox, _("Max seeding torrents:"), this->auto_delete_torrents_max_seeds,
						std::pair<int,int>(-1, INT_MAX), std::pair<int,int>(1, 10)
					);
				// Max seeding torrents num <--
			}
		// Delete torrents <--
	}
	// Daemon::Automation <--

	// OK, Cancel -->
	{
		Gtk::ButtonBox* button_box = Gtk::manage(new Gtk::HButtonBox());
		button_box->set_layout(Gtk::BUTTONBOX_END);
		button_box->set_spacing(m::gtk::HBOX_SPACING);
		main_vbox->pack_start(*button_box, false, false);


		Gtk::Button* button;

		button = Gtk::manage(new Gtk::Button(Gtk::Stock::CANCEL));
		button->signal_clicked().connect(sigc::mem_fun(*this, &Settings_window::on_cancel_button_callback));
		button_box->add(*button);

		button = Gtk::manage(new Gtk::Button(Gtk::Stock::OK));
		button->signal_clicked().connect(sigc::mem_fun(*this, &Settings_window::on_ok_button_callback));
		button_box->add(*button);
	}
	// OK, Cancel <--

	// Закрытие окна
	this->signal_delete_event().connect(sigc::mem_fun(*this, &Settings_window::on_close_callback));

	this->load_settings();

	// Выделяем вкладку по умолчанию -->
		this->show_all_children(); // Иначе сигнал не будет сгенерирован
		this->sections_view.get_selection()->select(sections_paths[Settings_window::CLIENT_MAIN]);
	// Выделяем вкладку по умолчанию <--

	this->show_all();
}



void Settings_window::add_spin_button(Gtk::VBox& parent_vbox, const std::string& label_string, Gtk::SpinButton& spin_button, const std::pair<int, int>& range, const std::pair<int, int>& increments)
{
	spin_button.set_range(range.first, range.second);
	spin_button.set_increments(increments.first, increments.second);
	m::gtk::vbox::add_widget_with_label(parent_vbox, label_string, spin_button);
}



void Settings_window::close(void)
{
	if(this->download_to_dialog.is_visible())
		this->download_to_dialog.response(Gtk::RESPONSE_CANCEL);

	this->hide();
}



void Settings_window::load_settings(void)
{
	const Status_bar_settings& status_bar_settings = this->client_settings.gui.main_window.status_bar;

	// GUI -->
		this->show_tray_icon.set_active(this->client_settings.gui.show_tray_icon);
		this->gui_update_interval.set_value(this->client_settings.gui.update_interval);
		this->max_log_lines.set_value(this->client_settings.gui.max_log_lines);

		this->status_bar_download_speed.set_active(			status_bar_settings.download_speed);
		this->status_bar_payload_download_speed.set_active(	status_bar_settings.payload_download_speed);

		this->status_bar_upload_speed.set_active(			status_bar_settings.upload_speed);
		this->status_bar_payload_upload_speed.set_active(	status_bar_settings.payload_upload_speed);

		this->status_bar_download.set_active(				status_bar_settings.download);
		this->status_bar_payload_download.set_active(		status_bar_settings.payload_download);

		this->status_bar_upload.set_active(					status_bar_settings.upload);
		this->status_bar_payload_upload.set_active(			status_bar_settings.payload_upload);

		this->status_bar_share_ratio.set_active(			status_bar_settings.share_ratio);
		this->status_bar_failed.set_active(					status_bar_settings.failed);
		this->status_bar_redundant.set_active(				status_bar_settings.redundant);
	// GUI <--

	// Main -->
		this->show_add_torrent_dialog.set_active(this->client_settings.gui.show_add_torrent_dialog);

		this->start_torrent_on_adding_check_button.set_active(this->client_settings.user.start_torrent_on_adding);

		this->download_to_button.set_filename(U2L(this->client_settings.user.download_to));

		if(this->client_settings.user.copy_finished_to != "")
		{
			this->copy_finished_to_check_button.set_active();
			this->copy_finished_to_button.set_filename(U2L(this->client_settings.user.copy_finished_to));
		}
		else
			this->copy_finished_to_check_button.set_active(false);

		this->open_command.set_text(this->client_settings.user.open_command);
	// Main <--

	// daemon settings -->
		// Сначала задаем to, чтобы обработчик сигнала выставил ограничение для from
		this->listen_port_to.set_value(daemon_settings.listen_ports_range.second);
		this->listen_port_from.set_value(daemon_settings.listen_ports_range.first);
		this->listen_port.set_label(daemon_settings.listen_port < 0 ? _("none") : m::to_string(daemon_settings.listen_port));

		this->dht.set_active(daemon_settings.dht);
		this->lsd.set_active(daemon_settings.lsd);
		this->upnp.set_active(daemon_settings.upnp);
		this->natpmp.set_active(daemon_settings.natpmp);
		this->pex.set_active(daemon_settings.pex);
		this->smart_ban.set_active(daemon_settings.smart_ban);

		this->download_rate_limit.set_value(daemon_settings.download_rate_limit);
		this->upload_rate_limit.set_value(daemon_settings.upload_rate_limit);

		this->max_uploads.set_value(daemon_settings.max_uploads);
		this->max_connections.set_value(daemon_settings.max_connections);

		this->auto_delete_torrents.set_active(daemon_settings.auto_delete_torrents);
		this->auto_delete_torrents_with_data.set_active(daemon_settings.auto_delete_torrents_with_data);
		this->auto_delete_torrents_max_seed_time.set_value(daemon_settings.auto_delete_torrents_max_seed_time < 0 ? -1 : daemon_settings.auto_delete_torrents_max_seed_time / 60);
		this->auto_delete_torrents_max_seeds.set_value(daemon_settings.auto_delete_torrents_max_seeds);
	// daemon settings <--
}



void Settings_window::on_auto_delete_torrents_toggled_callback(void)
{
	this->auto_delete_torrents_vbox.set_sensitive(this->auto_delete_torrents.get_active());
}



void Settings_window::on_cancel_button_callback(void)
{
	this->close();
	this->response(Gtk::RESPONSE_CANCEL);
}



void Settings_window::on_change_section_callback(const std::pair<Gtk::LinkButton*, Gtk::TreePath>& target)
{
	g_object_set(target.first->gobj(), "visited", false, NULL);
	this->sections_view.get_selection()->select(target.second);
}



bool Settings_window::on_close_callback(GdkEventAny* event)
{
	this->close();
	this->response(Gtk::RESPONSE_CANCEL);
	return true;
}



void Settings_window::on_copy_finished_to_path_toggled_callback(void)
{
	this->copy_finished_to_button.set_sensitive(this->copy_finished_to_check_button.get_active());
}



void Settings_window::on_listen_port_range_change_callback(void)
{
	this->listen_port_from.set_range(m::PORT_MIN, this->listen_port_to.get_value());
}



void Settings_window::on_ok_button_callback(void)
{
	this->save_settings();
	this->close();
	this->response(Gtk::RESPONSE_OK);
}



void Settings_window::on_section_changed_callback(void)
{
	this->sections_notebook.set_current_page(
		this->sections_view.get_selection()->get_selected()->get_value(
			this->sections_view.model_columns.id
		)
	);
}



void Settings_window::save_settings(void)
{
	Status_bar_settings& status_bar_settings = this->client_settings.gui.main_window.status_bar;

	// GUI -->
		this->client_settings.gui.show_tray_icon	= this->show_tray_icon.get_active();
		this->client_settings.gui.update_interval	= this->gui_update_interval.get_value();
		this->client_settings.gui.max_log_lines		= this->max_log_lines.get_value();

		status_bar_settings.download_speed			= this->status_bar_download_speed.get_active();
		status_bar_settings.payload_download_speed	= this->status_bar_payload_download_speed.get_active();

		status_bar_settings.upload_speed			= this->status_bar_upload_speed.get_active();
		status_bar_settings.payload_upload_speed	= this->status_bar_payload_upload_speed.get_active();

		status_bar_settings.download				= this->status_bar_download.get_active();
		status_bar_settings.payload_download		= this->status_bar_payload_download.get_active();

		status_bar_settings.upload					= this->status_bar_upload.get_active();
		status_bar_settings.payload_upload			= this->status_bar_payload_upload.get_active();

		status_bar_settings.share_ratio				= this->status_bar_share_ratio.get_active();
		status_bar_settings.failed					= this->status_bar_failed.get_active();
		status_bar_settings.redundant				= this->status_bar_redundant.get_active();
	// GUI <--

	// Main -->
		this->client_settings.gui.show_add_torrent_dialog = this->show_add_torrent_dialog.get_active();
		this->client_settings.user.start_torrent_on_adding = this->start_torrent_on_adding_check_button.get_active();

		this->client_settings.user.download_to = L2U(this->download_to_button.get_filename());

		if(this->copy_finished_to_check_button.get_active())
			this->client_settings.user.copy_finished_to = L2U(this->copy_finished_to_button.get_filename());
		else
			this->client_settings.user.copy_finished_to = "";

		this->client_settings.user.open_command = this->open_command.get_text();
		if(m::is_empty_string(this->client_settings.user.open_command))
			this->client_settings.user.open_command = "";
	// Main <--

	// daemon settings -->
		daemon_settings.listen_ports_range = std::pair<int, int>(this->listen_port_from.get_value(), this->listen_port_to.get_value());

		daemon_settings.dht = this->dht.get_active();
		daemon_settings.lsd = this->lsd.get_active();
		daemon_settings.upnp = this->upnp.get_active();
		daemon_settings.natpmp = this->natpmp.get_active();
		daemon_settings.smart_ban = this->smart_ban.get_active();
		daemon_settings.pex = this->pex.get_active();

		daemon_settings.download_rate_limit = this->download_rate_limit.get_value();
		daemon_settings.upload_rate_limit = this->upload_rate_limit.get_value();

		daemon_settings.max_uploads = this->max_uploads.get_value();
		daemon_settings.max_connections = this->max_connections.get_value();

		daemon_settings.auto_delete_torrents = this->auto_delete_torrents.get_active();
		daemon_settings.auto_delete_torrents_with_data = this->auto_delete_torrents_with_data.get_active();
		daemon_settings.auto_delete_torrents_max_seed_time = this->auto_delete_torrents_max_seed_time.get_value() < 0 ? -1 : this->auto_delete_torrents_max_seed_time.get_value() * 60;
		daemon_settings.auto_delete_torrents_max_seeds = this->auto_delete_torrents_max_seeds.get_value();
	// daemon settings <--
}

