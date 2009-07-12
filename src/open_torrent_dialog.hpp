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


#ifndef HEADER_OPEN_TORRENT_DIALOG
	#define HEADER_OPEN_TORRENT_DIALOG

	#include <boost/shared_ptr.hpp>

	#include <gtkmm/filechooserdialog.h>
	#include <gtkmm/window.h>

	#include "common.hpp"


	/// Окно выбора *.torrent файла для открытия.
	class Open_torrent_dialog: public Gtk::FileChooserDialog
	{
		private:
			class Private;


		public:
			Open_torrent_dialog(Gtk::Window& parent_window);

		private:
			~Open_torrent_dialog(void) {};


		private:
			boost::shared_ptr<Private>	priv;


		private:
			/// Обработчик сигнала на закрытие окна.
			void	on_open_response_cb(int response_id);
	};

#endif

