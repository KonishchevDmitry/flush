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


namespace m { namespace fs {


bool Stat::is_blk(void)
{
	return S_ISBLK(this->mode);
}



bool Stat::is_chr(void)
{
	return S_ISCHR(this->mode);
}



bool Stat::is_dir(void)
{
	return S_ISDIR(this->mode);
}



bool Stat::is_fifo(void)
{
	return S_ISFIFO(this->mode);
}



bool Stat::is_lnk(void)
{
	return S_ISLNK(this->mode);
}



bool Stat::is_reg(void)
{
	return S_ISREG(this->mode);
}



bool Stat::is_sock(void)
{
	return S_ISSOCK(this->mode);
}

}}

