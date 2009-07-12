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


#ifndef HEADER_MLIB_GTK_TREE_VIEW_SETTINGS
#define HEADER_MLIB_GTK_TREE_VIEW_SETTINGS

#include <deque>

#include <mlib/gtk/main.hpp>


namespace m { namespace gtk {

	extern int TREE_VIEW_COLUMNS_MIN_WIDTH;
	extern int TREE_VIEW_COLUMNS_DEFAULT_WIDTH;
	extern int TREE_VIEW_COLUMNS_MAX_WIDTH;



	class Tree_view_column_settings
	{
		public:
			Tree_view_column_settings(
				const std::string&	name	= "Unnamed",
				bool				visible	= true,
				int					width	= TREE_VIEW_COLUMNS_DEFAULT_WIDTH
			);


		public:
			std::string		name;
			bool			visible;
			int				width;


		public:
			void set(const std::string& name, const bool visible, const int width);
			void set_width(const int width);
	};



	class Tree_view_settings
	{
		public:
			enum Sort_order {SORT_ORDER_ASCENDING, SORT_ORDER_DESCENDING};


		public:
			Tree_view_settings(void);


		public:
			std::deque<Tree_view_column_settings>	columns;
			std::string								sort_column;
			Sort_order								sort_order;
	};

}}

#endif

