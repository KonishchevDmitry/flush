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


#include <cerrno>

#include <map>
#include <string>

#include <gtkmm/actiongroup.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/stock.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/uimanager.h>

#include <mlib/gtk/misc.hpp>
#include <mlib/gtk/tree_view.hpp>
#include <mlib/fs.hpp>
#include <mlib/libtorrent.hpp>

#include "application.hpp"
#include "client_settings.hpp"
#include "daemon_proxy.hpp"
#include "gui_lib.hpp"
#include "main.hpp"
#include "torrents_view.hpp"



// Torrents_view_model_columns -->
	Torrents_view_model_columns::Torrents_view_model_columns(void)
	{
		this->set_search_column(this->name);

		this->add(this->id);

		this->add(this->status_icon_id);
		this->add(this->status_icon);

		this->add(this->name);
		this->add(this->paused);
		this->add(this->status);
		this->add(this->progress);


		this->add(this->size);
		this->add(this->size_string);

		this->add(this->requested_size);
		this->add(this->requested_size_string);

		this->add(this->downloaded_requested_size);
		this->add(this->downloaded_requested_size_string);

		this->add(this->complete_percent);
		this->add(this->complete_percent_string);


		this->add(this->total_download);
		this->add(this->total_download_string);

		this->add(this->total_payload_download);
		this->add(this->total_payload_download_string);

		this->add(this->total_upload);
		this->add(this->total_upload_string);

		this->add(this->total_payload_upload);
		this->add(this->total_payload_upload_string);

		this->add(this->total_failed);
		this->add(this->total_failed_string);

		this->add(this->total_redundant);
		this->add(this->total_redundant_string);


		this->add(this->download_speed);
		this->add(this->download_speed_string);

		this->add(this->payload_download_speed);
		this->add(this->payload_download_speed_string);

		this->add(this->upload_speed);
		this->add(this->upload_speed_string);

		this->add(this->payload_upload_speed);
		this->add(this->payload_upload_speed_string);


		this->add(this->share_ratio);
		this->add(this->share_ratio_string);

		this->add(this->peers_num);
		this->add(this->peers_num_string);

		this->add(this->seeds_num);
		this->add(this->seeds_num_string);


		this->add(this->time_added);
		this->add(this->time_added_string);

		this->add(this->time_left);
		this->add(this->time_left_string);

		this->add(this->time_seeding);
		this->add(this->time_seeding_string);
	}
// Torrents_view_model_columns <--



// Torrents_view_columns -->
	Torrents_view_columns::Torrents_view_columns(const Torrents_view_model_columns& model_columns)
	:
		status_icon("", model_columns.status_icon),

		name(_("Name"), model_columns.name),
		status(_("Status"), status_renderer),

		size(_("Size"), model_columns.size_string),
		requested_size(_Q("Requested size|Requested"), model_columns.requested_size_string),
		downloaded_requested_size(_Q("Downloaded requested size|Downloaded"), model_columns.downloaded_requested_size_string),
		complete_percent(__("%% Complete"), model_columns.complete_percent_string),

		total_download(_Q("Total download|Download"), model_columns.total_download_string),
		total_payload_download(_Q("Total download (payload)|Down data"), model_columns.total_payload_download_string),
		total_upload(_Q("Total upload|Upload"), model_columns.total_upload_string),
		total_payload_upload(_Q("Total upload (payload)|Up data"), model_columns.total_payload_upload_string),
		total_failed(_Q("Total failed|Failed"), model_columns.total_failed_string),
		total_redundant(_Q("Total redundant|Redundant"), model_columns.total_redundant_string),

		download_speed(_Q("Download speed|Down speed"), model_columns.download_speed_string),
		payload_download_speed(_Q("Download speed (payload)|Down speed (data)"), model_columns.payload_download_speed_string),
		upload_speed(_Q("Upload speed|Up speed"), model_columns.upload_speed_string),
		payload_upload_speed(_Q("Upload speed (payload)|Up speed (data)"), model_columns.payload_upload_speed_string),

		share_ratio(_Q("Share ratio|Ratio"), model_columns.share_ratio_string),
		peers_num(_("Peers"), model_columns.peers_num_string),
		seeds_num(_("Seeds"), model_columns.seeds_num_string),

		time_added(_Q("Time added|Added"), model_columns.time_added_string),
		time_left(_Q("Time left|ETA"), model_columns.time_left_string),
		time_seeding(_Q("Time seeding|Seeding"), model_columns.time_seeding_string)
	{
		this->add("status_icon", &this->status_icon, _("Status icon"), _("Status icon"), false);
		this->status_icon.set_sort_column(model_columns.status_icon_id);

		M_GTK_TREE_VIEW_ADD_STRING_COLUMN(name, _("Name"))

		this->add("status", &this->status, _("Status"));
		this->status.set_sort_column(model_columns.status);
		this->status.add_attribute(this->status_renderer.property_text(), model_columns.status);
		this->status.add_attribute(this->status_renderer.property_value(), model_columns.progress);

		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(size, _("Size"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(requested_size, _("Requested size"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(downloaded_requested_size, _("Downloaded requested size"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(complete_percent, __("%% Complete"))

		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(total_download, _("Total download"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(total_payload_download, _("Total download (payload)"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(total_upload, _("Total upload"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(total_payload_upload, _("Total upload (payload)"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(total_failed, _("Total failed"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(total_redundant, _("Total redundant"))

		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(download_speed, _("Download speed"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(payload_download_speed, _("Download speed (payload)"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(upload_speed, _("Upload speed"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(payload_upload_speed, _("Upload speed (payload)"))

		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(share_ratio, _("Share ratio"))

		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(peers_num, _("Peers"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(seeds_num, _("Seeds"))

		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(time_added, _("Time added"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(time_left, _("Time left"))
		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(time_seeding, _("Time seeding"))
	}
// Torrents_view_columns <--



// Torrents_view -->
	Torrents_view::Torrents_view(const Torrent_files_view_settings& settings)
	:
		m::gtk::Tree_view<Torrents_view_columns, Torrents_view_model_columns, Gtk::ListStore>(settings),
		last_show_zero_values_setting(false)
	{
		// Всплывающее меню -->
			Glib::RefPtr<Gtk::ActionGroup> action_group;

			this->ui_manager = Gtk::UIManager::create();

			action_group = Gtk::ActionGroup::create();
			this->resume_action = Gtk::Action::create("resume", Gtk::Stock::MEDIA_PLAY, _("Resume"));
			action_group->add(
				this->resume_action,
				sigc::bind<Torrent_process_action>( sigc::mem_fun(*this, &Torrents_view::torrents_process_callback), RESUME )
			);
			this->pause_action = Gtk::Action::create("pause", Gtk::Stock::MEDIA_PAUSE, _("Pause"));
			action_group->add(
				this->pause_action,
				sigc::bind<Torrent_process_action>( sigc::mem_fun(*this, &Torrents_view::torrents_process_callback), PAUSE )
			);
			action_group->add(
				Gtk::Action::create("remove", Gtk::Stock::REMOVE, _("Remove")),
				sigc::bind<Torrent_process_action>( sigc::mem_fun(*this, &Torrents_view::torrents_process_callback), REMOVE )
			);
			action_group->add(
				Gtk::Action::create("remove_with_data", Gtk::Stock::DELETE, _("Remove with data")),
				sigc::bind<Torrent_process_action>( sigc::mem_fun(*this, &Torrents_view::torrents_process_callback), REMOVE_WITH_DATA )
			);
			this->ui_manager->insert_action_group(action_group);

			Glib::ustring ui_info =
				"<ui>"
				"	<popup name='popup_menu'>"
				"		<menuitem action='resume'/>"
				"		<menuitem action='pause'/>"
				"		<menuitem action='remove'/>"
				"		<menuitem action='remove_with_data'/>"
				"	</popup>"
				"</ui>";

			this->ui_manager->add_ui_from_string(ui_info);
		// Всплывающее меню <--

		// Изображения, символизирующие различные статусы торрента -->
		{
			gint width;
			gint height;

			MLIB_A(gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height));

			this->status_icons[Torrent_info::TORRENT_STATUS_ICON_PAUSED] =
				this->render_icon(Gtk::Stock::MEDIA_PAUSE, Gtk::ICON_SIZE_MENU);

			this->status_icons[Torrent_info::TORRENT_STATUS_ICON_ALLOCATING] =
				this->render_icon(Gtk::Stock::HARDDISK, Gtk::ICON_SIZE_MENU);

			this->status_icons[Torrent_info::TORRENT_STATUS_ICON_CHECKING] =
				this->render_icon(Gtk::Stock::FIND, Gtk::ICON_SIZE_MENU);

			try
			{
				this->status_icons[Torrent_info::TORRENT_STATUS_ICON_STALLED_DOWNLOAD] =
					Gtk::IconTheme::get_default()->load_icon(APP_CUSTOM_ICON_STALLED_DOWNLOAD, height);
			}
			catch(Gtk::IconThemeError&)
			{
				this->status_icons[Torrent_info::TORRENT_STATUS_ICON_STALLED_DOWNLOAD] =
					this->render_icon(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_MENU);
			}

			try
			{
				this->status_icons[Torrent_info::TORRENT_STATUS_ICON_DOWNLOADING] =
					Gtk::IconTheme::get_default()->load_icon(APP_CUSTOM_ICON_DOWNLOAD, height);
			}
			catch(Gtk::IconThemeError&)
			{
				this->status_icons[Torrent_info::TORRENT_STATUS_ICON_DOWNLOADING] =
					this->render_icon(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_MENU);
			}

			this->status_icons[Torrent_info::TORRENT_STATUS_ICON_SEEDING] =
				this->render_icon(Gtk::Stock::YES, Gtk::ICON_SIZE_MENU);

			try
			{
				this->status_icons[Torrent_info::TORRENT_STATUS_ICON_UPLOADING] =
					Gtk::IconTheme::get_default()->load_icon(APP_CUSTOM_ICON_UPLOAD, height);
			}
			catch(Gtk::IconThemeError&)
			{
				this->status_icons[Torrent_info::TORRENT_STATUS_ICON_UPLOADING] =
					this->render_icon(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_MENU);
			}
		}
		// Изображения, символизирующие различные статусы торрента <--

		// Устанавливаем обработчик сигнала на изменение списка выделенных торрентов
		this->get_selection()->signal_changed().connect(sigc::mem_fun(*this, &Torrents_view::on_selection_changed_callback));

		// Устанавливаем обработчик сигнала на активацию строки TreeView.
		this->signal_row_activated().connect(sigc::mem_fun(*this, &Torrents_view::on_row_activated_callback));
	}



	Torrent_process_actions Torrents_view::get_available_actions(void)
	{
		Torrent_process_actions actions = 0;

		std::deque<Gtk::TreeModel::iterator> iters = this->get_selected_rows();

		if(iters.size())
		{
			actions |= REMOVE | REMOVE_WITH_DATA;

			for(std::deque<Gtk::TreeModel::iterator>::iterator it = iters.begin(); it < iters.end(); it++)
			{
				if( (*it)->get_value(this->model_columns.paused) )
					actions |= RESUME;
				else
					actions |= PAUSE;

				if(actions & RESUME && actions & PAUSE)
					break;
			}
		}

		return actions;
	}



	void Torrents_view::on_mouse_right_button_click(const GdkEventButton* event)
	{
		Torrent_process_actions actions = this->get_available_actions();

		// Если есть, что делать с данными торрентами
		if(actions)
		{
			// Определяем, какие элементы меню необходимо отобразить
			this->pause_action->set_visible(actions & PAUSE);
			this->resume_action->set_visible(actions & RESUME);

			// Отображаем меню
			dynamic_cast<Gtk::Menu*>(this->ui_manager->get_widget("/popup_menu"))->popup(event->button, event->time);
		}
	}



	void Torrents_view::on_row_activated_callback(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column)
	{
		Torrent_id torrent_id = Torrent_id( this->model->get_iter(path)->get_value(this->model_columns.id) );
		std::string torrent_name = this->model->get_iter(path)->get_value(this->model_columns.name);

		std::vector<Torrent_file> files;
		std::string torrent_download_path;

		try
		{
			torrent_download_path = get_daemon_proxy().get_torrent_download_path(torrent_id);

			std::vector<Torrent_file_status> statuses;
			get_daemon_proxy().get_torrent_files_info(torrent_id, &files, &statuses, INIT_REVISION);
		}
		catch(m::Exception& e)
		{
			MLIB_W(__(
				"Opening torrent '%1' file(s) failed. %2",
				torrent_name, EE(e)
			));
		}

		if(!files.empty())
		{
			// Получаем путь любого файла торрента без начального "/".
			std::string file_path = files[0].path.substr(1);

			get_application().open_file(
				Path(torrent_download_path) / file_path.substr(0, file_path.find('/'))
			);
		}
	}



	void Torrents_view::on_selection_changed_callback(void)
	{
		Glib::RefPtr<Gtk::TreeSelection> selection = this->get_selection();

		if(selection->count_selected_rows() == 1)
		{
			this->torrent_selected_signal(
				Torrent_id(
					this->model->get_iter(
						*( selection->get_selected_rows().begin() )
					)->get_value(this->model_columns.id)
				)
			);
		}
		else
			this->torrent_selected_signal(Torrent_id());
	}



	void Torrents_view::process_torrents(Torrent_process_action action)
	{
		// Запрашиваем подтверждение у пользователя, если это необходимо -->
			switch(action)
			{
				case REMOVE:
					if(
						ok_cancel_dialog(
							_("Remove torrent(s)"),
							_("Are you sure want to remove selected torrent(s)?")
						) != m::gtk::RESPONSE_OK
					)
						return;
					break;

				case REMOVE_WITH_DATA:
					if(
						ok_cancel_dialog(
							_("Remove torrent(s) with data"),
							_("Are you sure want to remove selected torrent(s) with data?")
						) != m::gtk::RESPONSE_OK
					)
						return;
					break;

				case PAUSE:
				case RESUME:
					break;

				default:
					MLIB_LE();
					break;
			}
		// Запрашиваем подтверждение у пользователя, если это необходимо <--

		// Получаем список выделенных строк
		std::deque<Gtk::TreeModel::iterator> iters = this->get_selected_rows();

		// Получаем идентификаторы выделенных торрентов и отправляем их на обработку -->
			if(iters.size())
			{
				std::vector<Torrent_id> torrents_ids;
				torrents_ids.reserve(iters.size());

				for(std::deque<Gtk::TreeModel::iterator>::iterator it = iters.begin(); it < iters.end(); it++)
					torrents_ids.push_back( Torrent_id( (*it)->get_value(this->model_columns.id) ) );

				// Если торренты удаляются, то необходимо снять
				// выделение, т. к. при обновлении модели при
				// удалениии из нее каждой строки будет генерироваться
				// сигнал на изменение выделения. Если на удаление будет
				// отправлено несколько торрентов, то все они уже будут
				// удалены, но в модели еще останутся, а, следовательно,
				// каждый сигнал будет обновлять текущий Torrent_info_widget
				// для торрента, которого уже не существует, что вызовет
				// ошибку.
				// -->
					switch(action)
					{
						case REMOVE:
						case REMOVE_WITH_DATA:
							this->get_selection()->unselect_all();
							break;

						default:
							break;
					}
				// <--

				try
				{
					get_daemon_proxy().process_torrents(torrents_ids, action);
				}
				catch(m::Exception& e)
				{
					switch(action)
					{
						case REMOVE:
							MLIB_W(_("Removing torrent(s) failed."), __("Removing torrent(s) failed. %1", EE(e)));
							break;

						case REMOVE_WITH_DATA:
							MLIB_W(_("Removing torrent(s) with data failed."), __("Removing torrent(s) with data failed. %1", EE(e)));
							break;

						case PAUSE:
							MLIB_W(_("Pausing torrent(s) failed."), __("Pausing torrent(s) failed. %1", EE(e)));
							break;

						case RESUME:
							MLIB_W(_("Resuming torrent(s) failed."), __("Resuming torrent(s) failed. %1", EE(e)));
							break;

						default:
							MLIB_LE();
							break;
					}
				}

				// Нужно для того, чтобы:
				// 1) Пользователь сразу увидел внесенные им изменения.
				// 2) Если интервал обновления маленький и пользователь
				//    удалит торрент и тут же щелкнет по нему (еще
				//    неудалившемуся из модели), то программа выдаст
				//    warning, что такого торрента не существует.
				update_gui();
			}
		// Получаем идентификаторы выделенных торрентов и отправляем их на обработку <--
	}



	void Torrents_view::torrents_process_callback(Torrent_process_action action)
	{
		this->process_torrents(action);
	}



	void Torrents_view::save_settings(Torrents_view_settings& settings) const
	{
		m::gtk::Tree_view<Torrents_view_columns, Torrents_view_model_columns, Gtk::ListStore>::save_settings(settings);
	}



	void Torrents_view::update(std::vector<Torrent_info>::iterator infos_it, const std::vector<Torrent_info>::iterator& infos_end_it)
	{
		Gtk::TreeModel::iterator model_iter;
		std::map<Torrent_id, Torrent_info>::iterator info_map_iter;

		bool show_zero_values = get_client_settings().gui.show_zero_values;
		bool show_zero_setting_changed = show_zero_values != this->last_show_zero_values_setting;


		this->last_show_zero_values_setting = show_zero_values;

		// Создаем таблицу с информацией о торрентах -->
			std::map<Torrent_id, Torrent_info> info_map;

			for(; infos_it != infos_end_it; infos_it++)
				info_map.insert(std::pair<Torrent_id, Torrent_info>(infos_it->id, *infos_it));
		// Создаем таблицу с информацией о торрентах <--

		// Входим в режим редактирования
		this->editing_start();

		// Обновляем существующие строки -->
		{
			Gtk::TreeModel::Children rows = this->model->children();

			model_iter = rows.begin();
			while(model_iter)
			{
				info_map_iter = info_map.find(Torrent_id(model_iter->get_value(this->model_columns.id)));

				// Торрента с таким ID уже нет
				if(info_map_iter == info_map.end())
				{
					model_iter = this->model->erase(model_iter);
					continue;
				}
				else
				{
					// Обновляем старые значения
					this->update_row(model_iter, info_map_iter->second, false, show_zero_setting_changed, show_zero_values);

					// Удаляем из таблицы обработанную информацию
					info_map.erase(info_map_iter);
				}

				model_iter++;
			}
		}
		// Обновляем существующие строки <--

		// Теперь в таблице остались только те торренты, которые еще не присутствуют в списке.
		// Добавляем их.
		// -->
			for(info_map_iter = info_map.begin(); info_map_iter != info_map.end(); info_map_iter++)
			{
				model_iter = this->model->append();
				this->update_row(model_iter, info_map_iter->second, true, true, show_zero_values);
			}
		// <--

		// Выходим из режима редактирования
		this->editing_end();
	}



	void Torrents_view::update_row(Gtk::TreeModel::iterator &iter, const Torrent_info& torrent_info, bool force_update, bool zeros_force_update, bool show_zero_values)
	{
		#define set_size_value(id, zero_independent)													\
		{																								\
			bool real_force_update = (zero_independent) ? force_update : zeros_force_update;			\
			bool real_show_zero_values = (zero_independent) ? true : show_zero_values;					\
																										\
			if(m::gtk::update_row(row, this->model_columns.id, torrent_info.id) || real_force_update)	\
			{																							\
				m::gtk::update_row(																		\
					row, this->model_columns.id ## _string,												\
					m::size_to_string(torrent_info.id, real_show_zero_values)							\
				);																						\
			}																							\
		}

		#define set_speed_value(id, zero_independent)													\
		{																								\
			bool real_force_update = (zero_independent) ? force_update : zeros_force_update;			\
			bool real_show_zero_values = (zero_independent) ? true : show_zero_values;					\
																										\
			if(m::gtk::update_row(row, this->model_columns.id, torrent_info.id) || real_force_update)	\
			{																							\
				m::gtk::update_row(																		\
					row, this->model_columns.id ## _string,												\
					m::speed_to_string(torrent_info.id, real_show_zero_values)							\
				);																						\
			}																							\
		}

		#define set_int_value(id, zero_independent)														\
		{																								\
			bool real_force_update = (zero_independent) ? force_update : zeros_force_update;			\
			bool real_show_zero_values = (zero_independent) ? true : show_zero_values;					\
																										\
			if(m::gtk::update_row(row, this->model_columns.id, torrent_info.id) || real_force_update)	\
			{																							\
				if(real_show_zero_values || torrent_info.id)											\
				{																						\
					m::gtk::update_row(																	\
						row, this->model_columns.id ## _string,											\
						m::to_string(torrent_info.id)													\
					);																					\
				}																						\
				else																					\
					m::gtk::update_row(row, this->model_columns.id ## _string, "");						\
			}																							\
		}


		Gtk::TreeRow row = *iter;

		// Status icon -->
		{
			Torrent_info::Status_icon_id status_icon_id = torrent_info.get_status_icon_id();

			if(m::gtk::update_row(row, this->model_columns.status_icon_id, status_icon_id) || force_update)
				row[this->model_columns.status_icon] = this->status_icons[status_icon_id];
		}
		// Status icon <--

		m::gtk::update_row(row, this->model_columns.id, torrent_info.id);
		m::gtk::update_row(row, this->model_columns.name, torrent_info.name);
		m::gtk::update_row(row, this->model_columns.paused, torrent_info.paused);


		m::gtk::update_row(
			row, this->model_columns.status,
			torrent_info.get_status_string() + " " + m::to_string(torrent_info.progress) + " %"
		);
		m::gtk::update_row(row, this->model_columns.progress, torrent_info.progress);


		set_size_value(size, true)
		set_size_value(requested_size, true)
		set_size_value(downloaded_requested_size, true)

		// Complete percent -->
		{
			int complete_percent = torrent_info.get_complete_percent();

			if(m::gtk::update_row(row, this->model_columns.complete_percent, complete_percent) || force_update)
				m::gtk::update_row(row, this->model_columns.complete_percent_string, m::to_string(complete_percent) + " %");
		}
		// Complete percent <--


		set_size_value(total_download, false)
		set_size_value(total_payload_download, false)
		set_size_value(total_upload, false)
		set_size_value(total_payload_upload, false)

		set_size_value(total_failed, false)
		set_size_value(total_redundant, false)


		set_speed_value(download_speed, false)
		set_speed_value(payload_download_speed, false)
		set_speed_value(upload_speed, false)
		set_speed_value(payload_upload_speed, false)


		// Share ratio -->
		{
			Share_ratio share_ratio = torrent_info.get_share_ratio();

			if(m::gtk::update_row(row, this->model_columns.share_ratio, share_ratio) || zeros_force_update)
				m::gtk::update_row(row, this->model_columns.share_ratio_string, get_share_ratio_string(share_ratio, show_zero_values));
		}
		// Share ratio <--


		set_int_value(peers_num, false)
		set_int_value(seeds_num, false)


		// Time added
		if(m::gtk::update_row(row, this->model_columns.time_added, torrent_info.time_added) || force_update)
			m::gtk::update_row(row, this->model_columns.time_added_string, m::time_to_string_with_date(torrent_info.time_added));

		
		// Time left -->
		{
			Time time_left = torrent_info.get_time_left();

			if(m::gtk::update_row(row, this->model_columns.time_left, time_left) || zeros_force_update)
				m::gtk::update_row(row, this->model_columns.time_left_string, m::get_time_left_string(time_left, show_zero_values));
		}
		// Time left <--


		// Time seeding -->
			if(m::gtk::update_row(row, this->model_columns.time_seeding, torrent_info.time_seeding) || zeros_force_update)
			{
				m::gtk::update_row(
					row, this->model_columns.time_seeding_string,
					m::get_time_duration_string(torrent_info.time_seeding, show_zero_values)
				);
			}
		// Time seeding <--

		#undef set_int_value
		#undef set_speed_value
		#undef set_size_value
	}
// Torrents_view <--

