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


#include <mlib/main.hpp>

#include "dbus.hpp"


namespace m {

std::string EE(const DBus::Error& error)
{
	// DBus::Error содержит строку, в которой первая буква
	// заглавная, может быть несколько предложений
	// и вставляется перевод строки.

	std::string error_string = error.what();

	if(!error_string.empty() && error_string[error_string.size() - 1] == '\n')
		error_string = error_string.substr(0, error_string.size() - 1);

	return error_string;
}

}

