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


#ifndef HEADER_TORRENT_DETAILS_VIEW
	#define HEADER_TORRENT_DETAILS_VIEW

	#include <gtkmm/box.h>


	class Table;

	class Torrent_details_view
	:
		public Torrent_info_widget,
		public Gtk::VBox
	{
		public:
			Torrent_details_view(void);


		private:
			Gtk::ProgressBar*		status;

			Gtk::Label*				size;
			Gtk::Label*				requested_size;
			Gtk::Label*				downloaded_requested_size;
			Gtk::Label*				complete_percent;

			Gtk::Label*				total_download;
			Gtk::Label*				total_payload_download;
			Gtk::Label*				total_upload;
			Gtk::Label*				total_payload_upload;
			Gtk::Label*				total_failed;
			Gtk::Label*				total_redundant;

			Gtk::Label*				download_speed;
			Gtk::Label*				download_payload_speed;
			Gtk::Label*				upload_speed;
			Gtk::Label*				upload_payload_speed;

			Gtk::Label*				share_ratio;

			Gtk::Label*				peers;
			Gtk::Label*				seeds;

			Gtk::Label*				time_added;
			Gtk::Label*				time_left;
			Gtk::Label*				time_seeding;


			Gtk::Label*				tracker_status;

			Gtk::Label*				publisher_url;
			std::string				publisher_url_string;
			Gtk::EventBox*			publisher_url_event_box;


		public:
			/// Инициирует обновление виджета.
			virtual
			void		update(const Torrent_id& torrent_id);

		private:
			/// Сбрасывает все значения к значениям по умолчанию.
			void		clear(void);

			/// Обработчик сигнала на нажатие клик мышью по Publisher URL.
			bool		on_publisher_url_button_press_event_cb(GdkEventButton* event);

			/// Обработчик сигнала на попадание мыши в область видимости Publisher URL.
			bool		on_publisher_url_enter_notify_event_cb(GdkEventCrossing* event);

			/// Обработчик сигнала на выход мыши их области видимости Publisher URL.
			bool		on_publisher_url_leave_notify_event_cb(GdkEventCrossing* event);

			/// Устанавливает значение размера.
			void		set_size(Gtk::Label* label, Size size);

			/// Устанавливает значение скорости.
			void		set_speed(Gtk::Label* label, Speed speed);

			/// Устанавливает строку.
			void		set_string(Gtk::Label* label, const std::string& string);

			/// Устанавливает значение времени.
			void		set_time(Gtk::Label* label, Time time);
	};
#endif

