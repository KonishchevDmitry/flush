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


#ifndef HEADER_MLIB_SYS
#define HEADER_MLIB_SYS


#include <sys/stat.h>

#include <boost/noncopyable.hpp>

#include <mlib/main.hpp>

struct epoll_event;
struct tm;


namespace m { namespace sys {


/// Предназначен для автоматического закрытия файловых дескрипторов.
class File_holder: public boost::noncopyable
{
	public:
		inline explicit File_holder(void);

		inline explicit File_holder(int fd);

		inline ~File_holder(void);


	private:
		int	fd;


	public:
		/// @throw - m::Sys_exception.
		inline void	close(void);

		inline int	get(void) const;

		inline int	reset(void);

		/// @throw - m::Sys_exception.
		inline void	set(int fd);
};



/// Возвращается функцией unix_stat() и содержит информацию
/// о файле.
class Stat
{
	public:
		Stat(void) {};
		Stat(const struct stat& c_stat);


	public:
		/// ID of device containing file.
		dev_t		dev;

		/// Inode number.
		ino_t		ino;

		/// Protection.
		mode_t		mode;

		/// Number of hard links.
		nlink_t		nlink;

		/// User ID of owner.
		uid_t		uid;

		/// Group ID of owner.
		gid_t		gid;

		/// Device ID (if special file).
		dev_t		rdev;

		/// Total size, in bytes.
		off_t		size;

		/// Blocksize for file system I/O.
		blksize_t	blksize;

		/// Number of blocks allocated.
		blkcnt_t	blocks;

		/// Time of last access.
		time_t		atime;

		/// Time of last modification.
		time_t		mtime;

		/// Time of last status change.
		time_t		ctime;


	public:
		bool		is_blk(void);
		bool		is_chr(void);
		bool		is_dir(void);
		bool		is_fifo(void);
		bool		is_lnk(void);
		bool		is_reg(void);
		bool		is_sock(void);
};



/// Максимальный размер пути к файлу.
extern const size_t FILE_PATH_MAX_SIZE;


/// Переводит дескриптор в неблокирующий режим.
/// @throw - m::Sys_exception.
void				set_non_block(int fd);

/// Аналог системного setenv().
/// @throw - m::Exception.
void				setenv(const std::string& name, const std::string& value, bool overwrite);

/// Преобразовывает время в структуре tm в реальное время
/// (они различаются по годам).
void				tm_to_real_time(struct tm* date);

/// Аналог системного close().
/// @throw - m::Exception.
void				unix_close(int fd);

/// Аналог системного dup().
/// @throw - m::Exception.
int					unix_dup(int fd);

/// Аналог системного dup().
/// @throw - m::Exception.
void				unix_dup(int oldfd, int newfd);

/// Аналог системного epoll_ctl().
/// @throw - m::Sys_exception.
void				unix_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

/// Аналог системного epoll_create().
/// @throw - m::Sys_exception.
int					unix_epoll_create(void);

/// Аналог системного epoll_wait().
/// @throw - m::Sys_exception.
int					unix_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

/// Аналог системного unix_execvp;
/// @throw - m::Sys_exception.
void				unix_execvp(const std::string& command, const std::vector<std::string>& args);

/// Аналог системного fork().
/// @throw - m::Exception.
pid_t				unix_fork(void);

/// Аналог системного fstat.
/// @throw - m::Sys_exception.
Stat				unix_fstat(int fd);

/// Аналог системного get_cwd.
/// @throw - m::Sys_exception.
std::string			unix_get_cwd(void);

/// Аналог системного lstat.
/// @throw - m::Sys_exception.
Stat				unix_lstat(const std::string& path);

/// Аналог системного mkdir.
/// @throw - m::Sys_exception.
void				unix_mkdir(const std::string& path);

/// Аналог системного open.
/// @throw - m::Sys_exception.
int					unix_open(const std::string& path, int flags, mode_t mode = 0);

/// Аналог системного pipe().
/// @throw - m::Exception.
std::pair<int, int>	unix_pipe(void);

/// Аналог системного read.
/// @throw - m::Sys_exception.
ssize_t				unix_read(int fd, void* buf, size_t size, bool non_block = false);

/// Аналог системного readlink.
/// @throw - m::Sys_exception.
std::string			unix_readlink(const std::string& path);

/// Аналог системного rename.
/// @throw - m::Sys_exception.
void				unix_rename(const std::string& from, const std::string& to);

/// Аналог системного rmdir.
/// @throw - m::Sys_exception.
void				unix_rmdir(const std::string& path);

/// Аналог системного stat.
/// @throw - m::Sys_exception.
Stat				unix_stat(const std::string& path);

/// Аналог системного symlink.
/// @throw - m::Sys_exception.
void				unix_symlink(const std::string& old_path, const std::string& new_path);

/// Аналог системного unlink.
/// @throw - m::Sys_exception.
void				unix_unlink(const std::string& path);

/// Аналог системного utime.
void				unix_utime(const std::string& path, const Stat& file_stat);

/// Аналог системного write.
/// @throw - m::Sys_exception.
ssize_t				unix_write(int fd, const void* buf, size_t size, bool non_block = false);


}}

#include "sys.hh"

#endif

