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


#ifndef HEADER_MLIB_ERRORS
	#define HEADER_MLIB_ERRORS

	// Предоставляет функции для получения строкового представления ошибки.


	#include <string>

	#include <cerrno>


#ifdef MLIB_ENABLE_DBUS
	namespace DBus
	{
		class Error;
	}
#endif

	namespace Glib
	{
		class Error;
	}

#ifdef MLIB_ENABLE_LIBCONFIG
	namespace libconfig
	{
		class FileIOException;
		class ParseException;
	}
#endif

#ifdef MLIB_ENABLE_LIBTORRENT
	namespace libtorrent
	{
		class duplicate_torrent;
		class invalid_encoding;
		class invalid_torrent_file;
	}
#endif

	namespace m
	{
		class Exception
		{
			public:
				Exception(const char* file, const int line, const char* error = "");
				Exception(const char* file, const int line, const std::string& error = "");
				//Exception(const char* file, const int line, const std::string& error_title, const std::string& error);


			private:
				//const std::string	title;
				const std::string	error;


			public:
				/// Возвращает заголовок ошибки. Используется в качестве
				/// заголовка окна при отображении графических сообщений на
				/// основе Exception. Может быть "".
				//const char*		get_title(void) const;

				const char*		what(void) const;
		};


		class Sys_exception: public Exception
		{
			public:
				Sys_exception(const char* file, const int line, int errno_val);
				Sys_exception(const char* file, const int line, int errno_val, const std::string& error);

			public:
				/// Значение errno, соответствующее данной ошибке.
				const int	errno_val;
		};


		/// Макрос для генерации исключения m::Exception.
		#define M_THROW(args...)		throw m::Exception(__FILE__, __LINE__, args)

		/// Макрос для генерации исключения m::Sys_exception.
		#define M_THROW_SYS(args...)	throw m::Sys_exception(__FILE__, __LINE__, args)

		/// Макрос для генерации исключения m::Exception программистом.
		//#define M_THROW_WITH_TITLE(args...)	throw m::Exception(__FILE__, __LINE__, args)

		/// Макрос для генерации исключения m::Exception программистом.
		#define M_THROW_EMPTY()			throw m::Exception(__FILE__, __LINE__, "")



		/// Контейнер для ошибок.
		class Errors_pool: public std::string
		{
			public:
				void	throw_if_exists(void) const throw(m::Exception);


			public:
				Errors_pool& operator+=(const char* error);
				Errors_pool& operator+=(const std::string& error);
				operator bool(void) const;
		};



		namespace error_string
		{
			std::string		get(void);
			std::string		get(int errno_value);

	#ifdef MLIB_ENABLE_DBUS
			/// Возвращаемая строка начинается с заглавной буквы и оканчивается
			/// точкой. Может содержать несколько предложений.
			std::string		get(const DBus::Error& error);
	#endif

			std::string		get(const Glib::Error& error);

	#ifdef MLIB_ENABLE_LIBCONFIG
			std::string		get(const libconfig::FileIOException& error);
			std::string		get(const libconfig::ParseException& error);
	#endif

	#ifdef MLIB_ENABLE_LIBTORRENT
			std::string		get(const ::libtorrent::duplicate_torrent& error);
			std::string		get(const ::libtorrent::invalid_encoding& error);
			std::string		get(const ::libtorrent::invalid_torrent_file& error);
	#endif
			std::string		get(const std::exception& error);
			std::string		get(const std::ifstream& ifstream);
			std::string		get(const std::ofstream& ofstream);

			std::string		get(const m::Exception& error);
			std::string		get(const m::Errors_pool& errors);
		}

		template<class T> inline
		std::string		EE(const T& error)
						{ return m::error_string::get(error); }
	}

	#ifdef MLIB_ENABLE_ALIASES
		using m::Errors_pool;
		using m::EE;
	#endif

#endif

