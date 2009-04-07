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
#ifndef HEADER_MLIB_GTK_MISC
	#define HEADER_MLIB_GTK_MISC

	#include <string>

	#include <gtkmm/treemodelcolumn.h>

	#include "types.hpp"


	namespace m { namespace gtk {

	extern const unsigned int MOUSE_LEFT_BUTTON;
	extern const unsigned int MOUSE_RIGHT_BUTTON;

	/// Используется при создании Gtk::Window:
	/// window.set_border_width(WINDOW_BORDER_WIDTH)
	extern int WINDOW_BORDER_WIDTH;

	/// Используется при создании Gtk::Window:
	/// box.set_border_width(BOX_BORDER_WIDTH)
	extern int BOX_BORDER_WIDTH;

	/// Используется при создании Gtk::HBox:
	/// Gtk::HBox(false, m::gtk::HBOX_SPACING)
	extern int HBOX_SPACING;

	/// Используется при создании Gtk::VBox:
	/// Gtk::VBox(false, m::gtk::VBOX_SPACING)
	extern int VBOX_SPACING;

	/// Используется при формировании таблиц.
	///
	/// Например:
	/// Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name
	extern int TABLE_NAME_VALUE_SPACING;

	/// Используется при формировании таблиц.
	/// Величина отступа между строками.
	///
	/// Например:
	/// Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name
	/// [TABLE_ROWS_SPACING]
	/// Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name
	extern int TABLE_ROWS_SPACING;

	/// Используется при формировании таблиц.
	/// Величина отступа между колонками, содержащими пары (имя, значение).
	///
	/// Например:
	/// Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name[TABLE_NAME_VALUE_COLUMNS_SPACING]Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name
	/// [TABLE_ROWS_SPACING]
	/// Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name[TABLE_NAME_VALUE_COLUMNS_SPACING]Torrent name:[TABLE_NAME_VALUE_SPACING]some torrent name
	extern int TABLE_NAME_VALUE_COLUMNS_SPACING;


	enum Dialog_response {
		RESPONSE_OK,
		RESPONSE_CANCEL,
		RESPONSE_YES,
		RESPONSE_NO
	};



	/// Активизирует "режим дерева" для GtkCellRendererToggle.
	/// Модифицирует внутренние структуры GTK так, чтобы при упаковке
	/// GtkCellRendererToggle в одну колонку с другими GtkCellRenderer'ами
	/// GtkCellRendererToggle активизировался только в том случае, если
	/// пользователь кликнул мышкой именно по нему, а не по любому
	/// GtkCellRenderer'у данной колонки.
	void			activate_cell_renderer_toggle_tree_mode(void);

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
#endif

