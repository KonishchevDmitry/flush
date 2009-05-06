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


#ifndef HEADER_MLIB_STRING
	#define HEADER_MLIB_STRING

	#include <stdint.h>

	#include <climits>
	#include <cstring>
	#include <sstream>

	#ifdef ENABLE_NLS
		#include <libintl.h>
	#endif

	#include <boost/lexical_cast.hpp>

	#ifdef MLIB_ENABLE_FORMAT
		#include <boost/format.hpp>
	#endif

	#include <glibmm/convert.h>

	#include "errors.hpp"
	#include "messages.hpp"
	#include "types.hpp"


	#ifdef MLIB_ENABLE_FORMAT
		#define MLIB_FORMAT(args...) m::Format(__FILE__, __LINE__, args)
		#define MLIB_GETTEXT_FORMAT(string) m::gettext_format(__FILE__, __LINE__, string)

		#ifdef MLIB_ENABLE_ALIASES
			#define FORMAT MLIB_FORMAT
			#define _F MLIB_FORMAT
			#define __ MLIB_GETTEXT_FORMAT
		#endif
	#endif


	namespace m
	{
		#ifdef MLIB_ENABLE_FORMAT
			/// Обертка над boost::format.
			///
			/// Уже не используется, но оставлен и отрезан макросами, т. к.
			/// жалко удалять - вдруг приходится. :)
			///
			/// TODO: очень медленная реализация. Годится только в
			/// том случае, если не часто ее использовать.
			class Format: public boost::format
			{
				public:
					/// @param unicode_output - при преобразовании в строку текст будет преобразован в Unicode.
					/// @param block_exceptions - если строка неверного формата, то блокирует генерируемое boost'ом исключение.
					inline
					Format(const char* file, const int line, const std::string& format_string, bool unicode_output = false, bool block_exceptions = false);

					/// @param unicode_output - при преобразовании в строку текст будет преобразован в Unicode.
					/// @param block_exceptions - если строка неверного формата, то блокирует генерируемое boost'ом исключение.
					inline
					Format(const char* file, const int line, const boost::format& format, bool unicode_output = false, bool block_exceptions = false);


				private:
					/// Файл, в котором был создан объект.
					const char*	file;

					/// Строка в файле, в котором был создан объект.
					const int			line;

					/// Определяет, преобразовывать ли в строку в UTF-8.
					bool				unicode_output;

					/// Определяет, блокировать ли генерируемые boost'ом исключения.
					bool				block_exceptions;


				public:
					inline
					std::string		str(void) const;


				public:
					template<class T> inline
					Format operator%(const T& value) const;

					inline
					operator std::string(void) const;

					inline
					operator Glib::ustring(void) const;
			};
		#endif



		/// Обертка над gettext.
		inline
		std::string		_(const char* string);

	#ifndef MLIB_ENABLE_FORMAT
		/// Обертка над Glib::ustring::compose(_(...)).
		inline
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
		inline
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
	#endif

		/// То же, что и _(), но удаляет из строки все до первого символа '|'
		/// (включая этот символ).
		std::string		_Q(const char* string);

		/// Возвращает строковое представление периода времени вида
		/// 1d 5h 50m.
		std::string		get_time_duration_string(time_t time);

		// Возвращает строку, представляющую оставшееся время в удобном для
		// восприятия формате.
		//
		// Строка может содержать символ бесконечности, которого может не
		// оказаться в текущей локали.
		//
		// Кроме этого имеет некоторое максимальное значение time_left, при
		// превышении которого оставшееся время обозначается символом
		// бесконечности.
		Glib::ustring	get_time_left_string(Time time_left);

	#ifdef MLIB_ENABLE_FORMAT
		/// Обертка над gettext и Format.
		/// Создает объект Format, которому передается
		/// строка, "пропущенная" через gettext.
		inline
		Format			gettext_format(const char* file, const int line, const char* string);
	#endif

		/// Проверяет, одинаковы ли строки.
		inline
		bool			is_eq(const char* a, const char* b);

		/// Проверяет, пустая ли строка (содержит ли что-нибудь кроме
		/// пробельных символов).
		bool			is_empty_string(const Glib::ustring& string);

		/// Проверяет, является ли строка валидной UTF-8 строкой.
		bool			is_valid_utf(const Glib::ustring& string);

		/// Преобразовывает строку из кодировки локали в Unicode.
		Glib::ustring	L2U(const std::string& string);

		/// Возвращает строковое представление размера
		std::string		size_to_string(Size size);

		/// Возвращает строковое представление скорости передачи данных
		std::string		speed_to_string(Speed speed);

		/// Возвращает строковое представление времени в формате "%H:%M:%S %d.%m.%Y".
		std::string		time_to_string_with_date(Time time);

		/// Преобразовывает любое значение в строку.
		template<class Value> inline
		std::string		to_string(const Value& value);

		/// Удаляет из строки начальные и конечные пробельные символы.
		Glib::ustring	trim(const Glib::ustring& string);

		/// Делает первую букву строки заглавной.
		Glib::ustring	uppercase_first(const Glib::ustring& string);

		/// Преобразовывает строку из Unicode в кодировку локали.
		inline
		std::string		U2L(const char* string);

		/// Преобразовывает строку из Unicode в кодировку локали.
		std::string		U2L(const std::string& string);

		/// Преобразовывает строку из Unicode в кодировку локали.
		inline
		std::string		U2L(const Glib::ustring& string);
	}

	#include "string.hh"

	#ifdef MLIB_ENABLE_ALIASES
		using m::_;
		using m::_Q;

		#ifdef MLIB_ENABLE_FORMAT
			using m::Format;
			using m::_F;
		#else
			using m::__;
			using m::__Q;
			using m::_F;
			using m::_C;
		#endif

		using m::U2L;
		using m::L2U;

		#ifdef MLIB_ENABLE_LIBTORRENT
			// Макросы, хранящие внутри себя информацию о том, в какой
			// кодировке мы кладем в libtorrent и впоследствии забираем строки,
			// содержащие пути к файлам и папкам.
			//
			// Они необходимы потому, что libtorrent не поддерживает не-UTF-8
			// локали.
			//
			// Вообще говоря, хранить их мы всегда будем в кодировке локали.
			// Сделаны же они на случай, если в libtorrent появится поддержка
			// не UTF-8 локалей.

			#define LT2L(x) (x)
			#define LT2U(x) L2U(x)
			#define U2LT(x) U2L(x)
			#define L2LT(x) (x)
		#endif
	#endif

#endif

