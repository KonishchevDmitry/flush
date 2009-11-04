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


	#include <sqlite3.h>

#if MLIB_DEVELOP_MODE
	#include <glibmm/unicode.h>
	#include <glibmm/ustring.h>
#endif

	#include <mlib/main.hpp>
	#include <mlib/mutex.hpp>

	#include "sqlite.hpp"


namespace m { namespace sqlite {


namespace
{
#if MLIB_DEVELOP_MODE
	/// Удаляет все лишние символы из строки SQL запроса, чтобы облегчить ее
	/// чтение.
	Glib::ustring	format_query_string(const Glib::ustring& string);
#endif

	/// Анализирует полученное значение, как значение, возвращенное одной из
	/// sqlite3_* функций и генерирует исключение, если это значение
	/// свидетельствует об ошибке.
	/// @throw - m::Exception.
	void			sqlite_error_handler(int sqlite_errno);

	// Возвращает строковое представление ошибки SQLite.
	std::string		sqlite_strerror(int sqlite_errno);



#if MLIB_DEVELOP_MODE
	Glib::ustring format_query_string(const Glib::ustring& string)
	{
		size_t i;
		size_t size = string.size();

		bool first_space = true;
		Glib::ustring new_string;

		for(i = 0; i < size; i++)
		{
			Glib::ustring::value_type symbol = string[i];

			if(Glib::Unicode::isspace(symbol))
			{
				if(first_space)
				{
					if(i != 0)
						new_string += symbol;

					first_space = false;
				}
			}
			else
			{
				first_space = true;
				new_string += symbol;
			}
		}

		if(!new_string.empty())
			if(Glib::Unicode::isspace(new_string[new_string.size() - 1]))
				new_string = new_string.substr(0, new_string.size() - 1);

		return new_string;
	}
#endif



	void sqlite_error_handler(int sqlite_errno)
	{
		if(sqlite_errno)
			M_THROW(sqlite_strerror(sqlite_errno));
	}



	std::string sqlite_strerror(int sqlite_errno)
	{
		static const char* const errors_messages[] = {
			/* SQLITE_OK         */ _G("not an error"),
			/* SQLITE_ERROR      */ _G("SQL logic error or missing database"),
			/* SQLITE_INTERNAL   */ _G("internal error"),
			/* SQLITE_PERM       */ _G("access permission denied"),
			/* SQLITE_ABORT      */ _G("callback requested query abort"),
			/* SQLITE_BUSY       */ _G("database is locked"),
			/* SQLITE_LOCKED     */ _G("database table is locked"),
			/* SQLITE_NOMEM      */ _G("out of memory"),
			/* SQLITE_READONLY   */ _G("attempt to write a readonly database"),
			/* SQLITE_INTERRUPT  */ _G("interrupted"),
			/* SQLITE_IOERR      */ _G("disk I/O error"),
			/* SQLITE_CORRUPT    */ _G("database disk image is malformed"),
			/* SQLITE_NOTFOUND   */ _G("not found"),
			/* SQLITE_FULL       */ _G("database or disk is full"),
			/* SQLITE_CANTOPEN   */ _G("unable to open database file"),
			/* SQLITE_PROTOCOL   */ _G("protocol error"),
			/* SQLITE_EMPTY      */ _G("table contains no data"),
			/* SQLITE_SCHEMA     */ _G("database schema has changed"),
			/* SQLITE_TOOBIG     */ _G("string or blob too big"),
			/* SQLITE_CONSTRAINT */ _G("constraint failed"),
			/* SQLITE_MISMATCH   */ _G("datatype mismatch"),
			/* SQLITE_MISUSE     */ _G("library routine called out of sequence"),
			/* SQLITE_NOLFS      */ _G("large file support is disabled"),
			/* SQLITE_AUTH       */ _G("authorization denied"),
			/* SQLITE_FORMAT     */ _G("auxiliary database format error"),
			/* SQLITE_RANGE      */ _G("bind or column index out of range"),
			/* SQLITE_NOTADB     */ _G("file is encrypted or is not a database"),
		};

		if(sqlite_errno < 0 || sqlite_errno >= (int) M_STATIC_ARRAY_SIZE(errors_messages))
			return _("unknown error");
		else
			return errors_messages[sqlite_errno];
	}

}



// Db -->
namespace Db_aux
{
	class Private
	{
		public:
			Private(void);


		public:
			/// База данных sqlite.
			sqlite3*				db;

			/// Путь к открытой базе данных.
			std::string				db_path;

			/// Блокирует доступ к БД.
			Recursive_mutex			db_mutex;
	};



	Private::Private(void)
	:
		db(NULL)
	{
	}
}



Db::Db(void)
:
	priv(new Private)
{
};



Db::~Db(void)
{
	if(this->opened())
	{
		#if MLIB_DEBUG_MODE
			std::string db_path = this->get_path();
		#endif

		try
		{
			this->close();
		}
		catch(m::Exception& e)
		{
			MLIB_D(_C("DB '%1' closing error: %2.", db_path, EE(e)));
		}
	}
}



sqlite3* Db::c_obj(void)
{
	Scoped_lock lock(&priv->db_mutex);
	return priv->db;
}



void Db::close(void)
{
	Scoped_lock lock(&priv->db_mutex);

	if(!this->opened())
		M_THROW(_("Database is not opened yet."));

	sqlite3* db = priv->db;
	priv->db = NULL;

	MLIB_D(_C("Closing database '%1'...", priv->db_path));

	// Генерирует m::Exception.
	sqlite_error_handler( sqlite3_close(db) );

	MLIB_D(_C("Database '%1' has been successfully closed.", priv->db_path));
}



void Db::exec(std::string query_string)
{
	#if MLIB_DEVELOP_MODE
		query_string = format_query_string(query_string);
	#endif

	char *error_msg;
	Scoped_lock lock(&priv->db_mutex);

	if(sqlite3_exec(priv->db, query_string.c_str(), NULL, NULL, &error_msg) != SQLITE_OK)
	{
		std::string error_string = error_msg;
		sqlite3_free(error_msg);

		MLIB_D(_C("SQL exec error in query '%1': %2.", query_string, error_string));
		M_THROW(error_string);
	}
}



std::string Db::get_path(void)
{
	Scoped_lock lock(&priv->db_mutex);

	if(!this->opened())
		M_THROW(_("Database is not opened yet."));

	return priv->db_path;
}



void Db::open(const std::string& db_path)
{
	Scoped_lock lock(&priv->db_mutex);

	if(this->opened())
		M_THROW(_("Database is already opened."));

	sqlite3* db = NULL;

	MLIB_D(_C("Opening database '%1'...", db_path));

	try
	{
		sqlite_error_handler( sqlite3_open(U2L(db_path).c_str(), &db) );
	}
	catch(m::Exception& e)
	{
		sqlite3_close(db);
		throw;
	}

	priv->db = db;

	MLIB_D(_C("Database '%1' has been successfully opened.", db_path));
}



bool Db::opened(void) const
{
	Scoped_lock lock(&priv->db_mutex);
	return priv->db != NULL;
}



Query Db::query(std::string query_string)
{
#if MLIB_DEVELOP_MODE
	query_string = format_query_string(query_string);
#endif

	sqlite3_stmt* stmt;
	const char* query_string_tail;

#if MLIB_DEVELOP_MODE
	try
	{
#endif
		sqlite_error_handler(
			sqlite3_prepare_v2(this->c_obj(), query_string.c_str(), -1, &stmt, &query_string_tail) );
#if MLIB_DEVELOP_MODE
	}
	catch(m::Exception& e)
	{
		MLIB_D(_C("SQL exec error in query '%1': %2.", query_string, EE(e)));
		throw;
	}
#endif

	return Stmt::create(stmt);
}
// Db <--



// Stmt -->
Stmt::Stmt(sqlite3_stmt* stmt)
:
	stmt(stmt)
{
}



Stmt::~Stmt(void)
{
	if(!this->finalized())
	{
		try
		{
			this->finalize();
		}
		catch(m::Exception& e)
		{
			MLIB_D(_C("Stmt finalizing error: %1.", EE(e)));
		}
	}
}



Query Stmt::create(sqlite3_stmt* stmt)
{
	return Query(new Stmt(stmt));
}



void Stmt::finalize(void)
{
	if(this->finalized())
		M_THROW(_("Query is already finalized."));

	sqlite3_stmt* stmt = this->stmt;
	this->stmt = NULL;

	sqlite_error_handler( sqlite3_finalize(stmt) );
}



bool Stmt::finalized(void)
{
	return this->stmt == NULL;
}



void Stmt::get_value(int column, const void** value)
{
	*value = sqlite3_column_blob(this->stmt, column);
}



void Stmt::get_value(int column, int32_t* value)
{
	*value = sqlite3_column_int(this->stmt, column);
}



void Stmt::get_value(int column, uint32_t* value)
{
	*value = sqlite3_column_int(this->stmt, column);
}



void Stmt::get_value(int column, int64_t* value)
{
	*value = sqlite3_column_int64(this->stmt, column);
}



void Stmt::get_value(int column, uint64_t* value)
{
	*value = sqlite3_column_int64(this->stmt, column);
}



void Stmt::get_value(int column, double* value)
{
	*value = sqlite3_column_double(this->stmt, column);
}



void Stmt::get_value(int column, std::string* value)
{
	const char* text = (const char*) sqlite3_column_text(this->stmt, column);

	if(text)
		*value = text;
	else
		value->clear();
}



size_t Stmt::get_value_size(int column)
{
	return sqlite3_column_bytes(this->stmt, column);
}



bool Stmt::step(void)
{
	if(this->finalized())
		M_THROW(_("Operation on finalized query."));

	int result = sqlite3_step(this->stmt);

	switch(result)
	{
		case SQLITE_DONE:
			return false;
			break;

		case SQLITE_ROW:
			return true;
			break;

		default:
			M_THROW(sqlite_strerror(result));
			break;
	}
}
// Stmt <--


}}

