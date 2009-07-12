/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
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


// Предоставляет функции для выполнения ассинхронных операций
// с файловой системой, таких как копирование, удаление файлов и т. п.
//
// Внимание! Запрещается вызывать данные функции из конструкторов/деструкторов
// объектов, имеющих тип памяти static.
//
// Если объявлено макроопределение MLIB_ASYNC_FS_GLIB_SIGNALS, то по завершении
// каждой задачи генерирует сигнал.

#ifndef HEADER_MLIB_ASYNC_FS
#define HEADER_MLIB_ASYNC_FS

#ifndef MLIB_ENABLE_LIBS_FORWARDS
	#include <glibmm/dispatcher.h>
#endif

#include <mlib/main.hpp>


namespace m { namespace async_fs {

/// Уникальный идентификатор задачи async_fs.
typedef int Task_id;

/// Функции принимают параметр group, который служит для определения, связаны
/// ли задачи между собой. Если группы двух задач не связаны, то считается, что
/// эти задачи могут выполняться одновременно.

/// Аналогична m::fs::copy_files.
/// @param error_string - строка, которая будет предварять сообщение об ошибке,
/// если копирование завершится неудачей.
/// @return - идентификатор созданной задачи.
Task_id							copy_files(const std::string& group, const std::string& src_root, const std::string& dest_root, const std::vector<std::string>& files, const std::string& error_title, const std::string& error_string);

/// Асинхронно копирует файл или дерево каталогов from в to.
/// @param error_string - строка, которая будет предварять сообщение об ошибке,
/// если копирование завершится неудачей.
/// @return - идентификатор созданной задачи.
Task_id							cp(const std::string& group, const std::string& from, const std::string& to, const std::string& error_title, const std::string& error_string);

/// Возвращает сингнал, генерируемый при приостановке асинхронной файловой
/// системы, к которому можно привязать обработчик.
Glib::Dispatcher&				get_paused_signal(void);

/// Посылает запрос на приостановку работы асинхронной файловой системы.
/// Как только система будет приостановлена, сгенерируется сигнал.
/// @return - true, если в данный момент выполняется задание для группы group.
bool							pause_and_get_group_status(const std::string& group);

/// Снимает систему с паузы.
/// @return - true, если есть еще задачи и запустился поток для их обработки.
bool							resume(void);

/// Асинхронно удаляет файл или дерево каталогов.
/// @param error_string - строка, которая будет предварять сообщение об ошибке,
/// если копирование завершится неудачей.
/// @return - идентификатор созданной задачи.
Task_id							rm(const std::string& group, const std::string& path, const std::string& error_title, const std::string& error_string);

/// Аналогична m::fs::rm_files_with_empty_dirs.
/// @param error_string - строка, которая будет предварять сообщение об ошибке,
/// если копирование завершится неудачей.
/// @return - идентификатор созданной задачи.
Task_id							rm_files_with_empty_dirs(const std::string& group, const std::string& root, const std::vector<std::string>& files, const std::string& error_title, const std::string& error_string);

/// Асинхронно удаляет файл или дерево каталогов, если он (оно) существует.
/// @param error_string - строка, которая будет предварять сообщение об ошибке,
/// если копирование завершится неудачей.
/// @return - идентификатор созданной задачи.
Task_id							rm_if_exists(const std::string& group, const std::string& path, const std::string& error_title, const std::string& error_string);

#ifdef MLIB_ASYNC_FS_GLIB_SIGNALS
/// Возвращает сигнал, генерируемый при завершении задачи, к которому можно
/// привязать обработчик.
sigc::signal<void, Task_id>&	signal(void);
#endif

}}

#endif

