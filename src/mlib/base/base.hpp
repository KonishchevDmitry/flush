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


// Предоставляет базовые макросы и функции.

#ifndef HEADER_MLIB_BASE_BASE
#define HEADER_MLIB_BASE_BASE

#include <string>

#include <boost/foreach.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/version.hpp>


/// Этим символом помечается тот код, который написан для совместимости с
/// предыдущими версиями библиотек, чтобы его было легко найти.
#define M_LIBRARY_COMPATIBILITY


/// Возвращает тип переменной. Удобен в тех случаях, когда необходимо, к
/// примеру, получить тип итератора по контейнеру:
/// std::vector<int> vec;
/// M_TYPEOF(vec)::iterator vec_iter;
#define M_TYPEOF(variable) m::Get_type<BOOST_TYPEOF(variable)>::type

/// Возвращает тип итератора для контейнера container.
/// std::vector<int> vec;
/// M_ITER_TYPE(vec) vec_iter;
#define M_ITER_TYPE(container) M_TYPEOF(container)::iterator

/// Возвращает тип константного итератора для контейнера container.
/// std::vector<int> vec;
/// M_CONST_ITER_TYPE(vec) vec_iter;
#define M_CONST_ITER_TYPE(container) M_TYPEOF(container)::const_iterator

/// Подсчитывает размер статического массива.
#define M_STATIC_ARRAY_SIZE(array) ( sizeof array / sizeof *array )


// Макроопределения для обхода контейнеров.
// Сделаны для того, чтобы избежать написания очень длинных
// for инcтрукций.
// -->
	#define M_FOR_IT(container, iter) \
		for( \
			boost::remove_const<BOOST_TYPEOF(container)>::type::iterator iter = (container).begin(), \
			mlib_end_iter = (container).end(); \
			iter != mlib_end_iter; ++iter \
		)

	#define M_FOR_CONST_IT(container, iter) \
		for( \
			boost::add_const<BOOST_TYPEOF(container)>::type::const_iterator iter = (container).begin(), \
			mlib_end_iter = (container).end(); \
			iter != mlib_end_iter; ++iter \
		)

	#define M_FOR_REVERSE_IT(container, iter) \
		for( \
			boost::remove_const<BOOST_TYPEOF(container)>::type::reverse_iterator iter = (container).rbegin(), \
			mlib_end_iter = (container).rend(); \
			iter != mlib_end_iter; ++iter \
		)

	#define M_FOR_CONST_REVERSE_IT(container, iter) \
		for( \
			boost::add_const<BOOST_TYPEOF(container)>::type::const_reverse_iterator iter = (container).rbegin(), \
			mlib_end_iter = (container).rend(); \
			iter != mlib_end_iter; ++iter \
		)
// <--


/// Необходима только для работы с препроцессором. Там, где номер
/// версии используется не в условиях препроцессора, лучше использовать
/// m::get_version().
#define M_GET_VERSION(major, minor, sub_minor) ( (major) * 1000000 + (minor) * 1000 + (sub_minor) )

/// Необходима только для работы с препроцессором. Там, где номер
/// версии используется не в условиях препроцессора, лучше использовать
/// m::get_major_minor_version().
#define M_GET_MAJOR_MINOR_VERSION(version) ( (version) / 1000 * 1000 )

/// Возвращает версию boost.
#define M_BOOST_GET_VERSION() M_GET_VERSION(BOOST_VERSION / 100000, BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100)

#ifdef MLIB_ENABLE_LIBTORRENT
	#include <libtorrent/version.hpp>

	// До 0.14.3 LIBTORRENT_VERSION_TINY не существовало
	#if LIBTORRENT_VERSION_MAJOR == 0 && LIBTORRENT_VERSION_MINOR <= 14
		#ifndef LIBTORRENT_VERSION_TINY
			#define LIBTORRENT_VERSION_TINY 0
		#endif
	#endif

	/// Необходима только для работы с препроцессором. Там, где номер
	/// версии используется не в условиях препроцессора, лучше использовать
	/// m::libtorrent::get_version().
	#define M_LT_GET_VERSION() ( (LIBTORRENT_VERSION_MAJOR) * 1000000 + (LIBTORRENT_VERSION_MINOR) * 1000 + (LIBTORRENT_VERSION_TINY) )
	#define M_LT_GET_MAJOR_MINOR_VERSION() ( (LIBTORRENT_VERSION_MAJOR) * 1000000 + (LIBTORRENT_VERSION_MINOR) * 1000 )
#endif


namespace m {


/// Предназначен для получения типа (метапрограммирование).
template<class T>
struct Get_type
{
	typedef T type;
};



/// Класс, от которого можно наследоваться при создании виртуальных
/// классов, чтобы не создавать для каждого из них виртуальный деструктор.
class Virtual
{
	public:
		virtual ~Virtual(void) {}
};



// Типы -->
	typedef long long	Size;
	typedef long double	Size_float;
	typedef int32_t		Speed;

	/// К Time предъявляется следующее требование.
	/// Размер Time должен быть не меньше Size, чтобы
	/// вместить время в секундах, необходимое для того,
	/// чтобы скачать какой-либо файл. Т. е. для наихудшего
	/// случая, когда размер файла - максимальное значение
	/// Size, а скорость - 1 байт/сек.
	typedef long long	Time;

	/// Время в милисекундах.
	typedef long long	Time_ms;

	// Числовое представление версии приложения/библиотеки.
	// К примеру версия 1.12.4 должна записываться следующим
	// образом: 1012004.
	// Для извлечения из этого числа отдельных версий
	// (минорной, мажорной и т. п.) предназначены специальные
	// функции.
	typedef int32_t			Version;
	extern const Version	NO_VERSION;
// Типы <--



/// Возвращает major версию.
int				get_major_version(Version version);

/// Возвращает версию, включающую в себя major и minor версии,
/// т. е., иными словами, это та же version, но с обнуленной
/// sub-minor версией.
Version			get_major_minor_version(Version version);

/// Возвращает minor версию.
int				get_minor_version(Version version);

/// Возвращает sub-minor версию.
int				get_sub_minor_version(Version version);

/// Создает числовое представление версии.
Version			get_version(int major, int minor, int sub_minor);

/// Проверяет, одинаковы ли строки.
inline
bool			is_eq(const char* a, const char* b);

/// Проверяет, является ли версия правильно сформированной.
bool			is_valid_version(Version version);


}

#include "base.hh"

#ifdef MLIB_ENABLE_ALIASES
	#ifdef MLIB_ENABLE_LIBTORRENT
		namespace libtorrent {}
		namespace m {
			namespace libtorrent { using namespace ::libtorrent; }
			namespace lt = libtorrent;
		}
		namespace lt = libtorrent;
	#endif

	namespace boost { namespace filesystem {} }
	namespace fs = boost::filesystem;

	using m::Size;
	using m::Size_float;
	using m::Speed;
	using m::Time;
	using m::Time_ms;
	using m::Version;
#endif

#endif


