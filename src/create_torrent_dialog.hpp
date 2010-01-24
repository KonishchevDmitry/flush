/**************************************************************************
*                                                                         *
*   Flush - GTK-based BitTorrent client                                   *
*   http://sourceforge.net/projects/flush                                 *
*                                                                         *
*   Copyright (C) 2009-2010, Dmitry Konishchev                            *
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


#ifndef HEADER_CREATE_TORRENT_DIALOG
	#define HEADER_CREATE_TORRENT_DIALOG

	#include <string>

	#include <gtkmm/treemodel.h>

	#ifndef MLIB_ENABLE_LIBS_FORWARDS
		#include <gtkmm/checkbutton.h>
		#include <gtkmm/combobox.h>
		#include <gtkmm/entry.h>
	#endif

	#include <mlib/gtk/window.hpp>

	#include "common.hpp"


	/// Диалог, отображаемый при создании *.torrent файла.
	class Create_torrent_dialog: public m::gtk::Window
	{
		private:
			class Piece_sizes_model_columns: public Gtk::TreeModel::ColumnRecord
			{
				public:
					Piece_sizes_model_columns(void);


				public:
					Gtk::TreeModelColumn<Size>		   		size;
					Gtk::TreeModelColumn<Glib::ustring>		size_string;
			};

			class Trackers_list;


		public:
			Create_torrent_dialog(Gtk::Window& parent_window);
			~Create_torrent_dialog(void);


		private:
			Gtk::Entry*					torrent_files_path_entry;

			Gtk::CheckButton*			private_torrent_button;

			Piece_sizes_model_columns	piece_sizes_columns;
			Gtk::ComboBox*				piece_size_combo;

			Trackers_view*				trackers_view;


		private:
			/// При нажатии на кнопку Cancel.
			void 			on_cancel_button_callback(void);

			/// Обработчик сигнала на закрытие окна.
			bool			on_close_callback(GdkEventAny* event);

			/// Обработчик сигнала на завершение процесса создания торрента.
			void			on_finished_callback(void);

			/// При нажатии на кнопку OK.
			void			on_ok_button_callback(void);

			/// При нажатии на кнопку выбора файла или директории, из которых
			/// необходимо создать торрент.
			void			on_select_callback(bool directory);

			/// Сохраняет настройки.
			void			save_settings(Create_torrent_dialog_settings& settings);
	};

#endif

