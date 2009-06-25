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

#include <mlib/gtk/glade.hpp>
#include <mlib/gtk/tree_view.hpp>
#include <mlib/gtk/vbox.hpp>

#include "client_settings.hpp"
#include "daemon_settings.hpp"
#include "gui_lib.hpp"
#include "ip_filter.hpp"
#include "settings_window.hpp"



// Private -->
namespace Settings_window_aux
{
	class Private
	{
		public:
			struct Network_misc
			{
				Gtk::RadioButton*	random_port;
				Gtk::RadioButton*	custom_port;
				Gtk::Container*		custom_port_box;
				Gtk::SpinButton*	from_port;
				Gtk::SpinButton*	to_port;
				Gtk::Label*			current_port;

				Gtk::CheckButton*	dht;
				Gtk::CheckButton*	upnp;
				Gtk::CheckButton*	lsd;
				Gtk::CheckButton*	natpmp;
				Gtk::CheckButton*	pex;
				Gtk::CheckButton*	smart_ban;

				Gtk::SpinButton*	download_rate_limit;
				Gtk::SpinButton*	upload_rate_limit;
				Gtk::SpinButton*	max_uploads;
				Gtk::SpinButton*	max_connections;

				Gtk::CheckButton*	use_max_announce_interval;
				Gtk::SpinButton*	max_announce_interval;
			};

			struct Network
			{
				Ip_filter*		ip_filter;
				Network_misc	misc;
			};

			struct Auto_load
			{
				Gtk::CheckButton*		is;
				Gtk::Container*			container;

				Gtk::FileChooserButton*	from;
				Gtk::FileChooserButton*	to;

				Gtk::CheckButton*		copy;
				Gtk::FileChooserButton*	copy_to;

				Gtk::CheckButton*		delete_loaded;
			};

			struct Auto_clean
			{
				Gtk::SpinButton*		max_seeding_time;
				Auto_clean_type			max_seeding_time_type;
				Gtk::Button*			max_seeding_time_type_button;
				Gtk::Label*				max_seeding_time_type_label;

				Gtk::SpinButton*		max_ratio;
				Auto_clean_type			max_ratio_type;
				Gtk::Button*			max_ratio_type_button;
				Gtk::Label*				max_ratio_type_label;

				Gtk::SpinButton*		max_seeding_torrents;
				Auto_clean_type			max_seeding_torrents_type;
				Gtk::Button*			max_seeding_torrents_type_button;
				Gtk::Label*				max_seeding_torrents_type_label;
			};

			struct Automation
			{
				Auto_load	load;
				Auto_clean	clean;
			};

			struct Daemon
			{
				Network		network;
				Automation	automation;
			};


		public:
			Daemon			daemon;


		public:
			/// Обработчик сигнала на изменение диапазона прослушиваемых портов.
			void	on_listen_port_range_changed_cb(void);

			/// Обработчик сигнала на переключение флажка "случайный
			/// порт"-"задаваемый пользователем порт".
			void	on_random_port_toggled_cb(void);
	};



	void Private::on_listen_port_range_changed_cb(void)
	{
		Network_misc& misc = this->daemon.network.misc;
		misc.from_port->set_range(m::PORT_MIN, misc.to_port->get_value());
	}



	void Private::on_random_port_toggled_cb(void)
	{
		Network_misc& misc = this->daemon.network.misc;
		misc.custom_port_box->set_sensitive(!misc.random_port->get_active());
	}
}
// Private <--



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
		M_GTK_TREE_VIEW_ADD_STRING_COLUMN(name, _("Name"))
	}
// Sections_view <--



Settings_window::Settings_window(Gtk::Window& parent_window, Client_settings* client_settings, Daemon_settings* daemon_settings)
:
	m::gtk::Dialog(parent_window, format_window_title(_("Preferences"))),

	priv(new Private),

	client_settings(*client_settings),
	daemon_settings(*daemon_settings),

	download_to_dialog(
		*this, format_window_title(_("Please select torrents download directory")),
		Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER
	),
	download_to_button(download_to_dialog),

	copy_finished_to_dialog(
		*this, format_window_title(_("Please select directory for finished downloads copying")),
		Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER
	),
	copy_finished_to_button(copy_finished_to_dialog)
{
	const int tabs_border_width = m::gtk::BOX_BORDER_WIDTH * 3;

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
			sections_paths[CLIENT] = this->sections_view.model->get_path(iter);
			client_row = *iter;
			client_row[this->sections_view.model_columns.id] = CLIENT;
			client_row[this->sections_view.model_columns.name] = _("Client");

			iter = this->sections_view.model->append(client_row.children());
			sections_paths[CLIENT_MAIN] = this->sections_view.model->get_path(iter);
			row = *iter;
			row[this->sections_view.model_columns.id] = CLIENT_MAIN;
			row[this->sections_view.model_columns.name] = _("Main");

			iter = this->sections_view.model->append(client_row.children());
			sections_paths[CLIENT_GUI] = this->sections_view.model->get_path(iter);
			row = *iter;
			row[this->sections_view.model_columns.id] = CLIENT_GUI;
			row[this->sections_view.model_columns.name] = _("GUI");

			iter = this->sections_view.model->append();
			sections_paths[DAEMON] = this->sections_view.model->get_path(iter);
			daemon_row = *iter;
			daemon_row[this->sections_view.model_columns.id] = DAEMON;
			daemon_row[this->sections_view.model_columns.name] = _("Daemon");

			{
				Gtk::TreeRow network_row;

				iter = this->sections_view.model->append(daemon_row.children());
				sections_paths[DAEMON_NETWORK] = this->sections_view.model->get_path(iter);
				network_row = *iter;
				network_row[this->sections_view.model_columns.id] = DAEMON_NETWORK;
				network_row[this->sections_view.model_columns.name] = _("Network");

				iter = this->sections_view.model->append(network_row.children());
				sections_paths[DAEMON_NETWORK_MISC] = this->sections_view.model->get_path(iter);
				row = *iter;
				row[this->sections_view.model_columns.id] = DAEMON_NETWORK_MISC;
				row[this->sections_view.model_columns.name] = _("Misc");

				iter = this->sections_view.model->append(network_row.children());
				sections_paths[DAEMON_NETWORK_IP_FILTER] = this->sections_view.model->get_path(iter);
				row = *iter;
				row[this->sections_view.model_columns.id] = DAEMON_NETWORK_IP_FILTER;
				row[this->sections_view.model_columns.name] = _("IP filter");
			}

			iter = this->sections_view.model->append(daemon_row.children());
			sections_paths[DAEMON_AUTOMATION] = this->sections_view.model->get_path(iter);
			row = *iter;
			row[this->sections_view.model_columns.id] = DAEMON_AUTOMATION;
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
		settings_vbox->set_border_width(tabs_border_width);
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
						sections_paths[CLIENT_MAIN]
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
						sections_paths[CLIENT_GUI]
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
		settings_vbox->set_border_width(tabs_border_width);
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

		m::gtk::vbox::add_space(*settings_vbox);

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

		m::gtk::vbox::add_space(*settings_vbox);

		// run command
		m::gtk::vbox::add_widget_with_label(*settings_vbox, _("Open command:"), this->open_command, true, true);
	}
	// Client::Main <--


	// Client::GUI -->
	{
		Gtk::VBox* settings_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		settings_vbox->set_border_width(tabs_border_width);
		this->sections_notebook.append_page(*settings_vbox);

		m::gtk::vbox::add_big_header(*settings_vbox, _("Client::GUI"));

		// Miscellaneous -->
		{
			m::gtk::vbox::add_header(*settings_vbox, _("Miscellaneous"));

			// show_speed_in_window_title -->
				this->show_speed_in_window_title.set_label(_("Show speed in window title"));
				settings_vbox->pack_start(this->show_speed_in_window_title, false, false);
			// show_speed_in_window_title <--

			// show_zero_values -->
				this->show_zero_values.set_label(_("Show zero values in torrents info columns"));
				settings_vbox->pack_start(this->show_zero_values, false, false);
			// show_zero_values <--

			{
				Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox(false, m::gtk::HBOX_SPACING) );
				settings_vbox->pack_start(*hbox, false, false);

				Gtk::VBox* vbox = Gtk::manage( new Gtk::VBox(false, m::gtk::VBOX_SPACING) );
				hbox->pack_start(*vbox, false, false);

				// gui_update_interval -->
					add_spin_button(
						*vbox, _("GUI update interval (ms):"), this->gui_update_interval,
						std::pair<int,int>(GUI_MIN_UPDATE_INTERVAL, INT_MAX), std::pair<double,double>(1, 500)
					);
				// gui_update_interval <--

				// max_log_lines -->
					add_spin_button(
						*vbox, _("Max log lines:"), this->max_log_lines,
						std::pair<int,int>(-1, INT_MAX), std::pair<double,double>(1, 10)
					);
				// max_log_lines <--
			}

			// Tray -->
			{
				// show_tray_icon -->
					this->show_tray_icon.set_label(_("Show tray icon"));
					this->show_tray_icon.set_active();
					settings_vbox->pack_start(this->show_tray_icon, false, false);

					this->show_tray_icon.signal_toggled().connect(sigc::mem_fun(
						*this, &Settings_window::on_show_tray_icon_toggled_callback
					));
				// show_tray_icon <--


				Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
				settings_vbox->pack_start(*hbox, false, false);

				// Табуляция шириной в 4 пробела :)
				Gtk::Label* label = Gtk::manage(new Gtk::Label("    "));
				hbox->pack_start(*label, false, false);

				hbox->pack_start(this->tray_vbox, false, false);

				// Hide main window to tray at startup -->
					this->hide_app_to_tray_at_startup.set_label(_("Hide main window to tray at startup")),
					this->tray_vbox.pack_start(this->hide_app_to_tray_at_startup);
				// Hide main window to tray at startup <--

				{
					Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
					this->tray_vbox.pack_start(*hbox, false, false);

					// Minimize to tray -->
						this->minimize_to_tray.set_label(_("Minimize to tray")),
						hbox->pack_start(this->minimize_to_tray);
					// Minimize to tray <--

					// Close to tray -->
						this->close_to_tray.set_label(_("Close to tray")),
						hbox->pack_start(this->close_to_tray);
					// Close to tray <--
				}
			}
			// Tray <--
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
		settings_vbox->set_border_width(tabs_border_width);
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
						sections_paths[DAEMON_NETWORK]
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
						sections_paths[DAEMON_AUTOMATION]
					)
				)
			);
			hbox->pack_start(*button, false, false);
		// Automation <--
	}
	// Daemon <--


	// Daemon::Network -->
	{
		Glib::RefPtr<Gnome::Glade::Xml> glade = MLIB_GLADE_CREATE(
			std::string(APP_UI_PATH) + "/preferences.daemon.network.glade",
			"daemon_network_settings"
		);

		Gtk::VBox* settings_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		settings_vbox->set_border_width(tabs_border_width);
		this->sections_notebook.append_page(
			*MLIB_GLADE_GET_WIDGET(glade, "daemon_network_settings")
		);

		{
			Gtk::LinkButton* button;

			MLIB_GLADE_GET_WIDGET(glade, "daemon_network_misc_link", button);
			button->signal_clicked().connect(
				sigc::bind< std::pair<Gtk::LinkButton*, Gtk::TreePath> >(
					sigc::mem_fun(*this, &Settings_window::on_change_section_callback),
					std::pair<Gtk::LinkButton*, Gtk::TreePath>(
						button, sections_paths[DAEMON_NETWORK_MISC]
					)
				)
			);

			MLIB_GLADE_GET_WIDGET(glade, "daemon_network_ip_filter_link", button);
			button->signal_clicked().connect(
				sigc::bind< std::pair<Gtk::LinkButton*, Gtk::TreePath> >(
					sigc::mem_fun(*this, &Settings_window::on_change_section_callback),
					std::pair<Gtk::LinkButton*, Gtk::TreePath>(
						button, sections_paths[DAEMON_NETWORK_IP_FILTER]
					)
				)
			);
		}
	}
	// Daemon::Network <--


	// Daemon::Network::Misc -->
	{
		Private::Network_misc& misc = priv->daemon.network.misc;

		Glib::RefPtr<Gnome::Glade::Xml> glade = MLIB_GLADE_CREATE(
			std::string(APP_UI_PATH) + "/preferences.daemon.network.misc.glade",
			"network_misc_settings"
		);

		this->sections_notebook.append_page(
			*MLIB_GLADE_GET_WIDGET(glade, "network_misc_settings")
		);

		MLIB_GLADE_GET_WIDGET(glade, "random_port", misc.random_port);
		MLIB_GLADE_GET_WIDGET(glade, "custom_port", misc.custom_port);
		MLIB_GLADE_GET_WIDGET(glade, "custom_port_box", misc.custom_port_box);
		MLIB_GLADE_GET_WIDGET(glade, "from_port", misc.from_port);
		MLIB_GLADE_GET_WIDGET(glade, "to_port", misc.to_port);
		MLIB_GLADE_GET_WIDGET(glade, "current_port", misc.current_port);

		MLIB_GLADE_GET_WIDGET(glade, "dht", misc.dht);
		MLIB_GLADE_GET_WIDGET(glade, "upnp", misc.upnp);
		MLIB_GLADE_GET_WIDGET(glade, "lsd", misc.lsd);
		MLIB_GLADE_GET_WIDGET(glade, "natpmp", misc.natpmp);
		MLIB_GLADE_GET_WIDGET(glade, "pex", misc.pex);
		MLIB_GLADE_GET_WIDGET(glade, "smart_ban", misc.smart_ban);

		MLIB_GLADE_GET_WIDGET(glade, "download_rate_limit", misc.download_rate_limit);
		MLIB_GLADE_GET_WIDGET(glade, "upload_rate_limit", misc.upload_rate_limit);
		MLIB_GLADE_GET_WIDGET(glade, "max_uploads", misc.max_uploads);
		MLIB_GLADE_GET_WIDGET(glade, "max_connections", misc.max_connections);

		MLIB_GLADE_GET_WIDGET(glade, "use_max_announce_interval", misc.use_max_announce_interval);
		MLIB_GLADE_GET_WIDGET(glade, "max_announce_interval", misc.max_announce_interval);

		misc.random_port->signal_toggled().connect(
			sigc::mem_fun(*priv, &Private::on_random_port_toggled_cb));
		misc.from_port->signal_value_changed().connect(
			sigc::mem_fun(*priv, &Private::on_listen_port_range_changed_cb));
		misc.to_port->signal_value_changed().connect(
			sigc::mem_fun(*priv, &Private::on_listen_port_range_changed_cb));
	}
	// Daemon::Network::Misc <--


	// Daemon::IP filter -->
	{
		Glib::RefPtr<Gnome::Glade::Xml> glade = MLIB_GLADE_CREATE(
			std::string(APP_UI_PATH) + "/preferences.daemon.network.ip_filter.glade",
			"ip_filter_settings"
		);

		this->sections_notebook.append_page(
			*MLIB_GLADE_GET_WIDGET(glade, "ip_filter_settings")
		);
		MLIB_GLADE_GET_WIDGET_DERIVED(glade, "ip_filter", priv->daemon.network.ip_filter);
	}
	// Daemon::IP filter <--


	// Daemon::Automation -->
	{
		Private::Automation& automation = priv->daemon.automation;

		Glib::RefPtr<Gnome::Glade::Xml> glade = MLIB_GLADE_CREATE(
			std::string(APP_UI_PATH) + "/preferences.daemon.automation.glade",
			"automation_settings"
		);

		this->sections_notebook.append_page(
			*MLIB_GLADE_GET_WIDGET(glade, "automation_settings")
		);


		// Auto load -->
		{
			Private::Auto_load& load = automation.load;

			MLIB_GLADE_GET_WIDGET(glade, "auto_load", load.is);
			load.is->signal_toggled().connect(sigc::mem_fun(
				*this, &Settings_window::on_auto_load_torrents_toggled_callback
			));

			MLIB_GLADE_GET_WIDGET(glade, "auto_load_container", load.container);

			MLIB_GLADE_GET_WIDGET(glade, "auto_load_from", load.from);
			MLIB_GLADE_GET_WIDGET(glade, "auto_load_to", load.to);

			MLIB_GLADE_GET_WIDGET(glade, "auto_load_copy", load.copy);
			load.copy->signal_toggled().connect(sigc::mem_fun(
				*this, &Settings_window::on_auto_load_torrents_copy_to_toggled_callback
			));
			MLIB_GLADE_GET_WIDGET(glade, "auto_load_copy_to", load.copy_to);

			MLIB_GLADE_GET_WIDGET(glade, "auto_load_delete_loaded", load.delete_loaded);
		}
		// Auto load <--


		// Auto clean -->
		{
			Private::Auto_clean& clean = automation.clean;

			MLIB_GLADE_GET_WIDGET(glade, "auto_clean_max_seeding_time_type", clean.max_seeding_time_type_button);
			MLIB_GLADE_GET_WIDGET(glade, "auto_clean_max_seeding_time_label", clean.max_seeding_time_type_label);
			MLIB_GLADE_GET_WIDGET(glade, "auto_clean_max_seeding_time", clean.max_seeding_time);
			clean.max_seeding_time_type_button->signal_clicked().connect(sigc::mem_fun(
				*this, &Settings_window::on_auto_clean_max_seeding_time_clicked_cb
			));

			MLIB_GLADE_GET_WIDGET(glade, "auto_clean_max_ratio_type", clean.max_ratio_type_button);
			MLIB_GLADE_GET_WIDGET(glade, "auto_clean_max_ratio_label", clean.max_ratio_type_label);
			MLIB_GLADE_GET_WIDGET(glade, "auto_clean_max_ratio", clean.max_ratio);
			clean.max_ratio_type_button->signal_clicked().connect(sigc::mem_fun(
				*this, &Settings_window::on_auto_clean_max_ratio_clicked_cb
			));

			MLIB_GLADE_GET_WIDGET(glade, "auto_clean_max_seeding_torrents_type", clean.max_seeding_torrents_type_button);
			MLIB_GLADE_GET_WIDGET(glade, "auto_clean_max_seeding_torrents_label", clean.max_seeding_torrents_type_label);
			MLIB_GLADE_GET_WIDGET(glade, "auto_clean_max_seeding_torrents", clean.max_seeding_torrents);
			clean.max_seeding_torrents_type_button->signal_clicked().connect(sigc::mem_fun(
				*this, &Settings_window::on_auto_clean_max_seeding_torrents_clicked_cb
			));
		}
		// Auto clean <--
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
		this->sections_view.get_selection()->select(sections_paths[CLIENT_MAIN]);
	// Выделяем вкладку по умолчанию <--

	this->show_all();
}



Settings_window::~Settings_window(void)
{
	delete this->priv;
}



void Settings_window::add_spin_button(Gtk::VBox& parent_vbox, const std::string& label_string, Gtk::SpinButton& spin_button, const std::pair<int, int>& range, const std::pair<double, double>& increments)
{
	spin_button.set_alignment(Gtk::ALIGN_RIGHT);
	spin_button.set_range(range.first, range.second);
	spin_button.set_increments(increments.first, increments.second);
	m::gtk::vbox::add_widget_with_label(parent_vbox, label_string, spin_button);
}



void Settings_window::auto_clean_widgets_update_for(const Auto_clean_type& type, Gtk::Button* type_button, Gtk::Label* type_label, Gtk::Widget* widget)
{
	type_button->set_image(
		*Gtk::manage(new Gtk::Image( type.to_stock_id(), Gtk::ICON_SIZE_BUTTON ))
	);
	type_label->set_sensitive(type);
	widget->set_sensitive(type);
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
		this->show_speed_in_window_title.set_active(this->client_settings.gui.show_speed_in_window_title);
		this->show_zero_values.set_active(this->client_settings.gui.show_zero_values);

		this->gui_update_interval.set_value(this->client_settings.gui.update_interval);
		this->max_log_lines.set_value(this->client_settings.gui.max_log_lines);

		this->show_tray_icon.set_active(this->client_settings.gui.show_tray_icon);
		this->hide_app_to_tray_at_startup.set_active(this->client_settings.gui.hide_app_to_tray_at_startup);
		this->minimize_to_tray.set_active(this->client_settings.gui.minimize_to_tray);
		this->close_to_tray.set_active(this->client_settings.gui.close_to_tray);

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

		this->download_to_button.set_current_folder(U2L(this->client_settings.user.download_to));

		if(this->client_settings.user.copy_finished_to != "")
		{
			this->copy_finished_to_check_button.set_active();
			this->copy_finished_to_button.set_current_folder(U2L(this->client_settings.user.copy_finished_to));
		}
		else
			this->copy_finished_to_check_button.set_active(false);

		this->open_command.set_text(this->client_settings.user.open_command);
	// Main <--

	// daemon settings -->
	{
		Daemon_settings& settings = this->daemon_settings;

		// Network -->
			// Misc -->
			{
				Private::Network_misc& misc = priv->daemon.network.misc;

				misc.random_port	->set_active(settings.listen_random_port);
				// Сначала задаем to, чтобы обработчик сигнала выставил ограничение для from
				misc.to_port		->set_value(settings.listen_ports_range.second);
				misc.from_port		->set_value(settings.listen_ports_range.first);
				misc.custom_port	->set_active(!settings.listen_random_port);

				misc.current_port->set_label(
					settings.listen_port < m::PORT_MIN ? _("none") : m::to_string(settings.listen_port));

				misc.dht		->set_active(settings.dht);
				misc.lsd		->set_active(settings.lsd);
				misc.upnp		->set_active(settings.upnp);
				misc.natpmp		->set_active(settings.natpmp);
				misc.pex		->set_active(settings.pex);
				misc.smart_ban	->set_active(settings.smart_ban);

				misc.download_rate_limit->set_value(settings.download_rate_limit);
				misc.upload_rate_limit	->set_value(settings.upload_rate_limit);
				misc.max_uploads		->set_value(settings.max_uploads);
				misc.max_connections	->set_value(settings.max_connections);

				misc.use_max_announce_interval	->set_active(settings.use_max_announce_interval);
				misc.max_announce_interval		->set_value(settings.max_announce_interval / 60);
			}
			// Misc <--

			priv->daemon.network.ip_filter->set(settings.ip_filter);
		// Network <--

		// Automation -->
		{
			Private::Automation& automation = priv->daemon.automation;

			// Auto load -->
			{
				Daemon_settings::Torrents_auto_load& auto_load = daemon_settings.torrents_auto_load;

				automation.load.is->set_active(auto_load.is);
				automation.load.from->set_current_folder(U2L(auto_load.from));
				automation.load.to->set_current_folder(U2L(auto_load.to));
				automation.load.copy->set_active(auto_load.copy);
				automation.load.copy_to->set_current_folder(U2L(auto_load.copy_to));
				automation.load.delete_loaded->set_active(auto_load.delete_loaded);
			}
			// Auto load <--

			// Auto clean -->
			{
				Daemon_settings::Auto_clean& daemon_clean = daemon_settings.torrents_auto_clean;
				Private::Auto_clean& clean = automation.clean;

				clean.max_seeding_time_type = daemon_clean.max_seeding_time_type;
				clean.max_seeding_time->set_value(daemon_clean.max_seeding_time / 60);
				this->auto_clean_widgets_update_for(
					clean.max_seeding_time_type,
					clean.max_seeding_time_type_button,
					clean.max_seeding_time_type_label,
					clean.max_seeding_time
				);

				clean.max_ratio_type = daemon_clean.max_ratio_type;
				clean.max_ratio->set_value(daemon_clean.max_ratio);
				this->auto_clean_widgets_update_for(
					clean.max_ratio_type,
					clean.max_ratio_type_button,
					clean.max_ratio_type_label,
					clean.max_ratio
				);

				clean.max_seeding_torrents_type = daemon_clean.max_seeding_torrents_type;
				clean.max_seeding_torrents->set_value(daemon_clean.max_seeding_torrents);
				this->auto_clean_widgets_update_for(
					clean.max_seeding_torrents_type,
					clean.max_seeding_torrents_type_button,
					clean.max_seeding_torrents_type_label,
					clean.max_seeding_torrents
				);
			}
			// Auto clean <--
		}
		// Automation <--
	}
	// daemon settings <--
}



void Settings_window::on_auto_clean_max_ratio_clicked_cb(void)
{
	Private::Auto_clean& clean = priv->daemon.automation.clean;

	this->auto_clean_widgets_update_for(
		++clean.max_ratio_type,
		  clean.max_ratio_type_button,
		  clean.max_ratio_type_label,
		  clean.max_ratio
	);
}



void Settings_window::on_auto_clean_max_seeding_time_clicked_cb(void)
{
	Private::Auto_clean& clean = priv->daemon.automation.clean;

	this->auto_clean_widgets_update_for(
		++clean.max_seeding_time_type,
		  clean.max_seeding_time_type_button,
		  clean.max_seeding_time_type_label,
		  clean.max_seeding_time
	);
}



void Settings_window::on_auto_clean_max_seeding_torrents_clicked_cb(void)
{
	Private::Auto_clean& clean = priv->daemon.automation.clean;

	this->auto_clean_widgets_update_for(
		++clean.max_seeding_torrents_type,
		  clean.max_seeding_torrents_type_button,
		  clean.max_seeding_torrents_type_label,
		  clean.max_seeding_torrents
	);
}



void Settings_window::on_auto_load_torrents_toggled_callback(void)
{
	Private::Auto_load& load = priv->daemon.automation.load;

	load.container->set_sensitive(load.is->get_active());
	load.from->set_sensitive(load.is->get_active());
}



void Settings_window::on_auto_load_torrents_copy_to_toggled_callback(void)
{
	Private::Auto_load& load = priv->daemon.automation.load;
	load.copy_to->set_sensitive(load.copy->get_active());
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



void Settings_window::on_show_tray_icon_toggled_callback(void)
{
	this->tray_vbox.set_sensitive(this->show_tray_icon.get_active());
}



void Settings_window::save_settings(void)
{
	Status_bar_settings& status_bar_settings = this->client_settings.gui.main_window.status_bar;

	// GUI -->
		this->client_settings.gui.show_zero_values				= this->show_zero_values.get_active();

		this->client_settings.gui.update_interval				= this->gui_update_interval.get_value();
		this->client_settings.gui.max_log_lines					= this->max_log_lines.get_value();

		this->client_settings.gui.show_tray_icon				= this->show_tray_icon.get_active();
		this->client_settings.gui.hide_app_to_tray_at_startup	= this->hide_app_to_tray_at_startup.get_active();
		this->client_settings.gui.minimize_to_tray				= this->minimize_to_tray.get_active();
		this->client_settings.gui.close_to_tray					= this->close_to_tray.get_active();

		status_bar_settings.download_speed						= this->status_bar_download_speed.get_active();
		status_bar_settings.payload_download_speed				= this->status_bar_payload_download_speed.get_active();

		status_bar_settings.upload_speed						= this->status_bar_upload_speed.get_active();
		status_bar_settings.payload_upload_speed				= this->status_bar_payload_upload_speed.get_active();

		status_bar_settings.download							= this->status_bar_download.get_active();
		status_bar_settings.payload_download					= this->status_bar_payload_download.get_active();

		status_bar_settings.upload								= this->status_bar_upload.get_active();
		status_bar_settings.payload_upload						= this->status_bar_payload_upload.get_active();

		status_bar_settings.share_ratio							= this->status_bar_share_ratio.get_active();
		status_bar_settings.failed								= this->status_bar_failed.get_active();
		status_bar_settings.redundant							= this->status_bar_redundant.get_active();
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
	{
		Daemon_settings& settings = this->daemon_settings;

		// Network -->
			// Misc -->
			{
				Private::Network_misc& misc = priv->daemon.network.misc;

				settings.listen_random_port = misc.random_port->get_active();
				settings.listen_ports_range = std::pair<int, int>(
					misc.from_port->get_value(), misc.to_port->get_value());

				settings.dht		= misc.dht->get_active();
				settings.lsd		= misc.lsd->get_active();
				settings.upnp		= misc.upnp->get_active();
				settings.natpmp		= misc.natpmp->get_active();
				settings.smart_ban	= misc.smart_ban->get_active();
				settings.pex		= misc.pex->get_active();

				settings.download_rate_limit	= misc.download_rate_limit->get_value();
				settings.upload_rate_limit		= misc.upload_rate_limit->get_value();
				settings.max_uploads			= misc.max_uploads->get_value();
				settings.max_connections		= misc.max_connections->get_value();

				settings.use_max_announce_interval	= misc.use_max_announce_interval->get_active();
				settings.max_announce_interval		= misc.max_announce_interval->get_value() * 60;
			}
			// Misc <--

			// IP filter
			settings.ip_filter = priv->daemon.network.ip_filter->get();
		// Network <--

		// Automation -->
			// Torrents auto load -->
			{
				Daemon_settings::Torrents_auto_load& auto_load = settings.torrents_auto_load;
				Private::Auto_load& load = priv->daemon.automation.load;

				auto_load.is			= load.is->get_active();
				auto_load.from			= L2U(load.from->get_filename());
				auto_load.to			= L2U(load.to->get_filename());

				auto_load.copy			= load.copy->get_active();
				auto_load.copy_to		= L2U(load.copy_to->get_filename());

				auto_load.delete_loaded	= load.delete_loaded->get_active();
			}
			// Torrents auto load <--

			// Torrents auto clean -->
			{
				Daemon_settings::Auto_clean& auto_clean = settings.torrents_auto_clean;
				Private::Auto_clean& clean = priv->daemon.automation.clean;

				auto_clean.max_seeding_time				= clean.max_seeding_time->get_value() * 60;
				auto_clean.max_seeding_time_type		= clean.max_seeding_time_type;

				auto_clean.max_ratio					= clean.max_ratio->get_value();
				auto_clean.max_ratio_type				= clean.max_ratio_type;

				auto_clean.max_seeding_torrents			= clean.max_seeding_torrents->get_value();
				auto_clean.max_seeding_torrents_type	= clean.max_seeding_torrents_type;
			}
			// Torrents auto clean <--
		// Automation <--
	}
	// daemon settings <--
}

