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


#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>

#include "common.hpp"
#include "log_view.hpp"


Log_view::Log_view(void)
:
	buffer(this->get_buffer()),
	max_lines(-1)
{
	this->end_mark = this->buffer->create_mark("end", this->buffer->end());
	this->set_editable(false);
}



void Log_view::add_message(const Daemon_message& message)
{
	this->messages.push_back(message);
	this->remove_old_messages();
}



void Log_view::remove_old_lines(void)
{
	if(this->max_lines < 0)
		return;

	int log_lines = this->buffer->end().get_line() - this->buffer->begin().get_line();

	if(this->buffer->begin() != this->buffer->end())
		log_lines++;
	
	if(log_lines > this->max_lines)
	{
		int end_iter_line = log_lines - this->max_lines - 1;
		if(end_iter_line < 0)
			end_iter_line = 0;

		Gtk::TextBuffer::iterator end_line_iter = this->buffer->get_iter_at_line(end_iter_line);
		end_line_iter.set_line_offset(end_line_iter.get_chars_in_line());

		this->buffer->erase(this->buffer->begin(), end_line_iter);
	}
}



void Log_view::remove_old_messages(void)
{
	// Для упрощения считаем, что каждое сообщение состоит из одной строки.

	// Удаляем лишние сообщения, если они есть.
	if(this->messages.size() > static_cast<size_t>(this->max_lines) && this->max_lines >= 0)
		this->messages.erase(this->messages.begin(), this->messages.begin() + ( this->messages.size() - this->max_lines ) );
}



void Log_view::set_max_lines(const int& max_lines)
{
	this->max_lines = max_lines;
	this->remove_old_messages();
	this->remove_old_lines();
}



void Log_view::update(const Torrent_id& torrent_id)
{
	// Если есть еще неотображенные сообщения
	if(this->messages.size())
	{
		bool is_scroll_needed = false;

		// Определяем, нужно ли прокручивать текстовое поле -->
		{
			Gdk::Rectangle visible_rect;
			Gtk::TextBuffer::iterator bottom_iter;

			this->get_visible_rect(visible_rect);
			this->get_iter_at_location(bottom_iter, 0, visible_rect.get_y() + visible_rect.get_height());

			if(bottom_iter.get_line() == this->buffer->end().get_line())
				is_scroll_needed = true;
		}
		// Определяем, нужно ли прокручивать текстовое поле <--

		// Вставляем текст -->
		{
			std::string inserting_text;

			for(size_t i = 0; i < this->messages.size(); i++)
			{
				if(i || this->buffer->begin() != this->buffer->end())
					inserting_text += "\n";

				inserting_text += this->messages[i];
			}
			this->messages.clear();

			this->buffer->insert(this->buffer->end(), inserting_text);
		}
		// Вставляем текст <--

		// Удаляем лишние строки
		this->remove_old_lines();

		// Прокручиваем текстовое поле, если это необходимо -->
			if(is_scroll_needed)
			{
				// С прокруткой постоянно возникают какие-то проблемы.
				// Какой бы способ не использовался, в большинстве случаев
				// все нормально прокручивается, но иногда происходят сбои.
				// Поэтому используем сразу 2 способа прокрутки.

				// Используем функции TextBuffer'а -->
				{
					Gtk::TextBuffer::iterator end_iter = this->buffer->end();
					end_iter.set_line_offset(0);

					this->buffer->move_mark(this->end_mark, end_iter);
				#if GTK_CHECK_VERSION(3, 0, 0)
					this->scroll_to(this->end_mark);
				#else
					this->scroll_mark_onscreen(this->end_mark);
				#endif
				}
				// Используем функции TextBuffer'а <--

				// Используем функции ScrolledWindow -->
				{
					Gtk::ScrolledWindow scrolled_window;

					// Если наш родитель - ScrolledWindow (так должно быть всегда,
					// но на всякий случай лучше перестраховаться).
					if(this->get_parent()->get_name() == scrolled_window.get_name())
					{
						Gtk::ScrolledWindow* parent_scrolled_window = dynamic_cast<Gtk::ScrolledWindow *>(this->get_parent());

						parent_scrolled_window->get_vadjustment()->set_value(
							parent_scrolled_window->get_vadjustment()->get_upper()
						);
					}
				}
				// Используем функции ScrolledWindow <--
			}
		// Прокручиваем текстовое поле, если это необходимо <--
	}
}

