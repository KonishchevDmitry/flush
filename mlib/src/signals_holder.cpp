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


#include <sigc++/connection.h>

#include <mlib/gtk/dispatcher.hpp>

#include <mlib/main.hpp>

#include "signals_holder.hpp"


namespace m {


template <class T>
class Abstract_connection: public Signals_holder_aux::Connection
{
	public:
		Abstract_connection(const T& connection)
		: connection(connection) {}

	private:
		T connection;

	public:
		void disconnect(void)
		{ this->connection.disconnect(); }
};



Signals_holder::~Signals_holder(void)
{
	this->disconnect();
}



void Signals_holder::disconnect(void)
{
	BOOST_FOREACH(Connection_ptr& connection, this->connections)
		connection->disconnect();
	this->connections.clear();
}



void Signals_holder::push(const sigc::connection& connection)
{
	this->connections.push_back(Connection_ptr(
		new Abstract_connection<sigc::connection>(connection) ));
}



void Signals_holder::push(const m::gtk::Dispatcher_connection& connection)
{
	this->connections.push_back(Connection_ptr(
		new Abstract_connection<m::gtk::Dispatcher_connection>(connection) ));
}


}

