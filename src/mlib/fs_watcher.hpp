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


#ifndef HEADER_MLIB_FS_WATCHER
#define HEADER_MLIB_FS_WATCHER

// Предоставляет класс, который может использоваться для наблюдения за
// какой-либо директорией на предмет появления в ней новых файлов.


#include <sigc++/connection.h>
#include <sigc++/slot.h>

#include <mlib/main.hpp>



namespace m
{
	/// Используется для наблюдения за какой-либо директорией на предмет
	/// появления в ней новых файлов.
	///
	/// Внимание! Объект не является thread-safe. Взаимодействовать с ним
	/// можно только из одного потока.
	class Fs_watcher
	{
		private:
			class Implementation;


		public:
			Fs_watcher(void);
			~Fs_watcher(void);


		private:
			Implementation*				impl;


		public:
			/// Очищает очередь новых файлов.
			void				clear(void);

			/// Подключает обработчик сигнала на появление нового файла в
			/// мониторящейся директории.
			sigc::connection	connect(const sigc::slot<void>& slot);

			/// Извлекает путь к файлу из очереди новых файлов.
			/// Если директория была перемещена или удалена, то в очередь
			/// добавляется пустой путь ("").
			///
			/// @return - true, если файл был извлечен, или false, если в
			/// очереди больше не осталось файлов.
			bool				get(std::string* file_path);

			/// Возвращает директорию, которая мониторится в данный момент, или
			/// "", если директория не мониторится.
			std::string			get_watching_directory(void);

			/// Задает директорию для мониторинга и очищает очередь новых файлов.
			/// @throw - m::Exception.
			void				set_watching_directory(const std::string& directory);

			/// Снимает текущую директорию с мониторинга (если такая существует).
			void				unset_watching_directory(void);
	};
}

#endif

