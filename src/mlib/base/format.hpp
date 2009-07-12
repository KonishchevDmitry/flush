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


// Предоставляет функции для форматирования строк и работы с gettext.

#ifndef HEADER_MLIB_BASE_FORMAT
#define HEADER_MLIB_BASE_FORMAT


#include <string>
#include <vector>

#include <glibmm/ustring.h>


/// Идентификатор кодировки UTF-8, используемый iconv и прочими
/// библиотеками.
#define MLIB_UTF_CHARSET_NAME	"UTF-8"


namespace m {


/// Список кодировок, доступных для перекодирования функцией convert(),
/// завершаемый элементом { NULL, NULL }.
extern struct Charset
{
	const char*	title;
	const char*	name;
} const AVAILABLE_CHARSETS[];

/// Порядковый номер кодировки UTF-8 в AVAILABLE_CHARSETS.
extern const size_t UTF_CHARSET_ID;


// В glibmm-2.16.4 замечено, что программа не компилируется, когда
// Glib::ustring::compose() в качестве одного из параметров передать const
// char* (кроме параметра со строкой форматирования). Это проиходит из-за того,
// что в шаблонах возникает неопределенность. В glibmm-2.18.1 такой проблемы
// уже нет.
// -->
	namespace Format_aux
	{
		M_LIBRARY_COMPATIBILITY

		template<class T> inline
		const T&		correct_glib_format_value(const T& value);

		inline
		Glib::ustring	correct_glib_format_value(const char* value);
	}
// <--


typedef std::vector<std::string> String_vector;


/// Обертка над gettext.
std::string		_(const char* string);

/// Обертка над Glib::ustring::compose(_(...)).
Glib::ustring	__(const char* fmt);

/// Обертка над Glib::ustring::compose(_(...), ...).
template<class T1> inline
Glib::ustring	__(const char* fmt, const T1& a1);

/// Обертка над Glib::ustring::compose(_(...), ...).
template<class T1, class T2> inline
Glib::ustring	__(const char* fmt, const T1& a1, const T2& a2);

/// Обертка над Glib::ustring::compose(_(...), ...).
template<class T1, class T2, class T3> inline
Glib::ustring	__(const char* fmt, const T1& a1, const T2& a2, const T3& a3);

/// Обертка над Glib::ustring::compose(_Q(...)).
Glib::ustring	__Q(const char* fmt);

/// Обертка над Glib::ustring::compose(_Q(...), ...).
template<class T1> inline
Glib::ustring	__Q(const char* fmt, const T1& a1);

/// Обертка над Glib::ustring::compose(_Q(...), ...).
template<class T1, class T2> inline
Glib::ustring	__Q(const char* fmt, const T1& a1, const T2& a2);

/// Обертка над Glib::ustring::compose(_Q(...), ...).
template<class T1, class T2, class T3> inline
Glib::ustring	__Q(const char* fmt, const T1& a1, const T2& a2, const T3& a3);

/// Обертка над Glib::ustring::compose.
template<class T1> inline
Glib::ustring	_C(const std::string& fmt, const T1& a1);

/// Обертка над Glib::ustring::compose.
template<class T1, class T2> inline
Glib::ustring	_C(const char* fmt, const T1& a1, const T2& a2);

/// Обертка над Glib::ustring::compose.
template<class T1, class T2, class T3> inline
Glib::ustring	_C(const char* fmt, const T1& a1, const T2& a2, const T3& a3);

/// Обертка над Glib::ustring::format.
template<class T1> inline
Glib::ustring	_F(const T1& a1);

/// Обертка над Glib::ustring::format.
template<class T1, class T2> inline
Glib::ustring	_F(const T1& a1, const T2& a2);

/// Обертка над Glib::ustring::format.
template<class T1, class T2, class T3> inline
Glib::ustring	_F(const T1& a1, const T2& a2, const T3& a3);

/// То же, что и _(), но удаляет из строки все до первого символа '|'
/// (включая этот символ).
std::string		_Q(const char* string);

/// Преобразовывает строку из одной кодировки в другую.
std::string		convert(const std::string& string, const std::string& to_charset, const std::string& from_charset = MLIB_UTF_CHARSET_NAME);

#ifdef MLIB_ENABLE_LIBTORRENT
	/// Возвращает имя кодировки, в которой libtorrent необходимо передавать имена файлов.
	std::string	get_libtorrent_files_charset(void);
#endif

/// Проверяет, известна ли данная кодировка механизму перекодировки строк.
bool			is_valid_encoding_name(const std::string& encoding);

/// Проверяет, является ли строка валидной UTF-8 строкой.
bool			is_valid_utf(const Glib::ustring& string);

/// Преобразовывает строку из кодировки локали в Unicode.
Glib::ustring	L2U(const std::string& string);

/// Преобразовывает любое значение в строку.
template<class Value> inline
std::string		to_string(const Value& value);

/// Преобразовывает строку из Unicode в кодировку локали.
std::string		U2L(const std::string& string);

/// Делает строку "валидной" UTF-8 строкой, если она таковой не
/// является.
Glib::ustring	validate_utf(const Glib::ustring& string);


#ifdef MLIB_ENABLE_LIBTORRENT
	// Функции, хранящие внутри себя информацию о том, в какой
	// кодировке мы кладем в libtorrent и впоследствии забираем строки,
	// содержащие пути к файлам и папкам.
	//
	// Они необходимы потому, что libtorrent не поддерживает не-UTF-8
	// локали.
	//
	// Вообще говоря, хранить их мы всегда будем в кодировке локали.
	// Сделаны же они на случай, если в libtorrent появится поддержка
	// не UTF-8 локалей.

	inline std::string LT2L(const std::string& string) { return string; }
	inline std::string LT2U(const std::string& string) { return L2U(string); }
	inline std::string U2LT(const std::string& string) { return U2L(string); }
	inline std::string L2LT(const std::string& string) { return string; }
#endif

}

#include "format.hh"

#ifdef MLIB_ENABLE_ALIASES
	using m::String_vector;

	using m::_;
	using m::_Q;

	using m::_F;
	using m::_C;

	using m::__;
	using m::__Q;

	using m::U2L;
	using m::L2U;

	#ifdef MLIB_ENABLE_LIBTORRENT
		using m::LT2L;
		using m::LT2U;
		using m::U2LT;
		using m::L2LT;
	#endif
#endif

#endif

