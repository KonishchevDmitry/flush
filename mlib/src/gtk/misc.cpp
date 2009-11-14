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


#include <gtk/gtkcellrenderertoggle.h>
#include <gtk/gtkversion.h>
#include <gtk/gtkwindow.h>

#include <gdkmm/pixbuf.h>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treeviewcolumn.h>
#include <gtkmm/window.h>

#include <mlib/gtk/main.hpp>

#include "misc.hpp"



namespace m { namespace gtk {

namespace
{
	enum Message_type {
		MESSAGE_TYPE_INFO,
		MESSAGE_TYPE_WARNING
	};



	/// Функция, используемая для форматирования заголовка окна в соответствии с
	/// тем, как принято отображать заголовки в данном приложении.
	Glib::ustring (*format_window_title_function)(const Glib::ustring&) = NULL;



	/// Модифицированный обработчик сигнала на активацию GtkCellRendererToggle.
	gint			gtk_cell_renderer_toggle_activate(	GtkCellRenderer* cell,
											GdkEvent* event,
											GtkWidget* widget,
											const gchar* path,
											GdkRectangle* background_area,
											GdkRectangle* cell_area,
											GtkCellRendererState flags);

	/// Отображает сообщение пользователю, не возвращая управления до тех пор,
	/// пока он не нажмет кнопку OK.
	void			show_message(GtkWindow* parent_window, Message_type type, Glib::ustring title, Glib::ustring message);



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



	void show_message(GtkWindow* parent_window, Message_type type, Glib::ustring title, Glib::ustring message)
	{
		Gtk::StockID dialog_stock_id;

		Glib::ustring short_message;
		Glib::ustring long_message;
		const int labels_width = 300;

		// Определяем все переменные, которые зависят от типа сообщения -->
			switch(type)
			{
				case MESSAGE_TYPE_INFO:
					if(title == "")
						title = _("Information");
					dialog_stock_id = Gtk::Stock::DIALOG_INFO;
					break;

				case MESSAGE_TYPE_WARNING:
					if(title == "")
						title = _("Warning");
					dialog_stock_id = Gtk::Stock::DIALOG_WARNING;
					break;

				default:
					MLIB_LE();
					break;
			}
		// Определяем все переменные, которые зависят от типа сообщения <--

		MLIB_D(_C("GUI message: [%1] %2", title, message));

		// Когда выводится содержимое Errors_pool, сообщение всегда будет содержать
		// пустую строку, выводить которую не имеет смысла.
		if(!message.empty() && message[0] == '\n')
			message = message.substr(1);

		// Разбиваем сообщение на два - короткое и длинное.
		// В итоге должно получиться одно из двух:
		// * будет короткое сообщение в несколько строк и не будет длинного.
		// * будет короткое сообщение в одну строку и длинное, в которое будут
		//   помещены остальные строки.
		// -->
		{
			const size_t short_message_max_lines = 3;
			size_t lines_num = 0;
			size_t new_string_pos = 0;

			while(new_string_pos != message.npos && lines_num <= short_message_max_lines)
			{
				new_string_pos = message.find('\n', lines_num ? new_string_pos + 1 : 0);
				lines_num++;
			}

			if(lines_num > short_message_max_lines)
			{
				new_string_pos = message.find('\n');
				short_message = message.substr(0, new_string_pos);
				long_message = message.substr(new_string_pos + 1);
			}
			else
				short_message = message;
		}
		// <--

		Gtk::Dialog message_dialog(format_window_title(title), true);
		gtk_window_set_transient_for(GTK_WINDOW(message_dialog.gobj()), parent_window);
		message_dialog.property_destroy_with_parent() = true;
		message_dialog.set_border_width(m::gtk::WINDOW_BORDER_WIDTH);
		message_dialog.set_resizable(false);

		Gtk::HBox* main_hbox = Gtk::manage( new Gtk::HBox(false, m::gtk::HBOX_SPACING * 3) );
		message_dialog.get_vbox()->add(*main_hbox);

		// Image -->
		{
			Gtk::Image* error_image = Gtk::manage( new Gtk::Image(dialog_stock_id, Gtk::ICON_SIZE_DIALOG) );
			error_image->set_alignment(0, 0);
			main_hbox->pack_start(*error_image, false, false);
		}
		// Image <--

		Gtk::VBox* main_vbox = Gtk::manage( new Gtk::VBox(false, m::gtk::VBOX_SPACING * 3) );
		main_hbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
		main_hbox->pack_start(*main_vbox, false, false);

		// Title -->
		{
			Gtk::Label* title_label = Gtk::manage( new Gtk::Label() );
			title_label->set_markup("<span weight='bold' size='larger'>" + Glib::Markup::escape_text(title) + "</span>");
			title_label->set_line_wrap();
			title_label->set_selectable();
			title_label->set_alignment(0, 0);
			if(!long_message.empty())
				title_label->set_size_request(labels_width, -1);
			main_vbox->pack_start(*title_label, false, false);
		}
		// Title <--

		// Short message -->
		{
			Gtk::Label* message_label = Gtk::manage( new Gtk::Label(short_message) );
			message_label->set_line_wrap();
			message_label->set_selectable();
			message_label->set_alignment(0, 0);
			if(!long_message.empty())
				message_label->set_size_request(labels_width, -1);
			main_vbox->pack_start(*message_label, false, false);
		}
		// Short message <--

		// Long message -->
			if(!long_message.empty())
			{
				Gtk::ScrolledWindow* scrolled_window = Gtk::manage( new Gtk::ScrolledWindow() );
				scrolled_window->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
				scrolled_window->set_size_request(-1, 100);
				main_vbox->add(*scrolled_window);

				Gtk::VBox* vbox = Gtk::manage( new Gtk::VBox() );
				vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
				scrolled_window->add(*vbox);

				Gtk::Label* message_label = Gtk::manage( new Gtk::Label(long_message) );
				message_label->set_alignment(0, 0);
				message_label->set_selectable();
				vbox->add(*message_label);
			}
		// Long message <--

		message_dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
		message_dialog.show_all_children();
		message_dialog.run();
	}
}



// Message_button_desc -->
	Message_button_desc::Message_button_desc(int response, const Gtk::StockID& stock_id)
	:
		response(response),
		stock_id(stock_id)
	{
	}



	Message_button_desc::Message_button_desc(int response, const Glib::ustring& label, const Gtk::StockID& stock_id)
	:
		response(response),
		label(label),
		stock_id(stock_id)
	{
	}



	void Message_button_desc::add_to_dialog(Gtk::Dialog& dialog) const
	{
		Gtk::Button* button;

		if(label.empty())
			button = Gtk::manage( new Gtk::Button(this->stock_id) );
		else
		{
			button = Gtk::manage( new Gtk::Button(this->label) );
			button->set_image( *Gtk::manage( new Gtk::Image(this->stock_id, Gtk::ICON_SIZE_BUTTON) ));
		}

		#if GTK_CHECK_VERSION(2, 18, 0)
			button->set_can_default();
		#else
			button->property_can_default() = true;
		#endif
		dialog.add_action_widget(*button, this->response);
		button->show();
	}
// Message_button_desc <--



void activate_cell_renderer_toggle_tree_mode(void)
{
	GtkCellRendererClass *cell_class;
	GtkCellRendererToggle *toggle_renderer;

	toggle_renderer = GTK_CELL_RENDERER_TOGGLE(gtk_cell_renderer_toggle_new());
	cell_class = GTK_CELL_RENDERER_CLASS(GTK_WIDGET_GET_CLASS(toggle_renderer));
	cell_class->activate = gtk_cell_renderer_toggle_activate;
	gtk_object_destroy(GTK_OBJECT(toggle_renderer));
}



Glib::ustring format_window_title(const Glib::ustring& title)
{
	if(format_window_title_function)
		return (*format_window_title_function)(title);
	else
		return title;
}



Glib::RefPtr<Gdk::Pixbuf> get_stock_icon(const Gtk::StockID& id, const Gtk::IconSize& size)
{
	static Gtk::HBox some_widget;
	return some_widget.render_icon(id, size);
}



Glib::RefPtr<Gdk::Pixbuf> get_theme_icon(const std::string& name, const Gtk::IconSize& size)
{
	gint width;
	gint height;

	MLIB_A(Gtk::IconSize::lookup(size, width, height));

	try
	{
		return Gtk::IconTheme::get_default()->load_icon(
			name, height
			// Внимание!
			// В Ubuntu 8.04 (gtkmm-2.4 2.12) этот параметр обязателен. В
			// Ubuntu 9.04 (gtkmm-2.4 2.16) он точно не обязателен.
		#if !GTK_CHECK_VERSION(2, 16, 0)
			, static_cast<Gtk::IconLookupFlags>(0)
		#endif
		);
	}
	catch(Gtk::IconThemeError&)
	{
		return m::gtk::get_stock_icon(Gtk::Stock::MISSING_IMAGE, size);
	}
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
	Gtk::Widget* cur_widget = &widget;

	// Поднимаемся вверх по дереву контейнеров,
	// пока не наткнемся на Gtk::Window.
	while(cur_widget && !dynamic_cast<Gtk::Window*>(cur_widget))
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

	dialog.set_title(format_window_title(title));
	dialog.set_secondary_text(message);
	dialog.set_default_response(Gtk::RESPONSE_CANCEL);
	dialog.property_destroy_with_parent() = true;

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

	dialog.set_title(format_window_title(title));
	dialog.set_secondary_text(message);
	dialog.set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
	dialog.set_default_response(Gtk::RESPONSE_OK);
	dialog.property_destroy_with_parent() = true;
	dialog.run();
}



int message_with_buttons(Gtk::Window& parent_window, const std::string& title, const std::string& message, const std::vector<Message_button_desc>& buttons, int default_response)
{
	Gtk::MessageDialog dialog(
		parent_window, title, false,
		Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE,
		true
	);

	dialog.set_title(format_window_title(title));
	dialog.set_secondary_text(message);

	BOOST_FOREACH(const Message_button_desc& desc, buttons)
		desc.add_to_dialog(dialog);

	dialog.set_default_response(default_response);
	dialog.property_destroy_with_parent() = true;

	return dialog.run();
}



void set_format_window_title_function(Glib::ustring (*func)(const Glib::ustring&))
{
	format_window_title_function = func;
}



void show_info_message(GtkWindow* parent_window, const Glib::ustring& message)
{
	show_info_message(parent_window, "", message);
}



void show_info_message(Gtk::Window& parent_window, const Glib::ustring& message)
{
	show_info_message(parent_window.gobj(), "", message);
}



void show_info_message(GtkWindow* parent_window, const Glib::ustring& title, const Glib::ustring& message)
{
	show_message(parent_window, MESSAGE_TYPE_INFO, title, message);
}



void show_warning_message(GtkWindow* parent_window, const Glib::ustring& message)
{
	show_warning_message(parent_window, "", message);
}



void show_warning_message(Gtk::Window& parent_window, const Glib::ustring& message)
{
	show_warning_message(parent_window.gobj(), "", message);
}



void show_warning_message(GtkWindow* parent_window, const Glib::ustring& title, const Glib::ustring& message)
{
	show_message(parent_window, MESSAGE_TYPE_WARNING, title, message);
}



bool yes_no_dialog(Gtk::Window& parent_window, const std::string& title, const std::string& message)
{
	Gtk::MessageDialog dialog(
		parent_window, title, false,
		Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO,
		true
	);

	dialog.set_title(format_window_title(title));
	dialog.set_secondary_text(message);
	dialog.set_default_response(Gtk::RESPONSE_NO);
	dialog.property_destroy_with_parent() = true;

	return dialog.run() == Gtk::RESPONSE_YES;
}

}}

