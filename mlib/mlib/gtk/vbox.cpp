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


#include <glibmm/markup.h>

#include <gtkmm/label.h>
#include <gtkmm/separator.h>

#include <mlib/gtk/main.hpp>

#include "vbox.hpp"


namespace m { namespace gtk { namespace vbox {

void add_big_header(Gtk::VBox& parent_vbox, const std::string& header_string, bool add_space, bool add_separator, bool centered)
{
	Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
	parent_vbox.pack_start(*hbox, false, false);

	Gtk::Label* label = Gtk::manage(new Gtk::Label());
	label->set_markup("<b><big>" + Glib::Markup::escape_text(header_string) + "</big></b>");
#if GTK_CHECK_VERSION(3, 0, 0)
	label->set_alignment( centered ? Gtk::ALIGN_CENTER : Gtk::ALIGN_START );
#else
	label->set_alignment( centered ? Gtk::ALIGN_CENTER : Gtk::ALIGN_LEFT );
#endif
	hbox->pack_start(*label, true, true);

	if(add_separator)
		parent_vbox.pack_start(*Gtk::manage(new Gtk::HSeparator()), false, false);

	if(add_space)
		m::gtk::vbox::add_space(parent_vbox);
}



void add_header(Gtk::VBox& parent_vbox, const std::string& header_string, bool centered)
{
	Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
	parent_vbox.pack_start(*hbox, false, false);

	Gtk::Label* label = Gtk::manage(new Gtk::Label());
	label->set_markup("<b>" + Glib::Markup::escape_text(header_string) + "</b>");
#if GTK_CHECK_VERSION(3, 0, 0)
	label->set_alignment( centered ? Gtk::ALIGN_CENTER : Gtk::ALIGN_START );
#else
	label->set_alignment( centered ? Gtk::ALIGN_CENTER : Gtk::ALIGN_LEFT );
#endif
	hbox->pack_start(*label, true, true);
}



void add_labeled_string(Gtk::VBox& parent_vbox, const std::string& label_string, const std::string& string, bool right_alignment, bool expand)
{
	add_widget_with_label(
		parent_vbox, label_string + ":",
		* Gtk::manage( new Gtk::Label(string) ), right_alignment, expand
	);
}



void add_space(Gtk::VBox& parent_vbox)
{
	parent_vbox.pack_start(
		*( new Gtk::HBox(false, m::gtk::HBOX_SPACING) ),
		false, false, m::gtk::VBOX_SPACING
	);
}



void add_widget_with_label(Gtk::VBox& parent_vbox, const std::string& label_string, Gtk::Widget& widget, bool right_alignment, bool expand)
{
	Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
	parent_vbox.pack_start(*hbox, false, false);

	Gtk::Label* label = Gtk::manage( new Gtk::Label(label_string) );
	hbox->pack_start(*label, false, false);

	if(right_alignment)
		hbox->pack_end(widget, expand, expand);
	else
		hbox->pack_start(widget, expand, expand);
}



void add_widget_with_labeled_widget(Gtk::VBox& parent_vbox, Gtk::Widget& labeled_widget, Gtk::Widget& widget, bool right_alignment, bool expand)
{
	Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
	parent_vbox.pack_start(*hbox, false, false);

	hbox->pack_start(labeled_widget, false, false);

	if(right_alignment)
		hbox->pack_end(widget, expand, expand);
	else
		hbox->pack_start(widget, expand, expand);
}

}}}

