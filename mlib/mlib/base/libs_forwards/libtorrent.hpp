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


#ifndef HEADER_MLIB_LIBS_FORWARDS_LIBTORRENT
#define HEADER_MLIB_LIBS_FORWARDS_LIBTORRENT

namespace libtorrent {

	class invalid_encoding;

	class alert;
	class big_number;
	class entry;
	class peer_info;
	class session_status;
	class torrent_alert;
	class torrent_handle;
	class torrent_info;
	class torrent_status;

	typedef big_number sha1_hash;

}

#endif

