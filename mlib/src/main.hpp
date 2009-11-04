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


#ifndef HEADER_MLIB_MAIN
#define HEADER_MLIB_MAIN

// Включает в себя все основные файлы, которые, как правило, всегда
// используются при работе с mlib.
// + Наиболее часто используемые файлы стандартной библиотеки.

#include <cerrno>
#include <cstring>

#include <string>
#include <vector>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include <mlib/base/base.hpp>
#include <mlib/base/exception.hpp>
#include <mlib/base/format.hpp>
#include <mlib/base/messages.hpp>

#ifdef MLIB_ENABLE_LIBS_FORWARDS
	#include <mlib/base/libs_forwards.hpp>
#endif

#endif


