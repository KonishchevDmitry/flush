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


#include <libtorrent/torrent_info.hpp>

#include <gtk/gtkbox.h>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/expander.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/scrolledwindow.h>

#include <mlib/gtk/builder.hpp>
#include <mlib/gtk/dialog.hpp>
#include <mlib/gtk/expander_settings.hpp>
#include <mlib/libtorrent.hpp>
#include <mlib/string.hpp>

#include "add_torrent_dialog.hpp"
#include "application.hpp"
#include "common.hpp"
#include "gui_lib.hpp"
#include "main.hpp"
#include "torrent_files_view.hpp"
#include "trackers_view.hpp"



namespace Add_torrent_dialog_aux
{

// Private -->
	class Private
	{
		public:
			Private(const m::gtk::Builder& builder, Add_torrent_dialog* dialog);


		public:
			Add_torrent_dialog*				dialog;

			Gtk::Entry*						torrent_path;
			Gtk::Entry*						torrent_name;
			Gtk::CheckButton*				start_torrent;

			Gtk::Expander*					paths_expander;
			Gtk::FileChooserButton*			download_to;
			Gtk::CheckButton*				copy_when_finished;
			Gtk::FileChooserButton*			copy_when_finished_to;

			Gtk::Expander*					files_expander;
			Gtk::ScrolledWindow*			files_scrolled_window;
			Torrent_files_static_view*		torrent_files_view;

			Gtk::Expander*					trackers_expander;
			Trackers_view*					trackers_view;

			std::string						torrent_encoding;


		public:
			/// Подгоняет размер окна до оптимального.
			void	fit_window(void);

			/// Обработчик сигнала на нажатие на флажок "Copy when finished to".
			void	on_copy_when_finished_toggle_cb(void);

			/// Обработчик сигнала на разворачивание Gtk::Expander'а.
			void	on_expander_changed_cb(Gtk::Expander* expander);

			/// Обработчик сигнала на разворачивание виджетов "Files".
			void	on_files_expanded_cb(void);

			/// Сохраняет текущие параметры отображения диалога.
			void	save_settings(void) const;
	};



	Private::Private(const m::gtk::Builder& builder, Add_torrent_dialog* dialog)
	:
		dialog(dialog)
	{
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "torrent_path", 			this->torrent_path);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "torrent_name", 			this->torrent_name);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "start_torrent", 			this->start_torrent);

		MLIB_GTK_BUILDER_GET_WIDGET(builder, "paths_expander", 			this->paths_expander);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "download_to", 			this->download_to);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "copy_when_finished", 		this->copy_when_finished);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "copy_when_finished_to", 	this->copy_when_finished_to);

		MLIB_GTK_BUILDER_GET_WIDGET(builder, "files_expander", 			this->files_expander);
		MLIB_GTK_BUILDER_GET_WIDGET(builder, "files_scrolled_window", 	this->files_scrolled_window);

		MLIB_GTK_BUILDER_GET_WIDGET(builder, "trackers_expander", 		this->trackers_expander);
	}



	void Private::fit_window(void)
	{
		int window_width;
		int window_height;
		GtkRequisition request = this->dialog->size_request();

		this->dialog->get_size(window_width, window_height);
		this->dialog->resize(window_width, request.height);
	}



	void Private::on_copy_when_finished_toggle_cb(void)
	{
		this->copy_when_finished_to->set_sensitive( this->copy_when_finished->get_active() );
	}



	void Private::on_expander_changed_cb(Gtk::Expander* expander)
	{
		// Если данный Gtk::Expander свернули и Gtk::Expander файловых виджетов
		// не может занять освободившееся место.
		if(!expander->get_expanded() && !this->files_expander->get_expanded())
		{
			Gtk::Widget* widget = *expander->get_children().begin();

			// Временно удаляем из него виджет, чтобы он выставил реально
			// необходимый ему size request.
			expander->remove();

			// Подгоняем размер окна
			this->fit_window();

			// Возвращаем виджет на место
			expander->add(*widget);
		}
	}



	void Private::on_files_expanded_cb(void)
	{
		// Если Gtk::Expander с файловыми виджетами свернули
		if(!this->files_expander->get_expanded())
		{
			// Временно удаляем из него виджеты, чтобы он выставил реально
			// необходимый ему size request.
			this->files_expander->remove();
		}

		// Устанавливаем подходящее поведение внутри бокса
		gtk_box_set_child_packing(
			GTK_BOX(this->files_expander->get_parent()->gobj()),
			GTK_WIDGET(this->files_expander->gobj()),
			this->files_expander->get_expanded(), TRUE, 0, GTK_PACK_START
		);

		// Если Gtk::Expander с файловыми виджетами свернули
		if(!this->files_expander->get_expanded())
		{
			// Подгоняем размер окна
			this->fit_window();

			// Возвращаем виджеты на место
			this->files_expander->add(*this->files_scrolled_window);
		}
	}



	void Private::save_settings(void) const
	{
		Add_torrent_dialog_settings& settings = get_client_settings().gui.add_torrent_dialog;

		this->dialog->save_settings(settings.window);

		settings.paths_expander->get(*this->paths_expander);

		settings.files_expander->get(*this->files_expander);
		this->torrent_files_view->save_settings(settings.torrent_files_view);

		settings.trackers_expander->get(*this->trackers_expander);
	}
// Private <--

}



Add_torrent_dialog::Add_torrent_dialog(BaseObjectType* cobject, const m::gtk::Builder& builder)
:
	m::gtk::Dialog(cobject),
	priv(new Private(builder, this))
{
	this->set_title(format_window_title(this->get_title()));
}



Add_torrent_dialog::~Add_torrent_dialog(void)
{
	MLIB_D("Destroying add torrent dialog...");
}



void Add_torrent_dialog::on_hide(void)
{
	delete this;
}



void Add_torrent_dialog::on_response(int response)
{
	MLIB_D(_C("Add torrent dialog response: %1.", response));

	if(response == Gtk::RESPONSE_OK)
	{
		if(m::is_empty_string(priv->torrent_name->get_text()))
		{
			show_warning_message(
				*this, _("Invalid torrent name"),
				_("You have entered invalid torrent name. Please enter non-empty torrent name.")
			);

			return;
		}

		try
		{
			std::auto_ptr<String_vector> trackers = std::auto_ptr<String_vector>(
				new String_vector(priv->trackers_view->get()) );

			New_torrent_settings new_torrent_settings(
				priv->torrent_name->get_text(),
				priv->start_torrent->get_active(),
				L2U(priv->download_to->get_filename()),
				(
					priv->copy_when_finished->get_active()
					?
						L2U(priv->copy_when_finished_to->get_filename())
					:
						""
				),
				priv->torrent_encoding,
				priv->torrent_files_view->get_files_settings(),
				trackers
			);

			get_application().add_torrent(priv->torrent_path->get_text(), new_torrent_settings);
		}
		catch(m::Exception& e)
		{
			MLIB_W(
				_("Opening torrent failed"),
				__(
					"Opening torrent '%1' failed. %2",
					priv->torrent_path->get_text(), EE(e)
				)
			);
		}
	}

	priv->save_settings();
	this->hide();
}



void Add_torrent_dialog::process(Gtk::Window& parent_window, const std::string& torrent_path, const std::string& torrent_encoding)
{
	try
	{
		MLIB_D("Processing add torrent dialog...");

		const Add_torrent_dialog_settings& settings = get_client_settings().gui.add_torrent_dialog;
		const User_settings& user_settings = get_client_settings().user;


		this->init(parent_window, settings.window);

		// Генерирует m::Exception
		lt::torrent_info torrent_info(m::lt::get_torrent_metadata(torrent_path, torrent_encoding).info);


		// Torrent -->
			priv->torrent_path->set_text(torrent_path);
			priv->torrent_name->set_text(torrent_info.name());
			priv->start_torrent->set_active(user_settings.start_torrent_on_adding);
		// Torrent <--


		// Paths -->
			priv->paths_expander->property_expanded().signal_changed().connect(
				sigc::bind<Gtk::Expander*>(
					sigc::mem_fun(*priv, &Private::on_expander_changed_cb), priv->paths_expander ));
			settings.paths_expander->set(*priv->paths_expander);


			priv->download_to->set_title(
				format_window_title(priv->download_to->get_title()) );
			priv->download_to->set_filename(U2L(user_settings.download_to));


			priv->copy_when_finished->signal_toggled().connect(
				sigc::mem_fun(*priv, &Private::on_copy_when_finished_toggle_cb));

			priv->copy_when_finished_to->set_title(
				format_window_title(priv->copy_when_finished_to->get_title()) );

			if(user_settings.copy_finished_to != "")
			{
				priv->copy_when_finished->set_active();
				priv->copy_when_finished_to->set_filename(U2L(user_settings.copy_finished_to));
			}
		// Paths <--


		// Files -->
			priv->files_expander->property_expanded().signal_changed().connect(
				sigc::mem_fun(*priv, &Private::on_files_expanded_cb) );
			settings.files_expander->set(*priv->files_expander);


			priv->torrent_files_view = Gtk::manage(
				// Генерирует m::Exception
				new Torrent_files_static_view(torrent_info, settings.torrent_files_view)
			);

			{
				// Запрашиваем место под Torrent_files_view (иначе GTK выделит ему
				// самый минимум, который только возможен для ScrolledWindow).

				int max_height = 200;
				GtkRequisition request = priv->torrent_files_view->size_request();

				if(request.height > max_height)
					request.height = max_height;
				priv->files_scrolled_window->set_size_request(-1, request.height);
			}

			priv->files_scrolled_window->add(*priv->torrent_files_view);
		// Files <--


		// Trackers -->
			priv->trackers_expander->property_expanded().signal_changed().connect(
				sigc::bind<Gtk::Expander*>(
					sigc::mem_fun(*priv, &Private::on_expander_changed_cb), priv->trackers_expander ));
			settings.trackers_expander->set(*priv->trackers_expander);

			priv->trackers_view = Gtk::manage( new Trackers_view );
			priv->trackers_view->set(m::lt::get_torrent_trackers(torrent_info));
			priv->trackers_expander->add(*priv->trackers_view);
		// Trackers <--

		priv->torrent_encoding = torrent_encoding;

		this->show_all();
	}
	catch(m::Exception&)
	{
		delete this;
		throw;
	}
}

