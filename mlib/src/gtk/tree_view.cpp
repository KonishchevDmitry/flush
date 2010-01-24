/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
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


#include <gtk/gtkcellrenderertoggle.h>

#include <gtkmm/treeviewcolumn.h>

#include <mlib/gtk/main.hpp>
#include <mlib/gtk/tree_view_settings.hpp>

#include "tree_view.hpp"


namespace m { namespace gtk {

// Tree_view_model_columns -->
	Tree_view_model_columns::Tree_view_model_columns(void)
	: search_column(NULL)
	{
	}



	int Tree_view_model_columns::get_search_column_index(void)
	{
		if(!this->search_column)
			return -1;

		return this->search_column->index();
	}



	void Tree_view_model_columns::set_search_column(const Gtk::TreeModelColumnBase& search_column)
	{
		this->search_column = &search_column;
	}
// Tree_view_model_columns <--



// Tree_view_columns -->
	Tree_view_columns::Column::Column(const std::string& id, Gtk::TreeViewColumn* column, const std::string& menu_name, const std::string& description)
	:
		id(id),
		column(column),
		menu_name(menu_name),
		description(description)
	{
	}



	void Tree_view_columns::add(const std::string& id, Gtk::TreeViewColumn* column, const std::string& description, bool resizable)
	{
		this->add(id, column, column->get_title(), description, resizable);
	}



	void Tree_view_columns::add(const std::string& id, Gtk::TreeViewColumn* column, const std::string& menu_name, const std::string& description, bool resizable)
	{
		this->all.push_back(Column(id, column, menu_name, description));

		this->columns_by_ids.insert(
			std::pair<std::string, Gtk::TreeViewColumn*>(id, column)
		);

		this->ids_by_columns.insert(
			std::pair<Gtk::TreeViewColumn*, std::string>(column, id)
		);

		// Свойства колонки по умолчанию -->
			column->set_reorderable();
			column->set_resizable(resizable);

			if(resizable)
			{
				column->set_min_width(TREE_VIEW_COLUMNS_MIN_WIDTH);
				column->set_fixed_width(TREE_VIEW_COLUMNS_DEFAULT_WIDTH);
				column->set_max_width(TREE_VIEW_COLUMNS_MAX_WIDTH);
				column->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
			}
		// Свойства колонки по умолчанию <--
	}



	void Tree_view_columns::remove(Gtk::TreeViewColumn* column)
	{
		std::string id = this->ids_by_columns[column];
		this->ids_by_columns.erase(column);
		this->columns_by_ids.erase(id);

		M_FOR_IT(this->all, it)
		{
			if(it->column == column)
			{
				this->all.erase(it);
				break;
			}
		}
	}
// Tree_view_columns <--

}}

