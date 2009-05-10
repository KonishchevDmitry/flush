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


#include <gtkmm/toggleaction.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treepath.h>
#include <gtkmm/treeview.h>



namespace m { namespace gtk {

template<class Tree_view_columns, class Model_columns, class Model_type>
Tree_view<Tree_view_columns, Model_columns, Model_type>::
Tree_view(const Settings& settings)
:
	model_columns(),
	columns(model_columns),
	editing_mode(false)
{
	if(!this->columns.all.size())
		MLIB_LE();

	this->model = Model_type::create(model_columns);
	this->set_model(this->model);

	// Вставляем колонки -->
	{
		Gtk::Button* button;

		for(size_t i = 0; i < this->columns.all.size(); i++)
		{
			typename Tree_view_columns::Column& column = this->columns.all[i];

			// Вставляем колонку в TreeView
			this->append_column(*column.column);

			if( (button = get_tree_view_column_header_button(*column.column)) )
			{
				// Tooltip с расширенным описанием
				if(column.description != "")
					button->set_tooltip_text(column.description);

				// Назначаем обработчик нажатия правой кнопки мыши для заголовка колонки.
				button->signal_button_press_event().connect(
					sigc::mem_fun(
						*this,
						&Tree_view<Tree_view_columns, Model_columns, Model_type>::on_column_header_button_press_event_callback
					),
					false
				);
			}
		}
	}
	// Вставляем колонки <--

	// Добавляем мнимую колонку -->
		fake_column.set_min_width(0);
		fake_column.set_max_width(0);
		fake_column.set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
		this->append_column(this->fake_column);
	// Добавляем мнимую колонку <--

	// Применяем полученные настройки
	this->load_settings(settings);

	// Обработка правой кнопки мыши
	this->signal_button_press_event().connect(
		sigc::mem_fun(*this, &Tree_view<Tree_view_columns, Model_columns, Model_type>::on_button_press_event_callback),
		false
	);

	// Функция перетаскивания колонок
	this->set_column_drag_function(sigc::mem_fun(
		*this, &Tree_view<Tree_view_columns, Model_columns, Model_type>::column_drag_function
	));

	this->set_rules_hint();
	this->get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	this->set_search_column(this->model_columns.get_search_column_index());
}



template<class Tree_view_columns, class Model_columns, class Model_type>
void Tree_view<Tree_view_columns, Model_columns, Model_type>::
check_columns_visibility(Gtk::TreeViewColumn* preferred_column)
{
	Glib::ListHandle<Gtk::TreeViewColumn*> columns = this->get_columns();

	M_FOR_CONST_IT(columns, it)
	{
		if(*it == &this->fake_column)
			continue;

		if( (*it)->get_visible() )
			return;
	}

	if(!preferred_column)
		preferred_column = this->columns.all[0].column;

	preferred_column->set_visible();
}



template<class Tree_view_columns, class Model_columns, class Model_type>
bool Tree_view<Tree_view_columns, Model_columns, Model_type>::
column_drag_function(Gtk::TreeView* tree_view, Gtk::TreeViewColumn* dragged_column, Gtk::TreeViewColumn* left_column, Gtk::TreeViewColumn* right_column)
{
	// Если это правая граница, то не разрешаем - тут
	// находится мнимая колонка.
	if(right_column == NULL)
		return false;
	else
		return true;
}



template<class Tree_view_columns, class Model_columns, class Model_type>
void Tree_view<Tree_view_columns, Model_columns, Model_type>::
editing_end(void)
{
	MLIB_A(this->editing_mode);

	this->editing_mode = false;

	// Восстанавливаем параметры сортировки
	this->model->set_sort_column(this->editing_mode_sort_column_id, this->editing_mode_sort_order);
}



template<class Tree_view_columns, class Model_columns, class Model_type>
void Tree_view<Tree_view_columns, Model_columns, Model_type>::
editing_start(void)
{
	MLIB_A(!this->editing_mode);

	this->editing_mode = true;

	// Если в модели установлена сортировка по какой-либо колонке, то при изменении значения
	// этой колонки вся модель будет пересортирована и итератор станет недействительным,
	// в результате чего мы не сможем продвигаться дальше по строкам.
	//
	// Поэтому запоминаем параметры сортировки и отменяем ее на время изменения модели.
	// Кроме того это ускорит заполнение модели.
	this->model->get_sort_column_id(this->editing_mode_sort_column_id, this->editing_mode_sort_order);
	this->model->set_sort_column(Gtk::TreeSortable::DEFAULT_UNSORTED_COLUMN_ID, this->editing_mode_sort_order);
}



template<class Tree_view_columns, class Model_columns, class Model_type>
Gtk::TreeViewColumn* Tree_view<Tree_view_columns, Model_columns, Model_type>::
get_column_by_id(const std::string& column_id) const throw(m::Exception)
{
	std::map<std::string, Gtk::TreeViewColumn*>::const_iterator columns_iter =
		this->columns.columns_by_ids.find(column_id);

	// Колонки с таким идентификатором не существует
	if(columns_iter == this->columns.columns_by_ids.end())
		M_THROW(__("No such column: '%1'.", column_id));

	return columns_iter->second;
}



template<class Tree_view_columns, class Model_columns, class Model_type>
std::string Tree_view<Tree_view_columns, Model_columns, Model_type>::
get_column_id(const Gtk::TreeViewColumn* column) const throw(m::Exception)
{
	std::map<Gtk::TreeViewColumn*, std::string>::const_iterator ids_iter =
		this->columns.ids_by_columns.find(const_cast<Gtk::TreeViewColumn *>(column));

	// Такой колонки не существует
	if(ids_iter == this->columns.ids_by_columns.end())
		M_THROW_EMPTY();

	return ids_iter->second;
}



template<class Tree_view_columns, class Model_columns, class Model_type>
std::deque<Gtk::TreeModel::iterator> Tree_view<Tree_view_columns, Model_columns, Model_type>::
get_selected_rows(void)
{
	std::deque<Gtk::TreeModel::iterator> iters;
	Gtk::TreeView::Selection::ListHandle_Path rows_paths = this->get_selection()->get_selected_rows();
	
	M_FOR_CONST_IT(rows_paths, it)
		iters.push_back(this->model->get_iter(*it));
	
	return iters;
}



template<class Tree_view_columns, class Model_columns, class Model_type>
void Tree_view<Tree_view_columns, Model_columns, Model_type>::
handle_mouse_right_button_click(const GdkEventButton* const event)
{
	int cell_x, cell_y;
	Gtk::TreeModel::Path path;
	Gtk::TreeViewColumn* column;
	Glib::RefPtr<Gtk::TreeView::Selection> selection = this->get_selection();

	// Если щелчек пришелся на строку
	if(this->get_path_at_pos(event->x, event->y, path, column, cell_x, cell_y))
	{
		if(!selection->is_selected(path))
		{
			selection->unselect_all();
			selection->select(path);
		}
	}
	else
		selection->unselect_all();
}



template<class Tree_view_columns, class Model_columns, class Model_type>
void Tree_view<Tree_view_columns, Model_columns, Model_type>::
hide_column(Gtk::TreeViewColumn* const column)
{
	column->set_visible(false);

	// Проверяем, чтобы была видна хотя бы одна колонка
	this->check_columns_visibility(column);
}



template<class Tree_view_columns, class Model_columns, class Model_type>
void Tree_view<Tree_view_columns, Model_columns, Model_type>::
load_settings(const Settings& settings)
{
	Gtk::TreeViewColumn* column;

	// Настройки сортировки -->
		if(settings.sort_column != "")
		{
			column = NULL;

			try
			{
				column = this->get_column_by_id(settings.sort_column);
			}
			catch(m::Exception)
			{
				// Такой колонки уже не существует (конфигурационный файл устарел)
			}

			if(column)
			{
				int model_sort_column_id = column->get_sort_column_id();

				if(model_sort_column_id >= 0)
				{
					this->model->set_sort_column(
						model_sort_column_id,
						settings.sort_order == Tree_view_settings::SORT_ORDER_ASCENDING
							? Gtk::SORT_ASCENDING
							: Gtk::SORT_DESCENDING
					);
				}
			}
		}
	// Настройки сортировки <--

	// Применяем настройки к каждой колонке -->
	M_FOR_CONST_REVERSE_IT(settings.columns, settings_columns_iter)
	{
		try
		{
			column = this->get_column_by_id(settings_columns_iter->name);
		}
		catch(m::Exception)
		{
			// Такой колонки уже не существует (конфигурационный файл устарел)
			continue;
		}

		// Устанавливаем полученные настройки
		column->set_fixed_width(settings_columns_iter->width);
		column->set_visible(settings_columns_iter->visible);
		this->move_column_to_start(*column);
	}
	// Применяем настройки к каждой колонке <--

	// Проверяем, чтобы была видна хотя бы одна колонка
	this->check_columns_visibility();
}



template<class Tree_view_columns, class Model_columns, class Model_type>
bool Tree_view<Tree_view_columns, Model_columns, Model_type>::
on_button_press_event_callback(const GdkEventButton* const event)
{
	if(event->type != GDK_BUTTON_PRESS || event->button != m::gtk::MOUSE_RIGHT_BUTTON)
		return false;

	// Обрабатываем клик мышью
	this->handle_mouse_right_button_click(event);

	// Передаем управление обработчику нажатия на правую кнопку мыши
	this->on_mouse_right_button_click(event);

	return true;
}



template<class Tree_view_columns, class Model_columns, class Model_type>
bool Tree_view<Tree_view_columns, Model_columns, Model_type>::
on_column_header_button_press_event_callback(GdkEventButton* event)
{
	if(event->type != GDK_BUTTON_PRESS || event->button != m::gtk::MOUSE_RIGHT_BUTTON)
		return false;

	// Меню необходимо создавать каждый раз заново, чтобы имена колонок в нем
	// были в том порядке, в котором они находятся в данный момент в TreeView.

	Glib::RefPtr<Gtk::ActionGroup> action_group = Gtk::ActionGroup::create();
	Glib::ustring ui_info = "<ui><popup name='popup_menu'>";

	// Формируем меню -->
	{
		for(size_t i = 0; i < this->columns.all.size(); i++)
		{
			typename Tree_view_columns::Column& column = this->columns.all[i];

			// Создаем элемент меню для данной колонки -->
				action_group->add(
					Gtk::ToggleAction::create(
						column.id,
						column.column->get_title(),
						column.description,
						static_cast<bool>(column.column->get_visible())
					),
					sigc::bind<Gtk::TreeViewColumn*>(
						sigc::mem_fun(
							*this,
							column.column->get_visible()
								? &Tree_view<Tree_view_columns, Model_columns, Model_type>::hide_column
								: &Tree_view<Tree_view_columns, Model_columns, Model_type>::show_column
						),
						column.column
					)
				);
			// Создаем элемент меню для данной колонки <--

			// Добавляем элемент в меню
			ui_info += "<menuitem action='" + column.id + "'/>";
		}
	}
	// Формируем меню <--

	ui_info += "</popup></ui>";

	this->headers_menu_ui_manager = Gtk::UIManager::create();
	this->headers_menu_ui_manager->insert_action_group(action_group);
	this->headers_menu_ui_manager->add_ui_from_string(ui_info);

	Gtk::Menu* menu = dynamic_cast<Gtk::Menu*>(this->headers_menu_ui_manager->get_widget("/popup_menu"));
	menu->popup(event->button, event->time);

	return true;
}



template<class Tree_view_columns, class Model_columns, class Model_type>
void Tree_view<Tree_view_columns, Model_columns, Model_type>::
on_mouse_right_button_click(const GdkEventButton* const event)
{
	// Пустая функция, которую должны реализовать предки данного класса.
}



template<class Tree_view_columns, class Model_columns, class Model_type>
void Tree_view<Tree_view_columns, Model_columns, Model_type>::
remove_column(Gtk::TreeViewColumn& column)
{
	static_cast<Gtk::TreeView *>(this)->remove_column(column);
	this->columns.remove(&column);
}



template<class Tree_view_columns, class Model_columns, class Model_type>
void Tree_view<Tree_view_columns, Model_columns, Model_type>::
save_settings(Settings& settings) const
{
	std::string column_id;
	const Gtk::TreeViewColumn* column;
	Column_settings column_settings;

	// Сбрасываем все параментры к значениям по умолчанию
	settings = Settings();

	// Сохраняем настройки каждой колонки -->
	{
		// Получаем список всех колонок
		Glib::ListHandle<const Gtk::TreeViewColumn*> columns = this->get_columns();

		M_FOR_CONST_IT(columns, columns_iter)
		{
			column = *columns_iter;

			if(column == &this->fake_column)
				continue;

			// Получаем идентификатор колонки -->
				try
				{
					column_id = this->get_column_id(column);
				}
				catch(m::Exception)
				{
					MLIB_LE();
				}
			// Получаем идентификатор колонки <--

			// Сохраняем настройки колонки -->
				column_settings.set(
					column_id,
					column->get_visible(),
					column->property_width().get_value()
				);

				settings.columns.push_back(column_settings);
			// Сохраняем настройки колонки <--
		}
	}
	// Сохраняем настройки каждой колонки <--

	// Сохраняем настройки сортировки -->
	{
		int model_sort_column_id;
		Gtk::SortType sort_order;

		this->model->get_sort_column_id(model_sort_column_id, sort_order);

		// Если сортировка присутствует
		if(model_sort_column_id >= 0)
		{
			for(std::map<std::string, Gtk::TreeViewColumn*>::const_iterator
				it = this->columns.columns_by_ids.begin(); it != this->columns.columns_by_ids.end(); it++
			)
			{
				column = it->second;
				column_id = it->first;

				if(column->get_sort_column_id() == model_sort_column_id)
				{
					settings.sort_column = column_id;

					if(sort_order == Gtk::SORT_ASCENDING)
						settings.sort_order = Tree_view_settings::SORT_ORDER_ASCENDING;
					else
						settings.sort_order = Tree_view_settings::SORT_ORDER_DESCENDING;

					break;
				}
			}
		}
	}
	// Сохраняем настройки сортировки <--
}



template<class Tree_view_columns, class Model_columns, class Model_type>
void Tree_view<Tree_view_columns, Model_columns, Model_type>::
show_column(Gtk::TreeViewColumn* const column)
{
	column->set_visible(true);
}

}}

