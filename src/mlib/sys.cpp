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

#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdint.h>
#include <unistd.h>
#include <utime.h>

#include <cerrno>
#include <ctime>

#include <algorithm>
#include <fstream>

#include <boost/scoped_array.hpp>

#include <mlib/main.hpp>

#include "sys.hpp"


namespace m { namespace sys {

const size_t FILE_PATH_MAX_SIZE = 1024;



// Stat <--
	Stat::Stat(const struct stat& c_stat)
	:
		dev(c_stat.st_dev),
		ino(c_stat.st_ino),
		mode(c_stat.st_mode),
		nlink(c_stat.st_nlink),
		uid(c_stat.st_uid),
		gid(c_stat.st_gid),
		rdev(c_stat.st_rdev),
		size(c_stat.st_size),
		blksize(c_stat.st_blksize),
		blocks(c_stat.st_blocks),
		atime(c_stat.st_atime),
		mtime(c_stat.st_mtime),
		ctime(c_stat.st_ctime)
	{
	}



	bool Stat::is_blk(void) { return S_ISBLK(this->mode); }
	bool Stat::is_chr(void) { return S_ISCHR(this->mode); }
	bool Stat::is_dir(void) { return S_ISDIR(this->mode); }
	bool Stat::is_fifo(void) { return S_ISFIFO(this->mode); }
	bool Stat::is_lnk(void) { return S_ISLNK(this->mode); }
	bool Stat::is_reg(void) { return S_ISREG(this->mode); }
	bool Stat::is_sock(void) { return S_ISSOCK(this->mode); }
// Stat <--



void set_non_block(int fd)
{
	long flags;

	if(
		( flags = fcntl(fd, F_GETFL) ) == -1 ||
		fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1
	)
		M_THROW_SYS(errno);
}



void setenv(const std::string& name, const std::string& value, bool overwrite)
{
	if(::setenv(U2L(name).c_str(), U2L(value).c_str(), overwrite))
		M_THROW(EE());
}



void tm_to_real_time(struct tm* date)
{
	date->tm_year += 1900;
}



void unix_close(int fd)
{
	int rval;

	do
		rval = ::close(fd);
	while(rval && errno == EINTR);

	if(rval)
		M_THROW_SYS(errno);
}



int unix_dup(int fd)
{
	int new_fd = dup(fd);

	if(new_fd == -1)
		M_THROW(__("Can't duplicate a file descriptor: %1.", EE()));
	else
		return new_fd;
}



void unix_dup(int oldfd, int newfd)
{
	if(dup2(oldfd, newfd) == -1)
		M_THROW(__("Can't duplicate a file descriptor: %1.", EE()));
}



void unix_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	if(epoll_ctl(epfd, op, fd, event))
		M_THROW_SYS(errno);
}



int unix_epoll_create(void)
{
	int fd = epoll_create(0);
	if(fd < 0)
		M_THROW_SYS(errno);
	else
		return fd;
}



int unix_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int rval;

	do
		rval = epoll_wait(epfd, events, maxevents, timeout);
	while(rval < 0 && errno == EINTR);

	if(rval < 0)
		M_THROW_SYS(errno);
	else
		return rval;
}



void unix_execvp(const std::string& command, const std::vector<std::string>& args)
{
	std::vector<std::string> argv_strings;
	argv_strings.reserve(args.size() + 1);

	argv_strings.push_back(U2L(command));
	M_FOR_CONST_IT(args, it)
		argv_strings.push_back(U2L(*it));

	boost::scoped_array<char*> argv(new char*[argv_strings.size() + 1]);

	{
		char** arg = argv.get();

		M_FOR_CONST_IT(argv_strings, it)
			*arg++ = const_cast<char*>(it->c_str());

		*arg = NULL;
	}

	if(execvp(U2L(command).c_str(), argv.get()) < 0)
		M_THROW_SYS(errno);
}



pid_t unix_fork(void)
{
	pid_t pid = fork();

	if(pid == -1)
		M_THROW(__("Can't fork the process: %1.", EE()));
	else
		return pid;
}



Stat unix_fstat(int fd)
{
	struct stat stat_buf;

	if(fstat(fd, &stat_buf))
		M_THROW_SYS(errno);

	return stat_buf;
}



std::string unix_get_cwd(void)
{
	char *cwd;

	if( (cwd = get_current_dir_name()) )
	{
		std::string cwd_string = cwd;
		free(cwd);
		return L2U(cwd_string);
	}
	else
		M_THROW_SYS(errno);
}



Stat unix_lstat(const std::string& path)
{
	struct stat stat_buf;

	if(lstat(U2L(path).c_str(), &stat_buf))
		M_THROW_SYS(errno);

	return stat_buf;
}



void unix_mkdir(const std::string& path)
{
	if(mkdir(U2L(path).c_str(), 0777))
		M_THROW_SYS(errno);
}



int unix_open(const std::string& path, int flags, mode_t mode)
{
	int fd;

	if( (fd = open(U2L(path).c_str(), flags, mode)) >= 0 )
		return fd;
	else
		M_THROW_SYS(errno);
}



std::pair<int, int> unix_pipe(void)
{
	int fds[2];

	if(pipe(fds) == -1)
		M_THROW(__("Can't create a pipe: %1.", EE()));
	else
		return std::pair<int, int>(fds[0], fds[1]);
}



std::string unix_readlink(const std::string& path)
{
	char target_path_buf[FILE_PATH_MAX_SIZE];
	int written_bytes;

	written_bytes = readlink(U2L(path).c_str(), target_path_buf, sizeof target_path_buf);

	if(written_bytes >= int(sizeof target_path_buf))
		M_THROW_SYS(ENAMETOOLONG, _("too big link target path"));
	else if(written_bytes < 0)
		M_THROW_SYS(errno);

	target_path_buf[written_bytes] = '\0';

	return L2U(target_path_buf);
}



ssize_t unix_read(int fd, void* buf, size_t size, bool non_block)
{
	ssize_t readed_bytes;

	while(1)
	{
		if( (readed_bytes = read(fd, buf, size)) < 0 )
		{
			if(errno == EWOULDBLOCK && non_block)
				return 0;
			else if(errno == EINTR)
				continue;
			else
				M_THROW_SYS(errno);
		}
		else
		{
			errno = 0;
			return readed_bytes;
		}
	}
}



void unix_rename(const std::string& from, const std::string& to)
{
	if(rename(U2L(from).c_str(), U2L(to).c_str()))
		M_THROW_SYS(errno);
}



void unix_rmdir(const std::string& path)
{
	if(rmdir(U2L(path).c_str()))
		M_THROW_SYS(errno);
}



Stat unix_stat(const std::string& path)
{
	struct stat stat_buf;

	if(stat(U2L(path).c_str(), &stat_buf))
		M_THROW_SYS(errno);

	return stat_buf;
}



void unix_symlink(const std::string& old_path, const std::string& new_path)
{
	if(symlink(U2L(old_path).c_str(), U2L(new_path).c_str()) < 0)
		M_THROW_SYS(errno);
}



void unix_unlink(const std::string& path)
{
	if(unlink(U2L(path).c_str()))
		M_THROW_SYS(errno);
}



void unix_utime(const std::string& path, const Stat& file_stat)
{
	struct utimbuf time_buf;
	time_buf.actime = file_stat.atime;
	time_buf.modtime = file_stat.mtime;

	if(::utime(U2L(path).c_str(), &time_buf))
		M_THROW_SYS(errno);
}



ssize_t unix_write(int fd, const void* buf, size_t size, bool non_block)
{
	ssize_t written_bytes;

	while(1)
	{
		if( (written_bytes = write(fd, buf, size)) < 0 )
		{
			if(errno == EWOULDBLOCK && non_block)
				return 0;
			else if(errno == EINTR)
				continue;
			else
				M_THROW_SYS(errno);
		}
		else
			return written_bytes;
	}
}


}}

