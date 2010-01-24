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


#ifndef HEADER_MLIB_GTK_VBOX
#define HEADER_MLIB_GTK_VBOX

#include <gtkmm/box.h>

#include <mlib/gtk/main.hpp>


namespace m { namespace gtk { namespace vbox {

/// Добавляет большой заголовок в Gtk::VBox.
void	add_big_header(Gtk::VBox& parent_vbox, const std::string& header_string, bool add_space = true, bool add_separator = true, bool centered = false);

/// Добавляет заголовок в Gtk::VBox.
void	add_header(Gtk::VBox& parent_vbox, const std::string& header_string, bool centered = false);

/// Добавляет строку в Gtk::VBox так, чтобы это выглядело
/// следующим образом:
/// | [label_string]:           [string] |
void	add_labeled_string(Gtk::VBox& parent_vbox, const std::string& label_string, const std::string& string, bool right_alignment = true, bool expand = false);

/// Вставляет пустое пространство стандартного размера.
void	add_space(Gtk::VBox& parent_vbox);

/// Добавляет виджет в Gtk::VBox так, чтобы это выглядело
/// следующим образом:
/// | [label_string]            [widget] |
void	add_widget_with_label(Gtk::VBox& parent_vbox, const std::string& label_string, Gtk::Widget& widget, bool right_alignment = true, bool expand = false);

/// Добавляет виджеты в Gtk::VBox так, чтобы это выглядело
/// следующим образом:
/// | [labeled_widget]          [widget] |
void	add_widget_with_labeled_widget(Gtk::VBox& parent_vbox, Gtk::Widget& labeled_widget, Gtk::Widget& widget, bool right_alignment = true, bool expand = false);

}}}

#endif

