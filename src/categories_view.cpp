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

#include <mlib/gtk/misc.hpp>
#include <mlib/gtk/signal_proxy.hpp>

#include "app_icons.hpp"
#include "categories_view.hpp"
#include "common.hpp"


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

		// Фиктивная категория, соответствующая
		// всем остальным торрентам.
		CATEGORY_OTHER					= 1 << 7,

		/// Все торренты.
		CATEGORY_ALL					=
										CATEGORY_PAUSED |
										CATEGORY_CHECKING |
										CATEGORY_WAITING_FOR_DOWNLOAD |
										CATEGORY_DOWNLOADING |
										CATEGORY_SEEDING |
										CATEGORY_UPLOADING |
										CATEGORY_BROKEN_TRACKER |
										CATEGORY_OTHER
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
		private:
			enum {
				ROW_ALL,
				ROW_PAUSED,
				ROW_CHECKING,
				ROW_WAITING_FOR_DOWNLOAD,
				ROW_DOWNLOADING,
				ROW_SEEDING,
				ROW_UPLOADING,
				ROW_BROKEN_TRACKER,
				ROWS_NUM
			};


		public:
			Impl(const Categories_view::Settings& settings);


		public:
			/// Сигнал на изменение выделенных в данный момент категорий.
			sigc::signal<void>					changed_signal;

			/// Сигнал на необходимость обновления статистики по торрентам.
			sigc::signal<void>					needs_update_signal;

			Gtk::Frame							frame;

		private:
			Model_columns						columns;
			Glib::RefPtr<Gtk::ListStore>		model;

			Gtk::TreeRow						rows[ROWS_NUM];
			std::string							names[ROWS_NUM];

			Gtk::CellRendererPixbuf				icon_renderer;
			Gtk::CellRendererText				name_renderer;
			Gtk::TreeViewColumn					column_all;
			Gtk::TreeViewColumn					column_only_icons;

			Gtk::TreeView						tree_view;
			Glib::RefPtr<Gtk::TreeSelection>	selection;
			bool								block_selection_signal;

			// Текущие настройки.
			bool								show_names;
			bool								show_counters;

			/// Определяет, содержит ли колонка модели name в данный момент в
			/// точности имя категории.
			bool								has_only_name;


		public:
			Categories_filter			get_filter(void) const;
			std::vector<std::string>	get_selected_items(void) const;
			bool						get_show_counters(void) const;
			bool						get_show_names(void) const;
			void						set_show_counters(bool show = true);
			void						set_show_names(bool show = true);
			void						state_changed(bool widget_active);
			void						update(const std::vector<Torrent_info>& torrents);

		private:
			void						on_selection_changed_cb(void);
			void						settings_changed(void);
	};



	/// Возвращает категории, которым принадлежит торрент (не включая маску
	/// CATEGORY_ALL).
	int					get_categories(const Torrent_info& info);

	/// Возвращает имя категории.
	std::string			get_category_name(Torrent_category category);

	/// Возвращает категорию по ее имени.
	/// @throw - m::Exception.
	Torrent_category	get_category_by_name(std::string name);



	int get_categories(const Torrent_info& info)
	{
		int categories = 0;

		if(info.paused)
			categories |= CATEGORY_PAUSED;
		else
		{
			if(info.tracker_broken)
				categories |= CATEGORY_BROKEN_TRACKER;
		}

		switch(info.status)
		{
			case Torrent_info::QUEUED_FOR_CHECKING:
			case Torrent_info::CHECKING_FILES:
				categories |= CATEGORY_CHECKING;
				break;

			case Torrent_info::WAITING_FOR_METADATA_DOWNLOAD:
			case Torrent_info::WAITING_FOR_DOWNLOAD:
				categories |= CATEGORY_WAITING_FOR_DOWNLOAD;
				break;

			case Torrent_info::DOWNLOADING_METADATA:
			case Torrent_info::DOWNLOADING:
				categories |= CATEGORY_DOWNLOADING;
				break;

			case Torrent_info::SEEDING:
				categories |= CATEGORY_SEEDING;
				break;

			case Torrent_info::UPLOADING:
				categories |= CATEGORY_UPLOADING;
				break;

			case Torrent_info::ALLOCATING:
			case Torrent_info::UNKNOWN:
			default:
				categories |= CATEGORY_OTHER;
				break;
		}

		return categories;
	}



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



	Torrent_category get_category_by_name(std::string name)
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
			M_THROW(_("invalid category name"));
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

		column_only_icons("", this->columns.icon),

		tree_view(model),
		selection(tree_view.get_selection()),
		block_selection_signal(false),

		show_names(settings.show_names),
		show_counters(settings.show_counters),
		has_only_name(false)
	{
		this->tree_view.set_headers_visible(true);
		this->tree_view.set_tooltip_column(this->columns.tooltip.index());

		this->tree_view.append_column(this->column_only_icons);

		this->column_all.pack_start(this->icon_renderer, false);
		this->column_all.pack_start(this->name_renderer, true);
		this->column_all.add_attribute(this->icon_renderer.property_pixbuf(), this->columns.icon);
		this->column_all.add_attribute(this->name_renderer.property_text(), this->columns.name);
		this->column_all.set_sizing(Gtk::TREE_VIEW_COLUMN_AUTOSIZE);
		this->tree_view.set_search_column(this->columns.name);
		this->tree_view.append_column(this->column_all);

		this->selection->set_mode(Gtk::SELECTION_MULTIPLE);
		this->selection->signal_changed().connect(
			sigc::mem_fun(*this, &Impl::on_selection_changed_cb));

		// Кэшируем имена категорий -->
			this->names[ROW_ALL]					= _Q("All torrents (as short as possible)|All");
			this->names[ROW_PAUSED]					= _Q("Paused torrents (as short as possible)|Paused");
			this->names[ROW_CHECKING]				= _Q("Checking torrents (as short as possible)|Checking");
			this->names[ROW_WAITING_FOR_DOWNLOAD]	= _Q("Waiting for download torrents (as short as possible)|Down. wait");
			this->names[ROW_DOWNLOADING]			= _Q("Downloading torrents (as short as possible)|Downloading");
			this->names[ROW_SEEDING]				= _Q("Seeding torrents (as short as possible)|Seeding");
			this->names[ROW_UPLOADING]				= _Q("Uploading torrents (as short as possible)|Uploading");
			this->names[ROW_BROKEN_TRACKER]			= _Q("Torrents with tracker error (as short as possible)|Tracker error");
		// Кэшируем имена категорий <--

		// Заполняем модель -->
		{
			int selected_items = 0;

			M_FOR_CONST_IT(settings.selected_items, it)
			{
				try
				{
					selected_items |= get_category_by_name(*it);
				}
				catch(m::Exception& e)
				{
					MLIB_SW(__("Can't select torrent category '%1': %2.", EE(e)));
				}
			}

			Gtk::TreeRow row;
			Gtk::TreeRow all_row;

			row = all_row = *this->model->append();
			row[this->columns.id] = CATEGORY_ALL;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_ALL, Gtk::ICON_SIZE_MENU);
			row[this->columns.tooltip] = _("All torrents");

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_PAUSED;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_PAUSED, Gtk::ICON_SIZE_MENU);
			row[this->columns.tooltip] = _("Paused torrents");
			if(selected_items & CATEGORY_PAUSED) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_CHECKING;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_CHECKING, Gtk::ICON_SIZE_MENU);
			row[this->columns.tooltip] = _("Checking torrents");
			if(selected_items & CATEGORY_CHECKING) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_WAITING_FOR_DOWNLOAD;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_WAITING_FOR_DOWNLOAD, Gtk::ICON_SIZE_MENU);
			row[this->columns.tooltip] = _("Waiting for download torrents");
			if(selected_items & CATEGORY_WAITING_FOR_DOWNLOAD) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_DOWNLOADING;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_DOWNLOADING, Gtk::ICON_SIZE_MENU);
			row[this->columns.tooltip] = _("Downloading torrents");
			if(selected_items & CATEGORY_DOWNLOADING) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_SEEDING;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_SEEDING, Gtk::ICON_SIZE_MENU);
			row[this->columns.tooltip] = _("Finished (seeding) torrents");
			if(selected_items & CATEGORY_SEEDING) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_UPLOADING;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_UPLOADING, Gtk::ICON_SIZE_MENU);
			row[this->columns.tooltip] = _("Uploading torrents");
			if(selected_items & CATEGORY_UPLOADING) this->selection->select(row);

			row = *this->model->append();
			row[this->columns.id] = CATEGORY_BROKEN_TRACKER;
			row[this->columns.icon] = app_icons::get_pixbuf(app_icons::ICON_TORRENTS_TRACKER_ERROR, Gtk::ICON_SIZE_MENU);
			row[this->columns.tooltip] = _("Torrents with tracker error");
			if(selected_items & CATEGORY_BROKEN_TRACKER) this->selection->select(row);

			if(selected_items == CATEGORY_ALL)
				this->selection->select(all_row);
		}
		// Заполняем модель <--

		// Кэшируем итераторы к строкам -->
		{
			size_t id = 0;
			Gtk::TreeNodeChildren iters = this->model->children();

			M_FOR_CONST_IT(iters, it)
				this->rows[id++] = **it;
		}
		// Кэшируем итераторы к строкамп<--

		this->settings_changed();

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



	bool Impl::get_show_counters(void) const
	{
		return this->show_counters;
	}



	bool Impl::get_show_names(void) const
	{
		return this->show_names;
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

		this->changed_signal.emit();
	}



	void Impl::set_show_counters(bool show)
	{
		if(this->show_counters != show)
		{
			this->show_counters = show;
			this->settings_changed();
		}
	}



	void Impl::set_show_names(bool show)
	{
		if(this->show_names != show)
		{
			this->show_names = show;
			this->settings_changed();
		}
	}



	void Impl::settings_changed(void)
	{
		bool show = this->show_names || this->show_counters;
		this->column_all.set_visible(show);
		this->column_only_icons.set_visible(!show);

		if(this->show_counters)
			this->needs_update_signal();
		else if(this->show_names)
			this->update(std::vector<Torrent_info>());
	}



	void Impl::state_changed(bool widget_active)
	{
		this->changed_signal();

		if(widget_active)
			this->settings_changed();
	}



	void Impl::update(const std::vector<Torrent_info>& torrents)
	{
		if(this->show_names || this->show_counters)
		{
			if(this->show_counters)
			{
				std::vector<size_t> counters(ROWS_NUM);

				M_FOR_CONST_IT(torrents, it)
				{
					counters[ROW_ALL]++;

					int categories = get_categories(*it);

					if(categories & CATEGORY_PAUSED)
						counters[ROW_PAUSED]++;

					if(categories & CATEGORY_CHECKING)
						counters[ROW_CHECKING]++;

					if(categories & CATEGORY_WAITING_FOR_DOWNLOAD)
						counters[ROW_WAITING_FOR_DOWNLOAD]++;

					if(categories & CATEGORY_DOWNLOADING)
						counters[ROW_DOWNLOADING]++;

					if(categories & CATEGORY_SEEDING)
						counters[ROW_SEEDING]++;

					if(categories & CATEGORY_UPLOADING)
						counters[ROW_UPLOADING]++;

					if(categories & CATEGORY_BROKEN_TRACKER)
						counters[ROW_BROKEN_TRACKER]++;
				}

				if(this->show_names)
				{
					for(size_t i = 0; i < ROWS_NUM; ++i)
					{
						m::gtk::update_row(
							this->rows[i], this->columns.name,
							this->names[i] + " (" + m::to_string(counters[i]) + ")"
						);
					}
				}
				else
				{
					for(size_t i = 0; i < ROWS_NUM; ++i)
					{
						m::gtk::update_row(
							this->rows[i], this->columns.name,
							"(" + m::to_string(counters[i]) + ")"
						);
					}
				}

				this->has_only_name = false;
			}
			else
			{
				if(!this->has_only_name)
				{
					this->has_only_name = true;

					for(size_t i = 0; i < ROWS_NUM; ++i)
					{
						m::gtk::update_row(
							this->rows[i], this->columns.name, this->names[i]);
					}
				}
			}
		}
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
	return get_categories(info) & this->categories;
}



Categories_view_settings::Categories_view_settings(void)
:
	visible(true),
	show_names(true),
	show_counters(true)
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

	this->signal_hide().connect(
		sigc::bind<bool>(sigc::mem_fun(*impl, &Impl::state_changed), false));
	this->signal_show().connect(
		sigc::bind<bool>(sigc::mem_fun(*impl, &Impl::state_changed), true));
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
	settings->show_counters = impl->get_show_counters();
	settings->selected_items = impl->get_selected_items();
}



void Categories_view::show_counters(bool show)
{
	impl->set_show_counters(show);
}



void Categories_view::show_names(bool show)
{
	impl->set_show_names(show);
}



m::gtk::Signal_proxy<void> Categories_view::signal_changed(void)
{
	return impl->changed_signal;
}



m::gtk::Signal_proxy<void> Categories_view::signal_needs_update(void)
{
	return impl->needs_update_signal;
}



void Categories_view::update(const std::vector<Torrent_info>& torrents)
{
	if(this->is_visible())
		impl->update(torrents);
}

