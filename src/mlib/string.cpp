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


#include <glibmm/convert.h>
#include <glibmm/unicode.h>

#include "misc.hpp"
#include "string.hpp"


namespace m
{


const struct Charset AVAILABLE_CHARSETS[] = {
	{ "Arabic (IBM-864)",                  "IBM864"       },
	{ "Arabic (ISO-8859-6)",               "ISO-8859-6"   },
	{ "Arabic (Windows-1256)",             "WINDOWS-1256" },
	{ "Armenian (ARMSCII-8)",              "ARMSCII-8"    },
	{ "Baltic (ISO-8859-13)",              "ISO-8859-13"  },
	{ "Baltic (ISO-8859-4)",               "ISO-8859-4"   },
	{ "Baltic (Windows-1257)",             "WINDOWS-1257" },
	{ "Celtic (ISO-8859-14)",              "ISO-8859-14"  },
	{ "Central European (IBM-852)",        "IBM852"       },
	{ "Central European (ISO-8859-2)",     "ISO-8859-2"   },
	{ "Central European (Windows-1250)",   "WINDOWS-1250" },
	{ "Chinese Simplified (GB18030)",      "GB18030"      },
	{ "Chinese Simplified (GB2312)",       "GB2312"       },
	{ "Chinese Simplified (ISO-2022-CN)",  "ISO-2022-CN"  },
	{ "Chinese Traditional (Big5)",        "BIG5"         },
	{ "Chinese Traditional (Big5-HKSCS)",  "BIG5-HKSCS"   },
	{ "Cyrillic (IBM-855)",                "IBM855"       },
	{ "Cyrillic (ISO-8859-5)",             "ISO-8859-5"   },
	{ "Cyrillic (ISO-IR-111)",             "ISO-IR-111"   },
	{ "Cyrillic (KOI8-R)",                 "KOI8-R"       },
	{ "Cyrillic (Windows-1251)",           "WINDOWS-1251" },
	{ "Cyrillic/Russian (IBM-866)",        "IBM866"       },
	{ "Cyrillic/Ukrainian (KOI8-U)",       "KOI8-U"       },
	{ "Greek (ISO-8859-7)",                "ISO-8859-7"   },
	{ "Greek (Windows-1253)",              "WINDOWS-1253" },
	{ "Hebrew (IBM-862)",                  "IBM862"       },
	{ "Hebrew (Windows-1255)",             "WINDOWS-1255" },
	{ "Hebrew (ISO-8859-8)",               "ISO-8859-8"   },
	{ "Japanese (EUC-JP)",                 "EUC-JP"       },
	{ "Japanese (ISO-2022-JP)",            "ISO-2022-JP"  },
	{ "Korean (EUC-KR)",                   "EUC-KR"       },
	{ "Korean (ISO-2022-KR)",              "ISO-2022-KR"  },
	{ "Nordic (ISO-8859-10)",              "ISO-8859-10"  },
	{ "Romanian (ISO-8859-16)",            "ISO-8859-16"  },
	{ "South European (ISO-8859-3)",       "ISO-8859-3"   },
	{ "Thai (TIS-620)",                    "TIS-620"      },
	{ "Thai (ISO-8859-11)",                "ISO-8859-11"  },
	{ "Thai (Windows-874)",                "WINDOWS-874"  },
	{ "Turkish (IBM-857)",                 "IBM857"       },
	{ "Turkish (ISO-8859-9)",              "ISO-8859-9"   },
	{ "Turkish (Windows-1254)",            "WINDOWS-1254" },
	{ "Unicode (UTF-8)",                   "UTF-8"        },
	{ "Vietnamese (VISCII)",               "VISCII"       },
	{ "Vietnamese (Windows-1258)",         "WINDOWS-1258" },
	{ "Western (IBM-850)",                 "IBM850"       },
	{ "Western (ISO-8859-1)",              "ISO-8859-1"   },
	{ "Western (ISO-8859-15)",             "ISO-8859-15"  },
	{ "Western (Windows-1252)",            "WINDOWS-1252" },
	{ NULL,                                NULL           }
};

const size_t UTF_CHARSET_ID = 41;



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



std::string convert(const std::string& string, const std::string& to_charset, const std::string& from_charset)
{
	try
	{
		return Glib::convert_with_fallback(string, to_charset, from_charset);
	}
	catch(Glib::ConvertError&)
	{
		std::string broken_string;

		for(size_t i = 0; i < string.size(); i++)
		{
			if(::isprint(string[i]))
				broken_string += string[i];
			else
				broken_string += "%" + _F(std::hex, std::uppercase, (int) string[i]);
		}

		return broken_string + " " + _("[[Invalid encoding]]");
	}
}



#ifdef MLIB_ENABLE_LIBTORRENT
	std::string get_libtorrent_files_charset(void)
	{
		std::string locale_charset;
		Glib::get_charset(locale_charset);
		return locale_charset;
	}
#endif



std::string get_time_duration_string(time_t time, bool show_zero_values)
{
	if(time <= 0)
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



bool is_empty_string(const Glib::ustring& string)
{
	M_FOR_CONST_IT(string, it)
		if(!Glib::Unicode::isspace(*it))
			return false;

	return true;
}



bool is_valid_encoding_name(const std::string& encoding)
{
	try
	{
		Glib::convert("", encoding, encoding);
	}
	catch(Glib::ConvertError&)
	{
		return false;
	}

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
	catch(Glib::ConvertError&)
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
			return _("0 B/s");
		else
			return "";
	}
	else if(speed > 0)
		return size_to_string(speed) + _("/s");
	else
	{
		if(show_zero_values)
			return "∞";
		else
			return "";
	}
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
			return Glib::convert_with_fallback(string, locale_charset, MLIB_UTF_CHARSET_NAME);
		}
		catch(Glib::ConvertError&)
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

