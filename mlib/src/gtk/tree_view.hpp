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


#ifndef HEADER_MLIB_GTK_TREE_VIEW
#define HEADER_MLIB_GTK_TREE_VIEW

#include <deque>

#include <gtkmm/actiongroup.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treeviewcolumn.h>
#include <gtkmm/uimanager.h>

#include <mlib/gtk/main.hpp>
#include <mlib/gtk/tree_view_settings.hpp>


#define M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(id, description)					\
{																			\
	this->add(#id, &this->id, (description));								\
	this->id.set_sort_column(model_columns.id);								\
	this->id.get_first_cell_renderer()->property_xalign().set_value(1.0);	\
}

#define M_GTK_TREE_VIEW_ADD_STRING_COLUMN(id, description)	\
{															\
	this->add(#id, &this->id, (description));				\
	this->id.set_sort_column(model_columns.id);				\
}



namespace m { namespace gtk {

class Tree_view_model_columns: public Gtk::TreeModel::ColumnRecord
{
	public:
		Tree_view_model_columns(void);


	private:
		const Gtk::TreeModelColumnBase*		search_column;


	public:
		/// Возвращает индекс колонки, которая должна использоваться для
		/// поиска или -1, если данная колонка не была определена.
		int		get_search_column_index(void);

	protected:
		/// Устанавливает колонку в качестве колонки, используемой для поиска.
		void	set_search_column(const Gtk::TreeModelColumnBase& search_column);
};



class Tree_view_columns
{
	public:
		class Column
		{
			public:
				Column(
					const std::string&		id,
					Gtk::TreeViewColumn*	column,
					const std::string&		menu_name,
					const std::string&		description
				);

			public:
				std::string				id;
				Gtk::TreeViewColumn*	column;

				/// Имя данной колонки в меню, которое отображается по правому
				/// клику.
				std::string				menu_name;

				/// Описание. Отображается во всплывающих подсказках.
				std::string				description;
		};


	public:
		std::vector<Column>								all;
		std::map<std::string, Gtk::TreeViewColumn*>		columns_by_ids;
		std::map<Gtk::TreeViewColumn*, std::string>		ids_by_columns;


	public:
		void	remove(Gtk::TreeViewColumn* column);

	protected:
		void	add(const std::string& id, Gtk::TreeViewColumn* column, const std::string& description, bool resizable = true);

		/// @param menu_name - имя данной колонки в меню, которое отображается
		/// по правому клику.
		void	add(const std::string& id, Gtk::TreeViewColumn* column, const std::string& menu_name, const std::string& description, bool resizable = true);
};



template<class Tree_view_columns, class Model_columns, class Model_type>
class Tree_view: public Gtk::TreeView
{
	public:
		typedef Tree_view_settings Settings;
		typedef Tree_view_column_settings Column_settings;


	public:
		Tree_view(const Settings& settings = Settings());

	public:
		Model_columns					model_columns;
		Tree_view_columns				columns;
		Glib::RefPtr<Model_type>		model;
	
	private:
		Glib::RefPtr<Gtk::UIManager>	headers_menu_ui_manager;
		
		/// Мнимая колонка, которая вставляется в самый конец,
		/// чтобы самая последняя настоящая колонка не растягивалась
		/// до правой границы TreeView.
		Gtk::TreeViewColumn				fake_column;

		/// Хранит текущее состояние (в каком режиме находится TreeView).
		bool							editing_mode;

		/// Значение, которое необходимо восстановить при выходе
		/// из режима редактирования.
		int								editing_mode_sort_column_id;

		/// Значение, которое необходимо восстановить при выходе
		/// из режима редактирования.
		Gtk::SortType					editing_mode_sort_order;
	

	public:
// Теперь не используется, т. к. из-за этого глючит GTK - при переключении
// режимов начинают мигать активные в данный момент тултипы.
#if 0
		/// Выходит из режима редактирования модели TreeView.
		/// Используется для ускорения обновления модели и
		/// для обеспечения возможности итерации по модели,
		/// если в нее вставляются или удаляются строки.
		void									editing_end(void);

		/// Входит в режим редактирования модели TreeView.
		/// Используется для ускорения обновления модели и
		/// для обеспечения возможности итерации по модели,
		/// если в нее вставляются или удаляются строки.
		void									editing_start(void);
#endif

		/// Возвращает список итераторов выделенных строк.
		std::deque<Gtk::TreeModel::iterator>	get_selected_rows(void);

		/// Применяет настройки к TreeView.
		void									load_settings(const Settings& settings);

		/// Удаляет колонку из TreeView.
		void									remove_column(Gtk::TreeViewColumn& column);

		/// Сохраняет текущие настройки TreeView.
		void									save_settings(Settings& settings) const;
	
	private:
		/// Если все колонки скрыты, то делает видимой колонку preferred_column
		/// или первую колонку, если preferred_column не задана.
		void					check_columns_visibility(Gtk::TreeViewColumn* preferred_column = NULL);

		/// Функция, определяющая, можно ли переместить колонку в заданное место.
		bool					column_drag_function(Gtk::TreeView*, Gtk::TreeViewColumn*, Gtk::TreeViewColumn*, Gtk::TreeViewColumn*);

		/// Возвращает колонку по ее идентификатору.
		/// @throw - m::Exception.
		Gtk::TreeViewColumn*	get_column_by_id(const std::string& column_id) const;

		/// Возвращает имя колонки.
		/// @throw - m::Exception.
		std::string				get_column_id(const Gtk::TreeViewColumn* column) const;

		/// Обрабатывает нажатие правой кнопки мыши на TreeView для реализации
		/// всплывающего меню (выделяет или снимает выделение с тех строк, для
		/// которых это необходимо).
		void					handle_mouse_right_button_click(const GdkEventButton* const event);

		/// Отключает отображение колонки.
		void					hide_column(Gtk::TreeViewColumn* const column);

		/// Обработчик клика мышью по TreeView.
		bool					on_button_press_event_callback(const GdkEventButton* const event);

		/// Отображает всплывающее меню со списком колонок TreeView,
		/// с помощью которого можно включить или отключить отображение
		/// выбранной колонки.
		bool					on_column_header_button_press_event_callback(GdkEventButton* event);

		/// Обработчик нажатия на правую кнопку мыши по TreeView.
		virtual void			on_mouse_right_button_click(const GdkEventButton* const event);

		/// Включает отображение колонки.
		void					show_column(Gtk::TreeViewColumn* const column);
};
// Tree_view <--

}}

#include "tree_view.hh"

#endif

