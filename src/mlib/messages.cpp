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


#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "messages.hpp"
#include "misc.hpp"
#include "string.hpp"


namespace m {

namespace
{
	void error_function(const char* file, const int line, const std::string& message);
	void info_function(const char* file, const int line, const std::string& title, const std::string& message);
	void silent_warning_function(const char* file, const int line, const std::string& message);
	void warning_function(const char* file, const int line, const std::string& title, const std::string& message);



	void (* ERROR_FUNCTION)(const char* file, const int line, const std::string& message) = error_function;
	void (* INFO_FUNCTION)(const char* file, const int line, const std::string& title, const std::string& message) = info_function;
	void (* SILENT_WARNING_FUNCTION)(const char* file, const int line, const std::string& message) = silent_warning_function;
	void (* WARNING_FUNCTION)(const char* file, const int line, const std::string& title, const std::string& message) = warning_function;



	void error_function(const char* file, const int line, const std::string& message)
	{
		std::cerr
			<< U2L(m::get_log_debug_prefix(file, line))
			<< "E: " << U2L(message) << std::endl;
		std::cerr.flush();

		abort();
	}



	void info_function(const char* file, const int line, const std::string& title, const std::string& message)
	{
		std::cout
			#ifdef DEBUG_MODE
				<< U2L(m::get_log_debug_prefix(file, line))
			#endif
			<< "I: ";

		if(!title.empty())
			std::cout << "[" << U2L(title) << "] ";

		std::cout << U2L(message) << std::endl;

		std::cout.flush();
	}



	void silent_warning_function(const char* file, const int line, const std::string& message)
	{
		std::cerr
			#ifdef DEBUG_MODE
				<< U2L(m::get_log_debug_prefix(file, line))
			#endif
			<< "SW: " << U2L(message) << std::endl;

		std::cerr.flush();
	}



	void warning_function(const char* file, const int line, const std::string& title, const std::string& message)
	{
		std::cerr
			#ifdef DEBUG_MODE
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
	void debug(const char* file, const int line, const std::string& message)
	{
		std::cout
			<< U2L(m::get_log_debug_prefix(file, line))
			<< "D: " << U2L(message) << std::endl;
		std::cout.flush();
	}
#endif



void error(const char* file, const int line, const std::string& message)
{
	ERROR_FUNCTION(file, line, message);

	// Обманываем __attribute__ ((__noreturn__)) и
	// гарантируем, что функция не возвратит управление.
	abort();
}



void info(const char* file, const int line, const std::string& message)
{
	info(file, line, "", message);
}



void info(const char* file, const int line, const std::string& title, const std::string& message)
{
	INFO_FUNCTION(file, line, title, message);
}



void logical_error(const char* file, const int line)
{
	error(file, line, _("Logical error"));
}



void set_error_function(void (* function)(const char* file, const int line, const std::string& message))
{
	if(function)
		ERROR_FUNCTION = function;
	else
		ERROR_FUNCTION = error_function;
}



void set_info_function(void (* function)(const char* file, const int line, const std::string& title, const std::string& message))
{
	if(function)
		INFO_FUNCTION = function;
	else
		INFO_FUNCTION = info_function;
}



void set_silent_warning_function(void (* function)(const char* file, const int line, const std::string& message))
{
	if(function)
		SILENT_WARNING_FUNCTION = function;
	else
		SILENT_WARNING_FUNCTION = silent_warning_function;
}



void set_warning_function(void (* function)(const char* file, const int line, const std::string& title, const std::string& message))
{
	if(function)
		WARNING_FUNCTION = function;
	else
		WARNING_FUNCTION = warning_function;
}



void silent_warning(const char* file, const int line, const std::string& message)
{
	SILENT_WARNING_FUNCTION(file, line, message.c_str());
}



void warning(const char* file, const int line, const std::string& message)
{
	WARNING_FUNCTION(file, line, "", message.c_str());
}



void warning(const char* file, const int line, const std::string& title, const std::string& message)
{
	WARNING_FUNCTION(file, line, title, message);
}

}

