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


#ifdef MLIB_ENABLE_GTK
#ifndef HEADER_MLIB_CONTROLLED_LIST
	#define HEADER_MLIB_CONTROLLED_LIST

	#include <gtkmm/liststore.h>
	#include <gtkmm/treeview.h>

	#include <mlib/main.hpp>


	namespace m { namespace gtk {

	/// Список, в который можно добавлять элементы, удалять и перемещать их.
	class Controlled_list: public Gtk::TreeView
	{
		private:
			enum Direction { UP, DOWN };


		public:
			Controlled_list(
				Glib::RefPtr<Gtk::ListStore>	model,
				Gtk::Button*					remove_button = NULL,
				Gtk::Button*					up_button = NULL,
				Gtk::Button*					down_button = NULL
			);


		public:
			Glib::RefPtr<Gtk::ListStore>	model;

		private:
			Gtk::Button*					remove_button;
			Gtk::Button*					up_button;
			Gtk::Button*					down_button;


		public:
			/// Опускает выделенный в данный момент элемент вниз по списку или
			/// не делает ничего, если ни один элемент не выделен.
			void 						down(void);

			/// Удаляет выделенный в данный момент элемент из списка или
			/// не делает ничего, если ни один элемент не выделен.
			void						remove(void);

			/// Задает управляющие кнопки.
			void						set_buttons(Gtk::Button* remove_button, Gtk::Button* up_button, Gtk::Button* down_button);

			/// Опускает выделенный в данный момент элемент вверх по списку или
			/// не делает ничего, если ни один элемент не выделен.
			void 						up(void);

		private:
			/// Сигнал на изменение выделенных строк.
			void						on_selection_changed_cb(void);

			/// Перемещает выделенный в данный момент элемент на одну
			/// позицию вниз/вверх или не делает ничего, если ни один
			/// элемент не выдлен.
			void						push(Direction direction);

			/// Обновляет состояние управляющих кнопок.
			void						update_buttons(void);
	};

	}}

#endif
#endif

