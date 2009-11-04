/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
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

#include <mlib/gtk/main.hpp>

#include "tree_view_settings.hpp"


namespace m { namespace gtk {

int TREE_VIEW_COLUMNS_MIN_WIDTH = 1;
int TREE_VIEW_COLUMNS_DEFAULT_WIDTH = 100;
int TREE_VIEW_COLUMNS_MAX_WIDTH = 2000;



// Tree_view_column_settings -->
	Tree_view_column_settings::Tree_view_column_settings(const std::string& name, bool visible, int width)
	:
		name(name),
		visible(visible),
		width(width)
	{
	}



	void Tree_view_column_settings::set(const std::string& name, const bool visible, const int width)
	{
		this->name = name;
		this->visible = visible;
		this->width = width;
	}



	void Tree_view_column_settings::set_width(const int width)
	{
		if(TREE_VIEW_COLUMNS_MIN_WIDTH <= width && width <= TREE_VIEW_COLUMNS_MAX_WIDTH)
			this->width = width;
		else
			this->width = TREE_VIEW_COLUMNS_DEFAULT_WIDTH;
	}
// Tree_view_column_settings <--



// Tree_view_settings -->
	Tree_view_settings::Tree_view_settings(void)
	:
		sort_column(""),
		sort_order(SORT_ORDER_ASCENDING)
	{
	}
// Tree_view_settings <--

}}

