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


#ifndef HEADER_MLIB_MESSAGES
	#define HEADER_MLIB_MESSAGES

	#include <string>


	#ifdef MLIB_DEBUG_MODE
		#define MLIB_D(args...) ::m::debug(__FILE__, __LINE__, args)
	#else
		#define MLIB_D(args...)
	#endif

	/// Информационное сообщение.
	/// В GUI приложениях выводится на экран.
	#define MLIB_I(args...) ::m::info(__FILE__, __LINE__, args)

	/// Предупреждающее сообщение о какой-либо ошибке. В отличие от MLIB_W
	/// гораздо менее навязчивое - в GUI приложениях выводится только на
	/// консоль, а в консольных, например, может писаться в лог. Предназначено
	/// для индикации о каких-то внутренних ошибках, которые не должен видеть
	/// среднестатистический пользователь, но если заглянет в консоль, то увидит
	/// и сможет рассказать о них разарботчику.
	#define MLIB_SW(args...) ::m::silent_warning(__FILE__, __LINE__, args)

	/// Предупреждающее сообщение о какой-либо ошибке.
	/// В GUI приложениях выводится на экран.
	#define MLIB_W(args...) ::m::warning(__FILE__, __LINE__, args)

	#define MLIB_E(args...) ::m::error(__FILE__, __LINE__, args)
	#define MLIB_A(exp)		::m::assert_exp(__FILE__, __LINE__, exp)
	#define MLIB_LE()		::m::logical_error(__FILE__, __LINE__)


	namespace m {
		inline
		void	assert_exp(const char* file, const int line, bool expression);

	#ifdef MLIB_DEBUG_MODE
		void	debug(const char* file, const int line, const std::string& message);
	#endif

		void	error(const char* file, const int line, const std::string& message) __attribute__ ((__noreturn__));

		void	info(const char* file, const int line, const std::string& message);
		void	info(const char* file, const int line, const std::string& title, const std::string& message);

		void	logical_error(const char* file, const int line) __attribute__ ((__noreturn__));

		void	set_error_function(void (* function)(const char* file, const int line, const std::string& message));
		void	set_info_function(void (* function)(const char* file, const int line, const std::string& title, const std::string& message));
		void	set_silent_warning_function(void (* function)(const char* file, const int line, const std::string& message));
		void	set_warning_function(void (* function)(const char* file, const int line, const std::string& title, const std::string& message));

		void	silent_warning(const char* file, const int line, const std::string& message);

		void	warning(const char* file, const int line, const std::string& message);
		void	warning(const char* file, const int line, const std::string& title, const std::string& message);
	}

	#include "messages.hh"

#endif

