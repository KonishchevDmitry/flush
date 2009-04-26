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


#include <cerrno>

#include <boost/filesystem.hpp>

#include <glibmm/error.h>

#ifdef MLIB_ENABLE_DBUS
	#include <dbus-c++/error.h>
#endif

#ifdef MLIB_ENABLE_LIBCONFIG
	#include <libconfig.h++>
#endif

#ifdef MLIB_ENABLE_LIBTORRENT
	#include <libtorrent/bencode.hpp>
	#include <libtorrent/torrent_info.hpp>
#endif

#include "errors.hpp"
#include "messages.hpp"
#include "string.hpp"



namespace m { namespace error_string {


std::string get(void)
{
	return get(errno);
}



std::string get(int errno_value)
{
	return strerror(errno_value);
}



#ifdef MLIB_ENABLE_DBUS
	std::string get(const DBus::Error& error)
	{
		// DBus::Error содержит строку, в которой первая буква
		// заглавная, может быть несколько предложений
		// и вставляется перевод строки.

		std::string error_string = error.what();

		if(!error_string.empty() && error_string[error_string.size() - 1] == '\n')
			error_string = error_string.substr(0, error_string.size() - 1);

		return error_string;
	}
#endif



std::string get(const Glib::Error& error)
{
	return error.what();
}



#ifdef MLIB_ENABLE_LIBCONFIG
	std::string get(const libconfig::FileIOException& error)
	{
		return get(errno);
	}



	std::string get(const libconfig::ParseException& error)
	{
		libconfig::ParseException error_non_const_copy = error;
		return __("%1 at line %2", error_non_const_copy.getError(), error_non_const_copy.getLine());
	}
#endif



#ifdef MLIB_ENABLE_LIBTORRENT
	std::string get(const ::libtorrent::duplicate_torrent& error)
	{
		return _("torrent is already exists in this session");
	}



	std::string get(const ::libtorrent::invalid_encoding& error)
	{
		return _("bad torrent data");
	}



	std::string get(const ::libtorrent::invalid_torrent_file& error)
	{
		return _("this is not a torrent file");
	}
#endif



std::string get(const std::exception& error)
{
	return error.what();
}



std::string get(const std::ifstream& ifstream)
{
	return get(errno);
}



std::string get(const std::ofstream& ofstream)
{
	return get(errno);
}



std::string get(const m::Exception& error)
{
	return error.what();
}



std::string get(const m::Errors_pool& errors)
{
	return errors;
}

}}



namespace m {

// Exception -->
	Exception::Exception(const char* file, const int line, const char* error)
	:
		error(error)
	{
		#ifdef MLIB_DEBUG_MODE
			m::debug(file, line, __("Exception: '%1'.", error));
		#endif
	}

	Exception::Exception(const char* file, const int line, const std::string& error)
	:
		error(error)
	{
		#ifdef MLIB_DEBUG_MODE
			m::debug(file, line, __("Exception: '%1'.", error));
		#endif
	}



	//Exception::Exception(const char* file, const int line, const std::string& error_title, const std::string& error)
	//:
	//	title(title),
	//	error(error)
	//{
	//	#ifdef MLIB_DEBUG_MODE
	//		m::debug(file, line, __("Exception: [%1] '%2'.", title) % error);
	//	#endif
	//}



	//const char* Exception::get_title(void) const
	//{
	//	return this->title.c_str();
	//}



	const char* Exception::what(void) const
	{
		return this->error.c_str();
	}
// Exception <--



// Errors_pool -->
	void Errors_pool::throw_if_exists(void) const throw(m::Exception)
	{
		if(static_cast<const std::string>(*this) != "")
			M_THROW(static_cast<const std::string>(*this));
	}



	Errors_pool& Errors_pool::operator+=(const char* error)
	{
		// Чтобы при "склеивании" нескольких Errors_pool'ов между ними не
		// образовывались пустые строки.
		if(*error == '\n')
			*static_cast<std::string*>(this) += error;
		else
			*static_cast<std::string*>(this) += std::string("\n") + error;

		return *this;
	}



	Errors_pool& Errors_pool::operator+=(const std::string& error)
	{
		return *this += error.c_str();
	}



	Errors_pool::operator bool(void) const
	{
		return static_cast<const std::string>(*this) != "";
	}
// Errors_pool <--

}

