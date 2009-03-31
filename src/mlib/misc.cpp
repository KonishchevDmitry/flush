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


#ifndef MLIB_ENABLE_ALIASES
	#define MLIB_ENABLE_ALIASES
#endif

#include <semaphore.h>
#include <stdint.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <cerrno>
#include <ctime>

#include <fstream>

#include "messages.hpp"
#include "misc.hpp"
#include "string.hpp"
#include "types.hpp"



namespace m
{

uint16_t PORT_MIN = 0;
uint16_t PORT_MAX = 65535;



namespace
{
	/// Возвращает строку с именем файла и номером строки в нем для помещения ее в лог.
	std::string		get_log_src_path_string(const char* file, int line);

	/// Возвращает строку с текущим временем для помещения ее в лог.
	std::string		get_log_time_string(void);
}



namespace
{
	std::string get_log_src_path_string(const char* file, int line)
	{
		// Внимание!
		// Функция не должна выводить какие-либо сообщения, и использовать Format,
		// т. к. они сами будут вызывать ее.

		const int max_file_path_string_len = 20;
		const int max_file_path_string_size = max_file_path_string_len + 1;

		// На всякий случай выделим по-больше - кто знает, как целое число
		// представляется в текущей локали...
		const int max_line_string_len = 4 * 2;
		const int max_line_string_size = max_line_string_len + 1;

		char file_buf[max_file_path_string_size];
		char line_buf[max_line_string_size ];

		int diff = strlen(file) - max_file_path_string_len;

		if(diff > 0)
			file += diff;

		snprintf(file_buf, max_file_path_string_size, "%*s", max_file_path_string_len, file);
		snprintf(line_buf, max_line_string_size, "%04d", line);

		return std::string(file_buf) + ":" + line_buf;
	}



	std::string get_log_time_string(void)
	{
		// Внимание!
		// Функция не должна выводить какие-либо сообщения, и использовать Format,
		// т. к. они сами будут вызывать ее.

		const int time_string_len = 8;
		const int max_string_size = time_string_len + 4 + 1;
		char time_string_buf[max_string_size] = { '\0' };
		struct timeval tv;
		struct tm tm_val;

		if(gettimeofday(&tv, NULL) < 0)
			return _("[[gettimeofday time getting error]]");

		strftime(time_string_buf, max_string_size, "%H:%M:%S", localtime_r(&tv.tv_sec, &tm_val));
		snprintf(
			time_string_buf + time_string_len,
			max_string_size - time_string_len,
			".%03d", int(tv.tv_usec / 1000)
		);

		return time_string_buf;
	}
}



// Buffer -->
	namespace
	{
		size_t BUFFER_MAX_FILE_SIZE = 100 * 1000 * 1000;
		int IO_BUF_SIZE = 4048;
	}



	Buffer::Buffer(void)
	:
		buf(NULL),
		size(0),
		pos(0)
	{
	}



	Buffer::~Buffer(void)
	{
		::m::realloc(buf, 0);
	}



	char *Buffer::get_cur_ptr(void) const
	{
		return this->buf + this->pos;
	}



	char *Buffer::get_data(void) const
	{
		return this->buf;
	}



	Buffer &Buffer::increase_pos(const int inc)
	{
		if(this->pos + inc > this->size)
			MLIB_E(_("Buffer positioning error."));
		
		this->pos += inc;

		return *this;
	}



	void Buffer::load_file(const std::string &file_path) throw(m::Exception)
	{
		std::ifstream file;

		file.open(U2L(file_path).c_str(), std::ios::in | std::ios::binary);

		if(!file)
			M_THROW(EE(file));

		while(!file.eof())
		{
			if(this->get_size() >= BUFFER_MAX_FILE_SIZE)
				M_THROW(_("file too big"));

			this->reserve(IO_BUF_SIZE);

			file.read(this->get_cur_ptr(), IO_BUF_SIZE);
			if(file.bad())
				M_THROW(EE(file));

			*this += file.gcount();
		}

		file.close();

		if(file.bad())
			M_THROW(EE(file));
	}



	void Buffer::reserve(int reserve_size)
	{
		if(this->pos + reserve_size > this->size)
			this->resize(this->pos + reserve_size);
	}



	void Buffer::resize(int new_size)
	{
		this->buf = static_cast<char *>( m::realloc(buf, new_size) );
		this->size = new_size;
	}



	Buffer &Buffer::set_pos(int new_pos)
	{
		if(this->pos > this->size)
			MLIB_E(_("Buffer positioning error."));
		
		this->pos = new_pos;
		return *this;
	}



	Buffer &Buffer::operator=(int val)
	{
		return this->set_pos(val);
	}



	Buffer &Buffer::operator+(int val)
	{
		return this->increase_pos(val);
	}



	Buffer &Buffer::operator+=(int val)
	{
		return this->increase_pos(val);
	}
// Buffer <--



// Semaphore -->
	Semaphore::Semaphore(void)
	{
		MLIB_A(!sem_init(&this->semaphore, 0, 0));
	}



	Semaphore::~Semaphore(void)
	{
		MLIB_A(!sem_destroy(&this->semaphore));
	}



	void Semaphore::post(void)
	{
		MLIB_A(!sem_post(&this->semaphore));
	}



	void Semaphore::wait(void)
	{
		MLIB_A(!sem_wait(&this->semaphore));
	}
// Semaphore <--




std::string get_copyright_string(const std::string& author, const int start_year)
{
	time_t current_time = time(NULL);
	struct tm date;

	if(localtime_r(&current_time, &date))
	{
		m::tm_to_real_time(&date);

		if(date.tm_year > start_year)
			return __("Copyright (C) %1-%2 %3.", start_year, date.tm_year, author);
	}

	return __("Copyright (C) %1 %2.", start_year, author);
}



std::string	get_log_debug_prefix(const char* file, int line)
{
	return "(" + get_log_time_string() + ") (" + get_log_src_path_string(file, line) + "): ";
}



void* realloc(void *ptr, const size_t size)
{
	ptr = ::realloc(ptr, size);

	if(ptr == NULL && size)
		MLIB_E(__("Realloc failed: %1.", EE(errno)));

	return ptr;
}



void run(const std::string& cmd_name, const std::vector<std::string>& args) throw(m::Exception)
{
	switch(fork())
	{
		// Дочерний процесс
		case 0:
		{
			// Закрываем все открытые файловые дескрипторы -->
			{
				struct rlimit limits;

				if(getrlimit(RLIMIT_OFILE, &limits) < 0)
				{
					MLIB_SW(__("Running command '%1' failed. Can't get max opened file descriptors limit: %2.", cmd_name, EE(errno)));
					_exit(1);
				}

				for(int fd = STDERR_FILENO + 1; fd < static_cast<int>(limits.rlim_max); fd++)
					close(fd);
			}
			// Закрываем все открытые файловые дескрипторы <--

			char** argv;
			std::vector<std::string> argv_vector;

			// Формируем массив аргументов -->
			{
				argv_vector.push_back( U2L(cmd_name) );

				for(size_t i = 0; i < args.size(); i++)
					argv_vector.push_back( U2L(args[i]) );


				argv = new char*[argv_vector.size() + 1];

				for(size_t i = 0; i < argv_vector.size(); i++)
					argv[i] = const_cast<char *>( argv_vector[i].c_str() );

				argv[argv_vector.size()] = NULL;
			}
			// Формируем массив аргументов <--

			if(execvp(U2L(cmd_name).c_str(), argv) < 0)
				MLIB_SW(__("Running command '%1' failed: %2.", cmd_name, EE(errno)));

			_exit(1);
		}
		break;

		// Ошибка
		case -1:
			M_THROW(__("Can't fork process: %1.", EE(errno)));
			break;
		
		// Процесс успешно сдублирован
		default:
			break;
	}
}



void setenv(const std::string& name, const std::string& value, bool overwrite) throw(m::Exception)
{
	if(::setenv(U2L(name).c_str(), U2L(value).c_str(), overwrite))
		M_THROW(EE(errno));
}

}

