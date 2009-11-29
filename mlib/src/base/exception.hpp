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


// Предоставляет функции и классы для работы с исключениями.

#ifndef HEADER_MLIB_BASE_EXCEPTION
#define HEADER_MLIB_BASE_EXCEPTION

#include <exception>
#include <string>
#include <iosfwd>

#include <glibmm/error.h>


/// Макрос для генерации исключения m::Exception.
#define M_THROW(args...)		throw m::Exception(__FILE__, __LINE__, args)

/// Макрос для генерации пустого исключения m::Exception.
#define M_THROW_EMPTY()			throw m::Exception(__FILE__, __LINE__, "")

/// Макрос для генерации исключения m::Sys_exception.
#define M_THROW_SYS(args...)	throw m::Sys_exception(__FILE__, __LINE__, args)


namespace m
{
	/// Стандартное для MLib исключение. Используется практически везде.
	class Exception: public std::exception
	{
		public:
			explicit Exception(const char* file, size_t line, const std::string& error);
			~Exception(void) throw();


		private:
			const std::string	error;


		public:
			const char*		what(void) const throw();
	};


	/// Исключение, предназначенное для передачи системных ошибок (значений
	/// errno).
	class Sys_exception: public Exception
	{
		public:
			explicit Sys_exception(const char* file, size_t line, int errno_val);
			explicit Sys_exception(const char* file, size_t line, int errno_val, const std::string& error);

		public:
			/// Значение errno, соответствующее данной ошибке.
			const int	errno_val;
	};


	/// Контейнер для накапливания ошибок.
	class Errors_pool: public std::string
	{
		public:
			/// @throw - m::Exception.
			void	throw_if_exists(void) const;


		public:
			Errors_pool& operator+=(const char* error);
			Errors_pool& operator+=(const std::string& error);
			operator bool(void) const;
	};


	// Возвращают строковое представление ошибки по переданному исключению.
	//
	// Каждый заголовочный файл может перегружать эту функцию, добавляя
	// поддрежку собственных исключений.
	//
	// Внимание!
	// Запрещается создавать EE(int) и EE(std::exception&), т. к. если
	// программист забудет подключить нужный заголовочный файл, то есть
	// вероятность того, что это не вызовет ошибку, и будет вызвана функция
	// EE(std::exception&), если исключение наследуется от std::exception (что
	// бывает очень часто), или будет вызывана EE(int), если, к примеру,
	// исключение имеет оператор преобразования в bool.
	// -->

		/// Возвращает ошибку, соответствующую текущему errno.
		std::string		EE(void);
		std::string		EE(const int* errno_value);

		std::string		EE(const Glib::Error& error);

		std::string		EE(const std::exception& error);
		std::string		EE(const std::ifstream& ifstream);
		std::string		EE(const std::ofstream& ofstream);

		std::string		EE(const m::Exception& error);
		std::string		EE(const m::Errors_pool& errors);

	// <--


	namespace aliases
	{
		using m::Errors_pool;
		using m::EE;
	}
}

#endif

