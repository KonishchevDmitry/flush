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


#ifndef HEADER_MLIB_MISC
#define HEADER_MLIB_MISC

#include <semaphore.h>

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>

#include "errors.hpp"
#include "types.hpp"


/// Макроопределение для обхода контейнеров.
/// Сделано для того, чтобы избежать написания очень длинных
/// for инcтрукций.
#define M_FOR_IT(type, container, iter) for(type::iterator iter = (container).begin(); iter != (container).end(); iter++)

/// Макроопределение для обхода контейнеров.
/// Сделано для того, чтобы избежать написания очень длинных
/// for инcтрукций.
#define M_FOR_REVERSE_IT(type, container, iter) for(type::reverse_iterator iter = (container).rbegin(); iter != (container).rend(); iter++)

/// Макроопределение для обхода контейнеров.
/// Сделано для того, чтобы избежать написания очень длинных
/// for инcтрукций.
#define M_FOR_CONST_IT(type, container, iter) for(type::const_iterator iter = (container).begin(); iter != (container).end(); iter++)

/// Макроопределение для обхода контейнеров.
/// Сделано для того, чтобы избежать написания очень длинных
/// for инcтрукций.
#define M_FOR_CONST_REVERSE_IT(type, container, iter) for(type::const_reverse_iterator iter = (container).rbegin(); iter != (container).rend(); iter++)

/// Необходима только для работы с препроцессором. Там, где номер
/// версии используется не в условиях препроцессора, лучше использовать
/// m::get_version().
#define M_GET_VERSION(major, minor, sub_minor) ( (major) * 1000000 + (minor) * 1000 + (sub_minor) )

/// Необходима только для работы с препроцессором. Там, где номер
/// версии используется не в условиях препроцессора, лучше использовать
/// m::get_major_minor_version().
#define M_GET_MAJOR_MINOR_VERSION(version) ( (version) / 1000 * 1000 )

#ifdef MLIB_ENABLE_LIBTORRENT
	#include <libtorrent/version.hpp>

	/// Необходима только для работы с препроцессором. Там, где номер
	/// версии используется не в условиях препроцессора, лучше использовать
	/// m::libtorrent::get_version().
	#define M_LT_GET_VERSION() ( (LIBTORRENT_VERSION_MAJOR) * 1000000 + (LIBTORRENT_VERSION_MINOR) * 1000 )
	#define M_LT_GET_MAJOR_MINOR_VERSION() ( (LIBTORRENT_VERSION_MAJOR) * 1000000 + (LIBTORRENT_VERSION_MINOR) * 1000 )
#endif



namespace m {

/// Минимальный номер сетевого порта.
extern uint16_t PORT_MIN;

/// Максимальный номер сетевого порта.
extern uint16_t PORT_MAX;


class Buffer
{
	public:
		Buffer(void);
		~Buffer(void);


	private:
		char *		buf;
		size_t		size;
		size_t		pos;


	public:
		/// Возвращает указатель на текущую позицию в буфере.
		char *		get_cur_ptr(void) const;

		/// Возвращает указатель на начало данных буфера.
		char *		get_data(void) const;

		/// Возвращает текущий размер буфера.
		inline
		size_t		get_size(void) const;

		/// Смещает текущую позицию в буфере.
		Buffer &	increase_pos(const int inc);

		/// Загружает файл в буфер.
		/// Генерирует m::Exception, если файл имеет слишком
		/// большой размер.
		void		load_file(const std::string &file_path) throw(m::Exception);

		/// Увеличивает буфер, если это необходимо, на столько,
		/// чтобы в него могло поместиться еще reserve_size байт,
		/// начиная от текущей позиции.
		void		reserve(int reserve_size);

		/// Меняет текущий размер буфера.
		void		resize(int new_size);

		/// Устанавливает текущую позицию в буфере.
		Buffer &	set_pos(int new_pos);


	public:
		Buffer &	operator=(int val);
		Buffer &	operator+(int val);
		Buffer &	operator+=(int val);
};



/// Обертка над UNIX семафором.
class Semaphore: public boost::noncopyable
{
	public:
		Semaphore(void);
		~Semaphore(void);


	private:
		sem_t	semaphore;


	public:
		void	post(void);
		void	wait(void);
};



/// Возвращает строку копирайта.
/// @param start_year - год, в котором была написана программа.
std::string	get_copyright_string(const std::string& author, const int start_year);

/// Возвращает строку-префикс для сообщений, которые пишутся в лог.
std::string	get_log_debug_prefix(const char* file, int line);

/// Возвращает major версию.
inline
int			get_major_version(Version version);

/// Возвращает версию, включающую в себя major и minor версии,
/// т. е., иными словами, это та же version, но с обнуленной
/// sub-minor версией.
inline
Version		get_major_minor_version(Version version);

/// Возвращает minor версию.
inline
int			get_minor_version(Version version);

/// Возвращает sub-minor версию.
inline
int			get_sub_minor_version(Version version);

/// Создает числовое представление версии.
inline
Version		get_version(int major, int minor, int sub_minor);

/// Проверяет, является ли версия правильно сформированной.
inline
bool		is_valid_version(Version version);

/// Аналогична страндартному realloc, но в случае неудачной
/// попытки выделения памяти вызывает m::error.
void*		realloc(void *ptr, const size_t size);

/// Запускает приложение.
/// Внимание! Прежде чем вызывать данную функцию приложение должно позаботиться
/// о том, чтобы процесс не создавал процессов-зомби.
void		run(const std::string& cmd_name, const std::vector<std::string>& args) throw(m::Exception);

/// Обертка над setenv.
void		setenv(const std::string& name, const std::string& value, bool overwrite) throw(m::Exception);

/// Преобразовывает время в структуре tm в реальное время
/// (они различаются по годам).
inline
void		tm_to_real_time(struct tm* date);

}

#include "misc.hh"

#endif


