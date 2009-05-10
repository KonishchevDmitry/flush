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


#include <fstream>

#include <boost/filesystem.hpp>

#include <gtkmm/alignment.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/stock.h>

#include <libtorrent/create_torrent.hpp>

#include "mlib/fs.hpp"
#include "mlib/gtk/vbox.hpp"

#include "application.hpp"
#include "create_torrent_dialog.hpp"
#include "main.hpp"
#include "trackers_view.hpp"



namespace
{
	/// Данные, на основе которых создается торрент.
	struct Torrent_source
	{
		Torrent_source(
			const std::string& files,
			bool private_torrent,
			Size piece_size,
			const std::vector<std::string>& trackers,
			const std::string& save_path
		);

		/// Путь к файлу или папке, из которых надо создать торрент.
		std::string					files;

		/// Делать ли торрент приватным.
		bool						private_torrent;

		/// Размер одной части.
		Size						piece_size;

		/// Список трекеров.
		std::vector<std::string>	trackers;

		/// Путь, по которому необходимо записать *.torrent файл.
		std::string					save_path;
	};



	/// Диалог, отображаемый во время хеширования файлов торрента.
	/// Производит всю работу по созданию торрента.
	/// В случае успеха закрывает родительское окно.
	class Progress_dialog: public Gtk::Dialog
	{
		private:
			class Break_torrent_creating
			{
			};


		public:
			Progress_dialog(Gtk::Window& parent_window, const Torrent_source& torrent_source);
			~Progress_dialog(void);


		private:
			Gtk::ProgressBar*					progress_bar;
			double								progress;

			boost::mutex						mutex;

			/// Поток для создания торрента.
			boost::thread*						thread;

			/// Определяет, нужно ли прервать процесс создания торрента.
			bool								is_cancel;

			/// Данные, на основе которых создается торрент.
			Torrent_source						torrent_source;

			/// Сигнал на изменение прогресса в создании торрента.
			Glib::Dispatcher					progress_signal;

			/// Если при создании торрента происходит ошибка, то она
			/// сохраняется в данной переменной.
			std::auto_ptr<m::Exception>			error;

			/// Если торрент был успешно создан, выставляется в true;
			bool								is_created;

			/// Сигнал на завершение процесса создания торрента.
			Glib::Dispatcher					finished_signal;


		public:
			/// Проверяет, нужно ли добавлять данный файл в торрент.
			/// Всегда возвращает true.
			/// Генерирует Break_torrent_creating, если необходимо
			/// прервать процесс создания торрента.
			bool			check_added_file(const boost::filesystem::path& path) throw(Break_torrent_creating);

			/// Устанавливает текущий прогресс хэшированя файлов
			/// торрента.
			/// Генерирует Break_torrent_creating, если необходимо
			/// прервать процесс создания торрента.
			void			set_hash_progress(Size progress, Size hash_num) throw(Break_torrent_creating);

		private:
			/// При нажатии на кнопку Cancel.
			void 			on_cancel_button_callback(void);

			/// Обработчик сигнала на закрытие окна.
			bool			on_close_callback(GdkEventAny* event);

			/// Обработчик сигнала на завершение процесса создания торрента.
			void 			on_finished_callback(void);

			/// Обработчик сигнала на изменение прогресса в создании торрента.
			void 			on_progress_changed_callback(void);


		public:
			/// Поток для хеширования файлов торрента.
			void operator()(void);
	};



	// Torrent_source -->
		Torrent_source::Torrent_source(
			const std::string& files,
			bool private_torrent,
			Size piece_size,
			const std::vector<std::string>& trackers,
			const std::string& save_path
		)
		:
			files(files),
			private_torrent(private_torrent),
			piece_size(piece_size),
			trackers(trackers),
			save_path(save_path)
		{
		}
	// Torrent_source <--



	// Progress_dialog -->
		Progress_dialog::Progress_dialog(Gtk::Window& parent_window, const Torrent_source& torrent_source)
		:
			Gtk::Dialog(std::string(APP_NAME) + ": " + _("Hashing files..."), parent_window, true),
			thread(NULL),
			is_cancel(false),
			torrent_source(torrent_source),
			is_created(false)
		{
			this->property_destroy_with_parent() = true;
			this->set_border_width(m::gtk::BOX_BORDER_WIDTH);
			this->set_resizable(false);


			Gtk::VBox* main_vbox = this->get_vbox();
			main_vbox->set_spacing(m::gtk::VBOX_SPACING);

			Gtk::Label* label = Gtk::manage(new Gtk::Label(_("Hashing files. Please wait...")));
			main_vbox->pack_start(*label, false, false);

			this->progress_bar = Gtk::manage(new Gtk::ProgressBar());
			main_vbox->pack_start(*this->progress_bar, false, false);

			// Cancel -->
			{
				Gtk::HButtonBox* button_box = this->get_action_area();
				button_box->set_layout(Gtk::BUTTONBOX_CENTER);


				Gtk::Button* button;

				button = Gtk::manage(new Gtk::Button(Gtk::Stock::CANCEL));
				button->signal_clicked().connect(
					sigc::mem_fun(*this, &Progress_dialog::on_cancel_button_callback)
				);
				button_box->add(*button);
			}
			// Cancel <--

			this->show_all();

			// Закрытие окна
			this->signal_delete_event().connect(
				sigc::mem_fun(*this, &Progress_dialog::on_close_callback)
			);

			/// Сигнал на изменение прогресса в создании торрента.
			this->progress_signal.connect(
				sigc::mem_fun(*this, &Progress_dialog::on_progress_changed_callback)
			);

			/// Сигнал на завершение процесса создания торрента.
			this->finished_signal.connect(
				sigc::mem_fun(*this, &Progress_dialog::on_finished_callback)
			);

			// Запускаем процесс создания торрента
			this->thread = new boost::thread(boost::ref(*this));
		}



		Progress_dialog::~Progress_dialog(void)
		{
			if(this->thread)
			{
				this->thread->join();
				delete this->thread;
			}
		}



		bool Progress_dialog::check_added_file(const boost::filesystem::path& path) throw(Break_torrent_creating)
		{
			boost::mutex::scoped_lock lock(this->mutex);

			if(this->is_cancel)
				throw Break_torrent_creating();

			return true;
		}



		void Progress_dialog::on_cancel_button_callback(void)
		{
			boost::mutex::scoped_lock lock(this->mutex);
			this->get_vbox()->set_sensitive(false);
			this->is_cancel = true;
		}



		bool Progress_dialog::on_close_callback(GdkEventAny* event)
		{
			this->on_cancel_button_callback();
			return true;
		}



		void Progress_dialog::set_hash_progress(Size progress, Size hash_num) throw(Break_torrent_creating)
		{
			boost::mutex::scoped_lock lock(this->mutex);

			if(this->is_cancel)
				throw Break_torrent_creating();
			else
			{
				this->progress = double(Size_float(progress) / Size_float(hash_num));
				this->progress_signal();
			}
		}



		void Progress_dialog::on_finished_callback(void)
		{
			if(this->is_created)
			{
				Gtk::Window* parent_window = this->get_transient_for();

				MLIB_I(
					_("Torrent created"),
					__(
						"Torrent '%1' has been created successfully.",
						this->torrent_source.save_path
					)
				);

				delete parent_window;
			}
			else
			{
				if(this->error.get())
				{
					show_warning_message(
						*this, _("Creating torrent failed"),
						__(
							"Creating torrent '%1' failed. %2",
							this->torrent_source.save_path, EE(*this->error)
						)
					);
				}

				delete this;
			}
		}



		void Progress_dialog::on_progress_changed_callback(void)
		{
			boost::mutex::scoped_lock lock(this->mutex);

			this->progress_bar->set_fraction(this->progress);
		}



		void Progress_dialog::operator()(void)
		{
			try
			{
				lt::file_storage fs;
				std::auto_ptr<lt::create_torrent> torrent;

				MLIB_D(_C("Creating torrent '%1'...", this->torrent_source.save_path));

				// Создаем дескриптор торрента -->
				{
					try
					{
						add_files(
							fs, U2L(this->torrent_source.files),
							sigc::mem_fun(*this, &Progress_dialog::check_added_file)
						);
					}
					catch(Break_torrent_creating)
					{
						throw;
					}
					catch(boost::fs::basic_filesystem_error<boost::fs::path>& e)
					{
						M_THROW(__(
							"Error while reading torrent files. File '%1': %2.",
							e.path1().string(), EE(e)
						));
					}
					catch(...)
					{
						M_THROW(_("Libtorrent internal error happened while reading torrent files."));
					}

					try
					{
						torrent = std::auto_ptr<lt::create_torrent>(
							new lt::create_torrent(fs, this->torrent_source.piece_size)
						);

						for(size_t i = 0; i < this->torrent_source.trackers.size(); i++)
							torrent->add_tracker(this->torrent_source.trackers[i]);

						torrent->set_creator( (std::string(APP_NAME) + " " + APP_VERSION_STRING).c_str() );
						torrent->set_priv(this->torrent_source.private_torrent);
					}
					catch(...)
					{
						M_THROW(_("Libtorrent internal error happened while creating torrent."));
					}
				}
				// Создаем дескриптор торрента <--

				// Хешируем файлы торрента -->
					try
					{
						Size hash_num = torrent->num_pieces();

						lt::set_piece_hashes(
							*torrent, U2L(Path(this->torrent_source.files).dirname()),
							sigc::bind<Size>(
								sigc::mem_fun(*this, &Progress_dialog::set_hash_progress), hash_num
							)
						);
					}
					catch(Break_torrent_creating)
					{
						throw;
					}
					catch(boost::fs::basic_filesystem_error<boost::fs::path>& e)
					{
						M_THROW(__(
							"Error while reading torrent files. File '%1': %2.",
							e.path1().string(),
							#if M_BOOST_GET_VERSION() < M_GET_VERSION(1, 35, 0)
								M_LIBRARY_COMPATIBILITY
								strerror(e.system_error())
							#else
								strerror(e.code().value())
							#endif
						));
					}
					catch(...)
					{
						M_THROW(_("Libtorrent internal error happened while hashing torrent files."));
					}
				// Хешируем файлы торрента <--

				// Получаем данные торрента
				lt::entry torrent_entry = torrent->generate();

				// Если у нас не UTF-8 локаль, то libtorrent записал в торрент
				// все имена файлов в кодировке локали, поэтому исправляем их
				// на UTF-8.
				// -->
					if(!Glib::get_charset())
					{
						try
						{
							lt::entry* info_entry = torrent_entry.find_key("info");

							if(!info_entry)
								throw lt::type_error("");

							{
								lt::entry* name_entry = info_entry->find_key("name");

								if(!name_entry)
									throw lt::type_error("");

								*name_entry = L2U(name_entry->string());
							}

							{
								lt::entry* files_entry;

								if( (files_entry = info_entry->find_key("files")) )
								{
									M_FOR_IT(lt::entry::list_type, files_entry->list(), files_it)
									{
										lt::entry* path_entry = files_it->find_key("path");

										if(!path_entry)
											throw lt::type_error("");

										M_FOR_IT(lt::entry::list_type, path_entry->list(), names_it)
											*names_it = L2U(names_it->string());
									}
								}
							}
						}
						catch(lt::type_error)
						{
							M_THROW(_("Libtorrent return invalid torrent file."));
						}
					}
				// <--

				// Пишем *.torrent файл -->
					try
					{
						std::ofstream torrent_file;

						torrent_file.exceptions(
							std::ofstream::eofbit | std::ofstream::failbit | std::ofstream::badbit
						);

						torrent_file.open(
							U2L(this->torrent_source.save_path).c_str(),
							std::ios::out | std::ios::binary | std::ios::trunc
						);

						bencode( std::ostream_iterator<char>(torrent_file), torrent_entry );

						torrent_file.close();
					}
					catch(std::ofstream::failure& e)
					{
						M_THROW(__(
							"Can't write torrent file '%1': %2.",
							this->torrent_source.save_path, EE(errno)
						));
					}
					catch(...)
					{
						M_THROW(__(
							"Libtorrent internal error happened while creating torrent file '%1'.",
							this->torrent_source.save_path
						));
					}
				// Пишем *.torrent файл <--

				this->is_created = true;
			}
			catch(Break_torrent_creating)
			{
				MLIB_D(_C("Creating torrent '%1' has been canceled.", this->torrent_source.save_path));
			}
			catch(m::Exception& e)
			{
				this->error = std::auto_ptr<m::Exception>(new m::Exception(e));
			}

			// Оповещаем внешнюю среду о завершении процесса создания торрента.
			this->finished_signal();
		}
	// Progress_dialog <--
}



// Piece_sizes_model_columns -->
	Create_torrent_dialog::Piece_sizes_model_columns::Piece_sizes_model_columns(void)
	{
		this->add(this->size);
		this->add(this->size_string);
	}
// Piece_sizes_model_columns <--



// Create_torrent_dialog -->
	Create_torrent_dialog::Create_torrent_dialog(Gtk::Window& parent_window)
	:
		m::gtk::Window(
			parent_window,
			std::string(APP_NAME) + ": " + _("Creating torrent"),
			get_application().get_client_settings().gui.create_torrent_dialog.window, -1, -1
		),

		torrent_files_path_entry( Gtk::manage(new Gtk::Entry()) ),

		private_torrent_button( Gtk::manage(new Gtk::CheckButton()) ),

		piece_size_combo( Gtk::manage(new Gtk::ComboBox()) ),

		trackers_view( Gtk::manage(new Trackers_view) )
	{
		Gtk::VBox* main_vbox = Gtk::manage(new Gtk::VBox(false, m::gtk::VBOX_SPACING));
		this->add(*main_vbox);

		m::gtk::vbox::add_header(*main_vbox, _("Torrent:"));

		// Виджеты выбора файла или директории -->
		{
			Gtk::Label* label = Gtk::manage(new Gtk::Label());
			label->set_label(_("File or directory to create torrent from:"));
			label->set_alignment(Gtk::ALIGN_LEFT);
			main_vbox->pack_start(*label, false, false);

			Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
			main_vbox->pack_start(*hbox, false, false);

			this->torrent_files_path_entry->set_editable(false);
			hbox->pack_start(*this->torrent_files_path_entry, true, true);

			Gtk::Button* button;

			button = Gtk::manage(new Gtk::Button());
			button->set_image( *Gtk::manage( new Gtk::Image(Gtk::Stock::FILE, Gtk::ICON_SIZE_MENU) ));
			button->signal_clicked().connect(sigc::bind<bool>(
				sigc::mem_fun(*this, &Create_torrent_dialog::on_select_callback), false
			));
			hbox->pack_start(*button, false, false);

			button = Gtk::manage(new Gtk::Button());
			button->set_image( *Gtk::manage( new Gtk::Image(Gtk::Stock::DIRECTORY, Gtk::ICON_SIZE_MENU) ));
			button->signal_clicked().connect(sigc::bind<bool>(
				sigc::mem_fun(*this, &Create_torrent_dialog::on_select_callback), true
			));
			hbox->pack_start(*button, false, false);
		}
		// Виджеты выбора файла или директории <--

		// Private torrent -->
		{
			Gtk::Alignment* alignment = Gtk::manage(
				new Gtk::Alignment(Gtk::ALIGN_RIGHT, Gtk::ALIGN_CENTER, 0, 0)
			);
			main_vbox->pack_start(*alignment, false, false);

			this->private_torrent_button->set_label(_("Private torrent (DHT is not allowed)"));
			alignment->add(*this->private_torrent_button);
		}
		// Private torrent <--

		// Размер одной части -->
		{
			Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, m::gtk::HBOX_SPACING));
			main_vbox->pack_start(*hbox, false, false);

			Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(this->piece_sizes_columns);

			{
				Gtk::TreeModel::Row row;

				row = *(model->append());
				row[this->piece_sizes_columns.size] = 32 * 1024;
				row[this->piece_sizes_columns.size_string] = "32 " + _("KB");

				row = *(model->append());
				row[this->piece_sizes_columns.size] = 64 * 1024;
				row[this->piece_sizes_columns.size_string] = "64 " + _("KB");

				row = *(model->append());
				row[this->piece_sizes_columns.size] = 128 * 1024;
				row[this->piece_sizes_columns.size_string] = "128 " + _("KB");

				row = *(model->append());
				row[this->piece_sizes_columns.size] = 256 * 1024;
				row[this->piece_sizes_columns.size_string] = "256 " + _("KB");

				row = *(model->append());
				row[this->piece_sizes_columns.size] = 512 * 1024;
				row[this->piece_sizes_columns.size_string] = "512 " + _("KB");

				row = *(model->append());
				row[this->piece_sizes_columns.size] = 1024 * 1024;
				row[this->piece_sizes_columns.size_string] = "1 " + _("MB");

				row = *(model->append());
				row[this->piece_sizes_columns.size] = 2 * 1024 * 1024;
				row[this->piece_sizes_columns.size_string] = "2 " + _("MB");

				row = *(model->append());
				row[this->piece_sizes_columns.size] = 4 * 1024 * 1024;
				row[this->piece_sizes_columns.size_string] = "4 " + _("MB");

				row = *(model->append());
				row[this->piece_sizes_columns.size] = 8 * 1024 * 1024;
				row[this->piece_sizes_columns.size_string] = "8 " + _("MB");
			}

			this->piece_size_combo->set_model(model);
			this->piece_size_combo->set_active(4);
			this->piece_size_combo->pack_start(this->piece_sizes_columns.size_string);
			hbox->pack_end(*this->piece_size_combo, false, false);

			Gtk::Label* label = Gtk::manage(new Gtk::Label());
			label->set_label(_("Piece size:"));
			hbox->pack_end(*label, false, false);
		}
		// Размер одной части <--

		// Трекеры -->
			m::gtk::vbox::add_header(*main_vbox, _("Trackers:"));
			main_vbox->pack_start(*this->trackers_view, true, true);
		// Трекеры <--

		m::gtk::vbox::add_space(*main_vbox);

		// OK, Cancel -->
		{
			Gtk::ButtonBox* button_box = Gtk::manage(new Gtk::HButtonBox());
			button_box->set_layout(Gtk::BUTTONBOX_END);
			button_box->set_spacing(m::gtk::HBOX_SPACING);
			main_vbox->pack_start(*button_box, false, false);


			Gtk::Button* button;

			button = Gtk::manage(new Gtk::Button(Gtk::Stock::CANCEL));
			button->signal_clicked().connect(sigc::mem_fun(*this, &Create_torrent_dialog::on_cancel_button_callback));
			button_box->add(*button);

			button = Gtk::manage(new Gtk::Button(Gtk::Stock::OK));
			button->signal_clicked().connect(sigc::mem_fun(*this, &Create_torrent_dialog::on_ok_button_callback));
			button_box->add(*button);
		}
		// OK, Cancel <--

		// Закрытие окна
		this->signal_delete_event().connect(
			sigc::mem_fun(*this, &Create_torrent_dialog::on_close_callback)
		);

		this->show_all();
	}



	Create_torrent_dialog::~Create_torrent_dialog(void)
	{
		this->save_settings(get_application().get_client_settings().gui.create_torrent_dialog);
	}



	void Create_torrent_dialog::on_cancel_button_callback(void)
	{
		delete this;
	}



	bool Create_torrent_dialog::on_close_callback(GdkEventAny* event)
	{
		delete this;
		return true;
	}



	void Create_torrent_dialog::on_ok_button_callback(void)
	{
		if(this->torrent_files_path_entry->get_text() == "")
		{
			show_warning_message(
				*this, _("File or directory to create torrent from is not setted"),
				_("Please choose file or directory to create torrent from.")
			);
		}
		else
		{
			Create_torrent_dialog_settings& settings = get_application().get_client_settings().gui.create_torrent_dialog;

			// Запрашиваем путь для сохранения *.torrent файла -->
				Gtk::FileChooserDialog dialog(
					*this, _("Please choose a file to save the torrent"),
					Gtk::FILE_CHOOSER_ACTION_SAVE
				);

				dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
				dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
				dialog.set_default_response(Gtk::RESPONSE_OK);

				if(settings.save_to != "")
					dialog.set_current_folder(U2L(settings.save_to));

				// set_current_name принимает UTF
				dialog.set_current_name(
					Path(this->torrent_files_path_entry->get_text()).basename() + ".torrent"
				);

				// Добавляем фильтры по типам файлов -->
				{
					Gtk::FileFilter torrents_filter;
					torrents_filter.set_name(_("Torrent files"));
					torrents_filter.add_mime_type("application/x-bittorrent");
					dialog.add_filter(torrents_filter);

					Gtk::FileFilter any_filter;
					any_filter.set_name(_("Any files"));
					any_filter.add_pattern("*");
					dialog.add_filter(any_filter);
				}
				// Добавляем фильтры по типам файлов <--
			// Запрашиваем путь для сохранения *.torrent файла <--

			if(dialog.run() == Gtk::RESPONSE_OK)
			{
				settings.save_to = dialog.get_current_folder();

				// Запускаем процесс создания торрента
				new Progress_dialog(
					*this,
					Torrent_source(
						this->torrent_files_path_entry->get_text(),
						this->private_torrent_button->get_active(),
						this->piece_size_combo->get_active()->get_value(this->piece_sizes_columns.size),
						this->trackers_view->get(),
						L2U(dialog.get_filename())
					)
				);
			}
		}
	}



	void Create_torrent_dialog::on_select_callback(bool directory)
	{
		Create_torrent_dialog_settings& settings = get_application().get_client_settings().gui.create_torrent_dialog;

		Gtk::FileChooserDialog dialog(
			*this,
			(
				directory
				?
					_("Please choose a directory to create torrent from")
				:
					_("Please choose a file to create torrent from")
			),
			(
				directory
				?
					Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER
				:
					Gtk::FILE_CHOOSER_ACTION_OPEN
			)
		);

		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
		dialog.set_default_response(Gtk::RESPONSE_OK);

		if(this->torrent_files_path_entry->get_text() != "")
			dialog.set_current_folder( U2L(Path(this->torrent_files_path_entry->get_text()).dirname()) );
		else if(settings.get_from != "")
			dialog.set_current_folder( U2L(settings.get_from) );

		if(dialog.run() == Gtk::RESPONSE_OK)
		{
			settings.get_from = L2U(dialog.get_current_folder());
			this->torrent_files_path_entry->set_text(L2U(dialog.get_filename()));
		}
	}



	void Create_torrent_dialog::save_settings(Create_torrent_dialog_settings& settings)
	{
		m::gtk::Window::save_settings(settings.window);

		if(this->torrent_files_path_entry->get_text() != "")
			settings.get_from = Path(this->torrent_files_path_entry->get_text()).dirname();
	}
// Create_torrent_dialog

