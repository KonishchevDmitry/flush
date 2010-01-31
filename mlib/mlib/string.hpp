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


#ifndef HEADER_MLIB_STRING
#define HEADER_MLIB_STRING

#include <mlib/main.hpp>


namespace m {


/// Возвращает строковое представление периода времени вида
/// 1d 5h 50m.
std::string		get_time_duration_string(time_t time, bool show_zero_values = true);

/// Возвращает строку, представляющую оставшееся время в удобном для
/// восприятия формате.
///
/// Имеет некоторое максимальное значение time_left, при превышении
/// которого оставшееся время обозначается символом бесконечности.
Glib::ustring	get_time_left_string(Time time_left, bool show_zero_values = true);

/// Возвращает строку, представляющую оставшееся время в удобном для
/// восприятия формате, предполагая, что данное время имеет значение в
/// пределах часа. Генерируемая строка по возможности сохраняет
/// одинаковую ширину, чтобы при изменении оставшегося времени не
/// "дышали" GUI.
Glib::ustring	get_within_hour_time_left_string(Time time_left);

/// Проверяет, пустая ли строка (содержит ли что-нибудь кроме
/// пробельных символов).
bool			is_empty_string(const Glib::ustring& string);

/// Проверяет, вляется ли строка URL (http) адресом.
bool			is_url_string(const std::string& string);

/// Возвращает строковое представление размера
std::string		size_to_string(Size size, bool show_zero_values = true);

/// Возвращает строковое представление скорости передачи данных
std::string		speed_to_string(Speed speed, bool show_zero_values = true);

/// Разделяет строку на подстроки, при этом проверяя, чтобы в
/// результирующем векторе не было пустых строк.
String_vector	split(const std::string& string, char separator);

/// Возвращает строковое представление времени в формате "%H:%M:%S %d.%m.%Y".
std::string		time_to_string_with_date(Time time);

/// Удаляет из строки начальные и конечные пробельные символы.
Glib::ustring	trim(const Glib::ustring& string);

/// Делает первую букву строки заглавной.
Glib::ustring	uppercase_first(const Glib::ustring& string);


}

#endif

