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


#ifndef HEADER_ADD_TORRENT_DIALOG
#define HEADER_ADD_TORRENT_DIALOG


#include <boost/scoped_ptr.hpp>

#include <mlib/gtk/builder.hpp>
#include <mlib/gtk/dialog.hpp>

#include "common.hpp"



namespace Add_torrent_dialog_aux { class Private; }


/// Диалог, отображаемый при открытии *.torrent файла.
class Add_torrent_dialog: public m::gtk::Dialog
{
	private:
		typedef Add_torrent_dialog_aux::Private Private;


	public:
		Add_torrent_dialog(BaseObjectType* cobject, const m::gtk::Builder& builder);

	private:
		/// Диалог сам заботится о своем уничтожении.
		~Add_torrent_dialog(void);


	private:
		boost::scoped_ptr<Private>	priv;


	public:
		/// Начинает обработку запроса.
		/// @throw - m::Exception.
		void			process(Gtk::Window& parent_window, const std::string& torrent_path, const std::string& torrent_encoding);

	private:
		/// Обработчик сигнала на закрытие окна.
		virtual void	on_hide(void);

		/// Обработчик сигнала на реакцию пользователя.
		virtual void	on_response(int response);
};

#endif

