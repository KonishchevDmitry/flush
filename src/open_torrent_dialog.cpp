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


#include <gtkmm/combobox.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/stock.h>
#include <gtkmm/window.h>

#include "application.hpp"
#include "client_settings.hpp"
#include "common.hpp"
#include "gui_lib.hpp"
#include "main.hpp"
#include "main_window.hpp"
#include "open_torrent_dialog.hpp"



class Open_torrent_dialog::Private
{
	public:
		class Encoding_model_columns: public Gtk::TreeModel::ColumnRecord
		{
			public:
				Encoding_model_columns(void);


			public:
				Gtk::TreeModelColumn<Glib::ustring>	name;
				Gtk::TreeModelColumn<Glib::ustring>	title;
		};
	

	public:
		Private(void);


	public:
		Encoding_model_columns			encoding_columns;
		Glib::RefPtr<Gtk::ListStore>	encoding_model;
		Gtk::ComboBox					encoding_combo;
};



Open_torrent_dialog::Private::Private(void)
:
	encoding_model( Gtk::ListStore::create(this->encoding_columns) ),
	encoding_combo(encoding_model)
{
	const m::Charset* charset = m::AVAILABLE_CHARSETS;

	while(charset->name)
	{
		Gtk::TreeModel::Row row = *(this->encoding_model->append());
		row[this->encoding_columns.name] = charset->name;
		row[this->encoding_columns.title] = charset->title;
		charset++;
	}
}



Open_torrent_dialog::Private::Encoding_model_columns::Encoding_model_columns(void)
{
	this->add(this->name);
	this->add(this->title);
}



Open_torrent_dialog::Open_torrent_dialog(Gtk::Window& parent_window)
:
	Gtk::FileChooserDialog(
		parent_window, format_window_title(_("Please choose a torrent file")),
		Gtk::FILE_CHOOSER_ACTION_OPEN
	),
	priv( new Private )
{
	// Выбор кодировки *.torrent файла -->
	{
		Gtk::VBox* vbox = this->get_vbox();

		Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox(false, m::gtk::HBOX_SPACING) );
		vbox->pack_start(*hbox, false, false);

		priv->encoding_combo.set_active(m::UTF_CHARSET_ID);
		priv->encoding_combo.pack_start(priv->encoding_columns.title);
		hbox->pack_end(priv->encoding_combo, false, false);

		Gtk::Label* label = Gtk::manage( new Gtk::Label(_("Torrent file encoding:")) );
		hbox->pack_end(*label, false, false);

		hbox->show_all();
	}
	// Выбор кодировки *.torrent файла <--

	// Добавляем кнопки -->
		this->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		this->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
	// Добавляем кнопки <--

	this->set_select_multiple();
	this->set_default_response(Gtk::RESPONSE_OK);

	// Загружаем настройки -->
	{
		const std::string& from = get_application().get_client_settings().gui.open_torrents_from;

		if(from != "")
			this->set_current_folder(U2L(from));
	}
	// Загружаем настройки <--

	// Добавляем фильтры по типам файлов -->
	{
		Gtk::FileFilter torrents_filter;
		torrents_filter.set_name(_("Torrent files"));
		torrents_filter.add_mime_type("application/x-bittorrent");
		this->add_filter(torrents_filter);

		Gtk::FileFilter any_filter;
		any_filter.set_name(_("Any files"));
		any_filter.add_pattern("*");
		this->add_filter(any_filter);
	}
	// Добавляем фильтры по типам файлов <--

	this->signal_response().connect(
		sigc::mem_fun(*this, &Open_torrent_dialog::on_open_response_cb)
	);

	this->show();
}



void Open_torrent_dialog::on_open_response_cb(int response_id)
{
	if(response_id != Gtk::RESPONSE_OK)
	{
		delete this;
		return;
	}

	Glib::SListHandle<Glib::ustring> filenames = this->get_filenames();
	const std::string encoding = priv->encoding_combo.get_active()->get_value(priv->encoding_columns.name);
	get_application().get_client_settings().gui.open_torrents_from = L2U(this->get_current_folder());

	delete this;

	M_FOR_CONST_IT(filenames, it)
		get_main_window().open_torrent(L2U(*it), encoding);
}

