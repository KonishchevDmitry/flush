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


#ifdef MLIB_ENABLE_GTK

#include <gtkmm/button.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include <mlib/main.hpp>

#include "controlled_list.hpp"


namespace m { namespace gtk {

Controlled_list::Controlled_list(
	Glib::RefPtr<Gtk::ListStore>	model,
	Gtk::Button*					remove_button,
	Gtk::Button*					up_button,
	Gtk::Button*					down_button
)
:
	Gtk::TreeView(model),
	model(model),
	remove_button(remove_button),
	up_button(up_button),
	down_button(down_button)
{
	this->get_selection()->signal_changed().connect(
		sigc::mem_fun(*this, &Controlled_list::on_selection_changed_cb)
	);

	this->update_buttons();
}



void Controlled_list::down(void)
{
	this->push(DOWN);
}



void Controlled_list::on_selection_changed_cb(void)
{
	this->update_buttons();
}



void Controlled_list::push(Direction direction)
{
	Gtk::TreeSelection::ListHandle_Path selected_paths =
		this->get_selection()->get_selected_rows();

	if(selected_paths.empty())
		return;

	Gtk::TreeModel::Children rows = this->model->children();
	Gtk::TreeIter cur_iter = this->model->get_iter(*selected_paths.begin());
	Gtk::TreeIter target_iter = cur_iter;

	if(direction == UP)
	{
		if(cur_iter != rows.begin())
			this->model->iter_swap(cur_iter, --target_iter);
	}
	else
	{
		if(++target_iter != rows.end())
			this->model->iter_swap(cur_iter, target_iter);
	}

	this->update_buttons();
}



void Controlled_list::remove(void)
{
	Glib::RefPtr<Gtk::TreeSelection> selection = this->get_selection();
	Gtk::TreeSelection::ListHandle_Path selected_paths = selection->get_selected_rows();

	if(selected_paths.empty())
		return;

	Gtk::TreeModel::Children rows = this->model->children();
	Gtk::TreeIter next_iter = this->model->erase(
		this->model->get_iter(*selected_paths.begin())
	);

	if(!rows.empty())
	{
		if(next_iter == rows.end())
			--next_iter;

		selection->select(next_iter);
	}
}



void Controlled_list::set_buttons(Gtk::Button* remove_button, Gtk::Button* up_button, Gtk::Button* down_button)
{
	this->remove_button = remove_button;
	this->up_button = up_button;
	this->down_button = down_button;

	this->update_buttons();
}



void Controlled_list::up(void)
{
	this->push(UP);
}



void Controlled_list::update_buttons(void)
{
	Gtk::TreeModel::Children rows = this->model->children();
	Glib::RefPtr<Gtk::TreeSelection> selection = this->get_selection();
	Gtk::TreeSelection::ListHandle_Path selected_paths = selection->get_selected_rows();

	if(selected_paths.empty())
	{
		if(this->remove_button)	this->remove_button->set_sensitive(false);
		if(this->up_button)		this->up_button->set_sensitive(false);
		if(this->down_button)	this->down_button->set_sensitive(false);
	}
	else
	{
		if(this->remove_button)	this->remove_button->set_sensitive(true);
		if(this->up_button)		this->up_button->set_sensitive(!selection->is_selected(rows.begin()));
		if(this->down_button)	this->down_button->set_sensitive(!selection->is_selected(--rows.end()));
	}
}

}}

#endif

