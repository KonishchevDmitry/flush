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


#ifndef HEADER_MLIB_SIGNALS_HOLDER
#define HEADER_MLIB_SIGNALS_HOLDER

#include <boost/shared_ptr.hpp>

#ifndef MLIB_ENABLE_LIBS_FORWARDS
	#include <sigc++/connection.h>
#endif

#include <mlib/gtk/dispatcher.hxx>
#include <mlib/main.hpp>


namespace m {


namespace Signals_holder_aux {

	class Connection: private Virtual
	{
		public:
			virtual void disconnect(void) = 0;
	};

}


/// Предназначен для удержания connection'ов и их отсоединения при уничтожении
/// виджета.
class Signals_holder
{
	private:
		typedef Signals_holder_aux::Connection Connection;
		typedef boost::shared_ptr<Connection> Connection_ptr;


	public:
		~Signals_holder(void);


	private:
		std::vector<Connection_ptr>	connections;


	public:
		/// Отсоединяет все контролируемые в данный момент connection'ы.
		void	disconnect(void);

		/// Помещает очередной sigc::connection под контроль Signals_holder'а.
		void	push(const sigc::connection& connection);

		/// Помещает очередной m::gtk::Dispatcher::Copyright под контроль
		/// Signals_holder'а.
		void	push(const m::gtk::Dispatcher_connection& connection);
};


}

#endif

