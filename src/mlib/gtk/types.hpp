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


#ifndef HEADER_MLIB_GTK_TYPES
	#define HEADER_MLIB_GTK_TYPES

	#ifdef MLIB_ENABLE_GTK
		namespace Gtk
		{
			class Button;
			class ComboBox;
			class Dialog;
			class Entry;
			class EventBox;
			class FileChooserDialog;
			class LinkButton;
			class ListStore;
			class ListViewText;
			class MessageDialog;
			class ProgressBar;
			class StockID;
			class Table;
			class Toolbar;
			class TreeRow;
			class TreeViewColumn;
			class TreeView;
			class Widget;
			class Window;
		}

		namespace m { namespace gtk {
			class Link_button;
		}}
	#endif

	#ifdef MLIB_ENABLE_GLADE
		namespace Gnome { namespace Glade {
			class Xml;
		}}

		#include <glibmm/refptr.h>

		namespace m { namespace gtk {
			typedef ::Glib::RefPtr< ::Gnome::Glade::Xml > Glade_xml;
		}}
	#endif

	#ifdef MLIB_ENABLE_ALIASES
		#ifdef MLIB_ENABLE_GLADE
			using m::gtk::Glade_xml;
		#endif
	#endif

#endif

