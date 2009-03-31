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


#ifdef MLIB_ENABLE_GTK

#include <string>

#include <gtk/gtkcellrenderertoggle.h>

#include <gtkmm/messagedialog.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treeviewcolumn.h>
#include <gtkmm/window.h>

#include "../string.hpp"

#include "misc.hpp"



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



namespace
{
	/// Модифицированный обработчик сигнала на активацию GtkCellRendererToggle.
	gint gtk_cell_renderer_toggle_activate(	GtkCellRenderer* cell,
											GdkEvent* event,
											GtkWidget* widget,
											const gchar* path,
											GdkRectangle* background_area,
											GdkRectangle* cell_area,
											GtkCellRendererState flags);



	gint gtk_cell_renderer_toggle_activate(	GtkCellRenderer* cell,
											GdkEvent* event,
											GtkWidget* widget,
											const gchar* path,
											GdkRectangle* background_area,
											GdkRectangle* cell_area,
											GtkCellRendererState flags)
	{
		GtkCellRendererToggle* celltoggle = GTK_CELL_RENDERER_TOGGLE(cell);

		if(celltoggle->activatable)
		{
			if
			(
				!event ||
				event->type != GDK_BUTTON_PRESS ||
				(
					event->button.x >= cell_area->x &&
					event->button.x < cell_area->x + cell_area->width &&
					event->button.y >= cell_area->y &&
					event->button.y < cell_area->y + cell_area->height
				)
			)
			{
				g_signal_emit_by_name(cell, "toggled", path);
				return TRUE;
			}
		}

		return FALSE;
	}
}



void activate_cell_renderer_toggle_tree_mode(void)
{
	GtkCellRendererClass *cell_class;
	GtkCellRendererToggle *toggle_renderer;

	toggle_renderer = GTK_CELL_RENDERER_TOGGLE(gtk_cell_renderer_toggle_new());
	cell_class = GTK_CELL_RENDERER_CLASS(GTK_WIDGET_GET_CLASS(toggle_renderer));
	cell_class->activate = gtk_cell_renderer_toggle_activate;
	gtk_object_destroy(GTK_OBJECT(toggle_renderer));
}



Gtk::Button* get_tree_view_column_header_button(Gtk::TreeViewColumn& column)
{
	Gtk::Widget* widget;

	if( !( widget = column.get_widget() ) )
	{
		// Если для заголовка не установлен никакой виджет, устанавливаем свой Gtk::Label,
		// чтобы по нему можно было выйти на остальные виджеты заголовка.
		Gtk::Label* header_label = Gtk::manage(new Gtk::Label(column.get_title()));
		column.set_widget(*header_label);
		header_label->show();

		widget = header_label->get_parent();
	}

	// Поднимаемся вверх по дереву контейнеров,
	// пока не наткнемся на Gtk::Button.
	while(widget && widget->get_name() != "GtkButton")
		widget = widget->get_parent();

	return dynamic_cast<Gtk::Button*>(widget);
}



Gtk::Window* get_widget_window(Gtk::Widget& widget)
{
	std::string window_widget_name = Gtk::Window().get_name();
	Gtk::Widget* cur_widget = &widget;

	// Поднимаемся вверх по дереву контейнеров,
	// пока не наткнемся на Gtk::Window.
	while(cur_widget && cur_widget->get_name() != window_widget_name)
		cur_widget = cur_widget->get_parent();

	if(cur_widget)
		return dynamic_cast<Gtk::Window*>(cur_widget);
	else
		return NULL;
}



Dialog_response ok_cancel_dialog(Gtk::Window& parent_window, const std::string& title, const std::string& message)
{
	Gtk::MessageDialog dialog(
		parent_window, title, false,
		Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL,
		true
	);

	dialog.set_title(title);
	dialog.set_secondary_text(message);
	dialog.set_default_response(Gtk::RESPONSE_CANCEL);

	if(dialog.run() == Gtk::RESPONSE_OK)
		return RESPONSE_OK;
	else
		return RESPONSE_CANCEL;
}



void message(Gtk::Window& parent_window, const std::string& title, const std::string& message)
{
	Gtk::MessageDialog dialog(
		parent_window, title, false,
		Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK,
		true
	);

	dialog.set_title(title);
	dialog.set_secondary_text(message);
	dialog.set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
	dialog.set_default_response(Gtk::RESPONSE_OK);
	dialog.run();
}



bool yes_no_dialog(Gtk::Window& parent_window, const std::string& title, const std::string& message)
{
	Gtk::MessageDialog dialog(
		parent_window, title, false,
		Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO,
		true
	);

	dialog.set_title(title);
	dialog.set_secondary_text(message);
	dialog.set_default_response(Gtk::RESPONSE_NO);

	return dialog.run() == Gtk::RESPONSE_YES;
}

}}

#endif

