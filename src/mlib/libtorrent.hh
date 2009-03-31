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


namespace m { namespace libtorrent {

Torrent_file::Torrent_file(void)
{
}



Torrent_file::Torrent_file(int id, const std::string& path, Size size)
:
	id(id),
	path(path),
	size(size)
{
}



std::string Torrent_file::get_path(void)
{
	return this->path;
}



void Torrent_file::set_path(std::string path)
{
	this->path = path;
}



bool Torrent_file::operator<(const Torrent_file& file) const
{
	return this->path < file.path;
}



Version get_version(void)
{
	return ::m::get_version(LIBTORRENT_VERSION_MAJOR, LIBTORRENT_VERSION_MINOR, 0);
}

}}

