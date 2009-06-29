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


#include <sigc++/signal.h>

#include <gtkmm/enums.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeselection.h>
#include <gtkmm/treeview.h>

#include <mlib/gtk/signal_proxy.hpp>

#include "app_icons.hpp"
#include "categories_view.hpp"


namespace Categories_view_aux
{
	enum Torrent_category
	{
		/// На паузе.
		CATEGORY_PAUSED					= 1 << 0,

		/// Данные торрента проверяются, или стоят в очереди на проверку.
		CATEGORY_CHECKING				= 1 << 1,

		/// Скачивается (данные не идут).
		CATEGORY_WAITING_FOR_DOWNLOAD	= 1 << 2,

		/// Скачивается (данные идут).
		CATEGORY_DOWNLOADING			= 1 << 3,

		/// Раздается (данные не идут).
		CATEGORY_SEEDING				= 1 << 4,

		/// Раздается (данные идут).
		CATEGORY_UPLOADING				= 1 << 5,

		/// Ошибка трекера.
		CATEGORY_BROKEN_TRACKER			= 1 << 6,

		/// Все торренты.
		CATEGORY_ALL					=
										// Фиктивная категория, соответствующая
										// всем остальным торрентам.
										1 << 7 |
										CATEGORY_PAUSED |
										CATEGORY_CHECKING |
										CATEGORY_WAITING_FOR_DOWNLOAD |
										CATEGORY_DOWNLOADING |
										CATEGORY_SEEDING |
										CATEGORY_UPLOADING |
										CATEGORY_BROKEN_TRACKER
	};



	class Model_columns: public Gtk::TreeModel::ColumnRecord
	{
		public:
			Model_columns(void);


		public:
			Gtk::TreeModelColumn<int>							id;
			Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >	icon;
			Gtk::TreeModelColumn<Glib::ustring>					name;
			Gtk::TreeModelColumn<Glib::ustring>					tooltip;
	};



	class Impl
	{
		public:


		public:
			Impl(const Categories_view::Settings& settings);


		public:
			sigc::signal<void>					changed;
			Gtk::Frame							frame;

		private:
			Model_columns						columns;
			Glib::RefPtr<Gtk::ListStore>		model;

			Gtk::CellRendererPixbuf				icon_renderer;
			Gtk::CellRendererText				name_renderer;
			Gtk::TreeViewColumn					column_with_names;
			Gtk::TreeViewColumn					column_without_names;

			Gtk::TreeView						tree_view;
			Glib::RefPtr<Gtk::TreeSelection>	selection;
			bool								block_selection_signal;


		public:
			Categories_filter			get_filter(void) const;
			std::vector<std::string>	get_selected_items(void) const;
			bool						get_show_names(void) const;
			void						show_names(bool show = true);

		private:
			void	on_selection_changed_cb(void);
	};



	/// Возвращает имя категории.
	std::string			get_category_name(Torrent_category category);

	/// Возвращает категорию по ее имени.
	Torrent_category	get_category_by_name(std::string name) throw(m::Exception);



	std::string get_category_name(Torrent_category category)
	{
		switch(category)
		{
			case CATEGORY_PAUSED:
				return "paused";
				break;

			case CATEGORY_CHECKING:
				return "checking";
				break;

			case CATEGORY_WAITING_FOR_DOWNLOAD:
				return "waiting_for_download";
				break;

			case CATEGORY_DOWNLOADING:
				return "downloading";
				break;

			case CATEGORY_SEEDING:
				return "seeding";
				break;

			case CATEGORY_UPLOADING:
				return "uploading";
				break;

			case CATEGORY_BROKEN_TRACKER:
				return "tracker_error";
				break;

			case CATEGORY_ALL:
				return "all";
				break;

			default:
				MLIB_LE();
				break;
		}
	}



	Torrent_category get_category_by_name(std::string name) throw(m::Exception)
	{
		if(name == "paused")
			return CATEGORY_PAUSED;
		else if(name == "checking")
			return CATEGORY_CHECKING;
		else if(name == "waiting_for_download")
			return CATEGORY_WAITING_FOR_DOWNLOAD;
		else if(name == "downloading")
			return CATEGORY_DOWNLOADING;
		else if(name == "seeding")
			return CATEGORY_SEEDING;
		else if(name == "uploading")
			return CATEGORY_UPLOADING;
		else if(name == "tracker_error")
			return CATEGORY_BROKEN_TRACKER;
		else if(name == "all")
			return CATEGORY_ALL;
		else
			M_THROW(__("Invalid torrent category name: %1.", name));
	}



	Model_columns::Model_columns(void)
	{
		this->add(this->id);
		this->add(this->icon);
		this->add(this->name);
		this->add(this->tooltip);
	}



	Impl::Impl(const Categories_view::Settings& settings)
	:
		model( Gtk::ListStore::create(this->columns) ),

		column_without_names("", this->columns.icon),

		tree_view(model),
		selection(tree_view.get_selection()),
		block_selection_signal(false)
	{
		this->tree_view.set_headers_visible(false);
		this->tree_view.set_tooltip_column(this->columns.tooltip.index());

		this->column_with_names.pack_start(this->icon_renderer, false);
		this->column_with_names.pack_start(this->name_renderer, true);
		this->column_with_names.add_attribute(this->icon_renderer.property_pixbuf(), this->columns.icon);
		this->column_with_names.add_attribute(this->name_renderer.property_text(), this->columns.name);
		this->tree_view.set_search_column(this->columns.name);
		this->tree_view.append_column(this->column_with_names);

		this->tree_view.append_column(this->column_without_names);

		this->selection->set_mode(Gtk::SELECTION_MULTIPLE);
		this->selection->signal_changed().connect(
			sigc::mem_fun(*this, &Impl::on_selection_changed_cb));

		// Заполняем модель -->
		{
			int selected_items = 0;

			M_FOR_CONST_IT(settings.selected_items, it)
			{
				try
				{
					selected_items |= get_category_by_name(*it);
				}
				catch(m::Exception&)
				{
				}
			}

			Gtk::TreeRow row;
			Gtk::TreeRow all_row;

			all_row = row = *this->model->append();
			row[this->columns.id] = CATEGORY_ALL;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_ALL, Gtk::ICON_SIZE_MENU);
			row[this->columns.name] = _Q("All torrents (as short as possible)|All");
			row[this->columns.tooltip] = _("All torrents");

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_PAUSED;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_PAUSED, Gtk::ICON_SIZE_MENU);
			row[this->columns.name] = _Q("Paused torrents (as short as possible)|Paused");
			row[this->columns.tooltip] = _("Paused torrents");
			if(selected_items & CATEGORY_PAUSED) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_CHECKING;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_CHECKING, Gtk::ICON_SIZE_MENU);
			row[this->columns.name] = _Q("Checking torrents (as short as possible)|Checking");
			row[this->columns.tooltip] = _("Checking torrents");
			if(selected_items & CATEGORY_CHECKING) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_WAITING_FOR_DOWNLOAD;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_WAITING_FOR_DOWNLOAD, Gtk::ICON_SIZE_MENU);
			row[this->columns.name] = _Q("Waiting for download torrents (as short as possible)|Down. wait");
			row[this->columns.tooltip] = _("Waiting for download torrents");
			if(selected_items & CATEGORY_WAITING_FOR_DOWNLOAD) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_DOWNLOADING;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_DOWNLOADING, Gtk::ICON_SIZE_MENU);
			row[this->columns.name] = _Q("Downloading torrents (as short as possible)|Downloading");
			row[this->columns.tooltip] = _("Downloading torrents");
			if(selected_items & CATEGORY_DOWNLOADING) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_SEEDING;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_SEEDING, Gtk::ICON_SIZE_MENU);
			row[this->columns.name] = _Q("Seeding torrents (as short as possible)|Seeding");
			row[this->columns.tooltip] = _("Finished (seeding) torrents");
			if(selected_items & CATEGORY_SEEDING) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_UPLOADING;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_UPLOADING, Gtk::ICON_SIZE_MENU);
			row[this->columns.name] = _Q("Uploading torrents (as short as possible)|Uploading");
			row[this->columns.tooltip] = _("Uploading torrents");
			if(selected_items & CATEGORY_UPLOADING) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_BROKEN_TRACKER;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_TRACKER_ERROR, Gtk::ICON_SIZE_MENU);
			row[this->columns.name] = _Q("Torrents with tracker error (as short as possible)|Tracker error");
			row[this->columns.tooltip] = _("Torrents with tracker error");
			if(selected_items & CATEGORY_BROKEN_TRACKER) this->selection->select(row);

			if(selected_items == CATEGORY_ALL)
				this->selection->select(all_row);
		}
		// Заполняем модель <--

		this->frame.add(this->tree_view);
	}



	Categories_filter Impl::get_filter(void) const
	{
		int categories = 0;
		Gtk::TreeSelection::ListHandle_Path rows_paths = this->selection->get_selected_rows();

		M_FOR_CONST_IT(rows_paths, it)
			categories |= this->model->get_iter(*it)->get_value(this->columns.id);

		return categories;
	}



	std::vector<std::string> Impl::get_selected_items(void) const
	{
		std::vector<std::string> items;
		Gtk::TreeSelection::ListHandle_Path rows_paths = this->selection->get_selected_rows();

		M_FOR_CONST_IT(rows_paths, it)
		{
			items.push_back(get_category_name(Torrent_category(
				this->model->get_iter(*it)->get_value(this->columns.id)
			)));
		}

		return items;
	}



	bool Impl::get_show_names(void) const
	{
		return this->column_with_names.get_visible();
	}



	void Impl::on_selection_changed_cb(void)
	{
		if(this->block_selection_signal)
			return;

		{
			Gtk::TreeIter all_iter = this->model->children().begin();

			// Если пользователь каким-либо образом выделил группу "Все", то
			// оставляем выделение только на ней.
			if(this->selection->is_selected(all_iter))
			{
				this->block_selection_signal = true;
					this->selection->unselect_all();
					this->selection->select(all_iter);
				this->block_selection_signal = false;
			}
		}

		this->changed.emit();
	}



	void Impl::show_names(bool show)
	{
		this->column_with_names.set_visible(show);
		this->column_without_names.set_visible(!show);
	}
}

using namespace Categories_view_aux;



Categories_filter::Categories_filter(int categories)
:
	categories(categories)
{
}



bool Categories_filter::operator()(const Torrent_info& info) const
{
	if(
		this->categories == CATEGORY_ALL ||
		( info.paused && this->categories & CATEGORY_PAUSED ) ||
		( info.tracker_broken && this->categories & CATEGORY_BROKEN_TRACKER )
	)
		return true;
	else
	{
		switch(info.status)
		{
			case Torrent_info::ALLOCATING:
				return false;
				break;

			case Torrent_info::QUEUED_FOR_CHECKING:
			case Torrent_info::CHECKING_FILES:
				return this->categories & CATEGORY_CHECKING;
				break;

			case Torrent_info::WAITING_FOR_METADATA_DOWNLOAD:
			case Torrent_info::WAITING_FOR_DOWNLOAD:
				return this->categories & CATEGORY_WAITING_FOR_DOWNLOAD;
				break;

			case Torrent_info::DOWNLOADING_METADATA:
			case Torrent_info::DOWNLOADING:
				return this->categories & CATEGORY_DOWNLOADING;
				break;

			case Torrent_info::SEEDING:
				return this->categories & CATEGORY_SEEDING;
				break;

			case Torrent_info::UPLOADING:
				return this->categories & CATEGORY_UPLOADING;
				break;

			case Torrent_info::UNKNOWN:
			default:
				return false;
				break;
		}
	}
}



Categories_view_settings::Categories_view_settings(void)
:
	visible(true),
	show_names(true)
{
	selected_items.push_back(get_category_name(CATEGORY_ALL));
}



Categories_view::Categories_view(const Settings& settings)
:
	impl(new Impl(settings))
{
	this->set_no_show_all(true);
	this->add(impl->frame);
	impl->frame.show_all();

	if(settings.visible)
		this->show();
	this->show_names(settings.show_names);

	this->signal_hide().connect( sigc::mem_fun(impl->changed, &M_TYPEOF(impl->changed)::emit) );
	this->signal_show().connect( sigc::mem_fun(impl->changed, &M_TYPEOF(impl->changed)::emit) );
}



Categories_filter Categories_view::get_filter(void) const
{
	if(this->is_visible())
		return impl->get_filter();
	else
		return Categories_filter(CATEGORY_ALL);
}



void Categories_view::save_settings(Settings* settings) const
{
	settings->visible = this->is_visible();
	settings->show_names = impl->get_show_names();
	settings->selected_items = impl->get_selected_items();
}



void Categories_view::show_names(bool show)
{
	impl->show_names(show);
}



m::gtk::Signal_proxy<void> Categories_view::signal_changed(void)
{
	return impl->changed;
}

