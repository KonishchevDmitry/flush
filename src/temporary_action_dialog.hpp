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


#ifndef HEADER_TEMPORARY_ACTION_DIALOG
	#define HEADER_TEMPORARY_ACTION_DIALOG

	#include <boost/shared_ptr.hpp>

	#include <gtkmm/dialog.h>
	#include <gtkmm/window.h>

	#include <libglademm/xml.h>

	#include <mlib/gtk/dialog.hpp>



	namespace Temporary_action_dialog_aux { class Private; }

	/// Окно выбора времени, после истечения которого необходимо отменить
	/// выполненное над торрентами "временное действие".
	class Temporary_action_dialog: public m::gtk::Dialog
	{
		private:
			typedef Temporary_action_dialog_aux::Private Private;


		public:
			Temporary_action_dialog(BaseObjectType* cobject, const Glade_xml& glade);


		private:
			boost::shared_ptr<Private>	priv;


		public:
			/// Предназначена для инициализации виджета после конструирования
			/// его из Glade-представления.
			virtual
			void	init(Gtk::Window& parent_window, Temporary_action action, Torrents_group group);


		public:
			/// Возвращает установленное в данный момент время.
			Time	get_time(void) const;

		private:
			/// Обработчик сигнала на закрытие диалога.
			void	on_response_cb(int response_id) const;
	};

#endif

