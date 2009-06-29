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


#ifndef HEADER_MLIB_GTK_SIGNALS_HOLDER
#define HEADER_MLIB_GTK_SIGNALS_HOLDER

#include <vector>

#include <sigc++/connection.h>


namespace m { namespace gtk {

/// Предназначен для удержания sigc::connection'ов и их отсоединения при
/// уничтожении виджета.
class Signals_holder
{
	public:
		~Signals_holder(void);


	private:
		std::vector<sigc::connection>	connections;


	public:
		/// Отсоединяет все контролируемые в данный момент sigc::connection'ы.
		void	disconnect(void);

		/// Помещает очередной sigc::connection под контроль Signals_holder'а.
		void	push(const sigc::connection& connection);
};

}}

#endif

