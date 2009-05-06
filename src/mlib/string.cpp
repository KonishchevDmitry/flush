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


#include <glibmm/unicode.h>

#include "misc.hpp"
#include "string.hpp"


namespace m
{

std::string _Q(const char* string)
{
	const char* localized;

	#ifdef ENABLE_NLS
		localized = gettext(string);
	#else
		localized = string;
	#endif

	if(localized == string)
	{
		if( (localized = index(localized, '|')) )
			return localized + 1;
		else
			return string;
	}
	else
		return localized;
}



std::string get_time_duration_string(time_t time)
{
	if(time <= 0)
		return _("0m");

	int minutes = time / 60;
	int hours = minutes / 60;
	int days = hours / 24;
	int months = days / 30;

	std::string duration_string;

	days %= 30;
	minutes %= 60;
	hours %= 24;

	if(months)
		duration_string += " " + m::to_string(months) + _("M");

	if(days)
		duration_string += " " + m::to_string(days) + _("d");

	if(hours && !months)
		duration_string += " " + m::to_string(hours) + _("h");

	if(minutes && !days && !months)
		duration_string += " " + m::to_string(minutes) + _("m");

	return duration_string.substr(1);
}



Glib::ustring get_time_left_string(Time time_left)
{
	const long minute = 60;
	const long hour = 60 * minute;
	const long day = 24 * hour;
	const long month = 30 * day;


	if(time_left <= 0)
	{
		return "∞";
	}
	// Месяцы
	else if(time_left >= month)
	{
		// Если больше двух месяцев, то считаем
		// это время за бесконечность.
		if(time_left >= 2 * month)
			return "∞";
		else
		{
			int days;
			Glib::ustring time_left_string = m::to_string( time_left / month ) + _("M");

			if( (days = time_left % month / day) )
				time_left_string += " " + m::to_string(days) + _("d");

			return time_left_string;
		}
	}
	// Дни
	else if(time_left >= day)
	{
		int hours;
		Glib::ustring time_left_string = m::to_string( time_left / day ) + _("d");

		if( (hours = time_left % day / hour) )
			time_left_string += " " + m::to_string(hours) + _("h");

		return time_left_string;
	}
	// Часы
	else if(time_left >= hour)
	{
		int minutes;
		Glib::ustring time_left_string = m::to_string( time_left / hour ) + _("h");

		if( (minutes = time_left % hour / minute) )
			time_left_string += " " + m::to_string(minutes) + _("m");

		return time_left_string;
	}
	// Минуты
	else if(time_left >= 60)
	{
		int seconds;
		Glib::ustring time_left_string = m::to_string( time_left / minute ) + _("m");

		if( (seconds = time_left % minute) )
			time_left_string += " " + m::to_string(seconds) + _("s");

		return time_left_string;
	}
	// Cекунды
	else
		return m::to_string(time_left) + _("s");
}



bool is_empty_string(const Glib::ustring& string)
{
	M_FOR_CONST_IT(Glib::ustring, string, it)
		if(!Glib::Unicode::isspace(*it))
			return false;

	return true;
}



bool is_valid_utf(const Glib::ustring& string)
{
	return string.validate();
}



Glib::ustring L2U(const std::string& string)
{
	try
	{
		return Glib::locale_to_utf8(string);
	}
	catch(Glib::ConvertError)
	{
		Glib::ustring ustring;

		for(size_t i = 0; i < string.size(); i++)
		{
			if(Glib::Unicode::isprint(string[i]))
				ustring += string[i];
			else
				ustring += "%" + _F(std::hex, std::uppercase, (int) (unsigned char) string[i]);
		}

		return ustring + " " + _("[[Invalid encoding]]");
	}
}



std::string size_to_string(Size size)
{
	Size unit;
	int fraction = 0;
	std::string units;

	// Гигабайты
	if( (unit = size / (1024L * 1024L * 1024L)) )
	{
		// Десятки тысяч мегабайт
		if( size / (1000L * 1024L * 1024L) >= 10 )
		{
			units = _("GB");

			if(unit < 100)
				fraction = int(size / 1024 * 10 / (1024L * 1024L) - unit * 10);
		}
		else
		{
			units = _("MB");
			unit = size / (1024L * 1024L);
		}
	}
	// Мегабайты
	else if( (unit = size / (1024L * 1024L)) )
	{
		// Десятки тысяч килобайт
		if( size / (1000L * 1024L) >= 10 )
		{
			units = _("MB");

			if(unit < 100)
				fraction = int(size * 10 / (1024L * 1024L) - unit * 10);
		}
		else
		{
			units = _("KB");
			unit = size / 1024;
		}
	}
	// Килобайты
	else if( (unit = size / 1024) )
	{
		// Десятки тысяч байт
		if( size / 1000 >= 10 )
		{
			units = _("KB");

			if(unit < 100)
				fraction = int(size * 10 / 1024 - unit * 10);
		}
		else
		{
			units = _("B");
			unit = size;
		}
	}
	// Байты
	else
	{
		units = _("B");
		unit = size;
	}

	if(fraction)
		return m::to_string(unit) + "." + m::to_string(fraction) + " " + units;
	else
		return m::to_string(unit) + " " + units;
}



std::string speed_to_string(Speed speed)
{
	if(speed == 0)
	{
		// Может сильно ускорить работу функции,
		// т. к. нулевая скорость встречается очень
		// часто.
		return _("0 B/s");
	}
	else if(speed > 0)
		return size_to_string(speed) + _("/s");
	else
		return "∞";
}



std::string time_to_string_with_date(Time time)
{
	struct tm tm_time;
	time_t time_t_time = static_cast<time_t>(time);
	static char time_string_buf[20];

	MLIB_A(localtime_r(&time_t_time, &tm_time));
	MLIB_A(strftime(time_string_buf, sizeof time_string_buf, "%H:%M:%S %d.%m.%Y", &tm_time));

	return time_string_buf;
}



Glib::ustring trim(const Glib::ustring& string)
{
	size_t start_pos = 0;
	size_t size = string.size();

	for(; start_pos < string.size(); start_pos++)
		if(!Glib::Unicode::isspace(string[start_pos]))
			break;

	size -= start_pos;

	for(; size > 0; size--)
		if(!Glib::Unicode::isspace(string[start_pos + size - 1]))
			break;

	if(size != string.size())
		return string.substr(start_pos, size);
	else
		return string;
}



Glib::ustring uppercase_first(const Glib::ustring& string)
{
	if(string.empty())
		return string;
	else
		return Glib::Unicode::toupper(string[0]) + string.substr(1);
}



std::string U2L(const std::string& string)
{
	std::string locale_charset;

	if(!Glib::get_charset(locale_charset))
	{
		try
		{
			return Glib::convert_with_fallback(string, locale_charset, "UTF-8");
		}
		catch(Glib::ConvertError)
		{
			std::string broken_string;

			for(size_t i = 0; i < string.size(); i++)
			{
				if(::isprint(string[i]))
					broken_string += string[i];
				else
					broken_string += "%" + _F(std::hex, std::uppercase, (int) (unsigned char) string[i]);
			}

			return broken_string + " " + _("[[Invalid encoding]]");
		}
	}
	else
		return string;
}

}

