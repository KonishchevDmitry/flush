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


#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <sys/time.h>

#include <mlib/main.hpp>

#include "messages.hpp"


namespace m {

namespace
{
	void			error_function(const char* file, size_t line, const std::string& message);

	/// Возвращает строку с именем файла и номером строки в нем для помещения ее в лог.
	std::string		get_log_src_path_string(const char* file, int line);

	/// Возвращает строку с текущим временем для помещения ее в лог.
	std::string		get_log_time_string(void);

	void			info_function(const char* file, size_t line, const std::string& title, const std::string& message);
	void			silent_warning_function(const char* file, size_t line, const std::string& message);
	void			warning_function(const char* file, size_t line, const std::string& title, const std::string& message);



	void (* ERROR_FUNCTION)(const char* file, size_t line, const std::string& message) = error_function;
	void (* INFO_FUNCTION)(const char* file, size_t line, const std::string& title, const std::string& message) = info_function;
	void (* SILENT_WARNING_FUNCTION)(const char* file, size_t line, const std::string& message) = silent_warning_function;
	void (* WARNING_FUNCTION)(const char* file, size_t line, const std::string& title, const std::string& message) = warning_function;



	void error_function(const char* file, size_t line, const std::string& message)
	{
		std::cerr
			<< U2L(m::get_log_debug_prefix(file, line))
			<< "E: " << U2L(message) << std::endl;
		std::cerr.flush();

		abort();
	}



	std::string get_log_src_path_string(const char* file, int line)
	{
		// Внимание!
		// Функция не должна выводить какие-либо сообщения, и использовать Format,
		// т. к. они сами будут вызывать ее.

		const int max_file_path_string_len = 20;
		const int max_file_path_string_size = max_file_path_string_len + 1;

		// На всякий случай выделим по-больше - кто знает, как целое число
		// представляется в текущей локали...
		const int max_line_string_len = 4 * 2;
		const int max_line_string_size = max_line_string_len + 1;

		char file_buf[max_file_path_string_size];
		char line_buf[max_line_string_size ];

		int diff = strlen(file) - max_file_path_string_len;

		if(diff > 0)
			file += diff;

		snprintf(file_buf, max_file_path_string_size, "%*s", max_file_path_string_len, file);
		snprintf(line_buf, max_line_string_size, "%04d", line);

		return std::string(file_buf) + ":" + line_buf;
	}



	std::string get_log_time_string(void)
	{
		// Внимание!
		// Функция не должна выводить какие-либо сообщения, и использовать Format,
		// т. к. они сами будут вызывать ее.

		const int time_string_len = 8;
		const int max_string_size = time_string_len + 4 + 1;
		char time_string_buf[max_string_size] = { '\0' };
		struct timeval tv;
		struct tm tm_val;

		if(gettimeofday(&tv, NULL) < 0)
			return _("[[gettimeofday time getting error]]");

		strftime(time_string_buf, max_string_size, "%H:%M:%S", localtime_r(&tv.tv_sec, &tm_val));
		snprintf(
			time_string_buf + time_string_len,
			max_string_size - time_string_len,
			".%03d", int(tv.tv_usec / 1000)
		);

		return time_string_buf;
	}



	void info_function(const char* file, size_t line, const std::string& title, const std::string& message)
	{
		std::cout
			#ifdef MLIB_DEBUG_MODE
				<< U2L(m::get_log_debug_prefix(file, line))
			#endif
			<< "I: ";

		if(!title.empty())
			std::cout << "[" << U2L(title) << "] ";

		std::cout << U2L(message) << std::endl;

		std::cout.flush();
	}



	void silent_warning_function(const char* file, size_t line, const std::string& message)
	{
		std::cerr
			#ifdef MLIB_DEBUG_MODE
				<< U2L(m::get_log_debug_prefix(file, line))
			#endif
			<< "SW: " << U2L(message) << std::endl;

		std::cerr.flush();
	}



	void warning_function(const char* file, size_t line, const std::string& title, const std::string& message)
	{
		std::cerr
			#ifdef MLIB_DEBUG_MODE
				<< U2L(m::get_log_debug_prefix(file, line))
			#endif
			<< "W: ";

		if(!title.empty())
			std::cerr << "[" << U2L(title) << "] ";

		std::cerr << U2L(message) << std::endl;

		std::cerr.flush();
	}
}



#ifdef MLIB_DEBUG_MODE
	void debug(const char* file, size_t line, const std::string& message)
	{
		std::cout
			<< U2L(m::get_log_debug_prefix(file, line))
			<< "D: " << U2L(message) << std::endl;
		std::cout.flush();
	}
#endif



void error(const char* file, size_t line, const std::string& message)
{
	ERROR_FUNCTION(file, line, message);

	// Обманываем __attribute__ ((__noreturn__)) и
	// гарантируем, что функция не возвратит управление.
	abort();
}



std::string	get_log_debug_prefix(const char* file, size_t line)
{
	return "(" + get_log_time_string() + ") (" + get_log_src_path_string(file, line) + "): ";
}



void info(const char* file, size_t line, const std::string& message)
{
	info(file, line, "", message);
}



void info(const char* file, size_t line, const std::string& title, const std::string& message)
{
	INFO_FUNCTION(file, line, title, message);
}



void logical_error(const char* file, size_t line)
{
	error(file, line, _("Logical error"));
}



void set_error_function(void (* function)(const char* file, size_t line, const std::string& message))
{
	if(function)
		ERROR_FUNCTION = function;
	else
		ERROR_FUNCTION = error_function;
}



void set_info_function(void (* function)(const char* file, size_t line, const std::string& title, const std::string& message))
{
	if(function)
		INFO_FUNCTION = function;
	else
		INFO_FUNCTION = info_function;
}



void set_silent_warning_function(void (* function)(const char* file, size_t line, const std::string& message))
{
	if(function)
		SILENT_WARNING_FUNCTION = function;
	else
		SILENT_WARNING_FUNCTION = silent_warning_function;
}



void set_warning_function(void (* function)(const char* file, size_t line, const std::string& title, const std::string& message))
{
	if(function)
		WARNING_FUNCTION = function;
	else
		WARNING_FUNCTION = warning_function;
}



void silent_warning(const char* file, size_t line, const std::string& message)
{
	SILENT_WARNING_FUNCTION(file, line, message.c_str());
}



void warning(const char* file, size_t line, const std::string& message)
{
	WARNING_FUNCTION(file, line, "", message.c_str());
}



void warning(const char* file, size_t line, const std::string& title, const std::string& message)
{
	WARNING_FUNCTION(file, line, title, message);
}

}

