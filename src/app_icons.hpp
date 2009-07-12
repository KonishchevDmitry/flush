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


#ifndef HEADER_APP_ICONS
#define HEADER_APP_ICONS

#ifndef MLIB_ENABLE_LIBS_FORWARDS
	#include <gdkmm/pixbuf.h>

	#include <gtkmm/action.h>
	#include <gtkmm/toolbutton.h>
#endif

#include <gtkmm/enums.h>

#include "common.hpp"


namespace app_icons
{

enum Flags
{
	/// Торрент приостановлен.
	ICON_FLAG_PAUSE			= 1 << 10,

	/// Не удалось установить соединение с трекером.
	ICON_FLAG_TRACKER_ERROR	= 1 << 11,
};


enum Id
{
	/// Скачивание.
	ICON_DOWNLOAD,

	/// Скачивание и раздача.
	ICON_DOWNLOAD_AND_UPLOAD,

	/// Статистика торрента/сессии.
	ICON_STATISTICS,

	/// Выделяется место на диске для торрента.
	ICON_TORRENT_ALLOCATING,

	/// Данные торрента проверяются или стоят в очереди на проверку.
	ICON_TORRENT_CHECKING,

	/// Торрент скачивается (данные идут).
	ICON_TORRENT_DOWNLOADING,

	/// Торрент раздается (данные не идут).
	ICON_TORRENT_SEEDING,

	/// Торрент раздается (данные идут).
	ICON_TORRENT_UPLOADING,

	/// Торрент скачивается (данные не идут).
	ICON_TORRENT_WAITING_FOR_DOWNLOAD,

	/// Все торренты.
	ICON_TORRENTS_ALL,

	/// Торренты, которые проверяются или стоят в очереди на проверку.
	ICON_TORRENTS_CHECKING,

	/// Скачивающиеся торренты (данные идут).
	ICON_TORRENTS_DOWNLOADING,

	/// Приостановленные торренты.
	ICON_TORRENTS_PAUSED,

	/// Раздающиеся торренты (данные не идут).
	ICON_TORRENTS_SEEDING,

	/// Торренты, с ошибками трекера.
	ICON_TORRENTS_TRACKER_ERROR,

	/// Раздающиеся торренты (данные идут).
	ICON_TORRENTS_UPLOADING,

	/// Скачивающиеся торренты (данные не идут).
	ICON_TORRENTS_WAITING_FOR_DOWNLOAD,

	/// Раздача.
	ICON_UPLOAD,

	/// Выделяется место на диске для торрента.
	/// + На паузе.
	ICON_TORRENT_ALLOCATING_PAUSED = ICON_TORRENT_ALLOCATING | ICON_FLAG_PAUSE,

	/// Выделяется место на диске для торрента.
	/// + Ошибка трекера.
	ICON_TORRENT_ALLOCATING_WITH_BROKEN_TRACKER = ICON_TORRENT_ALLOCATING | ICON_FLAG_TRACKER_ERROR,

	/// Данные торрента проверяются или стоят в очереди на проверку.
	/// + На паузе.
	ICON_TORRENT_CHECKING_PAUSED = ICON_TORRENT_CHECKING | ICON_FLAG_PAUSE,

	/// Данные торрента проверяются или стоят в очереди на проверку.
	/// + Ошибка трекера.
	ICON_TORRENT_CHECKING_WITH_BROKEN_TRACKER = ICON_TORRENT_CHECKING | ICON_FLAG_TRACKER_ERROR,

	/// Торрент скачивается (данные идут).
	/// + На паузе.
	ICON_TORRENT_DOWNLOADING_PAUSED = ICON_TORRENT_DOWNLOADING | ICON_FLAG_PAUSE,

	/// Торрент скачивается (данные идут).
	/// + Ошибка трекера.
	ICON_TORRENT_DOWNLOADING_WITH_BROKEN_TRACKER = ICON_TORRENT_DOWNLOADING | ICON_FLAG_TRACKER_ERROR,

	/// Торрент раздается (данные не идут).
	/// + На паузе.
	ICON_TORRENT_SEEDING_PAUSED = ICON_TORRENT_SEEDING | ICON_FLAG_PAUSE,

	/// Торрент раздается (данные не идут).
	/// + Ошибка трекера.
	ICON_TORRENT_SEEDING_WITH_BROKEN_TRACKER = ICON_TORRENT_SEEDING | ICON_FLAG_TRACKER_ERROR,

	/// Торрент раздается (данные идут).
	/// + На паузе.
	ICON_TORRENT_UPLOADING_PAUSED = ICON_TORRENT_UPLOADING | ICON_FLAG_PAUSE,

	/// Торрент раздается (данные идут).
	/// + Ошибка трекера.
	ICON_TORRENT_UPLOADING_WITH_BROKEN_TRACKER = ICON_TORRENT_UPLOADING | ICON_FLAG_TRACKER_ERROR,

	/// Торрент скачивается (данные не идут).
	/// + На паузе.
	ICON_TORRENT_WAITING_FOR_DOWNLOAD_PAUSED = ICON_TORRENT_WAITING_FOR_DOWNLOAD | ICON_FLAG_PAUSE,

	/// Торрент скачивается (данные не идут).
	/// + Ошибка трекера.
	ICON_TORRENT_WAITING_FOR_DOWNLOAD_WITH_BROKEN_TRACKER = ICON_TORRENT_WAITING_FOR_DOWNLOAD | ICON_FLAG_TRACKER_ERROR,
};



/// Используется только для того, чтобы в программе можно было легко найти код,
/// в котором явно прописано имя иконки.
const char*					app_icon(const char* icon_name);

/// Создает Gtk::Action с иконкой id.
Glib::RefPtr<Gtk::Action>	create_action(const Glib::ustring& name, Id id, const Glib::ustring& label = Glib::ustring(), const Glib::ustring& tooltip = Glib::ustring());

/// Возвращает иконку по ее идентификатору или иконку несуществующего
/// изображения, если иконку найти не удалось.
Glib::RefPtr<Gdk::Pixbuf>	get_pixbuf(Id id, const Gtk::IconSize& size);

/// Устанавливает иконку для Gtk::ToolButton.
void						set_for_tool_button(Gtk::ToolButton& button, Id id);

}

#endif

