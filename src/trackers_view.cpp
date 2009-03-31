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


#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>
#include <gtkmm/table.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "mlib/gtk/misc.hpp"

#include "main.hpp"
#include "trackers_view.hpp"



// Gui <--
	class Trackers_view::Gui
	{
		public:
			class Model_columns: public Gtk::TreeModel::ColumnRecord
			{
				public:
					Model_columns(void);


				public:
					Gtk::TreeModelColumn<Glib::ustring>		url;
			};


		public:
			Gui(void);


		public:
			Gtk::Entry						tracker_path_entry;

			Model_columns					model_columns;
			Glib::RefPtr<Gtk::ListStore>	model;

			Gtk::CellRendererText			url_renderer;
			Gtk::TreeViewColumn				url_column;
			Gtk::TreeView					list;
	};



	Trackers_view::Gui::Gui(void)
	:
		model( Gtk::ListStore::create(this->model_columns) ),
		url_column("", this->url_renderer)
	{
		this->url_renderer.property_editable() = true;
		this->url_column.add_attribute(this->url_renderer.property_text(), this->model_columns.url);

		this->list.set_model(this->model);
		this->list.set_headers_visible(false);

		this->url_column.set_sizing(Gtk::TREE_VIEW_COLUMN_AUTOSIZE);
		this->list.append_column(this->url_column);
	}



	Trackers_view::Gui::Model_columns::Model_columns(void)
	{
		this->add(this->url);
	}
// Gui <--



Trackers_view::Trackers_view(void)
:
	gui( new Gui )
{
	Gtk::Button* button;

	Gtk::Table* table = Gtk::manage(new Gtk::Table(2, 2));
	table->set_border_width(m::gtk::BOX_BORDER_WIDTH);
	table->set_row_spacings(m::gtk::HBOX_SPACING);
	table->set_col_spacings(m::gtk::HBOX_SPACING);
	this->pack_start(*table, true, true);

	table->attach(this->gui->tracker_path_entry, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
	this->gui->tracker_path_entry.signal_activate().connect(sigc::mem_fun(
		*this, &Trackers_view::on_add_callback
	));

	button = Gtk::manage(new Gtk::Button(Gtk::Stock::ADD));
	button->signal_clicked().connect(sigc::mem_fun(
		*this, &Trackers_view::on_add_callback
	));
	table->attach(*button, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL);

	{
		Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
		scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		scrolledwindow->set_shadow_type(Gtk::SHADOW_IN);
		table->attach(*scrolledwindow, 0, 1, 1, 2);

		scrolledwindow->add(this->gui->list);
	}

	{
		Gtk::VBox* buttons_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		table->attach(*buttons_vbox, 1, 2, 1, 2, Gtk::FILL);

		button = Gtk::manage(new Gtk::Button(Gtk::Stock::REMOVE));
		button->signal_clicked().connect(sigc::mem_fun(
			*this, &Trackers_view::on_remove_callback
		));
		buttons_vbox->pack_start(*button, false, false);

		button = Gtk::manage(new Gtk::Button(Gtk::Stock::GO_UP));
		button->signal_clicked().connect(sigc::mem_fun(
			*this, &Trackers_view::on_up_callback
		));
		buttons_vbox->pack_start(*button, false, false);

		button = Gtk::manage(new Gtk::Button(Gtk::Stock::GO_DOWN));
		button->signal_clicked().connect(sigc::mem_fun(
			*this, &Trackers_view::on_down_callback
		));
		buttons_vbox->pack_start(*button, false, false);
	}
}



Trackers_view::~Trackers_view(void)
{
	delete this->gui;
}



void Trackers_view::append(const std::string& url) throw(m::Exception)
{
	std::string tracker_url = m::trim(url);

	// Простейшая проверка на валидность адреса -->
		if(tracker_url.size() < strlen("http://X") || tracker_url.substr(0, strlen("http://")) != "http://")
		{
			M_THROW(__(
				"Gotten invalid tracker URL ('%1'). Please inter a valid URL started from 'http://'.",
				tracker_url
			));
		}
	// Простейшая проверка на валидность адреса <--

	// Проверяем, нет ли уже такого трекера в модели -->
	{
		Gtk::TreeModel::Children rows = this->gui->model->children();
		Gtk::TreeIter iter = rows.begin();

		while(iter)
		{
			Gtk::TreeModel::Row row = *iter++;

			if(row[this->gui->model_columns.url] == tracker_url)
			{
				M_THROW(__(
					"Gotten invalid tracker URL ('%1'). This URL is already exists in the tracker list.",
					tracker_url
				));
			}
		}
	}
	// Проверяем, нет ли уже такого трекера в модели <--

	// Добавляем трекер в модель -->
	{
		Gtk::TreeModel::Row row = *this->gui->model->append();
		row[this->gui->model_columns.url] = tracker_url;
	}
	// Добавляем трекер в модель <--

	this->changed_signal();
}



std::vector<std::string> Trackers_view::get(void) const
{
	std::vector<std::string> trackers;
	Gtk::TreeModel::Children rows = this->gui->model->children();

	trackers.reserve(rows.size());

	for(size_t i = 0; i < rows.size(); i++)
		trackers.push_back(rows[i]->get_value(this->gui->model_columns.url));

	return trackers;
}



void Trackers_view::on_add_callback(void)
{
	try
	{
		this->append( this->gui->tracker_path_entry.get_text() );
		this->gui->tracker_path_entry.set_text("");
	}
	catch(m::Exception& e)
	{
		Gtk::Window* parent_window = dynamic_cast<Gtk::Window*>(this->get_toplevel());

		if(parent_window)
			show_warning_message(*parent_window, _("Invalid tracker URL"), EE(e));
		else
			MLIB_W(_("Invalid tracker URL"), EE(e));
	}
}



void Trackers_view::on_down_callback(void)
{
	this->push(DOWN);
}



void Trackers_view::on_remove_callback(void)
{
	this->remove();
}



void Trackers_view::on_up_callback(void)
{
	this->push(UP);
}



void Trackers_view::push(Direction direction)
{
	Gtk::TreeModel::Children rows = this->gui->model->children();
	Gtk::TreeSelection::ListHandle_Path selected_paths = this->gui->list.get_selection()->get_selected_rows();

	if(selected_paths.empty())
		return;

	Gtk::TreeIter cur_iter = this->gui->model->get_iter(*selected_paths.begin());
	Gtk::TreeIter target_iter = cur_iter;

	if(direction == UP)
	{
		if(cur_iter != rows.begin())
		{
			target_iter--;
			this->gui->model->iter_swap(cur_iter, target_iter);
			this->changed_signal();
		}
	}
	else
	{
		target_iter++;

		if(target_iter != rows.end())
		{
			this->gui->model->iter_swap(cur_iter, target_iter);
			this->changed_signal();
		}
	}
}



void Trackers_view::remove(void)
{
	Gtk::TreeSelection::ListHandle_Path selected_paths = this->gui->list.get_selection()->get_selected_rows();

	if(selected_paths.empty())
		return;

	this->gui->model->erase(
		this->gui->model->get_iter(*selected_paths.begin())
	);

	this->changed_signal();
}



void Trackers_view::set(const std::vector<std::string>& trackers)
{
	std::string selected_tracker;
	Glib::RefPtr<Gtk::TreeView::Selection> selection = this->gui->list.get_selection();

	// Получаем выделенный в данный момент трекер -->
	{
		Gtk::TreeSelection::ListHandle_Path selected_paths = selection->get_selected_rows();

		if(!selected_paths.empty())
		{
			Gtk::TreeIter iter = this->gui->model->get_iter(*selected_paths.begin());
			selected_tracker = iter->get_value(this->gui->model_columns.url);
		}
	}
	// Получаем выделенный в данный момент трекер <--

	this->gui->model->clear();

	for(size_t i = 0; i < trackers.size(); i++)
	{
		Gtk::TreeIter iter = this->gui->model->append();
		Gtk::TreeModel::Row row = *iter;

		if(selected_tracker == trackers[i])
			selection->select(iter);

		row[this->gui->model_columns.url] = trackers[i];
	}
}



sigc::signal<void> Trackers_view::signal_changed(void)
{
	return this->changed_signal;
}

