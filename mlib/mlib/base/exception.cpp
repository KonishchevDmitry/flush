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


#include <cerrno>

#include <mlib/main.hpp>

#include "exception.hpp"


namespace m {


	std::string EE(void)
	{
		return std::strerror(errno);
	}



	std::string EE(const int* errno_value)
	{
		return std::strerror(*errno_value);
	}



	std::string EE(const Glib::Error& error)
	{
		return error.what();
	}



	std::string EE(const std::exception& error)
	{
		return error.what();
	}



	std::string EE(const std::ifstream& ifstream)
	{
		return EE();
	}



	std::string EE(const std::ofstream& ofstream)
	{
		return EE();
	}



	std::string EE(const m::Exception& error)
	{
		return error.what();
	}



	std::string EE(const m::Errors_pool& errors)
	{
		return errors;
	}




// Exception -->
	Exception::Exception(const char* file, size_t line, const std::string& error)
	:
		error(error)
	{
		#ifdef MLIB_DEBUG_MODE
			::m::debug(file, line, __("Exception: '%1'.", this->error));
		#endif
	}



	Exception::~Exception(void) throw()
	{
	}



	const char* Exception::what(void) const throw()
	{
		return this->error.c_str();
	}
// Exception <--



// Sys_exception -->
	Sys_exception::Sys_exception(const char* file, size_t line, int errno_val)
	:
		Exception(file, line, EE(&errno_val)),
		errno_val(errno_val)
	{
	}



	Sys_exception::Sys_exception(const char* file, size_t line, int errno_val, const std::string& error)
	:
		Exception(file, line, error),
		errno_val(errno_val)
	{
	}
// Sys_exception <--



// Errors_pool -->
	void Errors_pool::throw_if_exists(void) const
	{
		if(std::string::compare(""))
			M_THROW(*this);
	}



	Errors_pool& Errors_pool::operator+=(const char* error)
	{
		// Чтобы при "склеивании" нескольких Errors_pool'ов между ними не
		// образовывались пустые строки.
		if(*error != '\n')
			std::string::append("\n");

		std::string::append(error);

		return *this;
	}



	Errors_pool& Errors_pool::operator+=(const std::string& error)
	{
		return *this += error.c_str();
	}



	Errors_pool::operator bool(void) const
	{
		return std::string::compare("");
	}
// Errors_pool <--


}

