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


#include <algorithm>
#include <functional>
#include <iterator>
#include <stack>

#include <gtk/gtk.h>

#include <gtkmm/alignment.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/stock.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treestore.h>

#include <mlib/gtk/dialog.hpp>
#include <mlib/gtk/misc.hpp>
#include <mlib/gtk/tree_view.hpp>
#include <mlib/gtk/vbox.hpp>
#include <mlib/fs.hpp>
#include <mlib/libtorrent.hpp>
#include <mlib/string.hpp>

#include "application.hpp"
#include "client_settings.hpp"
#include "common.hpp"
#include "daemon_proxy.hpp"
#include "gui_lib.hpp"
#include "main.hpp"
#include "torrent_files_view.hpp"



namespace
{
	// Change_path_dialog -->
		/// Диалог изменения расположения файла (папки)
		class Change_path_dialog: public m::gtk::Dialog
		{
			public:
				Change_path_dialog(Gtk::Window& parent, const std::string& path);


			private:
				Gtk::Entry*			path_entry;

			
			public:
				/// Если пользователь нажал на кнопку OK, то возвращает true и
				/// записывает в path новый путь.
				bool	run(std::string* path);

			private:
				/// Обработчик сигнала на активацию Gtk::Entry, задающую новый путь.
				void	on_path_entry_activate_callback(void);
		};



		Change_path_dialog::Change_path_dialog(Gtk::Window& parent, const std::string& path)
		:
			m::gtk::Dialog(parent, _("Change file or directory path"))
		{
			this->set_resizable(false);

		#if GTK_CHECK_VERSION(3, 0, 0)
			if(!parent.get_visible())
		#else
			if(!parent.is_visible())
		#endif
				this->set_position(Gtk::WIN_POS_CENTER);

			Gtk::VBox* main_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
			main_vbox->set_border_width(m::gtk::BOX_BORDER_WIDTH);
			this->get_vbox()->pack_start(*main_vbox, false, false);

			m::gtk::vbox::add_header(*main_vbox, _("Change file or directory path"), true);

			Gtk::Alignment* alignment = Gtk::manage(new Gtk::Alignment(0.5, 0.5, 0, 0));
			main_vbox->pack_start(*alignment, false, false);

			this->path_entry = Gtk::manage(new Gtk::Entry);
			this->path_entry->set_text(path);
			this->path_entry->signal_activate().connect(sigc::mem_fun(
				*this, &Change_path_dialog::on_path_entry_activate_callback
			));
			alignment->add(*this->path_entry);

			// Добавляем кнопки
			this->get_action_area()->property_layout_style() = Gtk::BUTTONBOX_CENTER;
			this->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			this->add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
			this->set_default_response(Gtk::RESPONSE_OK);

			this->show_all_children();
		}



		bool Change_path_dialog::run(std::string* path)
		{
			if(m::gtk::Dialog::run() == Gtk::RESPONSE_OK)
			{
				Path new_path = Path("/" + this->path_entry->get_text()).normalize();

				if(!new_path.is_absolute())
				{
					MLIB_W(
						_("Changing file or directory path failed"),
						_("Changing file or directory path failed. Invalid path has been gotten. Please enter a valid path.")
					);

					return run(path);
				}
				else
				{
					*path = new_path.string();
					return true;
				}
			}
			else
				return false;
		}



		void Change_path_dialog::on_path_entry_activate_callback(void)
		{
			this->response(Gtk::RESPONSE_OK);
		}
	// Change_path_dialog <--
}



// Torrent_files_view_model_columns -->
	Torrent_files_view_model_columns::Torrent_files_view_model_columns(void)
	{
		this->set_search_column(this->name);

		add(this->id);
		add(this->name);

		add(this->download);
		add(this->partially_download);

		add(this->size);
		add(this->size_string);

		add(this->priority);
		add(this->priority_string);

		add(this->progress);
		add(this->progress_string);
	}
// Torrent_files_view_model_columns <--



// Torrent_files_view_columns -->
	Torrent_files_view_columns::Torrent_files_view_columns(const Torrent_files_view_model_columns& model_columns)
	:
		progress_renderer(),
		name(_("Name")),
		size(_("Size"), model_columns.size_string),
		priority(_("Priority"), model_columns.priority_string),
		progress(_("Progress"), progress_renderer)
	{
		this->add("name", &this->name, _("Name"));
		this->name.set_sort_column(model_columns.name);

		this->name.pack_start(this->download_renderer, false);
		this->name.add_attribute(this->download_renderer.property_active(), model_columns.download);
		// В gtkmm пока что нет обертки для свойства "inconsistent"
		gtk_tree_view_column_add_attribute(
			this->name.gobj(), GTK_CELL_RENDERER(this->download_renderer.gobj()),
			"inconsistent", model_columns.partially_download.index()
		);

		this->name.pack_start(this->name_renderer, true);
		this->name.add_attribute(this->name_renderer.property_text(), model_columns.name);


		this->add("priority", &this->priority, _("Priority"));
		this->priority.set_sort_column(model_columns.priority);
	#if GTK_CHECK_VERSION(3, 0, 0)
		this->priority.get_first_cell()->property_xalign().set_value(0.5);
	#else
		this->priority.get_first_cell_renderer()->property_xalign().set_value(0.5);
	#endif


		M_GTK_TREE_VIEW_ADD_INTEGER_COLUMN(size, _("Size"))

		this->add("progress", &this->progress, _("Progress"));
		this->progress.set_sort_column(model_columns.progress);
		this->progress.add_attribute(this->progress_renderer.property_value(), model_columns.progress);
		this->progress.add_attribute(this->progress_renderer.property_text(), model_columns.progress_string);
	}
// Torrent_files_view_columns <--



// Torrent_files_view -->
	Torrent_files_view::Directory_status::Directory_status(void)
	:
		downloaded(0),
		priority_name(""),
		download(true),
		download_child(0)
	{
	}



	Torrent_files_view::Torrent_files_view(bool files_paths_changeable, const Torrents_view_settings& settings)
	:
		m::gtk::Tree_view<Torrent_files_view_columns, Torrent_files_view_model_columns, Gtk::TreeStore>(settings),
		display_revision(INIT_REVISION),
		files_paths_changeable(files_paths_changeable)
	{
		// Всплывающее меню -->
			Glib::RefPtr<Gtk::ActionGroup> action_group;

			this->ui_manager = Gtk::UIManager::create();

			action_group = Gtk::ActionGroup::create();

			this->download_action = Gtk::Action::create("download", Gtk::Stock::ADD, _("Download"));
			action_group->add(
				this->download_action,
				sigc::bind<Action>( sigc::mem_fun(*this, &Torrent_files_view::on_change_files_settings_callback), DOWNLOAD )
			);

			this->do_not_download_action = Gtk::Action::create("do_not_download", Gtk::Stock::REMOVE, _("Do not download"));
			action_group->add(
				this->do_not_download_action,
				sigc::bind<Action>( sigc::mem_fun(*this, &Torrent_files_view::on_change_files_settings_callback), DO_NOT_DOWNLOAD )
			);

			this->normal_priority_action = Gtk::Action::create("normal_priority", Gtk::Stock::APPLY, _("Normal priority"));
			action_group->add(
				this->normal_priority_action,
				sigc::bind<Action>( sigc::mem_fun(*this, &Torrent_files_view::on_change_files_settings_callback), NORMAL_PRIORITY )
			);

			this->high_priority_action = Gtk::Action::create("high_priority", Gtk::Stock::GO_UP, _("High priority"));
			action_group->add(
				this->high_priority_action,
				sigc::bind<Action>( sigc::mem_fun(*this, &Torrent_files_view::on_change_files_settings_callback), HIGH_PRIORITY )
			);

			this->change_path_action = Gtk::Action::create("change_path", Gtk::Stock::EDIT, _("Change path"));
			action_group->add(
				this->change_path_action,
				sigc::bind<Action>( sigc::mem_fun(*this, &Torrent_files_view::on_change_files_settings_callback), CHANGE_PATH )
			);

			this->ui_manager->insert_action_group(action_group);

			Glib::ustring ui_info =
				"<ui>"
				"	<popup name='popup_menu'>"
				"		<menuitem action='download'/>"
				"		<menuitem action='do_not_download'/>"
				"		<separator/>"
				"		<menuitem action='normal_priority'/>"
				"		<menuitem action='high_priority'/>"
				"		<separator/>"
				"		<menuitem action='change_path'/>"
				"	</popup>"
				"</ui>";

			this->ui_manager->add_ui_from_string(ui_info);
		// Всплывающее меню <--

		this->set_enable_tree_lines();

		this->columns.download_renderer.signal_toggled().connect(
			sigc::mem_fun(*this, &Torrent_files_view::on_toggle_download_status_callback)
		);
	}



	void Torrent_files_view::add_row_download_changes_ids(const Gtk::TreeRow& row, std::vector<int>& files_ids, bool download) const
	{
		// Если это директория
		if(row[this->model_columns.id] < 0)
		{
			Gtk::TreeNodeChildren rows(row.children());
			Gtk::TreeModel::iterator it = rows.begin();

			while(it)
				this->add_row_download_changes_ids(*it++, files_ids, download);
		}
		// Если это файл
		else
		{
			if(row[this->model_columns.download] != download)
				files_ids.push_back(row[this->model_columns.id]);
		}
	}



	void Torrent_files_view::add_row_priorities_changes_ids(const Gtk::TreeRow& row, std::vector<int>& files_ids, const std::string& priority_name) const
	{
		// Если это директория
		if(row[this->model_columns.id] < 0)
		{
			if(row[this->model_columns.priority] != priority_name)
			{
				Gtk::TreeNodeChildren rows(row.children());
				Gtk::TreeModel::iterator it = rows.begin();

				while(it)
					this->add_row_priorities_changes_ids(*it++, files_ids, priority_name);
			}
		}
		// Если это файл
		else
		{
			if(row[this->model_columns.priority] != priority_name)
				files_ids.push_back(row[this->model_columns.id]);
		}
	}



	void Torrent_files_view::change_row_files_paths(const Gtk::TreeRow& row, const std::string& path, std::vector<std::string>* paths) const
	{
		int file_id = row[this->model_columns.id];

		// Если это директория
		if(file_id < 0)
		{
			Gtk::TreeNodeChildren rows(row.children());
			Gtk::TreeModel::iterator it = rows.begin();

			while(it)
			{
				Gtk::TreeRow child_row = *it++;

				this->change_row_files_paths(
					child_row,
					// Удаляем лишние слэши, если, к примеру, path равен "/"
					Path(path + "/" + child_row[this->model_columns.name]).normalize(),
					paths
				);
			}
		}
		// Если это файл
		else
			paths->at(file_id) = path;
	}



	void Torrent_files_view::on_change_files_settings_callback(Action action)
	{
		// Получаем список выделенных строк
		std::deque<Gtk::TreeModel::iterator> iters = this->get_selected_rows();

		std::vector<int> files_ids;

		switch(action)
		{
			case DOWNLOAD:
			case DO_NOT_DOWNLOAD:
			{
				// Получаем идентификаторы файлов, настройки которых требуется изменить.
				for(size_t i = 0; i < iters.size(); i++)
					this->add_row_download_changes_ids(*iters[i], files_ids, action == DOWNLOAD);

				// Вносим изменения -->
					try
					{
						this->set_files_download_status(files_ids, action == DOWNLOAD);
					}
					catch(m::Exception& e)
					{
						MLIB_W(__("Changing torrent files download status failed. %1", EE(e)));
					}
				// Вносим изменения <--
			}
			break;

			case NORMAL_PRIORITY:
			case HIGH_PRIORITY:
			{
				// Приоритет -->
					Torrent_file_settings::Priority priority;

					switch(action)
					{
						case NORMAL_PRIORITY:
							priority = Torrent_file_settings::NORMAL;
							break;
						case HIGH_PRIORITY:
							priority = Torrent_file_settings::HIGH;
							break;
						default:
							MLIB_LE();
							break;
					}

					std::string priority_name = Torrent_file_settings::get_priority_name(priority);
				// Приоритет <--

				// Получаем идентификаторы файлов, настройки которых требуется изменить.
				for(size_t i = 0; i < iters.size(); i++)
					this->add_row_priorities_changes_ids(*iters[i], files_ids, priority_name);

				// Вносим изменения -->
					try
					{
						this->set_files_priority(files_ids, priority);
					}
					catch(m::Exception& e)
					{
						MLIB_W(__("Changing torrent files priorities failed. %1", EE(e)));
					}
				// Вносим изменения <--
			}
			break;

			case CHANGE_PATH:
			{
				if(iters.size() == 1)
					this->on_edit_path_callback(*iters[0]);
				else
					MLIB_D(_C("Gotten invalid number of rows (%1) for path changing.", iters.size()));
			}
			break;

			default:
				MLIB_LE();
				break;
		}

		// Отображаем внесенные изменения
		this->update();
	}



	void Torrent_files_view::on_edit_path_callback(const Gtk::TreeRow& path_row)
	{
		std::string cur_path_string;
		Gtk::Window* parent_window = get_widget_window(*this);

		if(!parent_window)
		{
			MLIB_D("Can't get torrent files view parent window.");
			return;
		}

		// Получаем путь файла -->
		{
			Gtk::TreeIter parent_iter;
			Gtk::TreeRow row = path_row;

			cur_path_string = "/" + row[this->model_columns.name];

			while(parent_iter = row.parent())
			{
				row = *parent_iter;
				cur_path_string = "/" + row[this->model_columns.name] + cur_path_string;
			}
		}
		// Получаем путь файла <--

		MLIB_D(_C("Path to change: '%1'.", cur_path_string));

		Change_path_dialog dialog(*parent_window, cur_path_string);

		while(1)
		{
			std::string new_path_string;

			// Получаем от пользователя новый путь
			if(!dialog.run(&new_path_string))
				return;

			// Файлам не разрешается принимать значение "/"
			if(path_row[this->model_columns.id] >= 0 && new_path_string == "/")
			{
				MLIB_W(
					_("Changing file path failed"),
					_("Changing file path failed. Invalid file path has been gotten. Please enter a valid path.")
				);

				continue;
			}

			MLIB_D(_C("New path: '%1'.", new_path_string));

			std::vector<std::string> new_paths;

			// Получаем новые пути -->
				std::transform(
					this->files.begin(), this->files.end(),
					std::back_insert_iterator< std::vector<std::string> >(new_paths),
					std::mem_fun_ref(&Torrent_file::get_path)
				);

				this->change_row_files_paths(path_row, new_path_string, &new_paths);

				try
				{
					// Проверяем, можем ли мы создать дерево файлов и каталогов
					// с новыми путями. Если у нас, к примеру, есть одинаковые
					// пути или файлы, которые в то же время являются и
					// каталогами, то сгенерируется исключение.
					m::fs::tree::create(new_paths);
				}
				catch(m::Exception& e)
				{
					MLIB_W(
						_("Changing file or directory path failed"),
						__("Changing file or directory path failed. %1", EE(e))
					);

					// Запрашиваем у пользователя путь заново
					continue;
				}
			// Получаем новые пути <--

			// Вносим необходимые изменения -->
				try
				{
					this->set_files_new_paths(new_paths);
				}
				catch(m::Exception& e)
				{
					MLIB_W(
						_("Changing file or directory path failed"),
						__("Changing file or directory path failed. %1", EE(e))
					);
				}
			// Вносим необходимые изменения <--

			break;
		}

		// Отображаем внесенные изменения
		this->update();
	}



	void Torrent_files_view::on_mouse_right_button_click(const GdkEventButton* const event)
	{
		// Получаем список выделенных строк
		std::deque<Gtk::TreeModel::iterator> iters = this->get_selected_rows();

		if(iters.size())
		{
			// Определяем, какие элементы меню необходимо отобразить -->
			{
				bool download_visible = false;
				bool do_not_download_visible = false;
				bool normal_priority_visible = false;
				bool high_priority_visible = false;

				for(size_t i = 0; i < iters.size(); i++)
				{
					Gtk::TreeRow row = *iters[i];

					if(!do_not_download_visible && row[this->model_columns.download])
						do_not_download_visible = true;

					if(!download_visible && !row[this->model_columns.download])
						download_visible = true;

					if(!normal_priority_visible && row[this->model_columns.priority] != "normal")
						normal_priority_visible = true;

					if(!high_priority_visible && row[this->model_columns.priority] != "high")
						high_priority_visible = true;
				}

				this->download_action->set_visible(download_visible);
				this->do_not_download_action->set_visible(do_not_download_visible);
				this->normal_priority_action->set_visible(normal_priority_visible);
				this->high_priority_action->set_visible(high_priority_visible);
				this->change_path_action->set_visible(this->files_paths_changeable && iters.size() == 1);
			}
			// Определяем, какие элементы меню необходимо отобразить <--

			// Отображаем меню
			dynamic_cast<Gtk::Menu*>(this->ui_manager->get_widget("/popup_menu"))->popup(event->button, event->time);
		}
	}



	void Torrent_files_view::on_toggle_download_status_callback(const Glib::ustring& path_string)
	{
		std::vector<int> files_ids;

		Gtk::TreeRow row = *this->model->get_iter(path_string);
		bool download = !row[this->model_columns.download];

		// Получаем список идентификаторов файлов, для которых
		// необходимо изменить приоритет.
		this->add_row_download_changes_ids(row, files_ids, download);

		try
		{
			// Вносим изменения
			this->set_files_download_status(files_ids, download);
		}
		catch(m::Exception& e)
		{
			MLIB_W(__(
				"Changing download status for '%1' failed. %2",
				row[this->model_columns.name], EE(e)
			));
		}

		// Отображаем внесенные изменения
		this->update();
	}



	void Torrent_files_view::update(void)
	{
		std::vector<Torrent_file> files;
		std::vector<Torrent_file_status> statuses;

		try
		{
			// Генерирует m::Exception
			Revision current_revision = get_files_info(&files, &statuses, this->display_revision);

			if(this->display_revision != current_revision)
			{
				this->files = files;

				/// Заполняем модель файлами торрента
				this->update_files();

				this->display_revision = current_revision;
			}
		}
		catch(m::Exception& e)
		{
			// Этого торрента уже вполне может не оказаться, т. к. между
			// обновлением списка торрентов и этим запросом торрент мог
			// быть удален, например, функцией автоматического удаления
			// старых торрентов.
			MLIB_D(_C("Updating torrent files list failed. %1", EE(e)));
			this->model->clear();
			this->files.clear();
			return;
		}

		this->update_rows(this->model->children(), statuses);
	}



	void Torrent_files_view::update_files(void)
	{
		std::stack<Directory_row> directories;
		Directory_row cur_dir;

		int id;
		std::string path;
		Size size;

		Gtk::TreeRow row;
		std::string name;
		size_t pos;

		this->model->clear();

		cur_dir.path = "/";

		std::vector<Torrent_file> files = this->files;
		std::sort(files.begin(), files.end());

		M_FOR_CONST_IT(files, it)
		{
			id = it->id;
			path = it->path;
			size = it->size;

			// Определяем, принадлежит ли файл/директория
			// текущей директории. Если не принадлежит, то
			// поднимаемся вверх по дереву каталогов, пока
			// не наткнемся на директорию, родительскую для
			// данного файла/директории.
			// -->
				while
				(
					(
						path.length() < cur_dir.path.length()
						||
						path.substr(0, cur_dir.path.length()) != cur_dir.path
					)
					&&
					cur_dir.path != "/"
				)
				{
					if(directories.size() < 1)
						MLIB_LE();

					// Прежде чем взять родительскую директорию из стэка
					// и заменить ей информацию о текущей, записываем
					// в модель посчитанный размер всех файлов директории
					// и добавляем его к размеру родительской.
					// -->
						cur_dir.row[this->model_columns.size] = cur_dir.size;
						cur_dir.row[this->model_columns.size_string] = m::size_to_string(cur_dir.size);

						directories.top().size += cur_dir.size;
					// <--

					cur_dir = directories.top();
					directories.pop();
				}
			// <--

			// Спускаемся вниз по директориям, пока не
			// наткнемся на файл.
			// -->
				while(1)
				{
					pos = path.find("/", cur_dir.path.length());
					name = path.substr(cur_dir.path.length(), pos - cur_dir.path.length());

					// Это файл
					if(pos == std::string::npos)
					{
						// Увеличиваем размер родительской директории
						cur_dir.size += size;

						if(cur_dir.path != "/")
							row = *this->model->append(cur_dir.row.children());
						else
							row = *this->model->append();

						row[this->model_columns.id] = id;
						row[this->model_columns.partially_download] = false;
						row[this->model_columns.name] = name;
						row[this->model_columns.size] = size;
						row[this->model_columns.size_string] = m::size_to_string(size);
						row[this->model_columns.progress] = 0;
						row[this->model_columns.progress_string] = "0%";

						break;
					}
					// Это директория
					else
					{
						directories.push(cur_dir);

						if(cur_dir.path != "/")
							cur_dir.row = *this->model->append(cur_dir.row.children());
						else
							cur_dir.row = *this->model->append();

						cur_dir.path += name + "/";
						cur_dir.size = 0;

						cur_dir.row[this->model_columns.id] = -1;
						cur_dir.row[this->model_columns.name] = name;
						cur_dir.row[this->model_columns.progress] = 0;
						cur_dir.row[this->model_columns.progress_string] = "0%";
					}
				}
			// <--
		}

		// Заносим в модель размеры оставшихся в стеке директорий -->
			while(directories.size())
			{
				// Прежде чем взять родительскую директорию из стэка
				// и заменить ей информацию о текущей, записываем
				// в модель посчитанный размер всех файлов директории
				// и добавляем его к размеру родительской.
				// -->
					cur_dir.row[this->model_columns.size] = cur_dir.size;
					cur_dir.row[this->model_columns.size_string] = m::size_to_string(cur_dir.size);

					directories.top().size += cur_dir.size;
				// <--

				cur_dir = directories.top();
				directories.pop();
			}

			// Информаци о последней директории заносить не надо,
			// т. к. это всегда будет корневая директория.
		// Заносим в модель размеры оставшихся в стеке директорий <--

		this->expand_all();
	}



	Torrent_files_view::Directory_status Torrent_files_view::update_rows(const Gtk::TreeNodeChildren& iters, const std::vector<Torrent_file_status>& statuses)
	{
		int progress;
		std::string priority_name;
		Directory_status directory_status;
		std::vector<Gtk::TreeRow> rows;

		// Получаем все строки модели.
		// Оперировать итераторами мы не можем, т. к. из-за сортировки
		// после каждого изменения какой-либо строки модель сортируется, и
		// итераторы начинают указывать не на те элементы (к примеру,
		// следующий итератор может указывать на ту строку, которая ранее
		// уже была обработана).
		// -->
			rows.reserve(iters.size());
			M_FOR_CONST_IT(iters, it)
				rows.push_back(*it);
		// <--

		for(size_t child_id = 0; child_id < rows.size(); child_id++)
		{
			Gtk::TreeRow row = *rows[child_id];
			int id = row[this->model_columns.id];

			// Если это директория
			if(id < 0)
			{
				Directory_status child_directory_status = this->update_rows(row.children(), statuses);

				// Статус директории -->
					if(child_directory_status.download)
						directory_status.download_child = true;
					else
					{
						if(child_directory_status.download_child)
							directory_status.download_child = true;

						directory_status.download = false;
					}

					if(child_id)
					{
						if(directory_status.priority_name != child_directory_status.priority_name)
							directory_status.priority_name = "";
					}
					else
						directory_status.priority_name = child_directory_status.priority_name;

					directory_status.downloaded += child_directory_status.downloaded;
				// Статус директории <--

				// Статус дочерней директории -->
					// download -->
						if(child_directory_status.download)
						{
							m::gtk::update_row(row, this->model_columns.download, true);
							m::gtk::update_row(row, this->model_columns.partially_download, false);
						}
						else
						{
							m::gtk::update_row(row, this->model_columns.download, false);
							m::gtk::update_row(row, this->model_columns.partially_download, child_directory_status.download_child);
						}
					// download <--

					// priority -->
						if(child_directory_status.priority_name.empty())
						{
							if(m::gtk::update_row(row, this->model_columns.priority, child_directory_status.priority_name))
								m::gtk::update_row(row, this->model_columns.priority_string, child_directory_status.priority_name);
						}
						else
						{
							if(m::gtk::update_row(row, this->model_columns.priority, child_directory_status.priority_name))
							{
								m::gtk::update_row(row, this->model_columns.priority_string,
									m::uppercase_first(
										Torrent_file_settings::get_priority_localized_name(
											Torrent_file_settings::get_priority_by_name(child_directory_status.priority_name)
										)
									)
								);
							}
						}
					// priority <--

					// progress -->
						if(row[this->model_columns.size])
							progress = child_directory_status.downloaded * 100 / row[this->model_columns.size];
						else
							progress = 100;

						if(m::gtk::update_row(row, this->model_columns.progress, progress))
							m::gtk::update_row(row, this->model_columns.progress_string, m::to_string(progress) + "%");
					// progress <--
				// Статус дочерней директории <--
			}
			// Если это файл
			else
			{
				if(static_cast<size_t>(id) >= statuses.size())
					MLIB_LE();

				priority_name = statuses[id].get_priority_name();

				// Статус директории -->
					if(statuses[id].download)
						directory_status.download_child = true;
					else
						directory_status.download = false;

					if(child_id)
					{
						if(directory_status.priority_name != priority_name)
							directory_status.priority_name = "";
					}
					else
						directory_status.priority_name = priority_name;

					directory_status.downloaded += statuses[id].downloaded;
				// Статус директории <--

				// Статус файла -->
					m::gtk::update_row(row, this->model_columns.download, statuses[id].download);

					if(row[this->model_columns.size])
					{
						progress = static_cast<int>(
							( static_cast<Size_float>(statuses[id].downloaded) / row[this->model_columns.size] ) * 100
						);
					}
					else
						progress = 100;

					if(m::gtk::update_row(row, this->model_columns.priority, priority_name))
					{
						m::gtk::update_row(row, this->model_columns.priority_string,
							m::uppercase_first(statuses[id].get_priority_localized_name())
						);
					}

					if(m::gtk::update_row(row, this->model_columns.progress, progress))
						m::gtk::update_row(row, this->model_columns.progress_string, m::to_string(progress) + "%");
				// Статус файла <--
			}
		}

		return directory_status;
	}
// Torrent_files_view <--



// Torrent_files_dynamic_view -->
	Torrent_files_dynamic_view::Torrent_files_dynamic_view(const Torrent_files_view_settings& settings)
	:
		Torrent_files_view(false, settings)
	{
		// Устанавливаем обработчик сигнала на активацию строки TreeView.
		this->signal_row_activated().connect(sigc::mem_fun(*this, &Torrent_files_dynamic_view::on_row_activated_callback));
	}



	Revision Torrent_files_dynamic_view::get_files_info(std::vector<Torrent_file> *files, std::vector<Torrent_file_status>* statuses, Revision revision)
	{
		if(this->cur_torrent_id)
		{
			try
			{
				return get_daemon_proxy().get_torrent_files_info(this->cur_torrent_id, files, statuses, revision);
			}
			catch(m::Exception)
			{
				this->cur_torrent_id = Torrent_id();
				throw;
			}
		}
		else
		{
			files->clear();
			statuses->clear();
			this->reset_display_revision();
			return FIRST_REVISION;
		}
	}



	void Torrent_files_dynamic_view::on_row_activated_callback(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column)
	{
		Path torrent_download_path;

		try
		{
			torrent_download_path = get_daemon_proxy().get_torrent_download_path(cur_torrent_id);
		}
		catch(m::Exception& e)
		{
			MLIB_W(EE(e));
		}

		Path torrent_file_path;
		Gtk::TreeModel::iterator it = this->model->get_iter(path);

		while(it)
		{
			torrent_file_path = Path(it->get_value(this->model_columns.name)) / torrent_file_path;
			it = it->parent();
		}

		get_application().open_uri(torrent_download_path / torrent_file_path);
	}



	void Torrent_files_dynamic_view::set_files_download_status(const std::vector<int>& files_ids, bool download)
	{
		// Сообщаем демону об изменениях в скачиваемых файлах
		get_daemon_proxy().set_files_download_status(this->cur_torrent_id, files_ids, download);
	}



	void Torrent_files_dynamic_view::set_files_new_paths(const std::vector<std::string>& paths)
	{
		// Пока данная возможность не реализована, и управление до сюда дойти
		// не должно.
		MLIB_LE();
	}



	void Torrent_files_dynamic_view::set_files_priority(const std::vector<int>& files_ids, Torrent_file_settings::Priority priority)
	{
		// Сообщаем демону об изменениях в приоритетах
		get_daemon_proxy().set_files_priority(this->cur_torrent_id, files_ids, priority);
	}



	void Torrent_files_dynamic_view::update(const Torrent_id& torrent_id)
	{
		if(torrent_id != this->cur_torrent_id)
		{
			this->cur_torrent_id = torrent_id;
			this->reset_display_revision();
		}

		Torrent_files_view::update();
	}
// Torrent_files_dynamic_view <--



// Torrent_files_static_view -->
	Torrent_files_static_view::Torrent_files_static_view(const lt::torrent_info& torrent_info, const Torrent_files_view_settings& settings)
	:
		Torrent_files_view(true, settings),
		files_revision(FIRST_REVISION)
	{
		// Эта колонка при статическом отображении не нужна
		this->remove_column(this->columns.progress);

		// Получаем файлы торрента
		// Генерирует m::Exception
		this->files = m::lt::get_torrent_files(torrent_info);

		// Формируем для файлов настройки по умолчанию
		files_settings.resize(this->files.size());

		// Обновляем модель
		this->update();
	}



	Revision Torrent_files_static_view::get_files_info(std::vector<Torrent_file> *files, std::vector<Torrent_file_status>* statuses, Revision revision)
	{
		if(revision != this->files_revision)
			*files = this->files;
		else
			files->clear();

		statuses->clear();
		statuses->reserve(this->files_settings.size());

		for(size_t i = 0; i < this->files_settings.size(); i++)
			statuses->push_back(Torrent_file_status(files_settings[i], 0));

		return this->files_revision;
	}



	const std::vector<Torrent_file_settings>& Torrent_files_static_view::get_files_settings(void) const
	{
		return this->files_settings;
	}



	void Torrent_files_static_view::set_files_download_status(const std::vector<int>& files_ids, bool download)
	{
		for(size_t i = 0; i < files_ids.size(); i++)
			this->files_settings[files_ids[i]].download = download;
	}



	void Torrent_files_static_view::set_files_new_paths(const std::vector<std::string>& paths)
	{
		MLIB_A(paths.size() == this->files.size());

		for(size_t i = 0; i < paths.size(); i++)
		{
			const std::string& new_path = paths[i];
			std::string& cur_path = this->files[i].path;
			std::string& settings_path = this->files_settings[i].path;

			if(cur_path != new_path)
			{
				cur_path = new_path;
				settings_path = new_path;
			}
		}

		this->files_revision++;
	}



	void Torrent_files_static_view::set_files_priority(const std::vector<int>& files_ids, Torrent_file_settings::Priority priority)
	{
		for(size_t i = 0; i < files_ids.size(); i++)
			this->files_settings[files_ids[i]].priority = priority;
	}
// Torrent_files_static_view <--

