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


#include <gtkmm/combobox.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/spinbutton.h>

#include <mlib/gtk/glade.hpp>

#include "client_settings.hpp"
#include "gui_lib.hpp"
#include "main.hpp"
#include "temporary_action_dialog.hpp"



// Private -->
namespace Temporary_action_dialog_aux
{

	class Private
	{
		public:
			class Predefined_time_model_columns: public Gtk::TreeModel::ColumnRecord
			{
				public:
					Predefined_time_model_columns(void);


				public:
					Gtk::TreeModelColumn<int>			time;
					Gtk::TreeModelColumn<Glib::ustring>	name;
			};


		public:
			Private(const Glade_xml& glade);


		public:
			Gtk::Label*						title_label;

			Gtk::RadioButton*				predefined_time_toggle;
			Gtk::RadioButton*				custom_time_toggle;

			Gtk::ComboBox*					predefined_time;
			Gtk::SpinButton*				custom_time;

			Predefined_time_model_columns	predefined_time_columns;
			Glib::RefPtr<Gtk::ListStore>	predefined_time_model;

			Gtk::Button*					cancel_button;
			Gtk::Button*					ok_button;


		private:
			/// Обработчик сигнала на переключение типа времени (custom,
			/// predefined).
			void	on_predefined_time_toggle_cb(void);
	};



	Private::Predefined_time_model_columns::Predefined_time_model_columns(void)
	{
		this->add(this->time);
		this->add(this->name);
	}



	Private::Private(const Glade_xml& glade)
	:
		predefined_time_model( Gtk::ListStore::create(this->predefined_time_columns) )
	{
		User_settings& settings = get_client_settings().user;


		MLIB_GLADE_GET_WIDGET(glade, "title_label", 			this->title_label);

		MLIB_GLADE_GET_WIDGET(glade, "predefined_time_toggle",	this->predefined_time_toggle);
		MLIB_GLADE_GET_WIDGET(glade, "custom_time_toggle",		this->custom_time_toggle);

		MLIB_GLADE_GET_WIDGET(glade, "predefined_time",			this->predefined_time);
		MLIB_GLADE_GET_WIDGET(glade, "custom_time",				this->custom_time);

		MLIB_GLADE_GET_WIDGET(glade, "cancel_button",			this->cancel_button);
		MLIB_GLADE_GET_WIDGET(glade, "ok_button",				this->ok_button);

		// Времена "из коробки" -->
		{
			int active = 0;

			// Заполняем модель -->
			{
				const struct
				{
					int			time;
					std::string	name;
				} predefined_times[] = {
					{	1,		_("1 minute")	},
					{	5,		_("5 minutes")	},
					{	10,		_("10 minutes")	},
					{	15,		_("15 minutes")	},
					{	30,		_("30 minutes")	},
					{	45,		_("45 minutes")	},
					{	60,		_("1 hour")		},
					{	120,	_("2 hours")	},
					{	240,	_("4 hours")	},
					{	480,	_("8 hours")	},
					{	720,	_("12 hours")	},
					{	1440,	_("1 day")		}
				};

				for(size_t i = 0; i < M_STATIC_ARRAY_SIZE(predefined_times); i++)
				{
					Gtk::TreeModel::Row row = *(this->predefined_time_model->append());
					row[this->predefined_time_columns.time] = predefined_times[i].time * 60;
					row[this->predefined_time_columns.name] = predefined_times[i].name;

					if(settings.temporary_action_last_time >= predefined_times[i].time * 60)
						active = i;
				}
			}
			// Заполняем модель <--

			this->predefined_time->set_model(this->predefined_time_model);
			this->predefined_time->pack_start(this->predefined_time_columns.name);
			this->predefined_time->set_active(active);
		}
		// Времена "из коробки" <--

		this->predefined_time_toggle->signal_toggled().connect(
			sigc::mem_fun(*this, &Private::on_predefined_time_toggle_cb));

		if(!settings.temporary_action_last_time_is_predefined)
			this->custom_time_toggle->set_active();

		this->custom_time->set_value(settings.temporary_action_last_time / 60);
	}



	void Private::on_predefined_time_toggle_cb(void)
	{
		this->predefined_time->set_sensitive(this->predefined_time_toggle->get_active());
		this->custom_time->set_sensitive(this->custom_time_toggle->get_active());
	}

}
// Private <--



Temporary_action_dialog::Temporary_action_dialog(BaseObjectType* cobject, const Glade_xml& glade)
:
	m::gtk::Dialog(cobject),
	priv( new Private(glade) )
{
	priv->cancel_button->signal_clicked().connect(
		sigc::bind<int>( sigc::mem_fun(*this, &Gtk::Dialog::response), Gtk::RESPONSE_CANCEL )
	);

	priv->ok_button->signal_clicked().connect(
		sigc::bind<int>( sigc::mem_fun(*this, &Gtk::Dialog::response), Gtk::RESPONSE_OK )
	);

	this->signal_response().connect(
		sigc::mem_fun(*this, &Temporary_action_dialog::on_response_cb));
}



void Temporary_action_dialog::init(Gtk::Window& parent_window, Temporary_action action, Torrents_group group)
{
	m::gtk::Dialog::init(parent_window);

	// Загловок окна -->
	{
		std::string title;
		std::string action_string;
		std::string group_string;

		switch(action)
		{
			case TEMPORARY_ACTION_RESUME:
				action_string = _Q("Temporary resume downloads|resume");
				break;

			case TEMPORARY_ACTION_PAUSE:
				action_string = _Q("Temporary pause downloads|pause");
				break;

			default:
				MLIB_LE();
				break;
		}

		switch(group)
		{
			case ALL:
				group_string = _Q("Temporary pause all torrents|all torrents");
				break;

			case DOWNLOADS:
				group_string = _Q("Temporary pause downloads|downloads");
				break;

			case UPLOADS:
				group_string = _Q("Temporary pause uploads|uploads");
				break;

			default:
				MLIB_LE();
				break;
		}

		title = __Q("Temporary pause downloads|Temporary %1 %2", action_string, group_string);

		this->set_title(format_window_title(title));
		priv->title_label->set_label("<b>" + Glib::Markup::escape_text(title) + "</b>");
	}
	// Загловок окна <--
}



Time Temporary_action_dialog::get_time(void) const
{
	if(priv->predefined_time_toggle->get_active())
		return priv->predefined_time->get_active()->get_value(priv->predefined_time_columns.time);
	else
		return priv->custom_time->get_value() * 60;
}



void Temporary_action_dialog::on_response_cb(int response_id) const
{
	if(response_id == Gtk::RESPONSE_OK)
	{
		User_settings& settings = get_client_settings().user;

		settings.temporary_action_last_time_is_predefined = priv->predefined_time_toggle->get_active();
		settings.temporary_action_last_time = this->get_time();
	}
}

