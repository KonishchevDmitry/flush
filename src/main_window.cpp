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


#include <deque>
#include <vector>

#include <gdkmm/pixbuf.h>

#include <gtkmm/aboutdialog.h>
#include <gtkmm/alignment.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/main.h>
#include <gtkmm/stock.h>

#include <mlib/gtk/vbox.hpp>

#include "add_torrent_dialog.hpp"
#include "application.hpp"
#include "client_settings.hpp"
#include "create_torrent_dialog.hpp"
#include "daemon_proxy.hpp"
#include "daemon_settings.hpp"
#include "log_view.hpp"
#include "main.hpp"
#include "main_window.hpp"
#include "settings_window.hpp"
#include "statistics_window.hpp"
#include "torrents_viewport.hpp"


// Интервал автоматического сохранения настроек.
#define SAVE_SETTINGS_INTERVAL ( 5 * 60 * 1000 ) // ms



// Change_rate_limit_dialog -->
	/// Диалог изменения скорости скачивания/раздачи.
	class Change_rate_limit_dialog: public Gtk::Dialog
	{
		public:
			Change_rate_limit_dialog(Gtk::Window& parent, Traffic_type traffic_type);


		private:
			const Traffic_type	traffic_type;
			Gtk::SpinButton*	rate_limit_button;

		
		public:
			void	run(void);

		private:
			/// Обработчик сигнала на активацию кнопки, задающей скорость.
			void	on_rate_limit_button_activate_callback(void);
	};



	Change_rate_limit_dialog::Change_rate_limit_dialog(Gtk::Window& parent, Traffic_type traffic_type)
	:
		Gtk::Dialog("", parent),
		traffic_type(traffic_type)
	{
		std::string title = this->traffic_type == DOWNLOAD ? _("Set download rate limit") : _("Set upload rate limit");

		this->set_title(std::string(APP_NAME) + ": " + title);
		this->set_resizable(false);

		if(!parent.is_visible())
			this->set_position(Gtk::WIN_POS_CENTER);

		Gtk::VBox* main_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		main_vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
		this->get_vbox()->pack_start(*main_vbox, false, false);

		m::gtk::vbox::add_header(*main_vbox, title, true);

		Gtk::Alignment* alignment = Gtk::manage(new Gtk::Alignment(0.5, 0.5, 0, 0));
		main_vbox->pack_start(*alignment, false, false);

		Gtk::HBox* main_hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
		alignment->add(*main_hbox);

		this->rate_limit_button = Gtk::manage(new Gtk::SpinButton);
			this->rate_limit_button->set_range(-1, INT_MAX);
			this->rate_limit_button->set_increments(1, 1000);

			this->rate_limit_button->signal_activate().connect(sigc::mem_fun(
				*this, &Change_rate_limit_dialog::on_rate_limit_button_activate_callback
			));
		main_hbox->pack_start(*this->rate_limit_button, false, false);

		Gtk::Label* label = Gtk::manage(new Gtk::Label(_("KB/s")));
		main_hbox->pack_start(*label, false, false);

		// Добавляем кнопки
		this->get_action_area()->property_layout_style() = Gtk::BUTTONBOX_CENTER;
		this->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		this->add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
		this->set_default_response(Gtk::RESPONSE_OK);

		this->show_all_children();
	}



	void Change_rate_limit_dialog::run(void)
	{
		try
		{
			this->rate_limit_button->set_value(get_daemon_proxy().get_rate_limit(this->traffic_type));
		}
		catch(m::Exception& e)
		{
			MLIB_W(_("Getting current rate limit failed"), EE(e));
			return;
		}

		if(Gtk::Dialog::run() == Gtk::RESPONSE_OK)
		{
			try
			{
				get_daemon_proxy().set_rate_limit(this->traffic_type, this->rate_limit_button->get_value());
			}
			catch(m::Exception& e)
			{
				MLIB_W(_("Setting current rate limit failed"), EE(e));
			}
		}
	}



	void Change_rate_limit_dialog::on_rate_limit_button_activate_callback(void)
	{
		this->response(Gtk::RESPONSE_OK);
	}
// Change_rate_limit_dialog <--



Main_window::Main_window(const Main_window_settings& settings)
:
	m::gtk::Window(
		std::string(APP_NAME) + (
			get_application().get_config_dir_path() == get_default_config_dir_path()
			?
				""
			:
				" (" + get_application().get_config_dir_path() + ")"
		),
		settings.window,
		800, 600, m::gtk::WINDOW_BORDER_WIDTH / 2
	),
	has_been_showed(false),
	settings_window(NULL)
{
	Client_settings& client_settings = get_client_settings();
	Main_window_settings& main_window_settings = client_settings.gui.main_window;

	// Трей
	show_tray_icon(client_settings.gui.show_tray_icon);

	// Меню -->
		Glib::RefPtr<Gtk::ActionGroup> action_group;

		this->ui_manager = Gtk::UIManager::create();

		action_group = Gtk::ActionGroup::create();

			action_group->add(Gtk::Action::create("file", _("_File")));
			action_group->add(
				Gtk::Action::create("create", Gtk::Stock::NEW, _("_Create")),
				sigc::mem_fun(*this, &Main_window::on_create_callback)
			);
			action_group->add(
				Gtk::Action::create("open", Gtk::Stock::OPEN, _("_Open")),
				sigc::mem_fun(*this, &Main_window::on_open_callback)
			);
			action_group->add(
				Gtk::Action::create("quit", Gtk::Stock::QUIT, _("_Quit")),
				sigc::mem_fun(*this, &Main_window::on_quit_callback
			));


			action_group->add(Gtk::Action::create("edit", _("_Edit")));
			action_group->add(
				Gtk::Action::create("statistics", Gtk::Stock::INDEX, _("_Statistics")),
				sigc::mem_fun(*this, &Main_window::on_show_statistics_callback)
			);
			action_group->add(
				Gtk::Action::create("preferences", Gtk::Stock::PREFERENCES, _("_Preferences")),
				sigc::mem_fun(*this, &Main_window::on_show_settings_window_callback)
			);


			action_group->add(Gtk::Action::create("torrents", _("_Torrents")));
			action_group->add(Gtk::Action::create("tray_torrents", Gtk::Stock::DND_MULTIPLE, _("_Torrents")));
			action_group->add(Gtk::Action::create("resume", Gtk::Stock::MEDIA_PLAY, _("_Resume")));
			action_group->add(
				Gtk::Action::create("resume_all", Gtk::Stock::SELECT_ALL, _("_All")),
				sigc::bind<Torrents_group>(
					sigc::mem_fun(*this, &Main_window::on_resume_torrents_callback),
					ALL
				)
			);
			action_group->add(
				Gtk::Action::create("resume_downloads", Gtk::Stock::GO_DOWN, _("_Downloads")),
				sigc::bind<Torrents_group>(
					sigc::mem_fun(*this, &Main_window::on_resume_torrents_callback),
					DOWNLOADS
				)
			);
			action_group->add(
				Gtk::Action::create("resume_uploads", Gtk::Stock::GO_UP, _("_Uploads")),
				sigc::bind<Torrents_group>(
					sigc::mem_fun(*this, &Main_window::on_resume_torrents_callback),
					UPLOADS
				)
			);


			action_group->add(Gtk::Action::create("pause", Gtk::Stock::MEDIA_PAUSE, _("_Pause")));
			action_group->add(
				Gtk::Action::create("pause_all", Gtk::Stock::SELECT_ALL, _("_All")),
				sigc::bind<Torrents_group>(
					sigc::mem_fun(*this, &Main_window::on_pause_torrents_callback),
					ALL
				)
			);
			action_group->add(
				Gtk::Action::create("pause_downloads", Gtk::Stock::GO_DOWN, _("_Downloads")),
				sigc::bind<Torrents_group>(
					sigc::mem_fun(*this, &Main_window::on_pause_torrents_callback),
					DOWNLOADS
				)
			);
			action_group->add(
				Gtk::Action::create("pause_uploads", Gtk::Stock::GO_UP, _("_Uploads")),
				sigc::bind<Torrents_group>(
					sigc::mem_fun(*this, &Main_window::on_pause_torrents_callback),
					UPLOADS
				)
			);


			action_group->add(
				Gtk::Action::create("set_download_rate_limit", Gtk::Stock::GO_DOWN, _("_Set download rate limit")),
				sigc::bind<Traffic_type>(
					sigc::mem_fun(*this, &Main_window::on_change_rate_limit_callback),
					DOWNLOAD
				)
			);
			action_group->add(
				Gtk::Action::create("set_upload_rate_limit", Gtk::Stock::GO_UP, _("_Set upload rate limit")),
				sigc::bind<Traffic_type>(
					sigc::mem_fun(*this, &Main_window::on_change_rate_limit_callback),
					UPLOAD
				)
			);


			action_group->add(Gtk::Action::create("help", _("_Help")));
			action_group->add(
				Gtk::Action::create("about", Gtk::Stock::ABOUT, _("_About")),
				sigc::mem_fun(*this, &Main_window::on_show_about_dialog_callback)
			);

		this->ui_manager->insert_action_group(action_group);

		Glib::ustring ui_info =
			"<ui>"
			"	<menubar name='menu_bar'>"
			"		<menu action='file'>"
			"			<menuitem action='create'/>"
			"			<menuitem action='open'/>"
			"			<menuitem action='quit'/>"
			"		</menu>"
			"		<menu action='edit'>"
			"			<menuitem action='statistics'/>"
			"			<menuitem action='preferences'/>"
			"		</menu>"
			"		<menu action='torrents'>"
			"			<menu action='resume'>"
			"				<menuitem action='resume_all'/>"
			"				<menuitem action='resume_downloads'/>"
			"				<menuitem action='resume_uploads'/>"
			"			</menu>"
			"			<menu action='pause'>"
			"				<menuitem action='pause_all'/>"
			"				<menuitem action='pause_downloads'/>"
			"				<menuitem action='pause_uploads'/>"
			"			</menu>"
			"		</menu>"
			"		<menu action='help'>"
			"			<menuitem action='about'/>"
			"		</menu>"
			"	</menubar>"
			"	<popup name='tray_popup_menu'>"
			"		<menuitem action='open'/>"
			"		<separator/>"
			"		<menu action='tray_torrents'>"
			"			<menu action='resume'>"
			"				<menuitem action='resume_all'/>"
			"				<menuitem action='resume_downloads'/>"
			"				<menuitem action='resume_uploads'/>"
			"			</menu>"
			"			<menu action='pause'>"
			"				<menuitem action='pause_all'/>"
			"				<menuitem action='pause_downloads'/>"
			"				<menuitem action='pause_uploads'/>"
			"			</menu>"
			"		</menu>"
			"		<separator/>"
			"		<menuitem action='set_download_rate_limit'/>"
			"		<menuitem action='set_upload_rate_limit'/>"
			"		<separator/>"
			"		<menuitem action='quit'/>"
			"	</popup>"
			"</ui>";

		this->ui_manager->add_ui_from_string(ui_info);
		this->add_accel_group(this->ui_manager->get_accel_group());
	// Меню <--

	Gtk::VBox* main_vbox = Gtk::manage(new Gtk::VBox());
	this->add(*main_vbox);

	// Панель меню -->
		Gtk::Widget* menu_bar = this->ui_manager->get_widget("/menu_bar");
		main_vbox->pack_start(*menu_bar, false, true);
	// Панель меню <--

	// Список торрентов -->
		this->torrents_viewport = Gtk::manage(new Torrents_viewport(main_window_settings.torrents_viewport));
		main_vbox->pack_start(*this->torrents_viewport, true, true);

		// Настройки отдельных виджетов
		this->torrents_viewport->get_log_view().set_max_lines(client_settings.gui.max_log_lines);
	// Список торрентов <--

	// status bar -->
	{
		Gtk::Alignment* alignment = Gtk::manage(new Gtk::Alignment());
		alignment->property_top_padding() = m::gtk::VBOX_SPACING / 2;
		main_vbox->pack_start(*alignment, false, false);

		alignment->add(this->status_bar);
		this->status_bar.push("");
	}
	// status bar <--

	// Устанавливаем интервал обновления GUI.
	this->set_gui_update_interval(client_settings.gui.update_interval);

	// Автоматическое сохранение настроек.
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Main_window::on_save_settings_timeout), SAVE_SETTINGS_INTERVAL);

	// Закрытие окна.
	this->signal_delete_event().connect(sigc::mem_fun(*this, &Main_window::on_close_callback));

	#ifndef DEVELOP_MODE
		if(client_settings.gui.show_tray_icon)
			this->show_all_children();
		else
			this->show_all();
	#else
		this->show_all();
	#endif
}



void Main_window::add_daemon_message(const Daemon_message& message) const
{
	this->torrents_viewport->get_log_view().add_message(message);
}



void Main_window::on_change_rate_limit_callback(Traffic_type traffic_type)
{
	Change_rate_limit_dialog dialog(*this, traffic_type);;
	dialog.run();
}



bool Main_window::on_close_callback(GdkEventAny* event)
{
	#ifndef DEVELOP_MODE
		if(!get_client_settings().gui.show_tray_icon)
			this->on_quit_callback();
	#else
		this->on_quit_callback();
	#endif

	return false;
}



void Main_window::on_create_callback(void)
{
	new Create_torrent_dialog(*this);
}



bool Main_window::on_gui_update_timeout(void)
{
	if(this->is_visible())
		this->update_gui();

	return true;
}



void Main_window::on_open_callback(void)
{
	std::string open_torrents_from = get_application().get_client_settings().gui.open_torrents_from;

	Gtk::FileChooserDialog* dialog = new Gtk::FileChooserDialog(
		*this, _("Please choose a torrent file"),
		Gtk::FILE_CHOOSER_ACTION_OPEN
	);

	// Добавляем кнопки -->
		dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dialog->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
	// Добавляем кнопки <--

	dialog->set_select_multiple();
	dialog->set_default_response(Gtk::RESPONSE_OK);

	if(open_torrents_from != "")
		dialog->set_current_folder(U2L(open_torrents_from));

	// Добавляем фильтры по типам файлов -->
	{
		Gtk::FileFilter torrents_filter;
		torrents_filter.set_name(_("Torrent files"));
		torrents_filter.add_mime_type("application/x-bittorrent");
		dialog->add_filter(torrents_filter);

		Gtk::FileFilter any_filter;
		any_filter.set_name(_("Any files"));
		any_filter.add_pattern("*");
		dialog->add_filter(any_filter);
	}
	// Добавляем фильтры по типам файлов <--

	dialog->signal_response().connect(
		sigc::bind<Gtk::FileChooserDialog*>(
			sigc::mem_fun(*this, &Main_window::on_open_response_callback),
			dialog
		)
	);

	dialog->show();
}



void Main_window::on_open_response_callback(int response_id, Gtk::FileChooserDialog* dialog)
{
	if(response_id != Gtk::RESPONSE_OK)
	{
		delete dialog;
		return;
	}

	Glib::SListHandle<Glib::ustring> filenames = dialog->get_filenames();
	get_application().get_client_settings().gui.open_torrents_from = L2U(dialog->get_current_folder());

	delete dialog;

	M_FOR_CONST_IT(Glib::SListHandle<Glib::ustring>, filenames, it)
		this->open_torrent(L2U(*it));
}



void Main_window::on_pause_torrents_callback(Torrents_group group)
{
	try
	{
		get_daemon_proxy().stop_torrents(group);
	}
	catch(m::Exception& e)
	{
		MLIB_W(_("Pausing torrents failed"), EE(e));
	}
}



void Main_window::on_quit_callback(void)
{
	// Сохраняем текущие настройки в конфиг.
	this->save_settings();

	// Может и не завершить работу, если открыт хотя бы один диалог.
	Gtk::Main::quit();
}



void Main_window::on_resume_torrents_callback(Torrents_group group)
{
	try
	{
		get_daemon_proxy().start_torrents(group);
	}
	catch(m::Exception& e)
	{
		MLIB_W(_("Pausing torrents failed"), EE(e));
	}
}



bool Main_window::on_save_settings_timeout(void)
{
	this->save_settings();
	return true;
}



void Main_window::on_show_about_dialog_callback(void)
{
	Gtk::AboutDialog dialog;

	std::vector<std::string> authors;
	authors.push_back(
		_("Konishchev Dmitry") + std::string(" <konishchev@gmail.com>\n") +
		std::string("http://konishchevdmitry.blogspot.com/")
	);

	dialog.set_transient_for(*this);
	dialog.set_position(Gtk::WIN_POS_CENTER_ON_PARENT);

	dialog.set_logo_icon_name(APP_UNIX_NAME);
	dialog.set_version(APP_VERSION_STRING);
	dialog.set_comments( _("GTK-based BitTorrent client") );
	dialog.set_authors(authors);
	dialog.set_copyright( m::get_copyright_string(_("Konishchev Dmitry"), APP_YEAR) );
	dialog.set_license(
		"This program is free software; you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation; either version 3 of the License, or\n"
		"(at your option) any later version.\n"
		"\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n"
	);
	dialog.set_website("http://sourceforge.net/projects/flush/");

	dialog.run();
}



void Main_window::on_show_settings_window_callback(void)
{
	Daemon_settings daemon_settings;
	Client_settings& client_settings = get_client_settings();

	int gui_update_interval = client_settings.gui.update_interval;
	int max_log_lines = client_settings.gui.max_log_lines;

	// Получаем настройки демона -->
		try
		{
			daemon_settings = get_daemon_proxy().get_settings();
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
			return;
		}
	// Получаем настройки демона <--

	Settings_window settings_window(*this, &client_settings, &daemon_settings);

	if(settings_window.run() == Gtk::RESPONSE_OK)
	{
		this->show_tray_icon(client_settings.gui.show_tray_icon);

		if(gui_update_interval != client_settings.gui.update_interval)
			this->set_gui_update_interval(client_settings.gui.update_interval);

		if(max_log_lines != client_settings.gui.max_log_lines)
			this->torrents_viewport->get_log_view().set_max_lines(client_settings.gui.max_log_lines);

		try
		{
			get_daemon_proxy().set_settings(daemon_settings);
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
		}

		// Обновляем GUI - возможно настройки повлияли на его внешний вид,
		// поэтому будет лучше, если пользователь сразу же увидит изменения.
		this->update_gui();

		// Сохраняем настройки клиента
		this->save_settings();
	}
}



void Main_window::on_show_statistics_callback(void)
{
	Statistics_window(*this).run();
}



void Main_window::on_tray_activated(void)
{
	if(this->is_visible())
		this->hide();
	else
		this->show();
}



void Main_window::on_tray_popup_menu(int button, int activate_time)
{
	Gtk::Menu* menu = dynamic_cast<Gtk::Menu*>(this->ui_manager->get_widget("/tray_popup_menu"));
	menu->popup(button, activate_time);
}



void Main_window::open_torrent(const std::string& torrent_path)
{
	Client_settings& client_settings = get_client_settings();

	try
	{
		if(client_settings.gui.show_add_torrent_dialog)
		{
			// Генерирует m::Exception
			new Add_torrent_dialog(*this, torrent_path);
		}
		else
		{
			get_application().add_torrent(
				torrent_path,
				New_torrent_settings(
					client_settings.user.start_torrent_on_adding,
					client_settings.user.download_to,
					client_settings.user.copy_finished_to,
					std::vector<Torrent_file_settings>()
				)
			);
		}

		// Чтобы торрент появился моментально.
		this->update_gui();
	}
	catch(m::Exception& e)
	{
		MLIB_W(_("Opening torrent failed"), __("Opening torrent '%1' failed. %2", torrent_path, EE(e)));
	}
}



void Main_window::set_gui_update_interval(int interval)
{
	this->gui_update_timeout_connection.disconnect();

	this->gui_update_timeout_connection = Glib::signal_timeout().connect(sigc::mem_fun(
		*this, &Main_window::on_gui_update_timeout), interval
	);
}



void Main_window::save_settings(void)
{
	// Получаем текущие настройки -->
		if(this->has_been_showed)
		{
			// Получаем настройки от GUI только в том случае, если окно было
			// отображено хотя бы один раз.
			//
			// Если окно ни разу не отображалось, то мы, скорее всего, получим
			// от GTK неверные значения.
			//
			// То, что мы отвергаем все настройки GUI - на самом деле не
			// страшно, как может показаться на первый взгляд, т. к. если окно
			// не было ни разу отображено, то и, соответсвенно, пользователь не
			// мог изменить какие-либо настройки.

			Main_window_settings& settings = get_client_settings().gui.main_window;

			m::gtk::Window::save_settings(settings.window);
			this->torrents_viewport->save_settings(settings.torrents_viewport);
		}
	// Получаем текущие настройки <--

	// Записываем настройки в конфиг
	get_application().save_settings();
}



void Main_window::show(void)
{
	this->has_been_showed = true;
	this->update_gui();
	Gtk::Window::show();
}



void Main_window::show_all(void)
{
	this->has_been_showed = true;
	this->update_gui();
	Gtk::Window::show_all();
}



void Main_window::show_tray_icon(bool show)
{
	if(show)
	{
		if(this->tray)
			return;

		// Создаем иконку в трее
		this->tray = Gtk::StatusIcon::create(APP_UNIX_NAME);

		// Обработчик нажатия левой кнопки мыши по значку в трее
		this->tray->signal_activate().connect(sigc::mem_fun(*this, &Main_window::on_tray_activated));

		// Обработчик нажатия правой кнопки мыши по значку в трее
		this->tray->signal_popup_menu().connect(sigc::mem_fun(*this, &Main_window::on_tray_popup_menu));
	}
	else
	{
		if(!this->tray)
			return;

		this->tray.reset();
	}
}



void Main_window::update_gui(void)
{
	// Обновляем список торрентов -->
		std::vector<Torrent_info> torrents_info;

		try
		{
			get_daemon_proxy().get_torrents(torrents_info);
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
		}

		torrents_viewport->update(torrents_info.begin(), torrents_info.end());
	// Обновляем список торрентов <--

	// Получаем информацию о текущей сессии
	// и заносим ее в строку статуса.
	// -->
	{
		try
		{
			Session_status session_status = get_daemon_proxy().get_session_status();

			const Status_bar_settings& status_bar_settings = get_client_settings().gui.main_window.status_bar;
			std::string space_string = "  ";
			std::string status_string;

			if(status_bar_settings.download_speed)
				status_string += space_string + _("Download speed") + ": " + m::speed_to_string(session_status.download_speed);

			if(status_bar_settings.payload_download_speed)
				status_string += space_string + _("Download speed (payload)") + ": " + m::speed_to_string(session_status.payload_download_speed);

			if(status_bar_settings.upload_speed)
				status_string += space_string + _("Upload speed") + ": " + m::speed_to_string(session_status.upload_speed);

			if(status_bar_settings.payload_upload_speed)
				status_string += space_string + _("Upload speed (payload)") + ": " + m::speed_to_string(session_status.payload_upload_speed);

			if(status_bar_settings.download)
				status_string += space_string + _("Downloaded") + ": " + m::size_to_string(session_status.download);

			if(status_bar_settings.payload_download)
				status_string += space_string + _("Download (payload)") + ": " + m::size_to_string(session_status.payload_download);

			if(status_bar_settings.upload)
				status_string += space_string + _("Uploaded") + ": " + m::size_to_string(session_status.upload);

			if(status_bar_settings.payload_upload)
				status_string += space_string + _("Upload (payload)") + ": " + m::size_to_string(session_status.payload_upload);

			if(status_bar_settings.share_ratio)
				status_string += space_string + _("Share ratio") + ": " + get_share_ratio_string(session_status.payload_upload, session_status.payload_download);

			if(status_bar_settings.failed)
				status_string += space_string + _("Failed") + ": " + m::size_to_string(session_status.failed);

			if(status_bar_settings.redundant)
				status_string += space_string + _("Redundant") + ": " + m::size_to_string(session_status.redundant);

			if(status_string.empty())
				this->status_bar.hide();
			else
			{
				this->status_bar.pop();
				this->status_bar.push(status_string.substr(space_string.size()));
				this->status_bar.show();
			}
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
		}
	}
	// <--
}

