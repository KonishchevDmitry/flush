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


#include <algorithm>
#include <functional>
#include <vector>

#include <sigc++/connection.h>

#include "signals_holder.hpp"


namespace m { namespace gtk {

Signals_holder::~Signals_holder(void)
{
	this->disconnect();
}



void Signals_holder::disconnect(void)
{
	std::for_each(this->connections.begin(), this->connections.end(),
		std::mem_fun_ref(&sigc::connection::disconnect));
	this->connections.clear();
}



void Signals_holder::push(const sigc::connection& connection)
{
	this->connections.push_back(connection);
}

}}

