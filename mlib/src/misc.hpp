/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
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


#ifndef HEADER_MLIB_MISC
#define HEADER_MLIB_MISC

#include <semaphore.h>

#include <boost/noncopyable.hpp>

#include <mlib/main.hpp>

#include "misc.hxx"


namespace m {


class Buffer
{
	public:
		Buffer(void);
		Buffer(const Buffer& buffer);
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
		size_t		get_size(void) const { return this->size; }

		/// Смещает текущую позицию в буфере.
		Buffer &	increase_pos(const int inc);

		/// Загружает файл в буфер.
		/// Генерирует исключение, если файл имеет слишком большой размер.
		/// @throw - m::Sys_exception.
		void		load_file(const std::string &file_path);

		/// Увеличивает буфер, если это необходимо, на столько,
		/// чтобы в него могло поместиться еще reserve_size байт,
		/// начиная от текущей позиции.
		void		reserve(int reserve_size);

		/// Меняет текущий размер буфера.
		void		resize(int new_size);

		/// Устанавливает текущую позицию в буфере.
		Buffer &	set_pos(int new_pos);

		/// Записывает данные буфера в файл.
		/// @throw - m::Sys_exception.
		void		write_file(const std::string& file_path) const;


	public:
		Buffer &	operator=(const Buffer& buffer);
		Buffer &	operator=(int val);
		Buffer &	operator+(int val);
		Buffer &	operator+=(int val);
};



// Обеспечивает связь между двумя потоками посредством pipe'ов.
class Connection
{
	public:
		Connection(void);
		~Connection(void);


	private:
		int	read_fd;
		int write_fd;


	public:
		/// Очищает Connection так, чтобы последущий wait() не вернул
		/// управления до тех пор, пока кто-нибудь не сделает post().
		void	clear(void);

		/// Возвращает дескриптор для чтения.
		int		get_read_fd(void);

		/// Записывает в write_fd 1 байт, пробуждая тем самым другой поток.
		void	post(void);

		/// Блокирует выполнение текущего потока до тех пор, пока не появятся
		/// данные в fd или в this->read_fd. Если возвращает false, то это
		/// означает, то в this->read_fd появились данные и один байт из них был
		/// взят. Возвращает true, если данные появились в fd.
		///
		/// @param prioritize_fd - если true, то первым делом просматривает fd,
		/// и только потом this->read_fd. Таким образом, в случае prioritize_fd
		/// == true функция возвратит false только тогда, когда в this->read_fd
		/// что-то есть, а в fd нет ничего.
		bool	wait_for_with_owning(int fd, bool prioritize_fd = false);

		/// Ожидает прихода данных в данный объект и объект second, отдавая при
		/// этом приоритет данному объекту.
		/// @return - true, если данные пришли в этот объект, false - если
		/// данные пришли в объект second.
		bool	wait_two_with_owning(Connection& second);

	private:
		/// Пытается захватить байт из read_fd.
		/// @return - true, если удалось захватить.
		bool	get(void);
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



/// Используется для ожидания группы дескрипторов через функцию wait_fds.
class Wait_entry
{
	public:
		Wait_entry(int fd, bool own = false)
		: fd(fd), own(own) {}


	public:
		/// Дескриптор, прихода данных в который мы ожидаем.
		int		fd;

		/// Определяет, нужно ли считывать 1 байт, при появлении данных в этом
		/// дескрипторе (удобно при использовании в качестве дескриптора
		/// дескриптора m::Connection);
		bool	own;
};



/// Минимальный номер сетевого порта.
extern uint16_t PORT_MIN;

/// Максимальный номер сетевого порта.
extern uint16_t PORT_MAX;



/// Закрывает все файловые дескрипторы, оставляя только stdin, stdout и stderr.
/// @throw - m::Exception.
void				close_all_fds(void);

/// Возвращает строку копирайта.
/// @param start_year - год, в котором была написана программа.
std::string			get_copyright_string(const std::string& author, const int start_year);

/// Преобразует указатель в целое число.
template <class T>
size_t				pointer_to_integer(T* ptr);

/// Аналогична страндартному realloc, но в случае неудачной
/// попытки выделения памяти вызывает m::error.
void*				realloc(void *ptr, const size_t size);

/// Запускает приложение.
/// Внимание! Прежде чем вызывать данную функцию приложение должно позаботиться
/// о том, чтобы процесс не создавал процессов-зомби.
/// @throw - m::Exception.
void				run(const std::string& cmd_name, const std::vector<std::string>& args);

/// Ожидает поступления данных в дескрипторах entries и возвращает идентификатор
/// дескриптора, в котором появились данные.
///
/// Внимание!
/// Если в Wait_entry::own == true и доступ к Wait_entry::fd является
/// блокирующим, то результат работы функции не определен.
size_t				wait_fds(const Wait_entry* entries, size_t size);


}

#include "misc.hh"

#endif

