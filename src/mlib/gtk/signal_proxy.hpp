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


#ifndef HEADER_MLIB_GTK_SIGNAL_PROXY
#define HEADER_MLIB_GTK_SIGNAL_PROXY

#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include "signal_proxy.hxx"


namespace m { namespace gtk {

template <typename T_return, typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6, typename T_arg7>
class Signal_proxy
{
	private:
		typedef sigc::slot<T_return, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>	Slot;
		typedef sigc::signal<T_return, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>	Signal;


	public:
		Signal_proxy(Signal& signal);


	private:
		Signal&	signal;


	public:
		sigc::connection	connect(const Slot& slot);
};

}}

#include "signal_proxy.hh"

#endif

