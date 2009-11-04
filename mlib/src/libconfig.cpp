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


#include <libconfig.h++>

#include <mlib/main.hpp>

#include "libconfig.hpp"


namespace m {


namespace libconfig {
	const Setting::Type Size_type = ::libconfig::Setting::TypeInt64;
	const Setting::Type Speed_type = ::libconfig::Setting::TypeInt64;
	const Setting::Type Time_type = ::libconfig::Setting::TypeInt64;
	const Setting::Type Version_type = ::libconfig::Setting::TypeInt64;
}



std::string EE(const libconfig::FileIOException& error)
{
	return EE();
}



std::string EE(const libconfig::ParseException& error)
{
	libconfig::ParseException error_non_const_copy = error;
	return __("%1 at line %2", error_non_const_copy.getError(), error_non_const_copy.getLine());
}


}

