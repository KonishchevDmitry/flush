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


// Предоставляет функции для выполнения операций с файловой системой,
// таких как копирование, удаление файлов, манипуляции с путями и т. п.

#ifndef HEADER_MLIB_FS
#define HEADER_MLIB_FS

#include <map>

#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <mlib/main.hpp>



namespace m {

namespace fs {

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
	///
	/// @throw - m::Exception.
	void	end_writing(const std::string& config_path);

	/// Возвращает путь к конфигурационному файлу или к временному файлу,
	/// в зависимости от того, была ли прервана работа программы в прошлый
	/// раз при записи конфигурационного файла.
	///
	/// Предназначена для использования вместе со start_writing в целях
	/// обеспечения защиты от порчи конфигурационных файлов при крахе
	/// программы/системы во время их записи.
	///
	/// @throw - m::Exception.
	std::string start_reading(const std::string& config_path);

	/// Возвращает путь к временному по отношению к config_path файлу.
	/// Программа должна сначала сохранить конфиг во временный файл,
	/// а затем выполнить end_writing, чтобы зафиксировать изменения
	/// в исходном файле.
	///
	/// Предназначена для использования вместе со start_reading в целях
	/// обеспечения защиты от порчи конфигурационных файлов при крахе
	/// программы/системы во время их записи.
	///
	/// @throw - m::Exception.
	std::string	start_writing(const std::string& config_path);
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
	/// @throw - m::Exception.
	void			cp(const std::string& src_prefix, const std::string& dest_prefix, const Directory_const_ptr& root);

	/// Создает дерево файлов и каталогов из списка файлов.
	/// @throw - m::Exception.
	Directory_ptr	create(const std::vector<std::string>& files);

	/// Удаляет все файлы дерева и директории, если они пусты.
	/// @throw - m::Exception.
	void			rm(const std::string& prefix, const Directory_const_ptr& root);
}



/// Внимание! Очень медленная реализация.
/// Сделана буквально на коленке.
/// Если в дальнейшем планируется активное
/// использование, то необходима доработка.
class Path: public boost::filesystem::path
{
	public:
		Path(void);
		Path(const char* path_string);
		Path(const std::string& path_string);
		Path(const Glib::ustring& path_string);
		Path(const boost::filesystem::path& path);


	private:
		boost::filesystem::path		path;


	public:
		/// Делает путь абсолютным.
		/// @throw - m::Exception.
		Path&			absolute(void);

		/// Возвращает имя файла.
		/// Внимание! Для "../../../" возвращает "..".
		std::string		basename(void) const;

		/// Возвращает родительскую директорию.
		std::string		dirname(void) const;

		/// Возвращает абсолютный путь.
		/// @throw - m::Exception.
		Path			get_absolute(void);

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
/// @throw - m::Exception.
void			copy_file(const std::string& from_path, const std::string& to_path, bool error_on_exists = false);

/// Копирует все файлы, перечисленные в массиве files, получая их пути
/// путем сложения src_root и элемента массива files. Причем пути к
/// файлам могут начинаться с "/" так, как будто они находятся в корне
/// файловой системы, которым является src_root.
/// @throw - m::Exception.
void			copy_files(const std::string& src_root, const std::string& dest_root, const std::vector<std::string>& files);

/// Рекурсивно копирует дерево директорий (аналогично cp -r).
/// @throw - m::Exception.
void			cp(const std::string& from, const std::string& to);

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
/// @throw - m::Exception.
bool			is_exists(const std::string& path, bool error_on_missing_parent = true);

/// Проверяет, существует файл или нет (ходит по ссылкам).
/// При ошибках не генерирует исключений, а просто возвращает false.
bool			is_exists_without_errors(const std::string& path);

/// Проверяет, существует файл или нет (не ходит по ссылкам).
/// Генерирует исключение, если родительской директории файла
/// не существует, или не хватает прав на определение существования
/// файла.
/// @throw - m::Exception.
bool			is_lexists(const std::string& path);

/// Создает директорию, если ее еще не существует.
/// @throw - m::Exception.
/// @return - true, если директория была создана данным вызовом и
/// false, если она уже существовала.
bool			mkdir_if_not_exists(const std::string& path);

/// Создает директорию, если ее еще не существует.
/// Если между проверкой существования директории
/// и ее созданием другой процесс создаст эту же
/// директорию, то не возвращает ошибки.
/// @throw - m::Exception.
void			mkdir_if_not_exists_with_race_conditions(const std::string& path);

/// Удаляет файл или дерево каталогов.
/// Если такого файла не существует, или из-за прав
/// доступа нет возможности определить, есть ли такой
/// файл или нет, то генерирует исключение.
/// @throw - m::Exception.
void			rm(const std::string& path);

/// Удаляет все файлы, перечисленные в массиве files, получая их пути
/// путем сложения root и элемента массива files. Причем пути к файлам
/// могут начинаться с "/" так, как будто они находятся в корне
/// файловой системы, которым является root. После удаления файлов
/// также удаляются все пустые директории находящиеся между root и
/// удаляемыми файлами.
/// Если указанного файла не существует, то это не является ошибкой.
/// @throw - m::Exception.
void			rm_files_with_empty_dirs(const std::string& root, const std::vector<std::string>& files);

/// Удаляет файл или дерево каталогов, если они существуют.
/// @throw - m::Exception.
void			rm_if_exists(const std::string& path);

/// Удаляет расширение из имени файла.
Glib::ustring	strip_extension(const Glib::ustring& file_name);

/// Сбрасывает все данные файла path, которые находятся в буферах ядра,
/// на диск.
/// @throw - m::Exception.
void			sync_file(const std::string& path);

}


std::string		EE(const boost::filesystem::filesystem_error& error);


namespace aliases
{
	using m::fs::Path;
	#warning
	using m::EE;
}

}

#endif

