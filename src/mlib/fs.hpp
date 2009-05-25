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


#ifndef HEADER_MLIB_FS
	#define HEADER_MLIB_FS

	// Предоставляет функции для выполнения операций с файловой системой,
	// таких как копирование, удаление файлов, манипуляции с путями и т. п.

	#include <sys/stat.h>

	#include <map>
	#include <sstream>
	#include <string>

	#include <boost/filesystem/path.hpp>
	#include <boost/shared_ptr.hpp>

	#include <glibmm/miscutils.h>
	#include <glibmm/ustring.h>

	#include "errors.hpp"
	#include "types.hpp"



	namespace error_string
	{
		/// Возвращает строку с ошибкой, которая соответствует данному
		/// исключению.
		/// Используется функцией EE().
		std::string		get(const boost::fs::basic_filesystem_error<boost::fs::path>& error);
	}


	namespace m
	{
	namespace fs
	{
		/// Предоставляет некоторые общие функции при работе с
		/// конфигурационными файлами.
		namespace config
		{
			/// Фиксирует в файле config_path изменения, сделанные во временном
			/// файле, возвращенном функцией start_writing.
			///
			/// Предназначена для использования вместе со start_reading в целях
			/// обеспечения защиты от порчи конфигурационных файлов при крахе
			/// программы/системы во время их записи.
			void	end_writing(const std::string& config_path) throw(m::Exception);

			/// Возвращает путь к конфигурационному файлу или к временному файлу,
			/// в зависимости от того, была ли прервана работа программы в прошлый
			/// раз при записи конфигурационного файла.
			///
			/// Предназначена для использования вместе со start_writing в целях
			/// обеспечения защиты от порчи конфигурационных файлов при крахе
			/// программы/системы во время их записи.
			std::string start_reading(const std::string& config_path) throw(m::Exception);

			/// Возвращает путь к временному по отношению к config_path файлу.
			/// Программа должна сначала сохранить конфиг во временный файл,
			/// а затем выполнить end_writing, чтобы зафиксировать изменения
			/// в исходном файле.
			///
			/// Предназначена для использования вместе со start_reading в целях
			/// обеспечения защиты от порчи конфигурационных файлов при крахе
			/// программы/системы во время их записи.
			std::string	start_writing(const std::string& config_path) throw(m::Exception);
		}



		/// Предоставляет функции и классы для построения дерева файлов и
		/// каталогов и функции для осуществления над ним различных операций,
		/// таких как копирование и удаление (реальных файлов).
		namespace tree
		{
			class File;
			class Directory;

			typedef boost::shared_ptr<File> File_ptr;
			typedef boost::shared_ptr<const File> File_const_ptr;

			typedef boost::shared_ptr<Directory> Directory_ptr;
			typedef boost::shared_ptr<const Directory> Directory_const_ptr;

			typedef std::map<std::string, File_ptr> Children;



			class File
			{
				protected:
					enum Type { FILE, DIRECTORY };


				public:
							File(const std::string& name);
					virtual	~File(void) {};

				protected:
					File(Type type, const std::string& name);


				public:
					const std::string	name;

				private:
					const Type			type;


				public:
					/// Определяет, является ли файл обычным файлом или
					/// директорией.
					bool	is_file(void) const;
			};



			class Directory: public File
			{
				public:
					Directory(const std::string& name);


				public:
					Children	children;
			};



			/// Копирует все файлы дерева.
			void			cp(const std::string& src_prefix, const std::string& dest_prefix, const Directory_const_ptr& root) throw(m::Exception);

			/// Создает дерево файлов и каталогов из списка файлов.
			Directory_ptr	create(const std::vector<std::string>& files) throw(m::Exception);

			/// Удаляет все файлы дерева и директории, если они пусты.
			void			rm(const std::string& prefix, const Directory_const_ptr& root) throw(m::Exception);
		}



		/// Внимание! Очень медленная реализация.
		/// Сделана буквально на коленке.
		/// Если в дальнейшем планируется активное
		/// использование, то необходима доработка.
		class Path: public boost::fs::path
		{
			public:
				Path(void);
				Path(const char* path_string);
				Path(const std::string& path_string);
				Path(const Glib::ustring& path_string);
				Path(const boost::fs::path& path);


			private:
				boost::fs::path		path;


			public:
				/// Делает путь абсолютным.
				Path&			absolute(void) throw(m::Exception);

				/// Возвращает имя файла.
				/// Внимание! Для "../../../" возвращает "..".
				std::string		basename(void) const;

				/// Возвращает родительскую директорию.
				std::string		dirname(void) const;

				/// Возвращает абсолютный путь.
				Path			get_absolute(void) throw(m::Exception);

				/// Возвращает нормализованный путь.
				Path			get_normalized(void) const;

				/// Определяет, абсолютный ли это путь или нет.
				bool			is_absolute(void) const;

				/// Определяет, является ли путь простым, а именно, не содержит
				/// ли он в себе "../", "/./" и т. д.
				bool			is_simple(void) const;

				/// Нормализует путь.
				Path&			normalize(void);


			public:
				Path operator/(const char* path) const;
				Path operator/(const std::string& path) const;
				Path operator/(const Path& path) const;
				operator std::string() const;
				operator Glib::ustring() const;
		};

		/// Для работы Glib::ustring::compose.
		std::wostream&	operator<<(std::wostream& stream, const Path& path);



		/// Возвращается функцией unix_stat и содержит информацию
		/// о файле.
		class Stat
		{
			public:
				Stat(void);
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
				inline
				bool		is_blk(void);
				inline
				bool		is_chr(void);
				inline
				bool		is_dir(void);
				inline
				bool		is_fifo(void);
				inline
				bool		is_lnk(void);
				inline
				bool		is_reg(void);
				inline
				bool		is_sock(void);
		};



		/// Если требуется выделить статический буфер для размещения в нем пути
		/// к файлу, то размер буфера лучше задавать по этой константе.
		/// Предполагается, что, если это не ошибочная ситуация, длина пути не
		/// должена привысить данной величины.
		extern const size_t MAX_FILE_PATH_SIZE;



		/// Проверяет, имеет ли файл file_name расширение extension.
		bool			check_extension(const std::string& file_name, const std::string& extension);

		/// Копирует файл/ссылку (не директорию) из from_path в to_path.
		/// @param error_on_exists - если true, то в случае, когда такой файл
		/// уже существует, будет сгенерировано исключение.
		void			copy_file(const std::string& from_path, const std::string& to_path, bool error_on_exists = false) throw(m::Exception);

		/// Копирует все файлы, перечисленные в массиве files, получая их пути
		/// путем сложения src_root и элемента массива files. Причем пути к
		/// файлам могут начинаться с "/" так, как будто они находятся в корне
		/// файловой системы, которым является src_root.
		void			copy_files(const std::string& src_root, const std::string& dest_root, const std::vector<std::string>& files) throw(m::Exception);

		/// Рекурсивно копирует дерево директорий (аналогично cp -r).
		void			cp(const std::string& from, const std::string& to) throw(m::Exception);

		/// Возвращает абсолютный путь, если path таким не является.
		/// В случае, если абсолютный путь получить не удалось,
		/// возвращает path.
		std::string		get_abs_path_lazy(const std::string& path);

		/// Возвращает путь к домашней директории пользователя.
		/// Если его получить не удалось, аварийно завершает программу.
		std::string		get_user_home_path(void);

		/// Проверяет, существует файл или нет (ходит по ссылкам).
		/// Генерирует исключение, если родительской директории файла
		/// не существует, или не хватает прав на определение существования
		/// файла.
		/// @param error_on_missing_parent - если false, то исключение не
		/// генерируется, если родительской директории не существует.
		bool			is_exists(const std::string& path, bool error_on_missing_parent = true) throw(m::Exception);

		/// Проверяет, существует файл или нет (ходит по ссылкам).
		/// При ошибках не генерирует исключений, а просто возвращает false.
		bool			is_exists_without_errors(const std::string& path);

		/// Проверяет, существует файл или нет (не ходит по ссылкам).
		/// Генерирует исключение, если родительской директории файла
		/// не существует, или не хватает прав на определение существования
		/// файла.
		bool			is_lexists(const std::string& path) throw(m::Exception);

		/// Создает директорию, если ее еще не существует.
		/// @return - true, если директория была создана данным вызовом и
		/// false, если она уже существовала.
		bool			mkdir_if_not_exists(const std::string& path) throw(m::Exception);

		/// Создает директорию, если ее еще не существует.
		/// Если между проверкой существования директории
		/// и ее созданием другой процесс создаст эту же
		/// директорию, то не возвращает ошибки.
		void			mkdir_if_not_exists_with_race_conditions(const std::string& path) throw(m::Exception);

		/// Удаляет файл или дерево каталогов.
		/// Если такого файла не существует, или из-за прав
		/// доступа нет возможности определить, есть ли такой
		/// файл или нет, то генерирует исключение.
		void			rm(const std::string& path) throw(m::Exception);

		/// Удаляет все файлы, перечисленные в массиве files, получая их пути
		/// путем сложения root и элемента массива files. Причем пути к файлам
		/// могут начинаться с "/" так, как будто они находятся в корне
		/// файловой системы, которым является root. После удаления файлов
		/// также удаляются все пустые директории находящиеся между root и
		/// удаляемыми файлами.
		/// Если указанного файла не существует, то это не является ошибкой.
		void			rm_files_with_empty_dirs(const std::string& root, const std::vector<std::string>& files) throw(m::Exception);

		/// Удаляет файл или дерево каталогов, если они существуют.
		void			rm_if_exists(const std::string& path) throw(m::Exception);

		/// Удаляет расширение из имени файла.
		Glib::ustring	strip_extension(const Glib::ustring& file_name);

		/// Аналог системного fstat.
		Stat			unix_fstat(int fd) throw(m::Sys_exception);

		/// Аналог системного get_cwd.
		std::string		unix_get_cwd(void) throw(m::Sys_exception);

		/// Аналог системного lstat.
		Stat			unix_lstat(const std::string& path) throw(m::Sys_exception);

		/// Аналог системного mkdir.
		void			unix_mkdir(const std::string& path) throw(m::Sys_exception);

		/// Аналог системного open.
		int				unix_open(const std::string& path, int flags, mode_t mode = 0) throw(m::Sys_exception);

		/// Аналог системного read.
		ssize_t			unix_read(int fd, void* buf, size_t size, bool non_block = false) throw(m::Sys_exception);

		/// Аналог системного readlink.
		std::string		unix_readlink(const std::string& path) throw(m::Sys_exception);

		/// Аналог системного rename.
		void			unix_rename(const std::string& from, const std::string& to) throw(m::Sys_exception);

		/// Аналог системного rmdir.
		void			unix_rmdir(const std::string& path) throw(m::Sys_exception);

		/// Аналог системного stat.
		Stat			unix_stat(const std::string& path) throw(m::Sys_exception);

		/// Аналог системного symlink.
		void			unix_symlink(const std::string& old_path, const std::string& new_path) throw(m::Sys_exception);

		/// Аналог системного unlink.
		void			unix_unlink(const std::string& path) throw(m::Sys_exception);

		/// Аналог системного utime.
		void			unix_utime(const std::string& path, const Stat& file_stat);

		/// Аналог системного write.
		ssize_t			unix_write(int fd, void* buf, size_t size, bool non_block = false) throw(m::Sys_exception);
	}
	}

	#include "fs.hh"

	#ifdef MLIB_ENABLE_ALIASES
		using m::fs::Path;
	#endif

#endif

