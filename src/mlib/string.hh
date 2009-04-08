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


// Ускоряем boost::lexical_cast -->
namespace boost
{
	template<> inline
	std::string lexical_cast(const int &value)
	{
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}


	template<> inline
	std::string lexical_cast(const long long &value)
	{
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}
}
// Ускоряем boost::lexical_cast <--



// В glibmm-2.16.4 замечено, что программа не компилируется, когда
// Glib::ustring::compose() в качестве одного из параметров передать const
// char* (кроме параметра со строкой форматирования). Это проиходит из-за того,
// что в шаблонах возникает неопределенность. В glibmm-2.18.1 такой проблемы
// уже нет.
// -->
	namespace
	{
		M_LIBRARY_COMPATIBILITY

		template<class T> inline
		const T&		correct_glib_format_value(const T& value);

		Glib::ustring	correct_glib_format_value(const char* value);



		template<class T>
		const T& correct_glib_format_value(const T& value)
		{
			return value;
		}



		Glib::ustring correct_glib_format_value(const char* value)
		{
			return Glib::ustring(value);
		}
	}
// <--



namespace m
{

#ifdef MLIB_ENABLE_FORMAT
// Format -->
	Format::Format(const char* file, const int line, const std::string& format_string, bool unicode_output, bool block_exceptions)
	:
		file(file),
		line(line),
		unicode_output(unicode_output),
		block_exceptions(block_exceptions)
	{
		try
		{
			boost::format::parse(format_string);
		}
		catch(boost::io::format_error)
		{
			if(block_exceptions)
				boost::format::parse(_("[[ Invalid format string ]]"));
			else
				m::error(this->file, this->line, _("Invalid format string."));
		}
	}



	Format::Format(const char* file, const int line, const boost::format& format, bool unicode_output, bool block_exceptions)
	:
		boost::format(format),
		file(file),
		line(line),
		unicode_output(unicode_output),
		block_exceptions(block_exceptions)
	{
	}



	std::string Format::str(void) const
	{
		try
		{
			return boost::format::str();
		}
		catch(boost::io::format_error)
		{
			if(block_exceptions)
				return _("[[ Invalid format string ]]");
			else
				m::error(this->file, this->line, _("Invalid format string."));
		}
	}



	template<class T>
	Format Format::operator%(const T& value) const
	{
		try
		{
			return Format(this->file, this->line, static_cast<boost::format>(*this) % value, this->unicode_output, this->block_exceptions);
		}
		catch(boost::io::format_error)
		{
			if(block_exceptions)
				return Format(this->file, this->line, _("[[ Invalid format string ]]"), this->unicode_output, this->block_exceptions);
			else
				m::error(this->file, this->line, _("Invalid format string."));
		}
	}



	Format::operator std::string(void) const
	{
		if(this->unicode_output)
			return L2U(this->str());
		else
			return this->str();
	}



	Format::operator Glib::ustring(void) const
	{
		if(this->unicode_output)
			return L2U(this->str());
		else
			return this->str();
	}
// Format <--
#endif



Glib::ustring __(const char* fmt)
{
	return Glib::ustring::compose(_(fmt) + "%1", correct_glib_format_value(""));
}



template<class T1>
Glib::ustring __(const char* fmt, const T1& a1)
{
	return Glib::ustring::compose(_(fmt), correct_glib_format_value(a1));
}



template<class T1, class T2>
Glib::ustring __(const char* fmt, const T1& a1, const T2& a2)
{
	return Glib::ustring::compose(_(fmt), correct_glib_format_value(a1), correct_glib_format_value(a2));
}



template<class T1, class T2, class T3>
Glib::ustring __(const char* fmt, const T1& a1, const T2& a2, const T3& a3)
{
	return Glib::ustring::compose(_(fmt), correct_glib_format_value(a1), correct_glib_format_value(a2), correct_glib_format_value(a3));
}



template<class T1>
Glib::ustring _C(const char* fmt, const T1& a1)
{
	return Glib::ustring::compose(fmt, correct_glib_format_value(a1));
}



template<class T1, class T2>
Glib::ustring _C(const char* fmt, const T1& a1, const T2& a2)
{
	return Glib::ustring::compose(fmt, correct_glib_format_value(a1), correct_glib_format_value(a2));
}



template<class T1, class T2, class T3>
Glib::ustring _C(const char* fmt, const T1& a1, const T2& a2, const T3& a3)
{
	return Glib::ustring::compose(fmt, correct_glib_format_value(a1), correct_glib_format_value(a2), correct_glib_format_value(a3));
}



template<class T1>
Glib::ustring _F(const T1& a1)
{
	return Glib::ustring::format(correct_glib_format_value(a1));
}



template<class T1, class T2>
Glib::ustring _F(const T1& a1, const T2& a2)
{
	return Glib::ustring::format(correct_glib_format_value(a1), correct_glib_format_value(a2));
}



template<class T1, class T2, class T3>
Glib::ustring _F(const T1& a1, const T2& a2, const T3& a3)
{
	return Glib::ustring::format(correct_glib_format_value(a1), correct_glib_format_value(a2), correct_glib_format_value(a3));
}



#ifdef MLIB_ENABLE_FORMAT
	Format gettext_format(const char* file, const int line, const char* string)
	{
		#ifdef ENABLE_NLS
			return Format(file, line, gettext(string), false, true);
		#else
			return Format(file, line, string, false, true);
		#endif
	}



	Format gettext_format_unicode(const char* file, const int line, const char* string)
	{
		#ifdef ENABLE_NLS
			return Format(file, line, gettext(string), true, true);
		#else
			return Format(file, line, string, true, true);
		#endif
	}
#endif



bool is_eq(const char* a, const char* b)
{
	return !strcmp(a, b);
}



template<class Value>
std::string to_string(const Value& value)
{
	return boost::lexical_cast<std::string>(value);
}



std::string U2L(const char* string)
{
	return U2L(std::string(string));
}



std::string U2L(const Glib::ustring& string)
{
	return U2L(static_cast<std::string>(string));
}
}

