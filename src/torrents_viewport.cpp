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


#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>

#include "client_settings.hpp"
#include "log_view.hpp"
#include "torrent_details_view.hpp"
#include "torrent_files_view.hpp"
#include "torrent_options_view.hpp"
#include "torrent_peers_view.hpp"
#include "torrents_view.hpp"
#include "torrents_viewport.hpp"



Torrents_viewport::Info_widget_handle::Info_widget_handle(const std::string& name, Gtk::ToggleButton& button, Info_widget& widget)
:
	name(name),
	button(&button),
	widget(&widget)
{
}



Torrents_viewport::Torrents_viewport(const Torrents_viewport_settings& settings)
:
	Gtk::VBox(),

	torrents_view_and_torrent_infos_vpaned(settings.torrents_view_and_torrent_infos_vpaned),

	info_mode(false),
	toggle_in_process(false),

	current_info_widget(NULL)
{
	// Torrents_view -->
		this->torrents_view = Gtk::manage(new Torrents_view(settings.torrents_view));
		this->torrents_view_scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		this->torrents_view_scrolled_window.add(*this->torrents_view);
		this->torrents_view_frame.add(this->torrents_view_scrolled_window);

		// Сигнал на изменение списка выделенных торрентов -->
			this->torrents_view->torrent_selected_signal.connect(
				sigc::mem_fun(*this, &Torrents_viewport::on_torrent_selected_callback)
			);
		// Сигнал на изменение списка выделенных торрентов <--
	// Torrents_view <--

	// torrent_infos_notebook -->
		this->torrent_infos_notebook.set_show_tabs(false);
		this->torrent_infos_notebook.set_show_border(false);
		this->torrents_view_and_torrent_infos_vpaned.add2(this->torrent_infos_notebook);
	// torrent_infos_notebook <--

	// Информационные виджеты -->
		// Torrent_details_view -->
			this->torrent_details_view = Gtk::manage(new Torrent_details_view());
			this->torrent_details_view_scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
			this->torrent_details_view_scrolled_window.add(*this->torrent_details_view);

			this->details_toggle_button.set_label(_("Details"));
			this->details_toggle_button.set_image( *Gtk::manage( new Gtk::Image(Gtk::Stock::PROPERTIES, Gtk::ICON_SIZE_MENU) ));

			this->add_info_widget(
				"details",
				this->torrent_details_view_scrolled_window,
				*this->torrent_details_view,
				this->details_toggle_button
			);
		// Torrent_details_view <--

		// Torrent_files_view -->
			this->torrent_files_view = Gtk::manage(new Torrent_files_dynamic_view(settings.torrent_files_view));
			this->torrent_files_view_scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
			this->torrent_files_view_scrolled_window.set_shadow_type(Gtk::SHADOW_IN);
			this->torrent_files_view_scrolled_window.add(*this->torrent_files_view);

			this->files_list_toggle_button.set_label(_("Files"));
			this->files_list_toggle_button.set_image( *Gtk::manage( new Gtk::Image(Gtk::Stock::HARDDISK, Gtk::ICON_SIZE_MENU) ));

			this->add_info_widget(
				"files_list",
				this->torrent_files_view_scrolled_window,
				*this->torrent_files_view,
				this->files_list_toggle_button
			);
		// Torrent_files_view <--

		// Torrent_peers_view -->
			this->torrent_peers_view = Gtk::manage(new Torrent_peers_view(settings.torrent_peers_view));
			this->torrent_peers_view_scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
			this->torrent_peers_view_scrolled_window.set_shadow_type(Gtk::SHADOW_IN);
			this->torrent_peers_view_scrolled_window.add(*this->torrent_peers_view);

			this->peers_list_toggle_button.set_label(_("Peers"));
			this->peers_list_toggle_button.set_image( *Gtk::manage( new Gtk::Image(Gtk::Stock::NETWORK, Gtk::ICON_SIZE_MENU) ));

			this->add_info_widget(
				"peers_list",
				this->torrent_peers_view_scrolled_window,
				*this->torrent_peers_view,
				this->peers_list_toggle_button
			);
		// Torrent_peers_view <--

		// Torrent_options_view -->
			this->torrent_options_view = Gtk::manage(new Torrent_options_view());
			this->torrent_options_view_scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
			this->torrent_options_view_scrolled_window.add(*this->torrent_options_view);

			this->options_toggle_button.set_label(_("Options"));
			this->options_toggle_button.set_image( *Gtk::manage( new Gtk::Image(Gtk::Stock::PREFERENCES, Gtk::ICON_SIZE_MENU) ));

			this->add_info_widget(
				"options",
				this->torrent_options_view_scrolled_window,
				*this->torrent_options_view,
				this->options_toggle_button
			);
		// Torrent_options_view <--

		// Log_view -->
			this->log_view = Gtk::manage(new Log_view());
			this->log_view_scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
			this->log_view_scrolled_window.set_shadow_type(Gtk::SHADOW_IN);
			this->log_view_scrolled_window.add(*this->log_view);

			this->log_toggle_button.set_label(_("Log"));
			this->log_toggle_button.set_image( *Gtk::manage( new Gtk::Image(Gtk::Stock::EDIT, Gtk::ICON_SIZE_MENU) ));

			this->add_info_widget(
				"log",
				this->log_view_scrolled_window,
				*this->log_view,
				this->log_toggle_button
			);
		// Log_view <--
	// Информационные виджеты <--

	// Т. к. по умолчанию режим без отображения информационных виджетов
	this->main_vbox.add(this->torrents_view_frame);
	this->torrents_view_and_torrent_infos_vpaned.show_all();

	// Упаковываем основные контейнеры -->
		this->pack_start(this->main_vbox, true, true);
		this->pack_start(this->info_toggle_buttons_hbox, false, false);
	// Упаковываем основные контейнеры <--

	// Загружаем полученные настройки -->
		if(settings.info_widget != "")
		{
			for(size_t widget_id = 0; widget_id < this->info_widgets.size(); widget_id++)
			{
				if(this->info_widgets[widget_id].name == settings.info_widget)
				{
					// Если такой виджет существует, то просто эмулируем
					// нажатие на него кнопки мыши.
					this->info_widgets[widget_id].button->set_active();
					break;
				}
			}
		}
	// Загружаем полученные настройки <--
}



void Torrents_viewport::add_info_widget(const std::string& name, Gtk::Widget& widget_container, Info_widget& widget, Gtk::ToggleButton& button)
{
	this->info_widgets.push_back(Info_widget_handle(name, button, widget));
	this->torrent_infos_notebook.append_page(widget_container);
	this->info_toggle_buttons_hbox.pack_start(button, false, false);

	button.signal_toggled().connect(
		sigc::bind<Gtk::ToggleButton*>(
			sigc::mem_fun(*this, &Torrents_viewport::on_toggle_info_callback),
			&button
		)
	);
}



void Torrents_viewport::on_toggle_info_callback(Gtk::ToggleButton* toggled_button)
{
	if(this->toggle_in_process)
		return;

	this->toggle_in_process = true;

	if(toggled_button->get_active())
	{
		// Отключаем остальные кнопки -->
		{
			for(size_t i = 0; i < this->info_widgets.size(); i++)
			{
				Gtk::ToggleButton& button = *this->info_widgets[i].button;

				if(&button != toggled_button && button.get_active())
				{
					button.set_active(false);

					// Т. к. в один момент больше одной кнопки не может
					// быть нажато, то можно выйти из цикла, если найдем
					// хотя бы одну.
					break;
				}
			}
		}
		// Отключаем остальные кнопки <--

		// Переключаемся на нужную вкладку -->
		{
			for(size_t i = 0; i < this->info_widgets.size(); i++)
			{
				if(this->info_widgets[i].button == toggled_button)
				{
					this->torrent_infos_notebook.set_current_page(i);
					this->current_info_widget = this->info_widgets[i].widget;
					break;
				}
			}

			MLIB_A(this->current_info_widget);

			// Инициируем перерисовку виджета
			this->current_info_widget->update(this->cur_torrent_id);
		}
		// Переключаемся на нужную вкладку <--
	}
	else
		this->current_info_widget = NULL;

	// Включаем соответствующий режим
	this->set_info_mode(toggled_button->get_active());

	this->toggle_in_process = false;
}



void Torrents_viewport::on_torrent_selected_callback(const Torrent_id& torrent_id)
{
	this->cur_torrent_id = torrent_id;

	if(this->current_info_widget)
		this->current_info_widget->torrent_changed(this->cur_torrent_id);
}



void Torrents_viewport::update(std::vector<Torrent_info>::iterator infos_it, const std::vector<Torrent_info>::iterator& infos_end_it)
{
	this->torrents_view->update(infos_it, infos_end_it);

	if(this->current_info_widget)
		this->current_info_widget->update(this->cur_torrent_id);
}



void Torrents_viewport::save_settings(Torrents_viewport_settings& settings) const
{
	// info_widget -->
	{
		settings.info_widget = "";

		if(this->current_info_widget)
		{
			for(size_t widget_id = 0; widget_id < this->info_widgets.size(); widget_id++)
			{
				if(this->info_widgets[widget_id].widget == this->current_info_widget)
				{
					settings.info_widget = this->info_widgets[widget_id].name;
					break;
				}
			}
		}
	}
	// info_widget <--

	this->torrents_view_and_torrent_infos_vpaned.save_settings(settings.torrents_view_and_torrent_infos_vpaned);
	this->torrents_view->save_settings(settings.torrents_view);
	this->torrent_files_view->save_settings(settings.torrent_files_view);
	this->torrent_peers_view->save_settings(settings.torrent_peers_view);
}



void Torrents_viewport::set_info_mode(bool info_mode)
{
	if(info_mode == this->info_mode)
		return;

	if(info_mode)
	{
		this->main_vbox.remove(this->torrents_view_frame);
		this->torrents_view_and_torrent_infos_vpaned.add1(this->torrents_view_frame);
		this->main_vbox.add(this->torrents_view_and_torrent_infos_vpaned);
	}
	else
	{
		this->torrents_view_and_torrent_infos_vpaned.remove(this->torrents_view_frame);
		this->main_vbox.remove(this->torrents_view_and_torrent_infos_vpaned);
		this->main_vbox.add(this->torrents_view_frame);
	}

	this->info_mode = info_mode;
}

