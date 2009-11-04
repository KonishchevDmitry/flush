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


#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>


#include <cerrno>
#include <fstream>

#include <mlib/main.hpp>
#include <mlib/sys.hpp>

#include "misc.hpp"


namespace m {


uint16_t PORT_MIN = 1;
uint16_t PORT_MAX = 65535;



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



	Buffer::Buffer(const Buffer& buffer)
	{
		*this = buffer;
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



	void Buffer::load_file(const std::string &file_path)
	{
		try
		{
			std::ifstream file;
			file.exceptions(file.failbit | file.badbit);

			file.open(U2L(file_path).c_str(), file.in | file.binary);
			file.exceptions(file.badbit);

			while(!file.eof())
			{
				if(this->get_size() >= BUFFER_MAX_FILE_SIZE)
				{
					// strerror == "File too large"
					M_THROW_SYS(EFBIG);
				}

				this->reserve(IO_BUF_SIZE);

				file.read(this->get_cur_ptr(), IO_BUF_SIZE);
				*this += file.gcount();
			}

			file.close();
		}
		catch(std::ofstream::failure& e)
		{
			M_THROW_SYS(errno);
		}
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



	void Buffer::write_file(const std::string& file_path) const
	{
		try
		{
			std::ofstream file;

			file.exceptions(file.eofbit | file.failbit | file.badbit);
			file.open(U2L(file_path).c_str(), file.out | file.binary | file.trunc);
			file.write(this->get_data(), this->get_size());
			file.close();
		}
		catch(std::ofstream::failure& e)
		{
			M_THROW_SYS(errno);
		}
	}



	Buffer &Buffer::operator=(const Buffer& buffer)
	{
		if(this != &buffer)
		{
			this->resize(buffer.get_size());
			memcpy(this->get_data(), buffer.get_data(), buffer.get_size());
		}

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



// Connection -->
	Connection::Connection(void)
	{
		int pipes[2];
		long flags;

		if(pipe(pipes) < 0)
			MLIB_E(__("Can't create a pipe: %1.", strerror(errno)));

		this->read_fd = pipes[0];
		this->write_fd = pipes[1];

		if(
			( flags = fcntl(this->read_fd, F_GETFL) ) == -1 ||
			fcntl(this->read_fd, F_SETFL, flags | O_NONBLOCK) == -1
		)
			MLIB_E(__("Can't set flags for a pipe: %1.", strerror(errno)));
	}



	Connection::~Connection(void)
	{
		if(close(this->write_fd))
			MLIB_SW(__("Error while closing a pipe: %1.", strerror(errno)));

		if(close(this->read_fd))
			MLIB_SW(__("Error while closing a pipe: %1.", strerror(errno)));
	}



	void Connection::clear(void)
	{
		while(this->get())
			;
	}



	bool Connection::get(void)
	{
		char byte;
		ssize_t rval;

		do
			rval = read(this->read_fd, &byte, sizeof byte);
		while(rval < 0 && errno == EINTR);

		switch(rval)
		{
			case -1:
				if(errno == EAGAIN)
					return false;
				else
					MLIB_E(__("Can't read from a pipe: %1.", strerror(errno)));
				break;

			case 1:
				return true;
				break;

			default:
				MLIB_LE();
				break;
		}
	}



	int Connection::get_read_fd(void)
	{
		return this->read_fd;
	}



	void Connection::post(void)
	{
		char byte;
		ssize_t rval;

		do
			rval = write(this->write_fd, &byte, sizeof byte);
		while(rval < 0 && errno == EINTR);

		if(rval < 0)
			MLIB_E(__("Can't write to a pipe: %1.", strerror(errno)));
	}



	bool Connection::wait_for_with_owning(int fd, bool prioritize_fd)
	{
		Wait_entry entries[] = {
			Wait_entry(prioritize_fd ? fd : this->get_read_fd(), prioritize_fd ? false : true),
			Wait_entry(prioritize_fd ? this->get_read_fd() : fd, prioritize_fd ? true: false)
		};

		return wait_fds(entries, M_STATIC_ARRAY_SIZE(entries));
	}



	bool Connection::wait_two_with_owning(Connection& second)
	{
		Wait_entry entries[] = {
			Wait_entry(this->get_read_fd(), true),
			Wait_entry(second.get_read_fd(), true),
		};

		return !wait_fds(entries, M_STATIC_ARRAY_SIZE(entries));
	}
// Connection <--



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



void close_all_fds(void)
{
	struct rlimit limits;

	if(getrlimit(RLIMIT_OFILE, &limits))
		M_THROW(__("Can't get max opened files limit: %1.", EE()));

	for(int fd = STDERR_FILENO + 1; fd < (int) limits.rlim_max; fd++)
		close(fd);
}



std::string get_copyright_string(const std::string& author, const int start_year)
{
	time_t current_time = time(NULL);
	struct tm date;

	if(localtime_r(&current_time, &date))
	{
		sys::tm_to_real_time(&date);

		if(date.tm_year > start_year)
			return __("Copyright (C) %1-%2 %3.", start_year, date.tm_year, author);
	}

	return __("Copyright (C) %1 %2.", start_year, author);
}



void* realloc(void *ptr, const size_t size)
{
	ptr = ::realloc(ptr, size);

	if(ptr == NULL && size)
		MLIB_E(__("Realloc failed: %1.", EE()));

	return ptr;
}



void run(const std::string& cmd_name, const std::vector<std::string>& args)
{
	if(!sys::unix_fork())
	{
		// Дочерний процесс

		try
		{
			// Закрываем все открытые файловые дескрипторы
			// Генерирует m::Exception.
			m::close_all_fds();

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
				M_THROW(EE());
		}
		catch(m::Exception& e)
		{
			MLIB_SW(__("Running command '%1' failed: %2.", cmd_name, EE(e)));
			_exit(1);
		}

		MLIB_LE();
	}
}



size_t wait_fds(const Wait_entry* entries, size_t size)
{
	sys::File_holder epoll_fd;

	try
	{
		size_t i;

		epoll_fd.set(sys::unix_epoll_create());

		epoll_event event;
		event.events = EPOLLIN;

		for(i = 0; i < size; i++)
		{
			const Wait_entry& entry = entries[i];
			event.data.fd = entry.fd;
			sys::unix_epoll_ctl(epoll_fd.get(), EPOLL_CTL_ADD, entry.fd, &event);
		}
	}
	catch(m::Sys_exception& e)
	{
		MLIB_E(__("Error while creating epoll instance: %1.", EE(e)));
	}


	struct epoll_event events[size];

	do
	{
		size_t result_id;

		// Ожидаем прихода данных -->
			int events_num;

			try
			{
				events_num = sys::unix_epoll_wait(epoll_fd.get(), events, M_STATIC_ARRAY_SIZE(events), -1);
			}
			catch(m::Sys_exception& e)
			{
				MLIB_E(__("Error while using epoll instance: %1.", EE(e)));
			}

			MLIB_A(events_num);
		// Ожидаем прихода данных <--

		// Определяем в порядке приоритетности, какой дескриптор сработал
		// -->
		{
			bool gotten = false;

			for(size_t i = 0; !gotten && i < size; i++)
			{
				for(int k = 0; !gotten && k < events_num; k++)
				{
					if(events[0].data.fd == entries[i].fd)
					{
						gotten = true;
						result_id = i;
					}
				}
			}

			MLIB_A(gotten);
		}
		// <--

		// Считываем данные, из дескриптора, если это необходимо
		// -->
			if(entries[result_id].own)
			{
				char byte;
				ssize_t rval;

				do
					rval = read(entries[result_id].fd, &byte, sizeof byte);
				while(rval < 0 && errno == EINTR);

				switch(rval)
				{
					case -1:
						if(errno != EAGAIN)
							MLIB_E(__("Can't read data from a file descriptor: %1.", strerror(errno)));
						break;

					case 1:
						return result_id;
						break;

					default:
						MLIB_LE();
						break;
				}
			}
			else
				return result_id;
		// <--
	}
	while(true);
}


}

