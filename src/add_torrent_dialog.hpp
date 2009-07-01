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

	#include <string>

	#include <gtkmm/button.h>
	#include <gtkmm/checkbutton.h>
	#include <gtkmm/entry.h>
	#include <gtkmm/filechooserbutton.h>
	#include <gtkmm/filechooserdialog.h>

	#include <libtorrent/torrent_info.hpp>

	#include <mlib/gtk/window.hpp>

	#include "torrent_files_view.hpp"



	/// Диалог, отображаемый при открытии *.torrent файла.
	class Add_torrent_dialog: public m::gtk::Window
	{
		public:
			/// @throw - m::Exception.
			Add_torrent_dialog(Gtk::Window& parent_window, const std::string& torrent_path, const std::string& torrent_encoding);

			~Add_torrent_dialog(void) {}


		private:
			Gtk::Entry					torrent_name_entry;
			Gtk::CheckButton			start_torrent_check_button;

			Gtk::FileChooserDialog		download_to_dialog;
			Gtk::FileChooserButton		download_to_button;

			Gtk::CheckButton			copy_when_finished_to_check_button;
			Gtk::FileChooserDialog		copy_when_finished_to_dialog;
			Gtk::FileChooserButton		copy_when_finished_to_button;

			std::string					torrent_path;
			lt::torrent_info			torrent_info;
			std::string					torrent_encoding;
			Torrent_files_static_view	torrent_files_view;


		private:
			/// Закрывает окно.
			void 			close(void);

			/// При нажатии на кнопку Cancel.
			void 			on_cancel_button_callback(void);

			/// Обработчик сигнала на закрытие окна.
			bool			on_close_callback(GdkEventAny* event);

			/// Обработчик сигнала на нажатие на флажок "Copy when finished to".
			void			on_copy_when_finished_to_toggled_callback(void);

			/// При нажатии на кнопку OK.
			void			on_ok_button_callback(void);

			/// Сохраняет текущие настройки.
			void			save_settings(Add_torrent_dialog_settings& settings);
	};

#endif

