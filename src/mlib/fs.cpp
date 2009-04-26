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

#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/version.hpp>

#include "fs.hpp"
#include "messages.hpp"
#include "misc.hpp"
#include "string.hpp"

#define M_FILE_PATH_MAX_SIZE 1024



namespace m
{
namespace error_string
{
	std::string get(const boost::fs::basic_filesystem_error<boost::fs::path>& error)
	{
		#if M_BOOST_GET_VERSION() < M_GET_VERSION(1, 35, 0)
			M_LIBRARY_COMPATIBILITY
			return strerror(error.system_error());
		#else
			return strerror(error.code().value());
		#endif
	}
}
}



namespace m
{
namespace fs
{
namespace config
{
	namespace
	{
		/// Возвращает путь ко временному файлу.
		inline
		std::string get_temp_config_file_path(const std::string& config_path);



		std::string get_temp_config_file_path(const std::string& config_path)
		{
			return config_path + ".new";
		}
	}



	std::string start_reading(const std::string& config_path) throw(m::Exception)
	{
		if(m::fs::is_exists(config_path))
			return config_path;
		else if(m::fs::is_exists(get_temp_config_file_path(config_path)))
			return get_temp_config_file_path(config_path);
		else
			return config_path;
	}



	std::string start_writing(const std::string& config_path) throw(m::Exception)
	{
		if(!m::fs::is_exists(config_path) && m::fs::is_exists(get_temp_config_file_path(config_path)))
			m::fs::cp(get_temp_config_file_path(config_path), config_path);

		return get_temp_config_file_path(config_path);
	}



	void end_writing(const std::string& config_path) throw(m::Exception)
	{
		if(m::fs::is_exists(config_path))
			m::fs::rm(config_path);

		m::fs::unix_rename(get_temp_config_file_path(config_path), config_path);
	}
}
}
}



namespace m
{
namespace fs
{
namespace tree
{
// File -->
	File::File(const std::string& name)
	:
		name(name),
		type(FILE)
	{
	}



	File::File(Type type, const std::string& name)
	:
		name(name),
		type(type)
	{
	}



	bool File::is_file(void) const
	{
		return this->type == FILE;
	}
// File <--



	Directory::Directory(const std::string& name)
	:
		File(DIRECTORY, name)
	{
	}



	void cp(const std::string& src_prefix, const std::string& dest_prefix, const Directory_const_ptr& root) throw(m::Exception)
	{
		Errors_pool errors;

		File_const_ptr file;
		std::string src_path;
		std::string dest_path;

		Children::const_iterator child_it = root->children.begin();
		Children::const_iterator child_end_it = root->children.end();


		for(; child_it != child_end_it; ++child_it)
		{
			file = child_it->second;

			src_path = src_prefix + child_it->first;
			dest_path = dest_prefix + child_it->first;


			if(file->is_file())
			{
				try
				{
					m::fs::copy_file(src_path, dest_path, true);
				}
				catch(m::Exception& e)
				{
					errors += __("Can't copy file '%1' to '%2': %3.", src_path, dest_path, EE(e));
				}
			}
			else
			{
				try
				{
					bool new_dir_created = mkdir_if_not_exists(dest_path);

					try
					{
						m::fs::tree::cp(src_path + "/", dest_path + "/", boost::dynamic_pointer_cast<const Directory>(file));
					}
					catch(m::Exception& e)
					{
						errors += EE(e);
					}

					try
					{
						// Копируем время модификации с исходной директории -->
							if(new_dir_created)
							{
								Stat dir_stat;

								try
								{
									dir_stat = unix_stat(src_path);
								}
								catch(m::Exception& e)
								{
									errors +=
										__(
											"Error while changing directory '%1' access and modification times. Can't stat directory '%2': %3.",
											dest_path, src_path, EE(e)
										);
									M_THROW_EMPTY();
								}

								try
								{
									unix_utime(dest_path, dir_stat);
								}
								catch(m::Exception& e)
								{
									errors +=
										__(
											"Error while changing directory '%1' access and modification times: %2.",
											dest_path, EE(e)
										);
									M_THROW_EMPTY();
								}
							}
						// Копируем время модификации с исходной директории <--
					}
					catch(m::Exception)
					{
						// Используется только для выхода из участков кода
					}
				}
				catch(m::Exception& e)
				{
					errors += __("Can't create directory '%1': %2.", dest_path, EE(e));
				}
			}
		}

		errors.throw_if_exists();
	}



	Directory_ptr create(const std::vector<std::string>& files) throw(m::Exception)
	{
		Directory_ptr root( new Directory("root") );


		for(size_t file_id = 0; file_id < files.size(); file_id++)
		{
			Path file_path = Path(files[file_id]).normalize();

			// Преобразовываем из абсолютного пути в относительный -->
			{
				std::string file_rel_path_string = file_path.string();

				if(!file_rel_path_string.empty() && file_rel_path_string.substr(0, 1) == "/")
					file_path = file_rel_path_string.substr(1);
			}
			// Преобразовываем из абсолютного пути в относительный <--

			// Какие-либо нестандартные пути вроде "/" и "../file" не
			// поддерживаем.
			if(!file_path.is_simple())
				M_THROW(__("Invalid file path '%1'.", files[file_id]));

			// Строим дерево для текущего файла -->
			{
				Directory_ptr cur_dir = root;
				Children::iterator child_it;

				size_t cur_pos = 0;
				size_t slash_pos;
				bool is_file;

				std::string file_name;
				Glib::ustring path_string = file_path.string();

				do
				{
					slash_pos = path_string.find('/', cur_pos);

					if(slash_pos == Glib::ustring::npos)
					{
						is_file = true;
						file_name = path_string.substr(cur_pos);
					}
					else
					{
						is_file = false;
						file_name = path_string.substr(cur_pos, slash_pos - cur_pos);
					}

					child_it = cur_dir->children.find(file_name);

					// Такого файла/директории в дереве еще нет
					if(child_it == cur_dir->children.end())
					{
						if(is_file)
							cur_dir->children[file_name] = File_ptr( new File(file_name) );
						else
						{
							Directory_ptr new_dir( new Directory(file_name) );
							cur_dir->children[file_name] = new_dir;
							cur_dir = new_dir;
						}
					}
					// Если на этом месте уже существует файл или директория
					else
					{
						if(is_file)
						{
							M_THROW(__(
								"Invalid file path '%1' - file with such path is already exists.",
								files[file_id]
							));
						}
						else
						{
							if(child_it->second->is_file())
							{
								M_THROW(__(
									"Invalid file path '%1' - file '%2' is already exits.",
									files[file_id], ( "/" + path_string.substr(0, slash_pos) )
								));
							}
							else
								cur_dir = boost::dynamic_pointer_cast<Directory>(child_it->second);
						}
					}

					cur_pos = slash_pos + 1;
				}
				while(!is_file);
			}
			// Строим дерево для текущего файла <--
		}

		return root;
	}



	void rm(const std::string& prefix, const Directory_const_ptr& root) throw(m::Exception)
	{
		Errors_pool errors;

		File_const_ptr file;
		std::string file_path;

		Children::const_iterator child_it = root->children.begin();
		Children::const_iterator child_end_it = root->children.end();


		for(; child_it != child_end_it; ++child_it)
		{
			file = child_it->second;
			file_path = prefix + child_it->first;

			try
			{
				if(!m::fs::is_exists(file_path, false))
					continue;
			}
			catch(m::Exception& e)
			{
				M_THROW(__("Can't stat '%1': %2.", file_path, EE(e)));
				continue;
			}

			if(file->is_file())
			{
				MLIB_D(_C("Removing file '%1'...", file_path));

				try
				{
					unix_unlink(file_path);
				}
				catch(m::Exception& e)
				{
					errors += __("Can't remove file '%1': %2.", file_path, EE(e));
				}
			}
			else
			{
				MLIB_D(_C("Removing directory '%1'...", file_path));

				try
				{
					m::fs::tree::rm(file_path + "/", boost::dynamic_pointer_cast<const Directory>(file));
				}
				catch(m::Exception& e)
				{
					errors += EE(e);
				}

				try
				{
					// Если директория пуста
					if(boost::fs::directory_iterator(U2L(file_path)) == boost::fs::directory_iterator())
						unix_rmdir(file_path);
				}
				catch(boost::filesystem::filesystem_error)
				{
					errors += __("Can't read directory '%1': %2.", file_path, EE(errno));
				}
				catch(m::Exception& e)
				{
					errors += __("Can't remove directory '%1': %2.", file_path, EE(e));
				}
			}
		}

		errors.throw_if_exists();
	}
}
}
}



namespace
{
	/// Базовая для is_exists и is_lexists функция.
	bool is_exists_base(const std::string& path, bool link_mode, bool error_on_missing_parent) throw(m::Exception);



	bool is_exists_base(const std::string& path, bool link_mode, bool error_on_missing_parent) throw(m::Exception)
	{
		struct stat stat_buf;

		if( !( link_mode ? ::lstat(U2L(path).c_str(), &stat_buf) : ::stat(U2L(path).c_str(), &stat_buf) ) )
			return 1;
		// Не существует либо самого файла,
		// либо одной из его родительских
		// директорий.
		else if(errno == ENOENT)
		{
			if(error_on_missing_parent)
			{
				std::string parent_dir_path = Path(path).dirname();

				if(!stat(U2L(parent_dir_path).c_str(), &stat_buf))
					return 0;
				else if(errno == ENOENT)
					M_THROW(_("parent directory is not exists"));
				else
					M_THROW(EE(errno));
			}
			else
				return 0;
		}
		// Любая другая ошибка - не хватает
		// прав доступа, родительская директория
		// (судя по пути) не является директорией
		// и т. п.
		else
			M_THROW(EE(errno));
	}
}



namespace m
{
namespace fs
{

// Path -->
	Path::Path(void)
	{
	}



	Path::Path(const char* path_string)
	: boost::fs::path(path_string)
	{
	}



	Path::Path(const std::string& path_string)
	: boost::fs::path(path_string)
	{
	}



	Path::Path(const Glib::ustring& path_string)
	: boost::fs::path(path_string)
	{
	}



	Path::Path(const boost::fs::path& path)
	: boost::fs::path(path)
	{
	}



	Path& Path::absolute(void) throw(m::Exception)
	{
		if(!this->is_absolute())
			*this = Path(unix_get_cwd() + "/" + this->string()).normalize();

		return *this;
	}



	std::string Path::basename(void) const
	{
		Glib::ustring path = this->get_normalized();
		size_t pos = path.rfind('/');

		if(pos == Glib::ustring::npos)
			return path;
		else
		{
			// "/"
			if(path.size() == 1)
				return path;
			else
				return path.substr(pos + 1);
		}
	}



	std::string Path::dirname(void) const
	{
		return (*this / "..").get_normalized();
	}



	Path Path::get_absolute(void) throw(m::Exception)
	{
		return Path(*this).absolute();
	}



	Path Path::get_normalized(void) const
	{
		return Path(*this).normalize();
	}



	bool Path::is_absolute(void) const
	{
		if(this->begin() != this->end() && *this->begin() == "/")
			return true;
		else
			return false;
	}



	bool Path::is_simple(void) const
	{
		Glib::ustring string = this->string();

		if(string.empty())
			return false;

		if(string[0] == '/')
			string = string.substr(1);

		if(!string.empty() && string[string.size() - 1] == '/')
			return false;

		if(
			( string.size() >= 2 && ( string.substr(0, 2) == "./" || string.substr(string.size() - 2, 2) == "/." ) ) ||
			( string.size() >= 3 && ( string.substr(0, 2) == "../" || string.substr(string.size() - 2, 2) == "/.." ) ) ||
			( string.find("/./") != Glib::ustring::npos || string.find("/../") != Glib::ustring::npos )
		)
			return false;

		return true;
	}



	Path& Path::normalize(void)
	{
		std::string new_path;

		// В некоторых системах пути, начинающиеся с двух слэшей, означают
		// super root и поэтому boost обрабатывает их не так, как пути с
		// одинарным слэшем. Мы не ставим своей целью поддержку таких систем и
		// поэтому превращаем двойной слэш в одинарный.
		// -->
		{
			Glib::ustring cur_path = *this;

			if(cur_path.size() >= 2 && cur_path.substr(0, 2) == "//")
				*this = cur_path.substr(1);
		}
		// <--

		if(this->begin() != this->end())
		{
			bool is_abs_path = false;
			std::vector<std::string> path_components;

			{
				boost::fs::path::iterator it = this->begin();

				if(*it == "/")
				{
					is_abs_path = true;
					it++;
				}

				for(; it != this->end(); it++)
					if(*it != "/" && *it != "." && *it != "")
						path_components.push_back(*it);
			}

			{
				for(size_t i = 0; i < path_components.size(); i++)
				{
					if(path_components[i] == "..")
					{
						if(i == 0)
						{
							if(is_abs_path)
								path_components.erase(path_components.begin() + i--);
							else
								;
						}
						else
						{
							if(path_components[i - 1] == "..")
								;
							else
							{
								path_components.erase(path_components.begin() + i--);
								path_components.erase(path_components.begin() + i--);
							}
						}
					}
					else
						;
				}
			}

			// Собираем все в одну кучу -->
			{
				size_t components_num = path_components.size();

				if(is_abs_path)
					new_path = "/";

				for(size_t i = 0; i < components_num; i++)
				{
					if(i != components_num - 1)
						new_path += path_components[i] + "/";
					else
						new_path += path_components[i];
				}
			}
			// Собираем все в одну кучу <--
		}

		if(new_path == "" && *this != "")
			new_path = ".";

		*this = new_path;

		return *this;
	}



	Path Path::operator/(const char* path) const
	{
		return Path(*this) /= path;
	}



	Path Path::operator/(const std::string& path) const
	{
		return Path(*this) /= path;
	}



	Path Path::operator/(const Path& path) const
	{
		return Path(*this) /= path;
	}



	Path::operator std::string() const
	{
		return this->string();
	}



	Path::operator Glib::ustring() const
	{
		return this->string();
	}



	std::wostream& operator<<(std::wostream& stream, const Path& path)
	{
		return stream << path.string();
	}
// Path <--



// Stat -->
	Stat::Stat(void)
	{
	}



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
// Stat <--



const size_t MAX_FILE_PATH_SIZE = 1024;



bool check_extension(const std::string& file_name, const std::string& extension)
{
	size_t name_size = file_name.size();
	size_t ext_size = extension.size();

	if(
		name_size > ext_size &&
		file_name.substr(name_size - ext_size - 1) == "." + extension
	)
		return true;
	
	return false;
}



void copy_file(const std::string& from_path, const std::string& to_path, bool error_on_exists) throw(m::Exception)
{
	// TODO:
	// Обработать ситуацию, когда тип файла меняется после
	// выполнения unix_lstat.
	// Также не копируются права доступа.

	Stat file_stat = unix_lstat(from_path);

	if(error_on_exists)
		if(m::fs::is_exists(to_path, false))
			M_THROW(_("file is already exists"));

	if(file_stat.is_reg())
	{
		try
		{
			boost::fs::copy_file(U2L(from_path), U2L(to_path));

			// m::Exception
			unix_utime(to_path, file_stat);
		}
		catch(boost::filesystem::filesystem_error)
		{
			M_THROW(EE(errno));
		}
	}
	else if(file_stat.is_dir())
		M_THROW(_("it is a directory"));
	else if(file_stat.is_lnk())
		unix_symlink(unix_readlink(from_path), to_path);
	else
		M_THROW(_("it is a special file"));
}



void copy_files(const std::string& src_root, const std::string& dest_root, const std::vector<std::string>& files) throw(m::Exception)
{
	MLIB_D(_C("Copying files from '%1' to '%2'...", src_root, dest_root));
		try
		{
			if(!m::fs::is_exists(src_root, false))
				M_THROW(__("No such file or directory ('%1').", src_root));
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Can't stat '%1': %2.", src_root, EE(e)));
		}

		try
		{
			if(!m::fs::is_exists(dest_root, false))
				M_THROW(__("No such file or directory ('%1').", dest_root));
		}
		catch(m::Exception& e)
		{
			M_THROW(__("Can't stat '%1': %2.", dest_root, EE(e)));
		}

		m::fs::tree::cp(
			Path(src_root).normalize().string() + "/",
			Path(dest_root).normalize().string() + "/",
			m::fs::tree::create(files)
		);
	MLIB_D("All files has been copied successfully.");
}



void cp(const std::string& from, const std::string& to) throw(m::Exception)
{
	// TODO:
	// Обработать ситуацию, когда тип файла меняется после
	// его получения.
	// Также не копируются права доступа.

	try
	{
		boost::fs::path from_path(U2L(from));
		boost::fs::path to_path(U2L(to));
		boost::fs::directory_iterator end_it;

		if(boost::fs::is_directory(from_path))
		{
			try
			{
				unix_mkdir(to);
			}
			catch(m::Exception& e)
			{
				M_THROW(__("Can't create directory '%1': %2.", to, EE(e)));
			}

			m::Errors_pool errors;

				for(boost::fs::directory_iterator it(from_path); it != end_it; it++)
				{
					try
					{
						#if BOOST_VERSION / 100 <= 1035
							m::fs::cp( L2U((from_path / it->leaf()).string()), L2U((to_path / it->leaf()).string()) );
						#else
							m::fs::cp( L2U((from_path / it->filename()).string()), L2U((to_path / it->filename()).string()) );
						#endif
					}
					catch(m::Exception& e)
					{
						errors += EE(e);
					}
				}

			errors.throw_if_exists();
		}
		else
		{
			try
			{
				m::fs::copy_file(from, to);
			}
			catch(m::Exception& e)
			{
				M_THROW(__("Error while copying '%1' to '%2': %3.", from, to, EE(e)));
			}
		}
	}
	catch(boost::filesystem::filesystem_error)
	{
		M_THROW(__("Error while reading '%1': %2.", from, EE(errno)));
	}
}



std::string get_abs_path_lazy(const std::string& path)
{
	try
	{
		return Path(path).absolute();
	}
	catch(m::Exception)
	{
		return path;
	}
}



std::string get_user_home_path(void)
{
	// TODO: реализовать также чтение /etc/passwd,
	// если переменная HOME не задана.

	#ifdef MLIB_DEVELOP_MODE
		// В режиме разработки для удобства отладки используется собственная
		// домашняя папка. Имя папки специально содержит кириллические символы
		// - так лучше всего тестировать работу с различными локалями, т. к.
		// ошибки сразу же вылезают наружу.
		static std::string home_path = unix_get_cwd() + "/домашняя папка";
		return home_path;
	#else
		std::string user_home_path = Glib::get_home_dir();

		if(user_home_path == "")
			MLIB_E(__("Can't get user home path."));

		return L2U(user_home_path);

		/*const char* user_home_path = getenv("HOME");

		if(!user_home_path)
			MLIB_E(__("Can't get user home path: %1.", EE(errno)));

		return L2U(user_home_path);*/
	#endif
}



bool is_exists(const std::string& path, bool error_on_missing_parent) throw(m::Exception)
{
	return is_exists_base(path, false, error_on_missing_parent);
}



bool is_exists_without_errors(const std::string& path)
{
	try
	{
		return is_exists(path, false);
	}
	catch(m::Exception)
	{
		return false;
	}
}



bool is_lexists(const std::string& path) throw(m::Exception)
{
	return is_exists_base(path, true, true);
}


bool mkdir_if_not_exists(const std::string& path) throw(m::Exception)
{
	if(is_lexists(path))
	{
		if(!unix_stat(path).is_dir())
			M_THROW(__("file '%1' is already exists", path));

		return false;
	}
	else
	{
		unix_mkdir(path);
		return true;
	}
}



void mkdir_if_not_exists_with_race_conditions(const std::string& path) throw(m::Exception)
{
	try
	{
		mkdir_if_not_exists(path);
	}
	catch(m::Exception& e)
	{
		try
		{
			if(unix_stat(path).is_dir())
				return;
		}
		catch(m::Exception)
		{
			throw e;
		}
	}
}



void rm(const std::string& path) throw(m::Exception)
{
	try
	{
		if(is_lexists(path))
		{
			if(unix_lstat(path).is_dir())
			{
				try
				{
					boost::fs::path dir_path(U2L(path));
					boost::fs::directory_iterator end_it;

					m::Errors_pool errors;

						for(boost::fs::directory_iterator it(dir_path); it != end_it; it++)
						{
							try
							{
								#if BOOST_VERSION / 100 <= 1035
									rm( L2U((dir_path / it->leaf()).string()) );
								#else
									rm( L2U((dir_path / it->filename()).string()) );
								#endif
							}
							catch(m::Exception& e)
							{
								errors += EE(e);
							}
						}

						try
						{
							unix_rmdir(path);
						}
						catch(m::Exception& e)
						{
							errors += __("Can't delete directory '%1': %2.", path, EE(e));
						}

					errors.throw_if_exists();
				}
				catch(boost::filesystem::filesystem_error)
				{
					M_THROW(__("Error while reading '%1': %2.", path, EE(errno)));
				}
			}
			else
				unix_unlink(path);
		}
		else
			M_THROW(__("file or directory '%1' is not exists", path));
	}
	catch(m::Exception& e)
	{
		M_THROW(__("Can't delete file '%1': %2.", path, EE(e)));
	}
}



void rm_files_with_empty_dirs(const std::string& root, const std::vector<std::string>& files) throw(m::Exception)
{
	MLIB_D(_C("Removing files with empty directories from '%1'...", root));
		m::fs::tree::rm(Path(root).normalize().string() + "/", m::fs::tree::create(files));
	MLIB_D("All files has been successfully removed.");
}



void rm_if_exists(const std::string& path) throw(m::Exception)
{
	if(is_lexists(path))
		rm(path);
}



Stat unix_fstat(int fd) throw(m::Exception)
{
	struct stat stat_buf;

	if(fstat(fd, &stat_buf))
		M_THROW(EE(errno));

	return stat_buf;
}



std::string unix_get_cwd(void) throw(m::Exception)
{
	char *cwd;

	if( (cwd = get_current_dir_name()) )
	{
		std::string cwd_string = cwd;
		free(cwd);
		return L2U(cwd_string);
	}
	else
		M_THROW(EE(errno));
}



Stat unix_lstat(const std::string& path) throw(m::Exception)
{
	struct stat stat_buf;

	if(lstat(U2L(path).c_str(), &stat_buf))
		M_THROW(EE(errno));

	return stat_buf;
}



void unix_mkdir(const std::string& path) throw(m::Exception)
{
	if(mkdir(U2L(path).c_str(), 0777))
		M_THROW(EE(errno));
}



int unix_open(const std::string& path, int flags, mode_t mode) throw(m::Exception)
{
	int fd;

	if( (fd = open(U2L(path).c_str(), flags, mode)) >= 0 )
		return fd;
	else
		M_THROW(EE(errno));
}



std::string unix_readlink(const std::string& path) throw(m::Exception)
{
	char target_path_buf[M_FILE_PATH_MAX_SIZE];
	int written_bytes;

	written_bytes = readlink(U2L(path).c_str(), target_path_buf, M_FILE_PATH_MAX_SIZE);

	if(written_bytes >= M_FILE_PATH_MAX_SIZE)
		M_THROW(_("too big link target path"));
	else if(written_bytes < 0)
		M_THROW(EE(errno));

	target_path_buf[written_bytes] = '\0';

	return L2U(target_path_buf);
}



ssize_t unix_read(int fd, void* buf, size_t size) throw(m::Exception)
{
	ssize_t readed_bytes;

	while(1)
	{
		if( (readed_bytes = read(fd, buf, size)) < 0 )
		{
			if(errno == EINTR)
				continue;
			else
				M_THROW(strerror(errno));
		}
		else
			return readed_bytes;
	}
}



void unix_rename(const std::string& from, const std::string& to) throw(m::Exception)
{
	if(rename(U2L(from).c_str(), U2L(to).c_str()))
		M_THROW(EE(errno));
}



void unix_rmdir(const std::string& path) throw(m::Exception)
{
	if(rmdir(U2L(path).c_str()))
		M_THROW(EE(errno));
}



Stat unix_stat(const std::string& path) throw(m::Exception)
{
	struct stat stat_buf;

	if(stat(U2L(path).c_str(), &stat_buf))
		M_THROW(EE(errno));

	return stat_buf;
}



void unix_symlink(const std::string& old_path, const std::string& new_path) throw(m::Exception)
{
	if(symlink(U2L(old_path).c_str(), U2L(new_path).c_str()) < 0)
		M_THROW(EE(errno));
}



void unix_unlink(const std::string& path) throw(m::Exception)
{
	if(unlink(U2L(path).c_str()))
		M_THROW(EE(errno));
}



void unix_utime(const std::string& path, const Stat& file_stat)
{
	struct utimbuf time_buf;
	time_buf.actime = file_stat.atime;
	time_buf.modtime = file_stat.mtime;

	if(utime(U2L(path).c_str(), &time_buf))
		M_THROW(EE(errno));
}

}
}

