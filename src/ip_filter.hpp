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


#ifndef HEADER_IP_FILTER
	#define HEADER_IP_FILTER

	#include <gtkmm/box.h>

	#include <libglademm/xml.h>

	#include "common.hpp"


	/// Список правил IP фильтра.
	class Ip_filter: public Gtk::VBox
	{
		private:
			class Private;


		public:
			Ip_filter(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& glade);
			~Ip_filter(void);


		private:
			Private*					priv;


		public:
			/// Добавляет правило в список.
			void							add(const Ip_filter_rule& rule);

			/// Возвращает текущий список правил IP-фильтра.
			std::vector<Ip_filter_rule>		get(void) const;

			/// Задает текущий список правил IP-фильтра.
			void							set(const std::vector<Ip_filter_rule>& ip_filter);

		private:
			/// Обработчик сигнала на нажатие кнопки "Добавить правило".
			void	on_add_button_clicked_cb(void);

			/// Обработчик сигнала на нажатие кнопки "Блокировать".
			void	on_block_button_clicked_cb(void);

			/// Обработчик сигнала на нажатие одной из управляющих кнопок IP-фильтра.
			void	on_control_button_clicked_cb(sigc::slot<void>& fun);

			/// Обработчик сигнала на нажатие кнопки "Редактировать".
			void	on_edit_button_clicked_cb(void);

			/// Сигнал на изменение выделения в списке правил IP-фильтра.
			void	on_selection_changed_cb(void);
	};

#endif

