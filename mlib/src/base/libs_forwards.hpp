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


// Предоставляет предварительные объявления для различных сторонних библиотек с
// целью снижения времени компиляции.
//
// Внимание!
// Использовать только в режиме разработчика, т. к. невозможно гарантировать,
// что данные объявления останутся актуальными в новых версиях библиотек.

#ifndef HEADER_MLIB_LIBS_FORWARDS
#define HEADER_MLIB_LIBS_FORWARDS

#include "libs_forwards/gdkmm.hpp"
#include "libs_forwards/glademm.hpp"
#include "libs_forwards/glibmm.hpp"
#include "libs_forwards/gtkmm.hpp"
#include "libs_forwards/libconfig.hpp"
#include "libs_forwards/libtorrent.hpp"
#include "libs_forwards/sigc++.hpp"
#include "libs_forwards/sqlite3.h"

#endif

