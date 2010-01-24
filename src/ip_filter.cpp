/**************************************************************************
*                                                                         *
*   Flush - GTK-based BitTorrent client                                   *
*   http://sourceforge.net/projects/flush                                 *
*                                                                         *
*   Copyright (C) 2009-2010, Dmitry Konishchev                            *
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


#include <algorithm>
#include <functional>

#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>
#include <gtkmm/treemodel.h>

#include <mlib/gtk/builder.hpp>
#include <mlib/gtk/controlled_list.hpp>

#include "common.hpp"
#include "ip_filter.hpp"
#include "main.hpp"



// Private <--
	class Ip_filter::Private
	{
		public:
			class Model_columns: public Gtk::TreeModel::ColumnRecord
			{
				public:
					Model_columns(void);


				public:
					Gtk::TreeModelColumn<Glib::ustring>					from;
					Gtk::TreeModelColumn<Glib::ustring>					to;
					Gtk::TreeModelColumn<bool>							block;
					Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >	icon;
					Gtk::TreeModelColumn<Glib::ustring>					fake;
			};


		public:
			Private(void);


		public:
			Gtk::CheckButton*				enabled;
			Gtk::Container*					widgets_container;

			Gtk::Entry*						from_entry;
			Gtk::Entry*						to_entry;
			Gtk::ToggleButton*				block_button;

			Gtk::Button*					add_button;
			Gtk::Button*					edit_button;
			Gtk::Button*					remove_button;
			Gtk::Button*					up_button;
			Gtk::Button*					down_button;

			Model_columns					columns;
			Glib::RefPtr<Gtk::ListStore>	model;
			m::gtk::Controlled_list*		list;
	};



	Ip_filter::Private::Model_columns::Model_columns(void)
	{
		this->add(this->from);
		this->add(this->to);
		this->add(this->block);
		this->add(this->icon);
		this->add(this->fake);
	}



	Ip_filter::Private::Private(void)
	:
		model( Gtk::ListStore::create(this->columns) )
	{
	}
// Private <--



Ip_filter::Ip_filter(BaseObjectType* cobject, const m::gtk::Builder& builder)
:
	Gtk::VBox(cobject),
	priv( new Private )
{
	// Виджеты из Glade -->
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "ip_filter_enabled",			priv->enabled);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "ip_filter_widgets_container",	priv->widgets_container);

		MLIB_GTK_BUILDER_GET_WIDGET(builder, "ip_filter_from",				priv->from_entry);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "ip_filter_to",				priv->to_entry);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "ip_filter_block",				priv->block_button);

		MLIB_GTK_BUILDER_GET_WIDGET(builder, "ip_filter_add",				priv->add_button);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "ip_filter_edit",				priv->edit_button);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "ip_filter_remove",			priv->remove_button);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "ip_filter_up",				priv->up_button);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "ip_filter_down",				priv->down_button);
	// Виджеты из Glade <--

	// Enabled -->
		priv->enabled->signal_clicked().connect(
			sigc::mem_fun(*this, &Ip_filter::on_enabled_toggled_cb)
		);
	// Enabled <--

	// Block button -->
		priv->block_button->set_image(
			*Gtk::manage(new Gtk::Image(Gtk::Stock::YES, Gtk::ICON_SIZE_BUTTON))
		);

		priv->block_button->signal_clicked().connect(
			sigc::mem_fun(*this, &Ip_filter::on_block_button_clicked_cb)
		);
	// Block button <--

	// Список фильтров -->
	{
		priv->list = Gtk::manage(new m::gtk::Controlled_list(
			priv->model, priv->remove_button, priv->up_button, priv->down_button
		));
		priv->list->set_headers_visible(false);

		priv->list->get_column_cell_renderer(
			priv->list->append_column("", priv->columns.from) - 1
		)->property_xalign() = 1;
		priv->list->append_column("", priv->columns.to);
		priv->list->append_column("", priv->columns.icon);
		priv->list->append_column("", priv->columns.fake);

		priv->list->get_selection()->signal_changed().connect(
			sigc::mem_fun(*this, &Ip_filter::on_selection_changed_cb)
		);

		Gtk::ScrolledWindow* scrolledwindow;
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "ip_filter_scrolled_window", scrolledwindow);
		scrolledwindow->add(*priv->list);
	}
	// Список фильтров <--

	// Управляющие кнопки -->
		priv->add_button->signal_clicked().connect(
			sigc::mem_fun(*this, &Ip_filter::on_add_button_clicked_cb)
		);

		priv->edit_button->signal_clicked().connect(
			sigc::mem_fun(*this, &Ip_filter::on_edit_button_clicked_cb)
		);

		priv->remove_button->signal_clicked().connect(
			sigc::bind< sigc::slot<void> >(
				sigc::mem_fun(*this, &Ip_filter::on_control_button_clicked_cb),
				sigc::mem_fun(*priv->list, &m::gtk::Controlled_list::remove)
			)
		);

		priv->up_button->signal_clicked().connect(
			sigc::bind< sigc::slot<void> >(
				sigc::mem_fun(*this, &Ip_filter::on_control_button_clicked_cb),
				sigc::mem_fun(*priv->list, &m::gtk::Controlled_list::up)
			)
		);

		priv->down_button->signal_clicked().connect(
			sigc::bind< sigc::slot<void> >(
				sigc::mem_fun(*this, &Ip_filter::on_control_button_clicked_cb),
				sigc::mem_fun(*priv->list, &m::gtk::Controlled_list::down)
			)
		);
	// Управляющие кнопки <--
}



Ip_filter::~Ip_filter(void)
{
	delete priv;
}



void Ip_filter::add(const Ip_filter_rule& rule)
{
	Gtk::TreeModel::Row row = *priv->model->append();

	row[priv->columns.from] = rule.from;
	row[priv->columns.to] = rule.to;
	row[priv->columns.block] = rule.block;
	row[priv->columns.icon] = this->render_icon(
		rule.block ? Gtk::Stock::CANCEL : Gtk::Stock::YES, Gtk::ICON_SIZE_MENU
	);
}



std::vector<Ip_filter_rule> Ip_filter::get(void) const
{
	std::vector<Ip_filter_rule> ip_filter;
	typedef Gtk::TreeModel::Children::iterator iter;
	Gtk::TreeModel::Children rows = priv->model->children();

	for(iter it = rows.begin(); it != rows.end(); ++it)
	{
		ip_filter.push_back(Ip_filter_rule(
			it->get_value(priv->columns.from),
			it->get_value(priv->columns.to),
			it->get_value(priv->columns.block)
		));
	}

	return ip_filter;
}



bool Ip_filter::get_enabled(void)
{
	return priv->enabled->get_active();
}



void Ip_filter::on_add_button_clicked_cb(void)
{
	Ip_filter_rule rule(
		priv->from_entry->get_text(), priv->to_entry->get_text(),
		priv->block_button->get_active()
	);

	try
	{
		rule.check();
	}
	catch(m::Exception& e)
	{
		show_warning_message(*this,
			_("Error while adding rule to IP filter"),
			__("Error while adding rule to IP filter: %1.", EE(e))
		);

		return;
	}

	this->add(rule);
}



void Ip_filter::on_control_button_clicked_cb(sigc::slot<void>& fun)
{
	fun();
}



void Ip_filter::on_block_button_clicked_cb(void)
{
	priv->block_button->set_image(*Gtk::manage(new Gtk::Image(
		priv->block_button->get_active() ? Gtk::Stock::CANCEL : Gtk::Stock::YES, Gtk::ICON_SIZE_BUTTON
	)));
}



void Ip_filter::on_edit_button_clicked_cb(void)
{
	Glib::RefPtr<Gtk::TreeSelection> selection = priv->list->get_selection();
	Gtk::TreeSelection::ListHandle_Path selected_paths = selection->get_selected_rows();

	if(selected_paths.empty())
		return;

	Gtk::TreeModel::Row row = *priv->model->get_iter(*selected_paths.begin());
	row[priv->columns.from] = priv->from_entry->get_text();
	row[priv->columns.to] = priv->to_entry->get_text();
	row[priv->columns.block] = priv->block_button->get_active();
	row[priv->columns.icon] = this->render_icon(
		row[priv->columns.block] ? Gtk::Stock::CANCEL : Gtk::Stock::YES, Gtk::ICON_SIZE_MENU
	);
}



void Ip_filter::on_enabled_toggled_cb(void)
{
	priv->widgets_container->set_sensitive(this->get_enabled());
}



void Ip_filter::on_selection_changed_cb(void)
{
	Glib::RefPtr<Gtk::TreeSelection> selection = priv->list->get_selection();
	Gtk::TreeSelection::ListHandle_Path selected_paths = selection->get_selected_rows();

	priv->edit_button->set_sensitive(!selected_paths.empty());

	if(selected_paths.empty())
		return;

	Gtk::TreeModel::Row row = *priv->model->get_iter(*selected_paths.begin());

	priv->from_entry->set_text(row[priv->columns.from]);
	priv->to_entry->set_text(row[priv->columns.to]);
	priv->block_button->set_active(row[priv->columns.block]);
}



void Ip_filter::set(const std::vector<Ip_filter_rule>& ip_filter)
{
	priv->model->clear();

	std::for_each(
		ip_filter.begin(), ip_filter.end(),
		sigc::mem_fun(*this, &Ip_filter::add)
	);
}



void Ip_filter::set_enabled(bool is)
{
	priv->enabled->set_active(is);
}

