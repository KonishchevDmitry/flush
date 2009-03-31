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


#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/table.h>

#include <mlib/gtk/misc.hpp>

#include "daemon_proxy.hpp"
#include "main.hpp"
#include "torrent_details_view.hpp"



// Table -->
	class Table: public Gtk::Table
	{
		public:
			Table(void);


		private:
			int			rows_num;
			int			columns_num;
			int			cur_row;
			int			cur_column;
			const int 	gtk_columns_per_column;

		public:
			/// Добавляет Gtk::Label в текущую колонку и возвращает указатель на
			/// созданный Gtk::Label.
			Gtk::Label*	attach(const std::string& name);

			/// Добавляет виджет в текущую колонку и возвращает его.
			template<class T>
			T*			attach(const std::string& name, T* widget);

			/// Устанавливает текущую колонку. В нее будет производиться
			/// вставка виджетов.
			void		set_cur_column(int column);

		private:
			/// Добавляет одну колонку.
			void		add_column(void);

			/// Добавляет одну строку.
			void		add_row(void);

			/// Обертка над Gtk::Table::resize.
			void		resize(int rows_num, int columns_num);
	};



	Table::Table(void)
	:
		Gtk::Table(1, 1),
		rows_num(0),
		columns_num(0),
		cur_row(0),
		cur_column(0),
		gtk_columns_per_column(2)
	{
		this->set_cur_column(0);
	}



	void Table::add_column(void)
	{
		this->resize(this->rows_num, this->columns_num + 1);
	}



	void Table::add_row(void)
	{
		this->resize(this->rows_num + 1, this->columns_num);
	}



	Gtk::Label* Table::attach(const std::string& name)
	{
		Gtk::Label* label = Gtk::manage( new Gtk::Label() );
		label->set_alignment(Gtk::ALIGN_RIGHT);
		return this->attach(name, label);
	}



	template<class T>
	T* Table::attach(const std::string& name, T* widget)
	{
		if(this->cur_row >= this->rows_num)
			this->add_row();

		Gtk::Label* label = Gtk::manage( new Gtk::Label(name + ":") );
		label->set_alignment(Gtk::ALIGN_LEFT);

		Gtk::Table::attach(
			*label,
			this->cur_column * this->gtk_columns_per_column,
			this->cur_column * this->gtk_columns_per_column + 1,
			this->cur_row, this->cur_row + 1,
			Gtk::FILL, Gtk::FILL
		);
		Gtk::Table::attach(
			*widget,
			this->cur_column * this->gtk_columns_per_column + 1,
			this->cur_column * this->gtk_columns_per_column + 2,
			this->cur_row, this->cur_row + 1,
			Gtk::FILL, Gtk::FILL
		);

		this->cur_row++;

		return widget;
	}



	void Table::resize(int rows_num, int columns_num)
	{
		this->rows_num = rows_num;
		this->columns_num = columns_num;

		Gtk::Table::resize(
			rows_num ? rows_num : 1,
			columns_num ? columns_num * this->gtk_columns_per_column : 1
		);

		this->set_row_spacings(m::gtk::TABLE_ROWS_SPACING);
		this->set_col_spacings(m::gtk::TABLE_NAME_VALUE_SPACING);

		for(int i = 0; i < this->columns_num; i++)
		{
			this->set_col_spacing(
				i * this->gtk_columns_per_column + this->gtk_columns_per_column - 1,
				m::gtk::TABLE_NAME_VALUE_COLUMNS_SPACING
			);
		}
	}



	void Table::set_cur_column(int column)
	{
		if(this->columns_num <= column)
			this->resize(this->rows_num, this->columns_num + 1);

		this->cur_row = 0;
		this->cur_column = column;
	}
// Table <--



// Torrent_details_view -->
	Torrent_details_view::Torrent_details_view(void)
	:
		table( Gtk::manage( new Table() ) ),
		status( Gtk::manage( new Gtk::ProgressBar() ) )
	{
		this->set_border_width(m::gtk::BOX_BORDER_WIDTH);

		Gtk::HBox* main_hbox = Gtk::manage( new Gtk::HBox() );
		this->pack_start(*main_hbox, false, false, 0);

		Gtk::VBox* main_vbox = Gtk::manage( new Gtk::VBox(false, m::gtk::VBOX_SPACING * 2) );
		main_hbox->pack_start(*main_vbox, false, false, 0);

		this->status->set_ellipsize(Pango::ELLIPSIZE_MIDDLE);
		main_vbox->pack_start(*this->status, false, false, 0);

		// table -->
			main_vbox->pack_start(*this->table, false, false, 0);

			this->size = this->table->attach(_("Size"));
			this->requested_size = this->table->attach(_("Requested size"));
			this->downloaded_requested_size = this->table->attach(_("Downloaded requested size"));
			this->complete_percent = this->table->attach(__("%% Complete"));

			this->share_ratio = this->table->attach(_("Share ratio"));

			this->peers = this->table->attach(_("Peers"));
			this->seeds = this->table->attach(_("Seeds"));

			this->time_added = this->table->attach(_("Time added"));
			this->time_left = this->table->attach(_("Time left"));
			this->time_seeding = this->table->attach(_("Time seeding"));

			this->table->set_cur_column(1);

			this->total_download = this->table->attach(_("Total download"));
			this->total_payload_download = this->table->attach(_("Total payload download"));
			this->total_upload = this->table->attach(_("Total upload"));
			this->total_payload_upload = this->table->attach(_("Total payload upload"));
			this->total_failed = this->table->attach(_("Total failed"));
			this->total_redundant = this->table->attach(_("Total redundant"));

			this->download_speed = this->table->attach(_("Download speed"));
			this->payload_download_speed = this->table->attach(_("Download payload speed"));
			this->upload_speed = this->table->attach(_("Upload speed"));
			this->payload_upload_speed = this->table->attach(_("Upload payload speed"));
		// table <--

		this->show_all();
	}



	void Torrent_details_view::clear(void)
	{
		this->status->set_text("");
		this->status->set_fraction(0);

		this->set_string(	this->size,							""	);
		this->set_string(	this->requested_size,				""	);
		this->set_string(	this->downloaded_requested_size,	""	);
		this->set_string(	this->complete_percent,				""	);

		this->set_string(	this->total_download,				""	);
		this->set_string(	this->total_payload_download,		""	);
		this->set_string(	this->total_upload,					""	);
		this->set_string(	this->total_payload_upload,			""	);
		this->set_string(	this->total_failed,					""	);
		this->set_string(	this->total_redundant,				""	);

		this->set_string(	this->download_speed,				""	);
		this->set_string(	this->payload_download_speed,		""	);
		this->set_string(	this->upload_speed,					""	);
		this->set_string(	this->payload_upload_speed,			""	);

		this->set_string(	this->share_ratio,					""	);

		this->set_string(	this->peers,						""	);
		this->set_string(	this->seeds,						""	);

		this->set_string(	this->time_added,					""	);
		this->set_string(	this->time_left,					""	);
		this->set_string(	this->time_seeding,					""	);
	}



	void Torrent_details_view::set_size(Gtk::Label* label, Size size)
	{
		this->set_string(label, m::size_to_string(size));
	}



	void Torrent_details_view::set_speed(Gtk::Label* label, Speed speed)
	{
		this->set_string(label, m::speed_to_string(speed));
	}



	void Torrent_details_view::set_string(Gtk::Label* label, const std::string& string)
	{
		label->set_label(string);
	}



	void Torrent_details_view::set_time(Gtk::Label* label, Time time)
	{
		this->set_string(label, m::time_to_string_with_date(time));
	}



	void Torrent_details_view::update(const Torrent_id& torrent_id)
	{
		if(torrent_id)
		{
			try
			{
				// m::Exception
				Torrent_details details = get_daemon_proxy().get_torrent_details(torrent_id);

				this->status->set_text(
					"  " + details.name + ": " +
					details.get_status_string() + " " +
					m::to_string(details.progress) + " %  "
				);
				this->status->set_fraction( static_cast<double>(details.progress) / 100 );

				this->set_size(		this->size,							details.size);
				this->set_size(		this->requested_size,				details.requested_size);
				this->set_size(		this->downloaded_requested_size,	details.downloaded_requested_size);
				this->set_string(	this->complete_percent,				m::to_string(details.get_complete_percent()) + " %");

				this->set_size(		this->total_download,				details.total_download);
				this->set_size(		this->total_payload_download,		details.total_payload_download);
				this->set_size(		this->total_upload,					details.total_upload);
				this->set_size(		this->total_payload_upload,			details.total_payload_upload);
				this->set_size(		this->total_failed,					details.total_failed);
				this->set_size(		this->total_redundant,				details.total_redundant);

				this->set_speed(	this->download_speed,				details.download_speed);
				this->set_speed(	this->payload_download_speed,		details.payload_download_speed);
				this->set_speed(	this->upload_speed,					details.upload_speed);
				this->set_speed(	this->payload_upload_speed,			details.payload_upload_speed);

				this->set_string(	this->share_ratio,					get_share_ratio_string(details.total_payload_upload, details.total_payload_download));

				this->set_string(	this->peers,						m::to_string(details.peers_num));
				this->set_string(	this->seeds,						m::to_string(details.seeds_num));

				this->set_time(		this->time_added,					details.time_added);
				this->set_string(	this->time_left,					m::get_time_left_string(details.get_time_left()));
				this->set_string(	this->time_seeding,					m::get_time_duration_string(details.time_seeding));
			}
			catch(m::Exception& e)
			{
				// Этого торрента уже вполне может не оказаться, т. к. между
				// обновлением списка торрентов и этим запросом торрент мог
				// быть удален, например, функцией автоматического удаления
				// старых торрентов.
				MLIB_D(_C("Updating torrent details view failed. %1", EE(e)));
				this->clear();
			}
		}
		else
			this->clear();
	}
// Torrent_details_view

