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


#include <mlib/gtk/main.hpp>

#include "toolbar.hpp"



namespace m { namespace gtk { namespace toolbar {


Gtk::ToolbarStyle convert(Style style)
{
	switch(style)
	{
		case DEFAULT:
			MLIB_LE();
			break;

		case ICONS:
			return Gtk::TOOLBAR_ICONS;
			break;

		case TEXT:
			return Gtk::TOOLBAR_TEXT;
			break;

		case BOTH:
			return Gtk::TOOLBAR_BOTH;
			break;

		case BOTH_HORIZ:
			return Gtk::TOOLBAR_BOTH_HORIZ;
			break;

		default:
			MLIB_LE();
			break;
	}
}



Style convert(Gtk::ToolbarStyle style)
{
	switch(style)
	{
		case Gtk::TOOLBAR_ICONS:
			return ICONS;
			break;

		case Gtk::TOOLBAR_TEXT:
			return TEXT;
			break;

		case Gtk::TOOLBAR_BOTH:
			return BOTH;
			break;

		case Gtk::TOOLBAR_BOTH_HORIZ:
			return BOTH_HORIZ;
			break;

		default:
			MLIB_LE();
			break;
	}
}



Style get_style_from_string(const std::string& string)
{
	if(string == "default")
		return DEFAULT;
	else if(string == "icons")
		return ICONS;
	else if(string == "text")
		return TEXT;
	else if(string == "both")
		return BOTH;
	else if(string == "both_horiz")
		return BOTH_HORIZ;
	else
		M_THROW(__("Invalid toolbar style name '%1'.", string));
}



std::string get_style_string_representation(Style style)
{
	switch(style)
	{
		case DEFAULT:
			return "default";
			break;

		case ICONS:
			return "icons";
			break;

		case TEXT:
			return "text";
			break;

		case BOTH:
			return "both";
			break;

		case BOTH_HORIZ:
			return "both_horiz";
			break;

		default:
			MLIB_LE();
			break;
	}
}



void set_style(Gtk::Toolbar& toolbar, Style style)
{
	if(style == DEFAULT)
		toolbar.unset_toolbar_style();
	else
		toolbar.set_toolbar_style(toolbar::convert(style));
}

}}}

