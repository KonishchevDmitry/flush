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


#ifndef HEADER_OPEN_MAGNET_DIALOG
#define HEADER_OPEN_MAGNET_DIALOG


#include <boost/scoped_ptr.hpp>

#include <mlib/gtk/builder.hpp>
#include <mlib/gtk/dialog.hpp>

#include "common.hpp"



namespace Open_magnet_dialog_aux { class Private; }


/// Диалог, отображаемый при открытии magnet-ссылки.
class Open_magnet_dialog: public m::gtk::Dialog
{
	private:
		typedef Open_magnet_dialog_aux::Private Private;


	public:
		Open_magnet_dialog(BaseObjectType* cobject, const m::gtk::Builder& builder);

	private:
		~Open_magnet_dialog(void);


	private:
		boost::scoped_ptr<Private>	priv;


	public:
		/// Создает диалог и производит все необходимые действия по обработке
		/// запроса.
		static void	create(Gtk::Window& parent_window);

		/// Возвращает magnet-ссылку, введенную пользователем.
		std::string	get_uri(void);

	private:
		/// Обработчик сигнала на закрытие окна.
		virtual void	on_hide(void);
};

#endif

