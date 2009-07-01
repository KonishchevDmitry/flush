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


#ifndef HEADER_TRACKERS_VIEW
	#define HEADER_TRACKERS_VIEW

	#include <string>
	#include <vector>

	#include <gtkmm/box.h>



	/// Список трекеров с возможностью их добавления/удаления.
	class Trackers_view: public Gtk::VBox
	{
		private:
			class Gui;
			enum Direction { UP, DOWN };


		public:
			Trackers_view(void);
			~Trackers_view(void);


		private:
			Gui*							gui;

			/// Генерируется каждый раз, когда пользователь изменяет список
			/// трекеров.
			sigc::signal<void>				changed_signal;


		public:
			/// Возвращает массив трекеров, присутствующих в данный
			/// момент в списке.
			std::vector<std::string>	get(void) const;

			/// Задает список трекеров для отображения.
			void						set(const std::vector<std::string>& trackers);

			/// Возвращает сигнал, генерируемый каждый раз, когда пользователь
			/// изменяет список трекеров.
			sigc::signal<void>			signal_changed(void);

		private:
			/// Добавляет торрент в список.
			/// @throw - m::Exception.
			void						append(const std::string& url);

			/// При нажатии на кнопку "Добавить трекер".
			void 						on_add_callback(void);

			/// При нажатии на кнопку "Опустить трекер в списке".
			void 						on_down_callback(void);

			/// При нажатии на кнопку "Удалить трекер".
			void 						on_remove_callback(void);

			/// При нажатии на кнопку "Поднять трекер в списке".
			void 						on_up_callback(void);

			/// Перемещает выделенный в данный момент трекер на одну
			/// позицию вниз/вверх или не делает ничего, если ни один
			/// трекер не выдлен.
			void						push(Direction direction);

			/// Удаляет выделенный в данный момент трекер из списка или
			/// не делает ничего, если ни один трекер не выделен.
			void						remove(void);
	};

#endif

