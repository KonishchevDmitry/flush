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


#include <string>
#include <deque>

#include <libconfig.h++>

#include <mlib/gtk/toolbar.hpp>
#include <mlib/fs.hpp>
#include <mlib/libconfig.hpp>

#include "categories_view.hpp"
#include "client_settings.hpp"
#include "common.hpp"


#define CHECK_OPTION_TYPE(setting, type, action)																		\
{																														\
	if(setting.getType() != type)																						\
	{																													\
		MLIB_SW(__("Client config: Bad option '%1' type at line %2.", setting.getName(), setting.getSourceLine()));	\
		action;																											\
	}																													\
}



namespace
{

/// Выводит отладочное сообщение о неверном значении опции.
template<class T>
void bad_option_value(const libconfig::Setting& setting, T value);

/// Выводит отладочное сообщение о неверной кодировке, в которой
/// представлено значение опции.
void invalid_option_utf_value(const libconfig::Setting& setting);

/// Выводит отладочное сообщение о неизвестной опции.
void unknown_option(const libconfig::Setting& setting);



template<class T>
void bad_option_value(const libconfig::Setting& setting, T value)
{
	MLIB_SW(__(
		"Client config: Bad option '%1' value '%2' at line %3.",
		setting.getName(), value, setting.getSourceLine()
	));
}



void invalid_option_utf_value(const libconfig::Setting& setting)
{
	MLIB_SW(__(
		"Client config: Invalid option '%1' UTF-8 value at line %2.",
		setting.getName(), setting.getSourceLine()
	));
}



void unknown_option(const libconfig::Setting& setting)
{
	MLIB_SW(__(
		"Client config: Unknown option '%1' at line %2.",
		setting.getName(), setting.getSourceLine()
	));
}

}



namespace config
{

class Config: private m::Virtual
{
	public:
		virtual void	read(const libconfig::Setting& root) = 0;
		virtual void	write(libconfig::Setting& root) const = 0;
};



class Categories_view: public ::Categories_view_settings, public Config
{
	public:
		virtual void	read(const libconfig::Setting& root);
		virtual void	write(libconfig::Setting& root) const;
};



template <typename smart_ptr>
const Config*	get(const smart_ptr& ptr);

template <typename smart_ptr>
Config*			get(smart_ptr& ptr);



template <typename smart_ptr>
const Config* get(const smart_ptr& ptr)
{
	const Config* config = dynamic_cast<const Config*>(ptr.get());
	MLIB_A(config);
	return config;
}



template <typename smart_ptr>
Config* get(smart_ptr& ptr)
{
	Config* config = dynamic_cast<Config*>(ptr.get());
	MLIB_A(config);
	return config;
}



// Categories_view -->
	void Categories_view::read(const libconfig::Setting& root)
	{
		for(int setting_id = 0; setting_id < root.getLength(); setting_id++)
		{
			const libconfig::Setting& setting = root[setting_id];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "visible"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->visible = setting;
			}
			else if(m::is_eq(setting_name, "show_names"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->show_names = setting;
			}
			else if(m::is_eq(setting_name, "show_counters"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->show_counters = setting;
			}
			else if(m::is_eq(setting_name, "selected_items"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeArray, continue)
				this->selected_items.clear();

				if(setting.getLength())
				{
					CHECK_OPTION_TYPE(setting[0], libconfig::Setting::TypeString, continue)
					this->selected_items.reserve(setting.getLength());

					for(int i = 0; i < setting.getLength(); i++)
						this->selected_items.push_back(static_cast<const char*>(setting[i]));
				}
			}
			else
				unknown_option(setting);
		}
	}



	void Categories_view::write(libconfig::Setting& root) const
	{
		root.add("visible", libconfig::Setting::TypeBoolean) = this->visible;
		root.add("show_names", libconfig::Setting::TypeBoolean) = this->show_names;
		root.add("show_counters", libconfig::Setting::TypeBoolean) = this->show_counters;

		{
			libconfig::Setting& setting = root.add("selected_items", libconfig::Setting::TypeArray);

			M_FOR_CONST_IT(this->selected_items, it)
				setting.add(libconfig::Setting::TypeString) = *it;
		}
	}
// Categories_view_config <--

}


// Tree_view_settings -->
	void Tree_view_settings::read_column_config(const libconfig::Setting& config_root, m::gtk::Tree_view_column_settings& column)
	{
		column.name = config_root.getName();

		// Для совместимости с версиями < 0.4
		// -->
			COMPATIBILITY

			if(column.name == "download_payload_speed")
				column.name = "payload_download_speed";
			else if(column.name == "upload_payload_speed")
				column.name = "payload_upload_speed";
		// <--

		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "visible"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				column.visible = setting;
			}
			else if(m::is_eq(setting_name, "width"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeInt, continue)
				column.set_width(setting);
			}
			else
				unknown_option(setting);
		}
	}



	void Tree_view_settings::write_column_config(libconfig::Setting& config_root, const m::gtk::Tree_view_column_settings& column) const
	{
		libconfig::Setting& setting = config_root.add(column.name, libconfig::Setting::TypeGroup);
		setting.add("visible", libconfig::Setting::TypeBoolean) = column.visible;
		setting.add("width", libconfig::Setting::TypeInt) = column.width;
	}



	void Tree_view_settings::read_config(const libconfig::Setting& config_root)
	{
		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "sort_column"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)
				this->sort_column = static_cast<const char*>(setting);
			}
			else if(m::is_eq(setting_name, "sort_order"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				if(std::string(static_cast<const char*>(setting)) == "ascending")
					this->sort_order = SORT_ORDER_ASCENDING;
				else if(std::string(static_cast<const char*>(setting)) == "descending")
					this->sort_order = SORT_ORDER_DESCENDING;
				else
					bad_option_value(setting, static_cast<const char*>(setting));
			}
			else if(m::is_eq(setting_name, "columns"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)

				for(int column_id = 0; column_id < setting.getLength(); column_id++)
				{
					CHECK_OPTION_TYPE(setting[column_id], libconfig::Setting::TypeGroup, continue)

					m::gtk::Tree_view_column_settings column;
					this->read_column_config(setting[column_id], column);
					this->columns.push_back(column);
				}
			}
			else
				unknown_option(setting);
		}
	}



	void Tree_view_settings::write_config(libconfig::Setting& config_root) const
	{
		if(this->sort_column != "")
		{
			config_root.add("sort_column", libconfig::Setting::TypeString) = this->sort_column;

			if(this->sort_order == SORT_ORDER_ASCENDING)
				config_root.add("sort_order", libconfig::Setting::TypeString) = "ascending";
			else
				config_root.add("sort_order", libconfig::Setting::TypeString) = "descending";
		}

		libconfig::Setting& setting = config_root.add("columns", libconfig::Setting::TypeGroup);
		M_FOR_CONST_IT(this->columns, it)
			this->write_column_config(setting, *it);
	}
// Tree_view_settings <--



// Paned_settings -->
	void Paned_settings::read_config(const libconfig::Setting& config_root)
	{
		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "position"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeInt, continue)

				if(static_cast<int>(setting) < 0)
					bad_option_value(setting, static_cast<int>(setting));
				else
					this->position = setting;
			}
			else
				unknown_option(setting);
		}
	}



	void Paned_settings::write_config(libconfig::Setting& config_root) const
	{
		config_root.add("position", libconfig::Setting::TypeInt) = this->position;
	}
// Paned_settings <--



// Window_settings -->
	void Window_settings::read_config(const libconfig::Setting& config_root)
	{
		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "width"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeInt, continue)
				this->width = setting;
			}
			else if(m::is_eq(setting_name, "height"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeInt, continue)
				this->height = setting;
			}
			// Совместимость с версиями <= 0.3, в которых присутствовала
			// опечатка в имени опции.
			COMPATIBILITY
			else if(m::is_eq(setting_name, "heigth"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeInt, continue)
				this->height = setting;
			}
			else
				unknown_option(setting);
		}
	}



	void Window_settings::write_config(libconfig::Setting& config_root) const
	{
		if(this->width > 0 && this->height > 0)
		{
			config_root.add("width", libconfig::Setting::TypeInt) = this->width;
			config_root.add("height", libconfig::Setting::TypeInt) = this->height;
		}
	}
// Window_settings <--



// Torrents_viewport_settings -->
	Torrents_viewport_settings::Torrents_viewport_settings(void)
	:
		categories_view(new config::Categories_view)
	{
	}



	Torrents_viewport_settings::~Torrents_viewport_settings(void)
	{
		// Для работы умных указателей.
	}



	void Torrents_viewport_settings::read_config(const libconfig::Setting& config_root, Version client_version)
	{
		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "info_widget"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)
				this->info_widget = static_cast<const char *>(setting);
			}
			else if(m::is_eq(setting_name, "torrents_view_and_torrent_infos_vpaned"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->torrents_view_and_torrent_infos_vpaned.read_config(setting);
			}
			else if(m::is_eq(setting_name, "categories_view"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				config::get(this->categories_view)->read(setting);
			}
			else if(m::is_eq(setting_name, "torrents_view"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)

				// В версиях < 0.5 не было колонки с изображением текущего
				// статуса, но по умолчанию, она должна появиться самой первой
				// (иначе будет очень некрасиво смотреться).
				// -->
					COMPATIBILITY
					if(client_version < M_GET_VERSION(0, 5, 0))
					{
						this->torrents_view.columns.push_back(
							m::gtk::Tree_view_column_settings("status_icon")
						);
					}
				// <--

				this->torrents_view.read_config(setting);
			}
			else if(m::is_eq(setting_name, "torrent_files_view"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->torrent_files_view.read_config(setting);
			}
			else if(m::is_eq(setting_name, "torrent_peers_view"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->torrent_peers_view.read_config(setting);
			}
			else
				unknown_option(setting);
		}
	}



	void Torrents_viewport_settings::write_config(libconfig::Setting& config_root) const
	{
		config_root.add("info_widget", libconfig::Setting::TypeString) = this->info_widget;

		this->torrents_view_and_torrent_infos_vpaned.write_config(
			config_root.add("torrents_view_and_torrent_infos_vpaned", libconfig::Setting::TypeGroup)
		);

		config::get(this->categories_view)->write(
			config_root.add("categories_view", libconfig::Setting::TypeGroup)
		);

		this->torrents_view.write_config(
			config_root.add("torrents_view", libconfig::Setting::TypeGroup)
		);

		this->torrent_files_view.write_config(
			config_root.add("torrent_files_view", libconfig::Setting::TypeGroup)
		);

		this->torrent_peers_view.write_config(
			config_root.add("torrent_peers_view", libconfig::Setting::TypeGroup)
		);
	}
// Torrents_viewport_settings <--



// Main_window_settings -->
	void Main_window_settings::read_config(const libconfig::Setting& config_root, Version client_version)
	{
		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "window"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->window.read_config(setting);
			}
			else if(m::is_eq(setting_name, "torrents_viewport"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->torrents_viewport.read_config(setting, client_version);
			}
			else if(m::is_eq(setting_name, "status_bar"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->status_bar.read_config(setting);
			}
			else
				unknown_option(setting);
		}
	}



	void Main_window_settings::write_config(libconfig::Setting& config_root) const
	{
		this->window.write_config(
			config_root.add("window", libconfig::Setting::TypeGroup)
		);
		this->torrents_viewport.write_config(
			config_root.add("torrents_viewport", libconfig::Setting::TypeGroup)
		);
		this->status_bar.write_config(
			config_root.add("status_bar", libconfig::Setting::TypeGroup)
		);
	}
// Main_window_settings <--



// Add_torrent_dialog_settings -->
	void Add_torrent_dialog_settings::read_config(const libconfig::Setting& config_root)
	{
		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "window"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->window.read_config(setting);
			}
			else if(m::is_eq(setting_name, "torrent_files_view"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->torrent_files_view.read_config(setting);
			}
			else
				unknown_option(setting);
		}
	}



	void Add_torrent_dialog_settings::write_config(libconfig::Setting& config_root) const
	{
		this->window.write_config(
			config_root.add("window", libconfig::Setting::TypeGroup)
		);
		this->torrent_files_view.write_config(
			config_root.add("torrent_files_view", libconfig::Setting::TypeGroup)
		);
	}
// Add_torrent_dialog_settings <--



// Create_torrent_dialog_settings -->
	void Create_torrent_dialog_settings::read_config(const libconfig::Setting& config_root)
	{
		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "window"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->window.read_config(setting);
			}
			else if(m::is_eq(setting_name, "get_from"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				if(m::is_valid_utf(static_cast<const char *>(setting)))
				{
					this->get_from = static_cast<const char *>(setting);

					if(!Path(this->get_from).is_absolute())
					{
						bad_option_value(setting, this->get_from);
						this->get_from = "";
					}
				}
				else
					invalid_option_utf_value(setting);
			}
			else if(m::is_eq(setting_name, "save_to"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				if(m::is_valid_utf(static_cast<const char *>(setting)))
				{
					this->save_to = static_cast<const char *>(setting);

					if(!Path(this->save_to).is_absolute())
					{
						bad_option_value(setting, this->save_to);
						this->save_to = "";
					}
				}
				else
					invalid_option_utf_value(setting);
			}
			else
				unknown_option(setting);
		}
	}



	void Create_torrent_dialog_settings::write_config(libconfig::Setting& config_root) const
	{
		this->window.write_config(
			config_root.add("window", libconfig::Setting::TypeGroup)
		);

		if(this->get_from != "")
			config_root.add("get_from", libconfig::Setting::TypeString) = this->get_from;

		if(this->save_to != "")
			config_root.add("save_to", libconfig::Setting::TypeString) = this->save_to;
	}
// Create_torrent_dialog_settings <--



// Gui_settings -->
	Gui_settings::Gui_settings(void)
	:
		show_speed_in_window_title(false),
		show_zero_values(false),

		show_toolbar(true),
		toolbar_style(m::gtk::toolbar::DEFAULT),

		show_tray_icon(true),
		hide_app_to_tray_at_startup(true),
		minimize_to_tray(true),
		close_to_tray(true),

		update_interval(1000),
		max_log_lines(100),
		show_add_torrent_dialog(true)
	{
	}



	void Gui_settings::read_config(const libconfig::Setting& config_root, Version client_version)
	{
		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "show_speed_in_window_title"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->show_speed_in_window_title = setting;
			}
			else if(m::is_eq(setting_name, "show_zero_values"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->show_zero_values = setting;
			}
			else if(m::is_eq(setting_name, "show_toolbar"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->show_toolbar = setting;
			}
			else if(m::is_eq(setting_name, "toolbar_style"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				try
				{
					this->toolbar_style = m::gtk::toolbar::get_style_from_string(
						static_cast<const char *>(setting)
					);
				}
				catch(m::Exception& e)
				{
					bad_option_value(setting, EE(e));
				}
			}
			else if(m::is_eq(setting_name, "show_tray_icon"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->show_tray_icon = setting;
			}
			else if(m::is_eq(setting_name, "hide_main_window_to_tray_at_startup"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->hide_app_to_tray_at_startup = setting;
			}
			else if(m::is_eq(setting_name, "minimize_to_tray"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->minimize_to_tray = setting;
			}
			else if(m::is_eq(setting_name, "close_to_tray"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->close_to_tray = setting;
			}
			else if(m::is_eq(setting_name, "update_interval"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeInt, continue)
				this->update_interval = setting;

				if(this->update_interval < GUI_MIN_UPDATE_INTERVAL)
				{
					bad_option_value(setting, static_cast<int>(setting));
					this->update_interval = GUI_MIN_UPDATE_INTERVAL;
				}
			}
			else if(m::is_eq(setting_name, "max_log_lines"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeInt, continue)
				this->max_log_lines = setting;
			}
			else if(m::is_eq(setting_name, "main_window"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->main_window.read_config(setting, client_version);
			}
			else if(m::is_eq(setting_name, "open_torrents_from"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				if(m::is_valid_utf(static_cast<const char *>(setting)))
				{
					this->open_torrents_from = static_cast<const char *>(setting);

					if(!Path(this->open_torrents_from).is_absolute())
					{
						bad_option_value(setting, this->open_torrents_from);
						this->open_torrents_from = "";
					}
				}
				else
					invalid_option_utf_value(setting);
			}
			else if(m::is_eq(setting_name, "show_add_torrent_dialog"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->show_add_torrent_dialog = setting;
			}
			else if(m::is_eq(setting_name, "add_torrent_dialog"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->add_torrent_dialog.read_config(setting);
			}
			else if(m::is_eq(setting_name, "create_torrent_dialog"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->create_torrent_dialog.read_config(setting);
			}
			else
				unknown_option(setting);
		}
	}



	void Gui_settings::write_config(libconfig::Setting& config_root) const
	{
		config_root.add("show_speed_in_window_title", libconfig::Setting::TypeBoolean) = this->show_speed_in_window_title;
		config_root.add("show_zero_values", libconfig::Setting::TypeBoolean) = this->show_zero_values;

		config_root.add("show_toolbar", libconfig::Setting::TypeBoolean) = this->show_toolbar;
		config_root.add("toolbar_style", libconfig::Setting::TypeString) = m::gtk::toolbar::get_style_string_representation(this->toolbar_style);

		config_root.add("show_tray_icon", libconfig::Setting::TypeBoolean) = this->show_tray_icon;
		config_root.add("hide_main_window_to_tray_at_startup", libconfig::Setting::TypeBoolean) = this->hide_app_to_tray_at_startup;
		config_root.add("minimize_to_tray", libconfig::Setting::TypeBoolean) = this->minimize_to_tray;
		config_root.add("close_to_tray", libconfig::Setting::TypeBoolean) = this->close_to_tray;

		config_root.add("update_interval", libconfig::Setting::TypeInt) = this->update_interval;
		config_root.add("max_log_lines", libconfig::Setting::TypeInt) = this->max_log_lines;

		this->main_window.write_config(
			config_root.add("main_window", libconfig::Setting::TypeGroup)
		);

		if(this->open_torrents_from != "")
			config_root.add("open_torrents_from", libconfig::Setting::TypeString) = this->open_torrents_from;

		config_root.add("show_add_torrent_dialog", libconfig::Setting::TypeBoolean) = this->show_add_torrent_dialog;
		this->add_torrent_dialog.write_config(
			config_root.add("add_torrent_dialog", libconfig::Setting::TypeGroup)
		);

		this->create_torrent_dialog.write_config(
			config_root.add("create_torrent_dialog", libconfig::Setting::TypeGroup)
		);
	}
// Gui_settings <--



// Status_bar_settings -->
	Status_bar_settings::Status_bar_settings(void)
	:
		download_speed(true),
		payload_download_speed(false),

		upload_speed(true),
		payload_upload_speed(false),

		download(true),
		payload_download(false),

		upload(true),
		payload_upload(false),

		share_ratio(true),
		failed(false),
		redundant(false)
	{
	}



	void Status_bar_settings::read_config(const libconfig::Setting& config_root)
	{
		config_root.lookupValue("download_speed", this->download_speed);
		config_root.lookupValue("payload_download_speed", this->payload_download_speed);

		config_root.lookupValue("upload_speed", this->upload_speed);
		config_root.lookupValue("payload_upload_speed", this->payload_upload_speed);

		config_root.lookupValue("download", this->download);
		config_root.lookupValue("payload_download", this->payload_download);

		config_root.lookupValue("upload", this->upload);
		config_root.lookupValue("payload_upload", this->payload_upload);

		config_root.lookupValue("share_ratio",  this->share_ratio);
		config_root.lookupValue("failed",  this->failed);
		config_root.lookupValue("redundant", this->redundant);
	}



	void Status_bar_settings::write_config(libconfig::Setting& config_root) const
	{
		config_root.add("download_speed", libconfig::Setting::TypeBoolean) = this->download_speed;
		config_root.add("payload_download_speed", libconfig::Setting::TypeBoolean) = this->payload_download_speed;

		config_root.add("upload_speed", libconfig::Setting::TypeBoolean) = this->upload_speed;
		config_root.add("payload_upload_speed", libconfig::Setting::TypeBoolean) = this->payload_upload_speed;

		config_root.add("download", libconfig::Setting::TypeBoolean) = this->download;
		config_root.add("payload_download", libconfig::Setting::TypeBoolean) = this->payload_download;

		config_root.add("upload", libconfig::Setting::TypeBoolean) = this->upload;
		config_root.add("payload_upload", libconfig::Setting::TypeBoolean) = this->payload_upload;

		config_root.add("share_ratio", libconfig::Setting::TypeBoolean) = this->share_ratio;
		config_root.add("failed", libconfig::Setting::TypeBoolean) = this->failed;
		config_root.add("redundant", libconfig::Setting::TypeBoolean) = this->redundant;
	}
// Status_bar_settings <--



// User_settings -->
	User_settings::User_settings(void)
	:
		start_torrent_on_adding(true),

		download_to(Path(m::fs::get_user_home_path()) / "downloads"),
		copy_finished_to(""),

		open_command("gnome-open"),

		temporary_action_last_time_is_predefined(true),
		temporary_action_last_time(30 * 60)
	{
	}



	// Т. к. раньше конфигурационный файл писался в кодировке локали.
	COMPATIBILITY
	void User_settings::read_config(const libconfig::Setting& config_root, Version client_version)
	{
		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "start_torrent_on_adding"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->start_torrent_on_adding = setting;
			}
			else if(m::is_eq(setting_name, "download_to"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				// Т. к. раньше конфигурационный файл писался в кодировке локали.
				COMPATIBILITY
				if(client_version < M_GET_VERSION(0, 4, 0))
					this->download_to = L2U(static_cast<const char*>(setting));
				else
				{
					if(m::is_valid_utf(static_cast<const char *>(setting)))
					{
						std::string path = static_cast<const char *>(setting);

						if(Path(path).is_absolute())
							this->download_to = path;
						else
							bad_option_value(setting, path);
					}
					else
						invalid_option_utf_value(setting);
				}
			}
			else if(m::is_eq(setting_name, "copy_finished_to"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				// Т. к. раньше конфигурационный файл писался в кодировке локали.
				COMPATIBILITY
				if(client_version < M_GET_VERSION(0, 4, 0))
					this->copy_finished_to = L2U(static_cast<const char*>(setting));
				else
				{
					if(m::is_valid_utf(static_cast<const char *>(setting)))
					{
						this->copy_finished_to = static_cast<const char*>(setting);

						if(!Path(this->copy_finished_to).is_absolute())
						{
							bad_option_value(setting, this->copy_finished_to);
							this->copy_finished_to = "";
						}
					}
					else
						invalid_option_utf_value(setting);
				}
			}
			else if(m::is_eq(setting_name, "open_command"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeString, continue)

				// Т. к. раньше конфигурационный файл писался в кодировке локали.
				COMPATIBILITY
				if(client_version < M_GET_VERSION(0, 4, 0))
					this->open_command = L2U(static_cast<const char*>(setting));
				else
				{
					if(m::is_valid_utf(static_cast<const char *>(setting)))
						this->open_command = static_cast<const char*>(setting);
					else
						invalid_option_utf_value(setting);
				}
			}
			else if(m::is_eq(setting_name, "temporary_action_last_time_is_predefined"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeBoolean, continue)
				this->temporary_action_last_time_is_predefined = setting;
			}
			else if(m::is_eq(setting_name, "temporary_action_last_time"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Time_type, continue)
				this->temporary_action_last_time = static_cast<m::libconfig::Time>(setting);
			}
			else
				unknown_option(setting);
		}
	}



	void User_settings::write_config(libconfig::Setting& config_root) const
	{
		config_root.add("start_torrent_on_adding", libconfig::Setting::TypeBoolean) = this->start_torrent_on_adding;

		config_root.add("download_to", libconfig::Setting::TypeString) = this->download_to;
		if(this->copy_finished_to != "")
			config_root.add("copy_finished_to", libconfig::Setting::TypeString) = this->copy_finished_to;

		config_root.add("open_command", libconfig::Setting::TypeString) = this->open_command;

		config_root.add("temporary_action_last_time_is_predefined", libconfig::Setting::TypeBoolean)
			= this->temporary_action_last_time_is_predefined;
		config_root.add("temporary_action_last_time", m::libconfig::Time_type)
			= static_cast<m::libconfig::Time>(this->temporary_action_last_time);
	}
// User_settings <--



// Client_settings -->
	void Client_settings::read_config(const std::string& config_path)
	{
		libconfig::Config config;
		std::string real_config_path = config_path;

		try
		{
			real_config_path = m::fs::config::start_reading(config_path);

			if(m::fs::is_exists(real_config_path))
				config.readFile(U2L(real_config_path).c_str());
		}
		catch(m::Exception& e)
		{
			M_THROW(__(
				"Reading configuration file '%1' failed. %2",
				m::fs::get_abs_path_lazy(config_path), EE(e)
			));
		}
		catch(libconfig::FileIOException& e)
		{
			M_THROW(__("Can't read configuration file '%1': %2.", m::fs::get_abs_path_lazy(real_config_path), EE(e)));
		}
		catch(libconfig::ParseException& e)
		{
			M_THROW(__("Can't parse configuration file '%1': %2.", m::fs::get_abs_path_lazy(real_config_path), EE(e)));
		}

		Version client_version = M_GET_VERSION(0, 0, 0);
		const libconfig::Setting& config_root = config.getRoot();

		// Получаем версию клиента, который производил запись конфига -->
			try
			{
				const libconfig::Setting& setting = config_root["version"];

				CHECK_OPTION_TYPE(setting, m::libconfig::Version_type, M_THROW_EMPTY())
				client_version = static_cast<m::libconfig::Version>(setting);
			}
			catch(m::Exception&)
			{
			}
			catch(libconfig::SettingNotFoundException&)
			{
			}
		// Получаем версию клиента, который производил запись конфига <--

		for(int i = 0; i < config_root.getLength(); i++)
		{
			const libconfig::Setting& setting = config_root[i];
			const char* setting_name = setting.getName();

			if(m::is_eq(setting_name, "version"))
			{
				CHECK_OPTION_TYPE(setting, m::libconfig::Version_type, continue)
			}
			else if(m::is_eq(setting_name, "gui"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->gui.read_config(setting, client_version);
			}
			else if(m::is_eq(setting_name, "user"))
			{
				CHECK_OPTION_TYPE(setting, libconfig::Setting::TypeGroup, continue)
				this->user.read_config(setting, client_version);
			}
			else
				unknown_option(setting);
		}
	}



	void Client_settings::write_config(const std::string& config_path) const
	{
		libconfig::Config config;
		libconfig::Setting& config_root = config.getRoot();

		// Пишем все необходимые настройки -->
			config_root.add("version", m::libconfig::Version_type) = static_cast<m::libconfig::Version>(APP_VERSION);

			this->gui.write_config(
				config_root.add("gui", libconfig::Setting::TypeGroup)
			);

			this->user.write_config(
				config_root.add("user", libconfig::Setting::TypeGroup)
			);
		// Пишем все необходимые настройки <--

		// Сохраняем полученные настройки в файл -->
			try
			{
				std::string real_config_path = m::fs::config::start_writing(config_path);
				config.writeFile(U2L(real_config_path).c_str());
				m::fs::sync_file(real_config_path);
				m::fs::config::end_writing(config_path);
			}
			catch(m::Exception& e)
			{
				M_THROW(__(
					"Writing configuration file '%1' failed. %2",
					m::fs::get_abs_path_lazy(config_path), EE(e)
				));
			}
			catch(libconfig::FileIOException& e)
			{
				M_THROW(__(
					"Can't write configuration file '%1': %2.",
					m::fs::get_abs_path_lazy(config_path), EE(e)
				));
			}
		// Сохраняем полученные настройки в файл <--
	}
// Client_settings <--

