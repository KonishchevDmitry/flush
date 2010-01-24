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


#include <gdk/gdk.h>

#include "main.hpp"



namespace m { namespace gtk {

const unsigned int MOUSE_LEFT_BUTTON = 1;
const unsigned int MOUSE_RIGHT_BUTTON = 3;

int WINDOW_BORDER_WIDTH = 5;
int BOX_BORDER_WIDTH = WINDOW_BORDER_WIDTH ;

int HBOX_SPACING = 5;
int VBOX_SPACING = 5;

int TABLE_NAME_VALUE_SPACING = HBOX_SPACING * 2;
int TABLE_ROWS_SPACING = VBOX_SPACING;
int TABLE_NAME_VALUE_COLUMNS_SPACING = HBOX_SPACING * 10;



Scoped_enter::Scoped_enter(void)
{
	gdk_threads_enter();
}



Scoped_enter::~Scoped_enter(void)
{
	gdk_threads_leave();
}

}}

