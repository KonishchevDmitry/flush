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


#include <gtkmm/expander.h>

#include "expander_settings.hpp"


namespace m { namespace gtk {

Expander_settings::Expander_settings(bool expanded)
:
	expanded(expanded)
{
}



void Expander_settings::get(const Gtk::Expander& expander)
{
	this->expanded = expander.get_expanded();
}



void Expander_settings::set(Gtk::Expander& expander) const
{
	expander.set_expanded(this->expanded);
}

}}

