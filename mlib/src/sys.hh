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


namespace m { namespace sys {


// File_holder -->
	File_holder::File_holder(void) : fd(-1) { }
	File_holder::File_holder(int fd) : fd(fd) { }



	File_holder::~File_holder(void)
	{
		try
		{
			this->close();
		}
		catch(m::Sys_exception&)
		{
		}
	}



	void File_holder::close(void)
	{
		if(this->get() >= 0)
			unix_close(this->reset());
	}



	int File_holder::get(void) const
	{
		return this->fd;
	}



	int File_holder::reset(void)
	{
		int fd = this->fd;
		this->fd = -1;
		return fd;
	}



	void File_holder::set(int fd)
	{
		this->close();
		this->fd = fd;
	}
// File_holder <--


}}

