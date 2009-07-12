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
#include <map>
#include <vector>

#include <gdk/gdk.h>

#include <gdkmm/pixbuf.h>

#include <gtkmm/aboutdialog.h>
#include <gtkmm/action.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/alignment.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/main.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/radiobuttongroup.h>
#include <gtkmm/separatortoolitem.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/statusicon.h>
#include <gtkmm/stock.h>
#include <gtkmm/toggleaction.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/uimanager.h>

#include <mlib/main.hpp>
#include <mlib/misc.hpp>
#include <mlib/string.hpp>

#include <mlib/gtk/action.hpp>
#include <mlib/gtk/glade.hpp>
#include <mlib/gtk/main.hpp>
#include <mlib/gtk/signal_proxy.hpp>
#include <mlib/gtk/toolbar.hpp>
#include <mlib/gtk/vbox.hpp>

#include "add_torrent_dialog.hpp"
#include "app_icons.hpp"
#include "application.hpp"
#include "categories_view.hpp"
#include "client_settings.hpp"
#include "common.hpp"
#include "create_torrent_dialog.hpp"
#include "daemon_proxy.hpp"
#include "daemon_settings.hpp"
#include "gui_lib.hpp"
#include "log_view.hpp"
#include "main.hpp"
#include "main_window.hpp"
#include "open_torrent_dialog.hpp"
#include "rss_settings_dialog.hpp"
#include "settings_window.hpp"
#include "statistics_window.hpp"
#include "temporary_action_dialog.hpp"
#include "torrents_viewport.hpp"


// Интервал автоматического сохранения настроек.
#define SAVE_SETTINGS_INTERVAL ( 5 * 60 * 1000 ) // ms



// Gui -->
	class Main_window::Gui
	{
		public:
			Gui(void);


		public:
			/// Определяет, было ли хотя бы один раз отображено окно, или за
			/// все время работы программы пользоватьель его так и не видел.
			bool								has_been_showed;

			/// Определяет, в каком положении надится в данный момент окно - в
			/// свернутом или нет (на самом деле не обязательно, что оно
			/// свернуто - также это может означать, что окно находится на другом
			/// рабочем столе.
			bool								iconified;

			/// Текст заголовка окна без дополнительной информации (текущих
			/// скоростях скачивания).
			std::string							orig_window_title;

			// Меню
			Glib::RefPtr<Gtk::ToggleAction>		menu_show_toolbar_action;

			Glib::RefPtr<Gtk::Action>			resume_temporary;
			Glib::RefPtr<Gtk::Action>			pause_temporary;
			Glib::RefPtr<Gtk::Action>			complete_temporary_action;
			Glib::RefPtr<Gtk::Action>			cancel_temporary_action;

			// Панель инструментов -->
				Gtk::Toolbar					toolbar;
				Gtk::ToolButton*				toolbar_resume_button;
				Gtk::ToolButton*				toolbar_pause_button;
				Gtk::ToolButton*				toolbar_remove_button;
				Gtk::ToolButton*				toolbar_remove_with_data_button;
			// Панель инструментов <--

			/// Status bar.
			Gtk::Statusbar						status_bar;

			/// Иконка в трее
			Glib::RefPtr<Gtk::StatusIcon>		tray;

			Torrents_viewport*					torrents_viewport;

			Glib::RefPtr<Gtk::UIManager>		ui_manager;

			/// Текущая "привязка" сигнала на автоматическое сохранение
			/// настроек.
			sigc::connection					autosave_settings_connection;

			/// Текущая "привязка" сигнала на изменение списка доступных в
			/// данный момент действий, которые можно совершить над выделенными
			/// торрентами.
			sigc::connection					torrent_process_actions_changed_connection;

			/// Текущая "привязка" сигнала на обновление GUI.
			sigc::connection					update_timeout_connection;


		public:
			/// Обработчик сигнала по клику на пункте меню "Отображать
			/// счетчики в категориях".
			void	on_show_categories_counters_toggle_cb(Glib::RefPtr<Gtk::ToggleAction> action);

			/// Обработчик сигнала по клику на пункте меню "Отображать
			/// имена категорий".
			void	on_show_categories_names_toggle_cb(Glib::RefPtr<Gtk::ToggleAction> action);

			/// Обработчик сигнала по клику на пункте меню "Отображать
			/// категории".
			void	on_show_categories_toggle_cb(Glib::RefPtr<Gtk::ToggleAction> action);
	};



	Main_window::Gui::Gui(void)
	:
		has_been_showed(false),
		iconified(false)
	{
	}



	void Main_window::Gui::on_show_categories_counters_toggle_cb(Glib::RefPtr<Gtk::ToggleAction> action)
	{
		this->torrents_viewport->show_categories_counters(action->get_active());
	}



	void Main_window::Gui::on_show_categories_names_toggle_cb(Glib::RefPtr<Gtk::ToggleAction> action)
	{
		this->torrents_viewport->show_categories_names(action->get_active());
	}



	void Main_window::Gui::on_show_categories_toggle_cb(Glib::RefPtr<Gtk::ToggleAction> action)
	{
		this->torrents_viewport->show_categories(action->get_active());
	}
// Gui <--



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

		this->set_title(format_window_title(title));
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
			this->rate_limit_button->set_increments(1, 100);

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
	m::gtk::Window("", settings.window, 800, 600, 0),
	gui(new Gui)
{
	Client_settings& client_settings = get_client_settings();
	Main_window_settings& main_window_settings = client_settings.gui.main_window;

	// Заголовок окна -->
		this->gui->orig_window_title = format_window_title();

		if(get_application().get_config_dir_path() != get_default_config_dir_path())
			this->gui->orig_window_title += " (" + get_application().get_config_dir_path() + ")";

		this->set_title(this->gui->orig_window_title);
	// Заголовок окна <--

	// Трей
	show_tray_icon(client_settings.gui.show_tray_icon);

	// Меню -->
		Glib::RefPtr<Gtk::ActionGroup> action_group;

		this->gui->ui_manager = Gtk::UIManager::create();

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
				m::gtk::create_action_with_icon_name("rss", app_icons::app_icon("rss"), _("_RSS")),
				sigc::mem_fun(*this, &Main_window::on_show_statistics_callback)
			);
			action_group->add(
				app_icons::create_action("statistics", app_icons::ICON_STATISTICS, _("_Statistics")),
				sigc::mem_fun(*this, &Main_window::on_show_statistics_callback)
			);
			action_group->add(
				Gtk::Action::create("preferences", Gtk::Stock::PREFERENCES, _("_Preferences")),
				sigc::mem_fun(*this, &Main_window::on_show_settings_window_callback)
			);


			action_group->add(Gtk::Action::create("view", _("_View")));

			// Toolbar -->
				action_group->add(Gtk::Action::create("toolbar", _("_Toolbar")));

				this->gui->menu_show_toolbar_action = Gtk::ToggleAction::create(
					"toolbar/show", _Q("'Show ...' toggle|_Show"), "",
					get_client_settings().gui.show_toolbar
				);
				action_group->add(
					this->gui->menu_show_toolbar_action,
					sigc::mem_fun(*this, &Main_window::on_show_toolbar_toggled_callback)
				);

				// Стиль панели инструментов -->
				{
					Gtk::RadioButtonGroup radio_group;
					std::map< m::gtk::toolbar::Style, Glib::RefPtr<Gtk::RadioAction> > toolbar_style_buttons;

					action_group->add(Gtk::Action::create("toolbar/style", _("Toolbar _style")));

					action_group->add(
						toolbar_style_buttons[m::gtk::toolbar::DEFAULT] = Gtk::RadioAction::create(
							radio_group, "toolbar/style/default", _("_Desktop default")
						),
						sigc::bind<m::gtk::toolbar::Style>(
							sigc::mem_fun(*this, &Main_window::change_toolbar_style),
							m::gtk::toolbar::DEFAULT
						)
					);

					action_group->add(
						toolbar_style_buttons[m::gtk::toolbar::ICONS] = Gtk::RadioAction::create(
							radio_group, "toolbar/style/icons", _("_Icons")
						),
						sigc::bind<m::gtk::toolbar::Style>(
							sigc::mem_fun(*this, &Main_window::change_toolbar_style),
							m::gtk::toolbar::ICONS
						)
					);

					action_group->add(
						toolbar_style_buttons[m::gtk::toolbar::TEXT] = Gtk::RadioAction::create(
							radio_group, "toolbar/style/text", _("_Text")
						),
						sigc::bind<m::gtk::toolbar::Style>(
							sigc::mem_fun(*this, &Main_window::change_toolbar_style),
							m::gtk::toolbar::TEXT
						)
					);

					action_group->add(
						toolbar_style_buttons[m::gtk::toolbar::BOTH] = Gtk::RadioAction::create(
							radio_group, "toolbar/style/both", _("_Both")
						),
						sigc::bind<m::gtk::toolbar::Style>(
							sigc::mem_fun(*this, &Main_window::change_toolbar_style),
							m::gtk::toolbar::BOTH
						)
					);

					action_group->add(
						toolbar_style_buttons[m::gtk::toolbar::BOTH_HORIZ] = Gtk::RadioAction::create(
							radio_group, "toolbar/style/both_horiz", _("Both _horizontal")
						),
						sigc::bind<m::gtk::toolbar::Style>(
							sigc::mem_fun(*this, &Main_window::change_toolbar_style),
							m::gtk::toolbar::BOTH_HORIZ
						)
					);

					toolbar_style_buttons[get_client_settings().gui.toolbar_style]->set_active();
				}
				// Стиль панели инструментов <--
			// Toolbar <--

			// Categories <--
			{
				Glib::RefPtr<Gtk::ToggleAction> action;

				action_group->add(Gtk::Action::create("categories", _("_Categories")));

				action = Gtk::ToggleAction::create(
					"categories/show", _Q("'Show ...' toggle|_Show"), "",
					get_client_settings().gui.main_window.torrents_viewport.categories_view->visible
				);
				action_group->add(
					action,
					sigc::bind< Glib::RefPtr<Gtk::ToggleAction> >(
						sigc::mem_fun(*this->gui, &Main_window::Gui::on_show_categories_toggle_cb), action)
				);

				action = Gtk::ToggleAction::create(
					"categories/show_names", _("Show _names"), "",
					get_client_settings().gui.main_window.torrents_viewport.categories_view->show_names
				);
				action_group->add(
					action,
					sigc::bind< Glib::RefPtr<Gtk::ToggleAction> >(
						sigc::mem_fun(*this->gui, &Main_window::Gui::on_show_categories_names_toggle_cb), action)
				);

				action = Gtk::ToggleAction::create(
					"categories/show_counters", _("Show _counters"), "",
					get_client_settings().gui.main_window.torrents_viewport.categories_view->show_counters
				);
				action_group->add(
					action,
					sigc::bind< Glib::RefPtr<Gtk::ToggleAction> >(
						sigc::mem_fun(*this->gui, &Main_window::Gui::on_show_categories_counters_toggle_cb), action)
				);
			}
			// Categories <--

			// Torrents -->
				action_group->add(Gtk::Action::create("torrents", _("_Torrents")));


				action_group->add(Gtk::Action::create("resume", Gtk::Stock::MEDIA_PLAY, _("_Resume")));
				action_group->add(
					app_icons::create_action("resume/all", app_icons::ICON_DOWNLOAD_AND_UPLOAD, _("_All")),
					sigc::bind<Torrents_group>(
						sigc::mem_fun(*this, &Main_window::on_resume_torrents_callback),
						ALL
					)
				);
				action_group->add(
					app_icons::create_action("resume/downloads", app_icons::ICON_DOWNLOAD, _("_Downloads")),
					sigc::bind<Torrents_group>(
						sigc::mem_fun(*this, &Main_window::on_resume_torrents_callback),
						DOWNLOADS
					)
				);
				action_group->add(
					app_icons::create_action("resume/uploads", app_icons::ICON_UPLOAD, _("_Uploads")),
					sigc::bind<Torrents_group>(
						sigc::mem_fun(*this, &Main_window::on_resume_torrents_callback),
						UPLOADS
					)
				);


				action_group->add(Gtk::Action::create("pause", Gtk::Stock::MEDIA_PAUSE, _("_Pause")));
				action_group->add(
					app_icons::create_action("pause/all", app_icons::ICON_DOWNLOAD_AND_UPLOAD, _("_All")),
					sigc::bind<Torrents_group>(
						sigc::mem_fun(*this, &Main_window::on_pause_torrents_callback),
						ALL
					)
				);
				action_group->add(
					app_icons::create_action("pause/downloads", app_icons::ICON_DOWNLOAD, _("_Downloads")),
					sigc::bind<Torrents_group>(
						sigc::mem_fun(*this, &Main_window::on_pause_torrents_callback),
						DOWNLOADS
					)
				);
				action_group->add(
					app_icons::create_action("pause/uploads", app_icons::ICON_UPLOAD, _("_Uploads")),
					sigc::bind<Torrents_group>(
						sigc::mem_fun(*this, &Main_window::on_pause_torrents_callback),
						UPLOADS
					)
				);


				// Temporary -->
					gui->resume_temporary = Gtk::Action::create(
						"resume_temporary", Gtk::Stock::MEDIA_PLAY, _("R_esume temporary"));
					action_group->add(gui->resume_temporary);

					action_group->add(
						app_icons::create_action("resume_temporary/all", app_icons::ICON_DOWNLOAD_AND_UPLOAD, _("_All")),
						sigc::bind< std::pair<Temporary_action,Torrents_group> >(
							sigc::mem_fun(*this, &Main_window::on_temporary_process_torrents_cb),
							std::pair<Temporary_action,Torrents_group>( TEMPORARY_ACTION_RESUME, ALL )
						)
					);
					action_group->add(
						app_icons::create_action("resume_temporary/downloads", app_icons::ICON_DOWNLOAD, _("_Downloads")),
						sigc::bind< std::pair<Temporary_action,Torrents_group> >(
							sigc::mem_fun(*this, &Main_window::on_temporary_process_torrents_cb),
							std::pair<Temporary_action,Torrents_group>( TEMPORARY_ACTION_RESUME, DOWNLOADS )
						)
					);
					action_group->add(
						app_icons::create_action("resume_temporary/uploads", app_icons::ICON_UPLOAD, _("_Uploads")),
						sigc::bind< std::pair<Temporary_action,Torrents_group> >(
							sigc::mem_fun(*this, &Main_window::on_temporary_process_torrents_cb),
							std::pair<Temporary_action,Torrents_group>( TEMPORARY_ACTION_RESUME, UPLOADS )
						)
					);


					gui->pause_temporary = Gtk::Action::create(
						"pause_temporary", Gtk::Stock::MEDIA_PAUSE, _("P_ause temporary"));
					action_group->add(gui->pause_temporary);

					action_group->add(
						app_icons::create_action("pause_temporary/all", app_icons::ICON_DOWNLOAD_AND_UPLOAD, _("_All")),
						sigc::bind< std::pair<Temporary_action,Torrents_group> >(
							sigc::mem_fun(*this, &Main_window::on_temporary_process_torrents_cb),
							std::pair<Temporary_action,Torrents_group>( TEMPORARY_ACTION_PAUSE, ALL )
						)
					);
					action_group->add(
						app_icons::create_action("pause_temporary/downloads", app_icons::ICON_DOWNLOAD, _("_Downloads")),
						sigc::bind< std::pair<Temporary_action,Torrents_group> >(
							sigc::mem_fun(*this, &Main_window::on_temporary_process_torrents_cb),
							std::pair<Temporary_action,Torrents_group>( TEMPORARY_ACTION_PAUSE, DOWNLOADS )
						)
					);
					action_group->add(
						app_icons::create_action("pause_temporary/uploads", app_icons::ICON_UPLOAD, _("_Uploads")),
						sigc::bind< std::pair<Temporary_action,Torrents_group> >(
							sigc::mem_fun(*this, &Main_window::on_temporary_process_torrents_cb),
							std::pair<Temporary_action,Torrents_group>( TEMPORARY_ACTION_PAUSE, UPLOADS )
						)
					);

					gui->complete_temporary_action = Gtk::Action::create(
						"complete_temporary_action", Gtk::Stock::APPLY, _("C_omplete pending temporary action"));
					action_group->add(
						gui->complete_temporary_action,
						sigc::bind<bool>( sigc::mem_fun(*this, &Main_window::on_interrupt_temporary_action_cb), true )
					);

					gui->cancel_temporary_action = Gtk::Action::create(
						"cancel_temporary_action", Gtk::Stock::STOP, _("_Cancel pending temporary action"));
					action_group->add(
						gui->cancel_temporary_action,
						sigc::bind<bool>( sigc::mem_fun(*this, &Main_window::on_interrupt_temporary_action_cb), false )
					);
				// Temporary <--
			// Torrents <--


			action_group->add(
				app_icons::create_action("set_upload_rate_limit", app_icons::ICON_UPLOAD, _("Set _upload rate limit")),
				sigc::bind<Traffic_type>(
					sigc::mem_fun(*this, &Main_window::on_change_rate_limit_callback),
					UPLOAD
				)
			);
			action_group->add(
				app_icons::create_action("set_download_rate_limit",
					app_icons::ICON_DOWNLOAD, _("Set _download rate limit")),
				sigc::bind<Traffic_type>(
					sigc::mem_fun(*this, &Main_window::on_change_rate_limit_callback),
					DOWNLOAD
				)
			);


			action_group->add(Gtk::Action::create("help", _("_Help")));
			action_group->add(
				Gtk::Action::create("about", Gtk::Stock::ABOUT, _("_About")),
				sigc::mem_fun(*this, &Main_window::on_show_about_dialog_callback)
			);

		this->gui->ui_manager->insert_action_group(action_group);

		Glib::ustring ui_info =
			"<ui>"
			"	<menubar name='menu_bar'>"

			"		<menu action='file'>"
			"			<menuitem action='create'/>"
			"			<menuitem action='open'/>"
			"			<menuitem action='quit'/>"
			"		</menu>"

			"		<menu action='edit'>"
#ifdef DEVELOP_MODE
			"			<menuitem action='rss'/>"
#endif
			"			<menuitem action='statistics'/>"
			"			<menuitem action='preferences'/>"
			"		</menu>"

			"		<menu action='view'>"
			"			<menu action='toolbar'>"
			"				<menuitem action='toolbar/show'/>"
			"				<menu action='toolbar/style'>"
			"					<menuitem action='toolbar/style/default'/>"
			"					<menuitem action='toolbar/style/icons'/>"
			"					<menuitem action='toolbar/style/text'/>"
			"					<menuitem action='toolbar/style/both'/>"
			"					<menuitem action='toolbar/style/both_horiz'/>"
			"				</menu>"
			"			</menu>"
			"			<menu action='categories'>"
			"				<menuitem action='categories/show'/>"
			"				<menuitem action='categories/show_names'/>"
			"				<menuitem action='categories/show_counters'/>"
			"			</menu>"
			"		</menu>"

			"		<menu action='torrents'>"

			"			<menu action='resume'>"
			"				<menuitem action='resume/all'/>"
			"				<menuitem action='resume/uploads'/>"
			"				<menuitem action='resume/downloads'/>"
			"			</menu>"
			"			<menu action='pause'>"
			"				<menuitem action='pause/all'/>"
			"				<menuitem action='pause/uploads'/>"
			"				<menuitem action='pause/downloads'/>"
			"			</menu>"

			"			<separator/>"

			"			<menu action='resume_temporary'>"
			"				<menuitem action='resume_temporary/all'/>"
			"				<menuitem action='resume_temporary/uploads'/>"
			"				<menuitem action='resume_temporary/downloads'/>"
			"			</menu>"
			"			<menu action='pause_temporary'>"
			"				<menuitem action='pause_temporary/all'/>"
			"				<menuitem action='pause_temporary/uploads'/>"
			"				<menuitem action='pause_temporary/downloads'/>"
			"			</menu>"
			"			<menuitem action='complete_temporary_action'/>"
			"			<menuitem action='cancel_temporary_action'/>"

			"		</menu>"

			"		<menu action='help'>"
			"			<menuitem action='about'/>"
			"		</menu>"

			"	</menubar>"


			"	<popup name='tray_popup_menu'>"
			"		<menuitem action='open'/>"

			"		<separator/>"

			"		<menu action='resume'>"
			"			<menuitem action='resume/all'/>"
			"			<menuitem action='resume/uploads'/>"
			"			<menuitem action='resume/downloads'/>"
			"		</menu>"
			"		<menu action='pause'>"
			"			<menuitem action='pause/all'/>"
			"			<menuitem action='pause/uploads'/>"
			"			<menuitem action='pause/downloads'/>"
			"		</menu>"

			"		<separator/>"

			"		<menu action='resume_temporary'>"
			"			<menuitem action='resume_temporary/all'/>"
			"			<menuitem action='resume_temporary/uploads'/>"
			"			<menuitem action='resume_temporary/downloads'/>"
			"		</menu>"
			"		<menu action='pause_temporary'>"
			"			<menuitem action='pause_temporary/all'/>"
			"			<menuitem action='pause_temporary/uploads'/>"
			"			<menuitem action='pause_temporary/downloads'/>"
			"		</menu>"
			"		<menuitem action='complete_temporary_action'/>"
			"		<menuitem action='cancel_temporary_action'/>"

			"		<separator/>"

			"		<menuitem action='set_upload_rate_limit'/>"
			"		<menuitem action='set_download_rate_limit'/>"

			"		<separator/>"

			"		<menuitem action='quit'/>"
			"	</popup>"

			"</ui>";

		this->gui->ui_manager->add_ui_from_string(ui_info);
		this->add_accel_group(this->gui->ui_manager->get_accel_group());
	// Меню <--

	Gtk::VBox* main_vbox = Gtk::manage(new Gtk::VBox());
	this->add(*main_vbox);

	// Панель меню -->
		Gtk::Widget* menu_bar = this->gui->ui_manager->get_widget("/menu_bar");
		main_vbox->pack_start(*menu_bar, false, true);
	// Панель меню <--

	// Панель инструментов
	main_vbox->pack_start(this->gui->toolbar, false, false);

	// Список торрентов -->
		this->gui->torrents_viewport = Gtk::manage(new Torrents_viewport(main_window_settings.torrents_viewport));
		main_vbox->pack_start(*this->gui->torrents_viewport, true, true);

		// Настройки отдельных виджетов
		this->gui->torrents_viewport->get_log_view().set_max_lines(client_settings.gui.max_log_lines);
	// Список торрентов <--

	// status bar -->
	{
		Gtk::Alignment* alignment = Gtk::manage(new Gtk::Alignment());
		alignment->property_top_padding() = m::gtk::VBOX_SPACING / 2;
		main_vbox->pack_start(*alignment, false, false);

		alignment->add(this->gui->status_bar);
		this->gui->status_bar.push("");
	}
	// status bar <--


	// Панель инструментов -->
	{
		// Заполнять ее лучше в самом конце, когда уже созданы все необходимые
		// виджеты.

		Gtk::ToolButton* button;


		button = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::NEW));
		button->set_label(_("Create"));
		button->set_tooltip_text(_("Create a new torrent"));
		button->set_is_important();
		this->gui->toolbar.append(
			*button,
			sigc::mem_fun(*this, &Main_window::on_create_callback)
		);

		button = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::OPEN));
		button->set_label(_("Open"));
		button->set_tooltip_text(_("Open a torrent"));
		button->set_is_important();
		this->gui->toolbar.append(
			*button,
			sigc::mem_fun(*this, &Main_window::on_open_callback)
		);


		this->gui->toolbar.append(
			*Gtk::manage(new Gtk::SeparatorToolItem())
		);


		button = this->gui->toolbar_resume_button = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::MEDIA_PLAY));
		button->set_label(_("Resume"));
		button->set_tooltip_text(_("Resume torrent(s)"));
		button->set_is_important();
		this->gui->toolbar.append(
			*button,
			sigc::bind<Torrent_process_action>(
				sigc::mem_fun(*this->gui->torrents_viewport, &Torrents_viewport::process_torrents),
				RESUME
			)
		);

		button = this->gui->toolbar_pause_button = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::MEDIA_PAUSE));
		button->set_label(_("Pause"));
		button->set_tooltip_text(_("Pause torrent(s)"));
		button->set_is_important();
		this->gui->toolbar.append(
			*button,
			sigc::bind<Torrent_process_action>(
				sigc::mem_fun(*this->gui->torrents_viewport, &Torrents_viewport::process_torrents),
				PAUSE
			)
		);

		button = this->gui->toolbar_remove_button = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::REMOVE));
		button->set_label(_("Remove"));
		button->set_tooltip_text(_("Remove torrent(s)"));
		button->set_is_important();
		this->gui->toolbar.append(
			*button,
			sigc::bind<Torrent_process_action>(
				sigc::mem_fun(*this->gui->torrents_viewport, &Torrents_viewport::process_torrents),
				REMOVE
			)
		);

		button = this->gui->toolbar_remove_with_data_button = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::DELETE));
		button->set_label(_("Remove with data"));
		button->set_tooltip_text(_("Remove torrent(s) with data"));
		button->set_is_important();
		this->gui->toolbar.append(
			*button,
			sigc::bind<Torrent_process_action>(
				sigc::mem_fun(*this->gui->torrents_viewport, &Torrents_viewport::process_torrents),
				REMOVE_WITH_DATA
			)
		);


		this->gui->toolbar.append(
			*Gtk::manage(new Gtk::SeparatorToolItem())
		);


		button = Gtk::manage(new Gtk::ToolButton());
		button->set_label(_("Statistics"));
		app_icons::set_for_tool_button(*button, app_icons::ICON_STATISTICS);
		button->set_tooltip_text(_("Statistics"));
		button->set_is_important();
		this->gui->toolbar.append(
			*button,
			sigc::mem_fun(*this, &Main_window::on_show_statistics_callback)
		);

		button = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::PREFERENCES));
		button->set_label(_("Preferences"));
		button->set_tooltip_text(_("Preferences"));
		button->set_is_important();
		this->gui->toolbar.append(
			*button,
			sigc::mem_fun(*this, &Main_window::on_show_settings_window_callback)
		);


		this->gui->toolbar.show_all_children();
		if(get_client_settings().gui.show_toolbar)
			this->gui->toolbar.show();

		this->gui->toolbar.set_no_show_all();
	}
	// Панель инструментов <--


	// Устанавливаем интервал обновления GUI
	this->set_gui_update_interval(client_settings.gui.update_interval);

	// Обновление доступных в данный момент кнопок -->
		this->on_torrent_process_actions_changed_callback(0);

		gui->torrent_process_actions_changed_connection =
			this->gui->torrents_viewport->signal_torrent_process_actions_changed().connect(
				sigc::mem_fun(*this, &Main_window::on_torrent_process_actions_changed_callback)
			);
	// Обновление доступных в данный момент кнопок <--

	// Автоматическое сохранение настроек
	gui->autosave_settings_connection = Glib::signal_timeout().connect(
		sigc::mem_fun(*this, &Main_window::on_save_settings_timeout), SAVE_SETTINGS_INTERVAL
	);

	// Обработчик сигнала на изменение состояния окна
	this->signal_window_state_event().connect(sigc::mem_fun(
		*this, &Main_window::on_window_state_changed_callback
	));

	// Закрытие окна
	this->signal_delete_event().connect(sigc::mem_fun(*this, &Main_window::on_close_callback));

	if(client_settings.gui.show_tray_icon && client_settings.gui.hide_app_to_tray_at_startup)
		this->show_all_children();
	else
		this->show_all();

	// В gtkmm 2.16.0 (Ubuntu 9.04) есть небольшая бага, из-за которой
	// this->show_all() отображает даже элементы меню, для которых была
	// выполнена Gtk::Action::set_visible(false).
	// Поэтому скрываем элементы меню в самый последний момент.
	COMPATIBILITY
	gui->complete_temporary_action->set_visible(true);
	gui->cancel_temporary_action->set_visible(true);
}



Main_window::~Main_window(void)
{
	gui->autosave_settings_connection.disconnect();
	gui->torrent_process_actions_changed_connection.disconnect();
	gui->update_timeout_connection.disconnect();
	delete gui;
}



void Main_window::add_daemon_message(const Daemon_message& message) const
{
	this->gui->torrents_viewport->get_log_view().add_message(message);
}



void Main_window::change_toolbar_style(m::gtk::toolbar::Style style)
{
	get_client_settings().gui.toolbar_style = style;
	m::gtk::toolbar::set_style(this->gui->toolbar, style);
}



void Main_window::hide(void)
{
	this->set_visible_in_wm(false);
	Gtk::Window::hide();
}



bool Main_window::is_visible_in_wm(void)
{
	return !(this->get_skip_taskbar_hint() && this->get_skip_pager_hint());
}



void Main_window::on_change_rate_limit_callback(Traffic_type traffic_type)
{
	Change_rate_limit_dialog dialog(*this, traffic_type);;
	dialog.run();
}



bool Main_window::on_close_callback(GdkEventAny* event)
{
	if(!get_client_settings().gui.show_tray_icon || !get_client_settings().gui.close_to_tray)
		this->on_quit_callback();

	return false;
}



void Main_window::on_create_callback(void)
{
	new Create_torrent_dialog(*this);
}



bool Main_window::on_gui_update_timeout(void)
{
	m::gtk::Scoped_enter gtk_lock;
	this->update_gui(false);
	return true;
}



void Main_window::on_interrupt_temporary_action_cb(bool complete)
{
	try
	{
		get_daemon_proxy().interrupt_temporary_action(complete);
	}
	catch(m::Exception& e)
	{
		MLIB_W(_("Interrupting the temporary action on torrents failed"), EE(e));
	}

	this->update_gui();
}



void Main_window::on_open_callback(void)
{
	new Open_torrent_dialog(*this);
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
	m::gtk::Scoped_enter gtk_lock;
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
		// Чтобы потом можно было не обновлять каждый раз заголовок, содержимое
		// которого будет постоянным.
		if(!client_settings.gui.show_speed_in_window_title)
			this->set_title(this->gui->orig_window_title);

		this->show_tray_icon(client_settings.gui.show_tray_icon);

		if(gui_update_interval != client_settings.gui.update_interval)
			this->set_gui_update_interval(client_settings.gui.update_interval);

		if(max_log_lines != client_settings.gui.max_log_lines)
			this->gui->torrents_viewport->get_log_view().set_max_lines(client_settings.gui.max_log_lines);

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



void Main_window::on_show_toolbar_toggled_callback(void)
{
	if(this->gui->menu_show_toolbar_action->get_active())
		this->gui->toolbar.show();
	else
		this->gui->toolbar.hide();

	get_client_settings().gui.show_toolbar = this->gui->menu_show_toolbar_action->get_active();
}



void Main_window::on_temporary_process_torrents_cb(const std::pair<Temporary_action,Torrents_group>& data)
{
	Glib::RefPtr<Gnome::Glade::Xml> glade = MLIB_GLADE_CREATE(
		std::string(APP_UI_PATH) + "/dialog.temporary_action.glade",
		"temporary_action_dialog"
	);

	Time time;

	// Запрашиваем у пользователя время -->
	{
		Temporary_action_dialog* dialog;
		MLIB_GLADE_GET_WIDGET_DERIVED(glade, "temporary_action_dialog", dialog);
		dialog->init(*this, data.first, data.second);

		if(dialog->run() == Gtk::RESPONSE_OK)
			time = dialog->get_time();
		else
			time = 0;

		delete dialog;
	}
	// Запрашиваем у пользователя время <--

	if(time)
	{
		try
		{
			get_daemon_proxy().process_torrents_temporary(data.first, data.second, time);
		}
		catch(m::Exception& e)
		{
			MLIB_W(_("Processing the temporary action on torrents failed"), EE(e));
		}

		this->update_gui();
	}
}



void Main_window::on_torrent_process_actions_changed_callback(Torrent_process_actions actions)
{
	this->gui->toolbar_resume_button->set_sensitive(actions & RESUME);
	this->gui->toolbar_pause_button->set_sensitive(actions & PAUSE);
	this->gui->toolbar_remove_button->set_sensitive(actions & REMOVE);
	this->gui->toolbar_remove_with_data_button->set_sensitive(actions & REMOVE_WITH_DATA);
}



void Main_window::on_tray_activated(void)
{
	if(this->is_visible() && !this->gui->iconified)
		this->hide();
	else
	{
		// Если окно в данный момен находися на другом рабочем столе в
		// свернутом состоянии, то тогда имеет смысл сначала скрыть его - тогда
		// Gnome перекинет его на текущий рабочий стол (IceWM 1.2.37 этого не
		// делает).
		this->hide();
		this->show();
	}
}



void Main_window::on_tray_popup_menu(int button, int activate_time)
{
	Gtk::Menu* menu = dynamic_cast<Gtk::Menu*>(this->gui->ui_manager->get_widget("/tray_popup_menu"));
	menu->popup(button, activate_time);
}



bool Main_window::on_window_state_changed_callback(const GdkEventWindowState* state)
{
	MLIB_D(_C(
		"Window state has been changed to %1 (%2).",
		state->new_window_state, state->changed_mask)
	);

	if(state->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
	{
		// Сохраняем текущее состояние окна
		this->gui->iconified = state->new_window_state & GDK_WINDOW_STATE_ICONIFIED;

		// Если окно свернули, то скрываем окно в трей, если этого требуют
		// настройки. При восстановлении окна всегда отображаем его - настройки
		// могли поменяться, пока оно было свернуто.
		if(
			!this->gui->iconified ||
			( get_client_settings().gui.show_tray_icon && get_client_settings().gui.minimize_to_tray )
		)
			this->set_visible_in_wm(!this->gui->iconified);

		if(!this->gui->iconified)
			this->update_gui(false);
	}

	return true;
}



void Main_window::open_torrent(const std::string& torrent_path, const std::string& torrent_encoding)
{
	Client_settings& client_settings = get_client_settings();

	try
	{
		if(client_settings.gui.show_add_torrent_dialog)
		{
			// Генерирует m::Exception
			new Add_torrent_dialog(*this, torrent_path, torrent_encoding);
		}
		else
		{
			get_application().add_torrent(
				torrent_path,
				New_torrent_settings(
					client_settings.user.start_torrent_on_adding,
					client_settings.user.download_to,
					client_settings.user.copy_finished_to,
					torrent_encoding,
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
	this->gui->update_timeout_connection.disconnect();

	this->gui->update_timeout_connection = Glib::signal_timeout().connect(sigc::mem_fun(
		*this, &Main_window::on_gui_update_timeout), interval
	);
}



void Main_window::save_settings(void)
{
	// Получаем текущие настройки -->
		if(this->gui->has_been_showed)
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
			this->gui->torrents_viewport->save_settings(settings.torrents_viewport);
		}
	// Получаем текущие настройки <--

	// Записываем настройки в конфиг
	get_application().save_settings();
}



void Main_window::set_visible_in_wm(bool visible)
{
	// Убираем окно с панели управления.
	this->set_skip_taskbar_hint(!visible);

	// Убираем окно из меню, вызываемом по Alt+Tab (IceWM 1.2.37 на вызов этой
	// функции никак не реагирует, а вот Gnome обрабатывает нормально).
	this->set_skip_pager_hint(!visible);
}



void Main_window::show(void)
{
	this->gui->has_been_showed = true;
	this->update_gui();

	Gtk::Window::show();
	this->set_visible_in_wm();

	// Чтобы из трея окно вылезало полностью, а не попадало только на панель
	// задач, если перед свертыванием его в трей, оно было там.
	if(this->gui->iconified)
		this->deiconify();
}



void Main_window::show_all(void)
{
	Gtk::Window::show_all_children();
	this->show();
}



void Main_window::show_tray_icon(bool show)
{
	if(show)
	{
		if(this->gui->tray)
			this->gui->tray->set_visible(true);
		else
		{
			// Создаем иконку в трее
			this->gui->tray = Gtk::StatusIcon::create(APP_UNIX_NAME);

			// Обработчик нажатия левой кнопки мыши по значку в трее
			this->gui->tray->signal_activate().connect(sigc::mem_fun(*this, &Main_window::on_tray_activated));

			// Обработчик нажатия правой кнопки мыши по значку в трее
			this->gui->tray->signal_popup_menu().connect(sigc::mem_fun(*this, &Main_window::on_tray_popup_menu));
		}
	}
	else
	{
		if(this->gui->tray)
			this->gui->tray->set_visible(false);
	}
}



void Main_window::update_gui(bool force)
{
	enum Update_type {
		UPDATE_WINDOW_TITLE	= 1 << 0,
		UPDATE_WIDGETS		= 1 << 1,
		UPDATE_TRAY			= 1 << 2,
		UPDATE_ALL			= UPDATE_WINDOW_TITLE | UPDATE_WIDGETS | UPDATE_TRAY
	};

	int update_flags = UPDATE_ALL;


	// Определяем, какие элементы нам необходимо обновить -->
		if(!force)
		{
			if(!this->is_visible())
				update_flags &= ~(UPDATE_WINDOW_TITLE | UPDATE_WIDGETS);

			if(this->gui->iconified)
			{
				update_flags &= ~UPDATE_WIDGETS;

				if(!this->is_visible_in_wm())
					update_flags &= ~UPDATE_WINDOW_TITLE;
			}
		}

		if(!get_client_settings().gui.show_speed_in_window_title)
			update_flags &= ~UPDATE_WINDOW_TITLE;

		if(!get_client_settings().gui.show_tray_icon)
			update_flags &= ~UPDATE_TRAY;
	// Определяем, какие элементы нам необходимо обновить <--

	// Обновляем список торрентов
	if(update_flags & UPDATE_WIDGETS)
		this->gui->torrents_viewport->update();

	// Получаем информацию о текущей сессии
	// и отображаем ее в элементах GUI.
	// -->
		if( update_flags & (UPDATE_WINDOW_TITLE | UPDATE_WIDGETS | UPDATE_TRAY) )
		{
			try
			{
				Session_status session_status = get_daemon_proxy().get_session_status();

				// Заголовок окна -->
					if(update_flags & UPDATE_WINDOW_TITLE)
					{
						this->set_title(
							__Q(
								"Download/Upload|D: %1, U: %2 - %3",
								m::speed_to_string(session_status.payload_download_speed),
								m::speed_to_string(session_status.payload_upload_speed),
								this->gui->orig_window_title
							)
						);
					}
				// Заголовок окна <--

				// Временные действия -->
					if(update_flags & UPDATE_WIDGETS || update_flags & UPDATE_TRAY)
					{
						bool active = session_status.temporary_action_active;

						gui->pause_temporary->set_visible(!active);
						gui->resume_temporary->set_visible(!active);
						gui->complete_temporary_action->set_visible(active);
						gui->cancel_temporary_action->set_visible(active);
					}
				// Временные действия <--

				// Строка статуса -->
					if(update_flags & UPDATE_WIDGETS)
					{
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
							this->gui->status_bar.hide();
						else
						{
							this->gui->status_bar.pop();
							this->gui->status_bar.push(status_string.substr(space_string.size()));
							this->gui->status_bar.show();
						}
					}
				// Строка статуса <--

				// Трей -->
					if(update_flags & UPDATE_TRAY)
					{
						this->gui->tray->set_tooltip(
							std::string(APP_NAME) + "\n" +
							__Q(
								"Download speed|Down: %1 (%2) / %3",
								m::speed_to_string(session_status.download_speed),
								m::speed_to_string(session_status.payload_download_speed),
								m::speed_to_string(session_status.download_rate_limit)
							)
							+ "\n" +
							__Q(
								"Upload speed|Up: %1 (%2) / %3",
								m::speed_to_string(session_status.upload_speed),
								m::speed_to_string(session_status.payload_upload_speed),
								m::speed_to_string(session_status.upload_rate_limit)
							)
						);
					}
				// Трей <--
			}
			catch(m::Exception& e)
			{
				if(update_flags & UPDATE_WINDOW_TITLE)
					this->set_title(this->gui->orig_window_title);

				if(update_flags & UPDATE_WIDGETS)
					this->gui->status_bar.hide();

				if(update_flags & UPDATE_TRAY)
					this->gui->tray->set_tooltip("");

				MLIB_W(EE(e));
			}
		}
	// <--
}

