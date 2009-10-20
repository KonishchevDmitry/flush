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


#ifndef HEADER_MLIB_GTK_MISC
#define HEADER_MLIB_GTK_MISC

#ifndef MLIB_ENABLE_LIBS_FORWARDS
	#include <gdkmm/pixbuf.h>

	#include <gtkmm/button.h>
	#include <gtkmm/dialog.h>
#endif
	#include <gtkmm/stock.h>
#ifndef MLIB_ENABLE_LIBS_FORWARDS
	#include <gtkmm/treemodelcolumn.h>
	#include <gtkmm/treeviewcolumn.h>
	#include <gtkmm/window.h>
#endif

#include <mlib/gtk/main.hpp>

#include "misc.hxx"


namespace m { namespace gtk {


/// Описывает кнопку, которую необходимо создать внутри диалога.
class Message_button_desc
{
	public:
		Message_button_desc(int response, const Gtk::StockID& stock_id);
		Message_button_desc(int response, const Glib::ustring& label, const Gtk::StockID& stock_id);

	private:
		Message_button_desc();


	private:
		int				response;
		Glib::ustring	label;
		Gtk::StockID	stock_id;


	public:
		void	add_to_dialog(Gtk::Dialog& dialog) const;
};



/// Активизирует "режим дерева" для GtkCellRendererToggle.
/// Модифицирует внутренние структуры GTK так, чтобы при упаковке
/// GtkCellRendererToggle в одну колонку с другими GtkCellRenderer'ами
/// GtkCellRendererToggle активизировался только в том случае, если
/// пользователь кликнул мышкой именно по нему, а не по любому
/// GtkCellRenderer'у данной колонки.
void			activate_cell_renderer_toggle_tree_mode(void);

/// Возвращает Stock'овую иконку.
Glib::RefPtr<
Gdk::Pixbuf>	get_stock_icon(const Gtk::StockID& id, const Gtk::IconSize& size);

/// Возвращает иконку с именем name из текущей темы или иконку
/// несуществующего изображения, если иконки с таким именем нет в текущей
/// теме.
Glib::RefPtr<
Gdk::Pixbuf>	get_theme_icon(const std::string& name, const Gtk::IconSize& size);

/// Возвращает кнопку, находящуюся в заголовке колонки GtkTreeView,
/// или NULL, если кнопку найти не удалось.
Gtk::Button*	get_tree_view_column_header_button(Gtk::TreeViewColumn& column);

/// Возвращает окно, в котором находится виджет или NULL, если виджет не
/// находится в окне.
Gtk::Window*	get_widget_window(Gtk::Widget& widget);

/// Отображает диалог с запросом ответа у пользователя и приостанавливает
/// выполнение программы до тех пор, пока не будет получен ответ.
/// Возвращает RESPONSE_OK или RESPONSE_CANCEL.
Dialog_response	ok_cancel_dialog(Gtk::Window& parent_window, const std::string& title, const std::string& message);

/// Отображает простое GTK сообщение и блокирует выполнение программы до
/// тех пор, пока пользователь не нажмет на кнопку OK.
void			message(Gtk::Window& parent_window, const std::string& title, const std::string& message);

/// Отображает диалог с кнопками, блокирующий выполнение программы.
/// Возвращает соответствующий response id.
int				message_with_buttons(Gtk::Window& parent_window, const std::string& title, const std::string& message, const std::vector<Message_button_desc>& buttons, int default_response);

/// Функция предназначена для увеличения скорости обновления Gtk::TreeView.
/// При обновлении модели GTK не обращает внимания на значение, и если оно
/// такое же как и раньше, то все равно перерисовывает Gtk::TreeView, что
/// очень сильно замедляет работу.
/// @return true - если строка была обновлена.
template<class Column_type, class Value_type> inline
bool			update_row(const Gtk::TreeRow& row, const Gtk::TreeModelColumn<Column_type>& column, const Value_type& value);

/// Отображает диалог с запросом ответа у пользователя и приостанавливает
/// выполнение программы до тех пор, пока не будет получен ответ.
/// @return - true, если пользователь ответил "да".
bool			yes_no_dialog(Gtk::Window& parent_window, const std::string& title, const std::string& message);

}}

#include "misc.hh"

#endif

