/**************************************************************************
*                                                                         *
*   Flush - GTK-based BitTorrent client                                   *
*   http://sourceforge.net/projects/flush                                 *
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


#ifndef HEADER_CLIENT_CMD_OPTIONS
	#define HEADER_CLIENT_CMD_OPTIONS

	#include <string>
	#include <deque>

	#include "common.hpp"


	class Client_cmd_options
	{
		public:
			/// Определяет None-значение для чисел.
			static int					invalid_number_value;

			/// Определяет None-значение для скоростей.
			static Speed				invalid_speed_value;


		public:
			Client_cmd_options(void);


		public:
			/// Путь к директории с конфигурационными файлами.
			std::string					config_path;

			/// Список *.torrent файлов, которые необходимо скачать.
			std::deque<std::string>		torrents_paths;


			/// Ограничение на скорость скачивания (КБ/с).
			Speed						download_rate_limit;

			/// Ограничение на скорость отдачи (КБ/с).
			Speed						upload_rate_limit;


			/// Максимальное количество соединений для раздачи,
			/// которое может быть открыто.
			int							max_uploads;

			/// Максимальное количество соединений, которое может
			/// быть открыто.
			int							max_connections;


			/// Запустить торренты.
			Torrents_group				start;

			/// Приостановить торренты.
			Torrents_group				stop;


			/// Только передать опции командной строки уже запущенной копии
			/// и не запускать новую копию, если в данный момент нет
			/// запущенной копии, которой можно было бы передать настройки.
			bool						only_pass;


		public:
			/// Производит всю необходимую работу по "расфасовке"
			/// опций командной строки в члены класса.
			///
			/// Внимание! Принимаемый массив строк должен быть в кодировке
			/// локали.
			/// @throw - m::Exception.
			void						parse(int argc, char *argv[]);

			/// "Перегоняет" полученные переменные обратно в массив
			/// аргументов командной строки.
			///
			/// Внимание!
			/// Записывет только те опции, которые имеет смысл передавать
			/// по DBus уже работающей копии.
			std::vector<std::string>	to_strings(void);

		private:
			/// Производит проверку переданного значения - оно должно быть больше либо равно value.
			/// @throw - m::Exception.
			template<class T>
			void	greater_or_equal_check(const std::string option_name, const T value, T target_value);

			/// Производит проверку полученных значений для start и stop и
			/// устанавливает в соответствии с ними значения членов класса.
			/// @throw - m::Exception.
			void	start_stop_check(const std::string option_name, const std::string& string_value, Torrents_group* target_value);
	};

#endif

