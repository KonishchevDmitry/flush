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


#include "base.hpp"


namespace m {


const Version NO_VERSION = 0;



int get_major_version(Version version)
{
	return int(version / 1000000);
}



Version get_major_minor_version(Version version)
{
	return version / 1000 * 1000;
}



int get_minor_version(Version version)
{
	return int(version / 1000 % 1000);
}



int get_sub_minor_version(Version version)
{
	return int(version % 1000);
}



Version get_version(int major, int minor, int sub_minor)
{
	return Version(major * 1000000) + Version(minor * 1000) + sub_minor;
}



bool is_valid_version(Version version)
{
	return version > 0;
}


}

