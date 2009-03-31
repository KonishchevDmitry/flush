/**************************************************************************
*                                                                         *
*   Flush - GTK-based BitTorrent client                                   *
*   http://sourceforge.net/projects/flush                                 *
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



template<class T>
void Statistics_window::attach_value(const std::string& name, T value, std::string (*value_to_string_function)(T))
{
	Gtk::Label* label;

	this->add_row();

	label = Gtk::manage( new Gtk::Label(name + ":  ") );
	label->set_alignment(Gtk::ALIGN_LEFT);
	this->table.attach(*label, 0, 1, this->rows_num - 1, this->rows_num);

	label = Gtk::manage( new Gtk::Label( (*value_to_string_function)(value) ) );
	label->set_alignment(Gtk::ALIGN_RIGHT);
	this->table.attach(*label, 1, 2, this->rows_num - 1, this->rows_num);
}

