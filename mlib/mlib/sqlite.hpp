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


// Предоставляет thread-safe-C++-обертку для SQLite.


#ifndef HEADER_MLIB_SQLITE
#define HEADER_MLIB_SQLITE

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#ifndef MLIB_ENABLE_LIBS_FORWARDS
	#include <sqlite3.h>
#endif

#include <mlib/main.hpp>

#include "sqlite.hxx"


namespace m { namespace sqlite {


namespace Db_aux { class Private; }

class Db: private boost::noncopyable
{
	private:
		typedef Db_aux::Private Private;


	public:
		Db(void);
		~Db(void);


	private:
		boost::scoped_ptr<Private>	priv;


	public:
		/// Возвращает C-объект библиотеки sqlite.
		sqlite3*		c_obj(void);

		/// Закрывает базу данных.
		/// @throw - m::Exception.
		void			close(void);

		/// Выполняет запрос к БД.
		/// @throw - m::Exception.
		void			exec(std::string query_string);

		/// Возвращает путь к открытой в данной момент базе данных.
		/// Если БД еще не открыта, генерирует исключение.
		/// @throw - m::Exception.
		std::string		get_path(void);

		/// Открывает базу данных.
		/// @throw - m::Exception.
		void			open(const std::string& db_path);

		/// Возвращает true, если база данных открыта.
		bool			opened(void) const;

		/// Аналог sqlite3_prepare().
		/// @throw - m::Exception.
		Query			query(std::string query_string);
};



class Stmt: private boost::noncopyable
{
	private:
		Stmt(void);
		Stmt(sqlite3_stmt* stmt);

	public:
		~Stmt(void);


	private:
		sqlite3_stmt* stmt;


	public:
		/// Создает объект Query, с которым будет взаимодействовать
		/// пользователь.
		static Query	create(sqlite3_stmt* stmt);

		/// Аналог sqlite3_finalize().
		/// @throw - m::Exception.
		void			finalize(void);

		/// Проверяет, был ли уже завершен данный запрос.
		bool			finalized(void);

		/// Аналог sqlite3_column_blob().
		void			get_value(int column, const void** value);

		/// Аналог sqlite3_column_int().
		void			get_value(int column, int32_t* value);

		/// Аналог sqlite3_column_int().
		void			get_value(int column, uint32_t* value);

		/// Аналог sqlite3_column_int64().
		void			get_value(int column, int64_t* value);

		/// Аналог sqlite3_column_int64().
		void			get_value(int column, uint64_t* value);

		/// Аналог sqlite3_column_double().
		void			get_value(int column, double* value);

		/// Аналог sqlite3_column_text().
		void			get_value(int column, std::string* value);

		/// Аналог sqlite3_column_bytes().
		size_t			get_value_size(int column);

		/// Аналог sqlite3_step().
		/// @throw - m::Exception.
		/// @return - true, если в результате выполнения функции выбралась
		/// очередная строка, false - если запрос успешно отработал и не вернул
		/// ни одной строки.
		bool			step(void);
};


}}

#endif

