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


#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>

#include <mlib/gtk/signal_proxy.hpp>
#include <mlib/signals_holder.hpp>

#include "categories_view.hpp"
#include "client_settings.hpp"
#include "common.hpp"
#include "daemon_proxy.hpp"
#include "log_view.hpp"
#include "main_window.hpp"
#include "main.hpp"
#include "torrent_details_view.hpp"
#include "torrent_files_view.hpp"
#include "torrent_options_view.hpp"
#include "torrent_peers_view.hpp"
#include "torrents_view.hpp"
#include "torrents_viewport.hpp"


namespace Torrents_viewport_aux
{

class Private
{
	public:
		Private(const Torrents_viewport_settings& settings);

	public:
		/// Сигнал на изменение списка действий, которые можно выполнить над
		/// торрентом(ами), выделенным(ми) в данный момент.
		sigc::signal<void,
		Torrent_process_actions>	torrent_process_actions_changed_signal;


		std::vector<Torrent_info>	torrents;


		Gtk::HBox*					torrents_hbox;

		Categories_view*			categories_view;

		Gtk::Frame					torrents_view_frame;
		Gtk::ScrolledWindow			torrents_view_scrolled_window;
		Torrents_view*				torrents_view;


		/// Указатель на текущий информационный виджет.
		Info_widget*				current_info_widget;

		/// Выделенный в данный момент торрент.
		Torrent_id					cur_torrent_id;


		m::Signals_holder			sholder;


	public:
		/// Обработчик сигнала на изменение списка выделенных торрентов.
		void	on_torrent_selected_callback(const Torrent_id& torrent_id);

		/// Перерисовывает виджет, отображающий категории торрентов.
		void	redraw_categories_view(void);

		/// Перерисовывает виджет, отображающий список торрентов.
		void	redraw_torrents_view(void);
};



Private::Private(const Torrents_viewport_settings& settings)
:
	current_info_widget(NULL)
{
	this->torrents_hbox = Gtk::manage( new Gtk::HBox );

	// Categories_view
	this->categories_view = Gtk::manage( new Categories_view(*settings.categories_view) );
	this->torrents_hbox->pack_start(*this->categories_view, false, false);

	// Torrents_view -->
		this->torrents_view = Gtk::manage( new Torrents_view(settings.torrents_view) );
		this->torrents_view_scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		this->torrents_view_scrolled_window.add(*this->torrents_view);
		this->torrents_view_frame.add(this->torrents_view_scrolled_window);

		// Сигнал на изменение списка выделенных торрентов
		this->sholder.push(
			this->torrents_view->torrent_selected_signal.connect(
				sigc::mem_fun(*this, &Private::on_torrent_selected_callback))
		);

		this->torrents_hbox->pack_start(this->torrents_view_frame, true, true);
	// Torrents_view <--

	// Обработчик сигнала на изменение списка интересующих пользователя
	// категорий.
	this->sholder.push(
		this->categories_view->signal_changed().connect(
			sigc::mem_fun(*this, &Private::redraw_torrents_view))
	);

	// Обработчик сигнала на требование информации о текущих торрентах.
	this->sholder.push(
		this->categories_view->signal_needs_update().connect(
			sigc::mem_fun(*this, &Private::redraw_categories_view))
	);
}



void Private::on_torrent_selected_callback(const Torrent_id& torrent_id)
{
	this->cur_torrent_id = torrent_id;

	// Сообщаем "наверх" о том, что действия, которые можно выполнять над
	// выделенными в данный момент торрентами изменились.
	this->torrent_process_actions_changed_signal(
		this->torrents_view->get_available_actions());

	if(this->current_info_widget)
		this->current_info_widget->torrent_changed(this->cur_torrent_id);
}



void Private::redraw_categories_view(void)
{
	this->categories_view->update(this->torrents);
}



void Private::redraw_torrents_view(void)
{
	class Filter: public Torrents_view_filter, public Categories_filter
	{
		public:
			Filter(const Categories_filter& filter)
			: Categories_filter(filter) {}

		private:
			Filter(void);


		public:
			virtual bool operator()(const Torrent_info& info) const
			{
				return Categories_filter::operator()(info);
			}
	};


	Filter filter = this->categories_view->get_filter();

	this->torrents_view->update(this->torrents, filter);

	// Сообщаем "наверх" о том, что действия, которые можно выполнять над
	// выделенными в данный момент торрентами изменились.
	this->torrent_process_actions_changed_signal(
		this->torrents_view->get_available_actions());
}

}



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

	priv(new Private(settings)),

	torrents_view_and_torrent_infos_vpaned(settings.torrents_view_and_torrent_infos_vpaned),

	info_mode(false),
	toggle_in_process(false)
{
	// Т. к. по умолчанию режим без отображения информационных виджетов
	this->main_vbox.add(*priv->torrents_hbox);

	// torrent_infos_notebook -->
		this->torrent_infos_notebook.set_show_tabs(false);
		this->torrent_infos_notebook.set_show_border(false);
		this->torrents_view_and_torrent_infos_vpaned.add2(this->torrent_infos_notebook);
	// torrent_infos_notebook <--

	// Информационные виджеты -->
		// Torrent_details_view -->
			this->torrent_details_view = Gtk::manage(new Torrent_details_view);
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
					priv->current_info_widget = this->info_widgets[i].widget;
					break;
				}
			}

			MLIB_A(priv->current_info_widget);

			// Инициируем перерисовку виджета
			priv->current_info_widget->update(priv->cur_torrent_id);
		}
		// Переключаемся на нужную вкладку <--
	}
	else
		priv->current_info_widget = NULL;

	// Включаем соответствующий режим
	this->set_info_mode(toggled_button->get_active());

	this->toggle_in_process = false;
}



void Torrents_viewport::process_torrents(Torrent_process_action action)
{
	priv->torrents_view->process_torrents(action);
}



void Torrents_viewport::save_settings(Torrents_viewport_settings& settings) const
{
	// info_widget -->
	{
		settings.info_widget = "";

		if(priv->current_info_widget)
		{
			for(size_t widget_id = 0; widget_id < this->info_widgets.size(); widget_id++)
			{
				if(this->info_widgets[widget_id].widget == priv->current_info_widget)
				{
					settings.info_widget = this->info_widgets[widget_id].name;
					break;
				}
			}
		}
	}
	// info_widget <--

	this->torrents_view_and_torrent_infos_vpaned.save_settings(settings.torrents_view_and_torrent_infos_vpaned);
	priv->categories_view->save_settings(settings.categories_view.get());
	priv->torrents_view->save_settings(settings.torrents_view);
	this->torrent_files_view->save_settings(settings.torrent_files_view);
	this->torrent_peers_view->save_settings(settings.torrent_peers_view);
}



void Torrents_viewport::set_info_mode(bool info_mode)
{
	if(info_mode == this->info_mode)
		return;

	if(info_mode)
	{
		this->main_vbox.remove(*priv->torrents_hbox);
		this->torrents_view_and_torrent_infos_vpaned.pack1(*priv->torrents_hbox, false, false);
		this->main_vbox.add(this->torrents_view_and_torrent_infos_vpaned);
	}
	else
	{
		this->torrents_view_and_torrent_infos_vpaned.remove(*priv->torrents_hbox);
		this->main_vbox.remove(this->torrents_view_and_torrent_infos_vpaned);
		this->main_vbox.add(*priv->torrents_hbox);
	}

	this->info_mode = info_mode;
}



void Torrents_viewport::show_categories(bool show)
{
	if(show)
		priv->categories_view->show();
	else
		priv->categories_view->hide();
}



void Torrents_viewport::show_categories_names(bool show)
{
	priv->categories_view->show_names(show);
}



void Torrents_viewport::show_categories_counters(bool show)
{
	priv->categories_view->show_counters(show);
}



m::gtk::Signal_proxy<void, Torrent_process_actions> Torrents_viewport::signal_torrent_process_actions_changed(void)
{
	return priv->torrent_process_actions_changed_signal;
}



void Torrents_viewport::update(void)
{
	std::vector<Torrent_info> torrents;

	try
	{
		get_daemon_proxy().get_torrents(torrents);
	}
	catch(m::Exception& e)
	{
		MLIB_W(EE(e));
	}

	priv->torrents.swap(torrents);

	priv->redraw_categories_view();
	priv->redraw_torrents_view();

	if(priv->current_info_widget)
		priv->current_info_widget->update(priv->cur_torrent_id);
}

