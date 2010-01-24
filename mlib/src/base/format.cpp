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


#ifdef ENABLE_NLS
	#include <libintl.h>
#endif

#include <glibmm/convert.h>

#include <mlib/main.hpp>

#include "format.hpp"



namespace m {


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



std::string _(const char* string)
{
	#ifdef ENABLE_NLS
		return gettext(string);
	#else
		return string;
	#endif
}



Glib::ustring __(const char* fmt)
{
	try
	{
		return Glib::ustring::compose(_(fmt) + "%1", Format_aux::correct_glib_format_value(""));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



Glib::ustring __Q(const char* fmt)
{
	try
	{
		return Glib::ustring::compose(_Q(fmt) + "%1", Format_aux::correct_glib_format_value(""));
	}
	catch(Glib::ConvertError&)
	{
		return _("[[Invalid encoding]]");
	}
}



const char* _G(const char* string)
{
	#ifdef ENABLE_NLS
		return gettext(string);
	#else
		return string;
	#endif
}



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



Glib::ustring validate_utf(const Glib::ustring& string)
{
	if(string.validate())
		return string;
	else
		return m::convert(string, MLIB_UTF_CHARSET_NAME, MLIB_UTF_CHARSET_NAME);
}


}

