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


#ifndef HEADER_TORRENT_FILES_VIEW
	#define HEADER_TORRENT_FILES_VIEW

	#include <gtkmm/cellrendererprogress.h>
	#include <gtkmm/treemodel.h>
	#include <gtkmm/treemodelcolumn.h>
	#include <gtkmm/treestore.h>
	#include <gtkmm/treeviewcolumn.h>

	#include <mlib/gtk/tree_view.hpp>
	#include <mlib/libtorrent.hpp>

	#include "client_settings.hpp"



	class Torrent_files_view_model_columns: public m::gtk::Tree_view_model_columns
	{
		public:
			Torrent_files_view_model_columns(void);


		public:
			Gtk::TreeModelColumn<int>				id;
			Gtk::TreeModelColumn<Glib::ustring>		name;

			Gtk::TreeModelColumn<bool>				download;
			Gtk::TreeModelColumn<bool>				partially_download;

			Gtk::TreeModelColumn<Size>				size;
			Gtk::TreeModelColumn<Glib::ustring>		size_string;

			Gtk::TreeModelColumn<Glib::ustring>		priority;
			Gtk::TreeModelColumn<Glib::ustring>		priority_string;

			Gtk::TreeModelColumn<int>				progress;
			Gtk::TreeModelColumn<Glib::ustring>		progress_string;
	};



	class Torrent_files_view_columns: public m::gtk::Tree_view_columns
	{
		public:
			Torrent_files_view_columns(const Torrent_files_view_model_columns& model_columns);


		public:
			Gtk::CellRendererToggle		download_renderer;
			Gtk::CellRendererText		name_renderer;
			Gtk::CellRendererProgress	progress_renderer;

			Gtk::TreeViewColumn			name;
			Gtk::TreeViewColumn			size;
			Gtk::TreeViewColumn			priority;
			Gtk::TreeViewColumn			progress;
	};



	/// Базовый абстрактный класс для отображения файлов торрента.
	class Torrent_files_view
	:
		public m::gtk::Tree_view<Torrent_files_view_columns, Torrent_files_view_model_columns, Gtk::TreeStore>
	{
		public:
			enum Action { DOWNLOAD, DO_NOT_DOWNLOAD, NORMAL_PRIORITY, HIGH_PRIORITY, CHANGE_PATH };

		private:
			struct Directory_row
			{
				/// Путь к директории (со слешем на конце).
				std::string		path;

				/// Размер всех файлов директории.
				Size			size;

				/// Строка, представляющая данную директорию в модели.
				Gtk::TreeRow	row;
			};

			class Directory_status
			{
				public:
					Directory_status(void);


				public:
					/// Размер скачанных данных в директории.
					Size					downloaded;

					/// Приоритет директории или "", если файлы внутри
					/// директории имеют разные приоритеты.
					std::string				priority_name;

					/// Скачивается ли данная директория.
					bool					download;

					/// Скачивается ли хотя бы один файл или поддиректория
					/// данной директории.
					bool					download_child;
			};


		public:
			Torrent_files_view(bool files_paths_changeable, const Torrent_files_view_settings& settings);


		private:
			/// Номер ревизии, которую в данный момент отображает
			/// Torrent_files_view.
			Revision						display_revision;

			/// Отображаемые в данный момент файлы.
			std::vector<Torrent_file>		files;

			/// Определяет, разрешается ли пользователю менять расположение файлов.
			bool							files_paths_changeable;

			Glib::RefPtr<Gtk::Action>		download_action;
			Glib::RefPtr<Gtk::Action>		do_not_download_action;
			Glib::RefPtr<Gtk::Action>		normal_priority_action;
			Glib::RefPtr<Gtk::Action>		high_priority_action;
			Glib::RefPtr<Gtk::Action>		change_path_action;
			Glib::RefPtr<Gtk::UIManager>	ui_manager;


		protected:
			/// Устанавливает номер отображаемой ревизии в начальное состояние.
			inline
			void				reset_display_revision(void);

			/// Обновляет информацию о файлах.
			void				update(void);

		private:
			/// Дописывает в files_ids идентификаторы файлов, находящихся в
			/// строке row, у которых флаг скачивания не равен download.
			void				add_row_download_changes_ids(const Gtk::TreeRow& row, std::vector<int>& files_ids, bool download) const;

			/// Дописывает в files_ids идентификаторы файлов, находящихся в
			/// строке row, у которых приоритет не равен priority_name.
			void				add_row_priorities_changes_ids(const Gtk::TreeRow& row, std::vector<int>& files_ids, const std::string& priority_name) const;

			/// Изменяет пути всех файлов, входящих в row.
			void				change_row_files_paths(const Gtk::TreeRow& row, const std::string& path, std::vector<std::string>* paths) const;

			/// Записывает в files список файлов торрента, а в statuses текущий статус файлов.
			/// Если переданная ревизия равна текущей, то не пишет в files ничего и заполняет
			/// только statuses.
			/// @return - текущую ревизию.
			virtual
			Revision			get_files_info(std::vector<Torrent_file> *files, std::vector<Torrent_file_status>* statuses, Revision revision) throw(m::Exception) = 0;

			/// Обработчик сигнала на изменение настроек файлов.
			void				on_change_files_settings_callback(Action action);

			/// Обработчик сигнала на изменение расположения файла.
			void				on_edit_path_callback(const Gtk::TreeRow& path_row);

			/// Обработчик сигнала на нажатие правой кнопки мыши.
			virtual
			void				on_mouse_right_button_click(const GdkEventButton* const event);

			/// Обработчик сигнала на нажатие по флажку, определяющему,
			/// нужно ли скачивать торрент или нет.
			void				on_toggle_download_status_callback(const Glib::ustring& path_string);

			/// Устанавливает флаг скачивания для файлов с идентификаторами files_ids.
			virtual
			void				set_files_download_status(const std::vector<int>& files_ids, bool download) throw(m::Exception) = 0;

			/// Устанавливает новые пути для файлов.
			virtual
			void				set_files_new_paths(const std::vector<std::string>& paths) throw(m::Exception) = 0;

			/// Устанавливает приоритет priority для файлов с идентификаторами files_ids.
			virtual
			void				set_files_priority(const std::vector<int>& files_ids, Torrent_file_settings::Priority priority) throw(m::Exception) = 0;

			/// Заполняет модель файлами торрента.
			void				update_files(void);

			/// Обновляет информацию о текущем состоянии файлов торрента в
			/// директории, представленной списком rows, и возвращает текущую
			/// информацию об обработанной только что директории.
			Directory_status	update_rows(const Gtk::TreeNodeChildren& rows, const std::vector<Torrent_file_status>& statuses);
	};



	/// Виджет для динамического отображения файлов
	/// торрента (предоставляет возможности отображения
	/// и обновления информации о текущем статусе закачки).
	class Torrent_files_dynamic_view
	:
		public Torrent_info_widget,
		public Torrent_files_view
	{
		public:
			Torrent_files_dynamic_view(const Torrent_files_view_settings& settings);


		private:
			/// Идентификатор торрента, файлы которого отображаются в данный момент.
			Torrent_id	cur_torrent_id;


		public:
			/// Переключается на торрент torrent_id, если в данный момент
			/// отображается информация о другом торренте.
			void		set_torrent_id(const Torrent_id& torrent_id);

			/// Инициирует обновление виджета.
			void		update(const Torrent_id& torrent_id);

		private:
			/// Записывает в files список файлов торрента, а в statuses текущий статус файлов.
			/// Если переданная ревизия равна текущей, то не пишет в files ничего и заполняет
			/// только statuses.
			/// @return - текущую ревизию.
			Revision	get_files_info(std::vector<Torrent_file> *files, std::vector<Torrent_file_status>* statuses, Revision revision) throw(m::Exception);

			/// Обработчик сигнала на активацию строки TreeView.
			void		on_row_activated_callback(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);

			/// Устанавливает флаг скачивания для файлов с идентификаторами files_ids.
			void		set_files_download_status(const std::vector<int>& files_ids, bool download) throw(m::Exception);

			/// Устанавливает новые пути для файлов.
			void		set_files_new_paths(const std::vector<std::string>& paths) throw(m::Exception);

			/// Устанавливает приоритет priority для файлов с идентификаторами files_ids.
			void		set_files_priority(const std::vector<int>& files_ids, Torrent_file_settings::Priority priority) throw(m::Exception);
	};



	/// Виджет для статического отображения файлов
	/// торрента (просто отображает файлы отдельно
	/// взятого торрент файла).
	class Torrent_files_static_view
	:
		public Torrent_files_view
	{
		public:
			Torrent_files_static_view(const lt::torrent_info& torrent_info, const Torrent_files_view_settings& settings) throw(m::Exception);


		private:
			/// Текущая ревизия файлов.
			Revision							files_revision;

			/// Список файлов торрента.
			std::vector<Torrent_file>			files;

			/// Текущее состояние файлов торрента.
			std::vector<Torrent_file_settings>	files_settings;


		public:
			/// Возвращает текущие настройки файлов.
			const std::vector<Torrent_file_settings>&	get_files_settings(void) const;

		private:
			/// Записывает в files список файлов торрента, а в statuses текущий статус файлов.
			/// Если переданная ревизия равна текущей, то не пишет в files ничего и заполняет
			/// только statuses.
			/// @return - текущую ревизию.
			Revision			get_files_info(std::vector<Torrent_file> *files, std::vector<Torrent_file_status>* statuses, Revision revision) throw(m::Exception);

			/// Устанавливает флаг скачивания для файлов с идентификаторами files_ids.
			void				set_files_download_status(const std::vector<int>& files_ids, bool download) throw(m::Exception);

			/// Устанавливает новые пути для файлов.
			void				set_files_new_paths(const std::vector<std::string>& paths) throw(m::Exception);

			/// Устанавливает приоритет priority для файлов с идентификаторами files_ids.
			void				set_files_priority(const std::vector<int>& files_ids, Torrent_file_settings::Priority priority) throw(m::Exception);
	};


	#include "torrent_files_view.hh"

#endif

