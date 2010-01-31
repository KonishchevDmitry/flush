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


#include <locale>
#include <string>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include <glibmm/convert.h>
#include <glibmm/ustring.h>


namespace m {


namespace Format_aux
{
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



template<class T1>
Glib::ustring __(const char* fmt, const T1& a1)
{
	try
	{
		return Glib::ustring::compose(_(fmt), Format_aux::correct_glib_format_value(a1));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1, class T2>
Glib::ustring __(const char* fmt, const T1& a1, const T2& a2)
{
	try
	{
		return Glib::ustring::compose(_(fmt), Format_aux::correct_glib_format_value(a1), Format_aux::correct_glib_format_value(a2));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1, class T2, class T3>
Glib::ustring __(const char* fmt, const T1& a1, const T2& a2, const T3& a3)
{
	try
	{
		return Glib::ustring::compose(_(fmt), Format_aux::correct_glib_format_value(a1), Format_aux::correct_glib_format_value(a2), Format_aux::correct_glib_format_value(a3));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1>
Glib::ustring __Q(const char* fmt, const T1& a1)
{
	try
	{
		return Glib::ustring::compose(_Q(fmt), Format_aux::correct_glib_format_value(a1));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1, class T2>
Glib::ustring __Q(const char* fmt, const T1& a1, const T2& a2)
{
	try
	{
		return Glib::ustring::compose(_Q(fmt), Format_aux::correct_glib_format_value(a1), Format_aux::correct_glib_format_value(a2));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1, class T2, class T3>
Glib::ustring __Q(const char* fmt, const T1& a1, const T2& a2, const T3& a3)
{
	try
	{
		return Glib::ustring::compose(_Q(fmt), Format_aux::correct_glib_format_value(a1), Format_aux::correct_glib_format_value(a2), Format_aux::correct_glib_format_value(a3));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1>
Glib::ustring _C(const char* fmt, const T1& a1)
{
	try
	{
		return Glib::ustring::compose(fmt, Format_aux::correct_glib_format_value(a1));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1, class T2>
Glib::ustring _C(const char* fmt, const T1& a1, const T2& a2)
{
	try
	{
		return Glib::ustring::compose(fmt, Format_aux::correct_glib_format_value(a1), Format_aux::correct_glib_format_value(a2));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1, class T2, class T3>
Glib::ustring _C(const char* fmt, const T1& a1, const T2& a2, const T3& a3)
{
	try
	{
		return Glib::ustring::compose(fmt, Format_aux::correct_glib_format_value(a1), Format_aux::correct_glib_format_value(a2), Format_aux::correct_glib_format_value(a3));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1, class T2, class T3, class T4> inline
Glib::ustring	_C(const char* fmt, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
{
	try
	{
		return Glib::ustring::compose(fmt,
			Format_aux::correct_glib_format_value(a1),
			Format_aux::correct_glib_format_value(a2),
			Format_aux::correct_glib_format_value(a3),
			Format_aux::correct_glib_format_value(a4)
		);
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1, class T2, class T3, class T4, class T5> inline
Glib::ustring	_C(const char* fmt, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
{
	try
	{
		return Glib::ustring::compose(fmt,
			Format_aux::correct_glib_format_value(a1),
			Format_aux::correct_glib_format_value(a2),
			Format_aux::correct_glib_format_value(a3),
			Format_aux::correct_glib_format_value(a4),
			Format_aux::correct_glib_format_value(a5)
		);
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1>
Glib::ustring _F(const T1& a1)
{
	try
	{
		return Glib::ustring::format(Format_aux::correct_glib_format_value(a1));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1, class T2>
Glib::ustring _F(const T1& a1, const T2& a2)
{
	try
	{
		return Glib::ustring::format(Format_aux::correct_glib_format_value(a1), Format_aux::correct_glib_format_value(a2));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class T1, class T2, class T3>
Glib::ustring _F(const T1& a1, const T2& a2, const T3& a3)
{
	try
	{
		return Glib::ustring::format(Format_aux::correct_glib_format_value(a1), Format_aux::correct_glib_format_value(a2), Format_aux::correct_glib_format_value(a3));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



template<class Value>
std::string to_string(const Value& value)
{
	return boost::lexical_cast<std::string>(value);
}



template<class Value>
std::string to_string_non_locale(const Value& value)
{
	std::ostringstream oss;
	oss.imbue(std::locale::classic());
	oss << value;
	return oss.str();
}


}

