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


#include <boost/algorithm/string/split.hpp>

#include <glibmm/convert.h>
#include <glibmm/unicode.h>

#include <mlib/main.hpp>

#include "string.hpp"


namespace m
{


namespace
{
	template<class T>
	class Is_this
	{
		public:
			Is_this(T value): value(value) {};


		private:
			T	value;


		public:
			bool	operator()(T value) const { return this->value == value; }
	};
}



std::string get_time_duration_string(time_t time, bool show_zero_values)
{
	if(time < 60)
	{
		if(show_zero_values)
			return _("0m");
		else
			return "";
	}

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



Glib::ustring get_time_left_string(Time time_left, bool show_zero_values)
{
	const long minute = 60;
	const long hour = 60 * minute;
	const long day = 24 * hour;
	const long month = 30 * day;


	if(time_left <= 0)
	{
		if(show_zero_values)
			return "∞";
		else
			return "";
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



Glib::ustring get_within_hour_time_left_string(Time time_left)
{
	const Time minute = 60;

	// В пределах часа
	if(time_left <= 60 * 60)
	{
		return
			m::to_string( time_left / minute ) + _Q("minutes|m") + " " +
			m::to_string( time_left % minute ) + _Q("seconds|s");
	}
	// Такого большого времени быть не должно, но обработать его мы обязаны.
	else
		return get_time_left_string(time_left);
}



bool is_empty_string(const Glib::ustring& string)
{
	M_FOR_CONST_IT(string, it)
		if(!Glib::Unicode::isspace(*it))
			return false;

	return true;
}



bool is_url_string(const std::string& string)
{
	// Простейшая проверка на валидность адреса

	if(
		string.size() < strlen("http://X") ||
		string.substr(0, strlen("http://")) != "http://" ||
		string.find(' ') != string.npos
	)
		return false;
	else
		return true;
}



std::string size_to_string(Size size, bool show_zero_values)
{
	if(!show_zero_values && size <= 0)
		return "";

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



std::string speed_to_string(Speed speed, bool show_zero_values)
{
	// Может сильно ускорить работу функции,
	// т. к. нулевая скорость встречается очень
	// часто.
	if(speed == 0)
	{
		if(show_zero_values)
			return _("0 KB/s");
		else
			return "";
	}
	// Значения, меньшие десятка тысяч байт, size_to_string() отображает в
	// байтах, что для скорости является не очень естественным представлением.
	else if(speed < 10 * 1000)
	{
		if(speed >= 100)
		{
			int fraction = int(speed * 10 / 1024 % 10);

			if(fraction)
				return m::to_string(speed / 1024) + "." + m::to_string(fraction) + " " + _("KB/s");
			else
				return m::to_string(speed / 1024) + " " + _("KB/s");
		}
		else
			return m::to_string(speed) + " " + _("B/s");
	}
	else if(speed > 0)
		return size_to_string(speed, show_zero_values) + _("/s");
	else
	{
		if(show_zero_values)
			return "∞";
		else
			return "";
	}
}



String_vector split(const std::string& string, char separator)
{
	String_vector strings;
	boost::split(strings, string, Is_this<M_TYPEOF(separator)>(separator));
	return strings;
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

}

