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


#ifndef HEADER_MLIB_GTK_SIGNAL_PROXY_FWD
#define HEADER_MLIB_GTK_SIGNAL_PROXY_FWD

#include <sigc++/signal.h>

namespace m { namespace gtk {

template <typename T_return, typename T_arg1 = sigc::nil, typename T_arg2 = sigc::nil, typename T_arg3 = sigc::nil, typename T_arg4 = sigc::nil, typename T_arg5 = sigc::nil, typename T_arg6 = sigc::nil, typename T_arg7 = sigc::nil>
class Signal_proxy;

}}

#endif

