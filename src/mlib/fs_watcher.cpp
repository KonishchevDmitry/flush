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


#ifdef MLIB_ENABLE_INOTIFY
	#include <queue>

	#include <sys/inotify.h>
	#include <cerrno>
	#include <unistd.h>

	#include <boost/ref.hpp>
	#include <boost/thread.hpp>
	#include <boost/thread/mutex.hpp>
#endif

#include <glibmm/dispatcher.h>

#ifdef MLIB_ENABLE_INOTIFY
	#include <mlib/fs.hpp>
	#include <mlib/sys.hpp>
	#include <mlib/misc.hpp>
#endif
#include <mlib/main.hpp>

#include "fs_watcher.hpp"



namespace m
{


class Fs_watcher::Implementation
{
	#ifdef MLIB_ENABLE_INOTIFY
		public:
			Implementation(void);
			~Implementation(void);


		private:
			/// Для синхронизации потоков.
			boost::mutex			mutex;


			/// Дескриптор inotify.
			int						fd;

			/// Дескриптор мониторящегося в данный момент запроса.
			int						watch_fd;


			/// Директория, мониторящаяся данный момент.
			std::string				watching_directory;


			/// Связь между потоками.
			m::Connection			connection;

			/// Дескриптор запущенного в данный момент потока, если он запущен.
			boost::thread*			thread;


			/// Файлы, которые были обнаружены в мониторящейся в данный момент
			/// директории.
			std::queue<std::string>	new_files;
	#endif

			/// Сигнал, который будет породжаться в момент получения информации
			/// о том, что в директорию был добавлен файл.
			Glib::Dispatcher		new_file_signal;

		public:
			/// Очищает очередь новых файлов.
			void				clear(void);

			/// Подключает обработчик сигнала на появление нового файла в
			/// мониторящейся директории.
			sigc::connection	connect(const sigc::slot<void>& slot);

			/// Извлекает путь к файлу из очереди новых файлов.
			/// Если директория была перемещена или удалена, то в очередь
			/// добавляется пустой путь ("").
			///
			/// @return - true, если файл был извлечен, или false, если в
			/// очереди больше не осталось файлов.
			bool				get(std::string* file_path);

			/// Возвращает директорию, которая мониторится в данный момент, или
			/// "".
			std::string			get_watching_directory(void);

			/// Задает директорию для мониторинга.
			/// Если directory == "", то это означает, что необходимо
			/// прекратить мониторинг директории, если какая-либо директория в
			/// данный момент мониторится.
			/// @throw - m::Exception.
			void				set_watching_directory(const std::string& directory);

			/// Снимает текущую директорию с мониторинга (если такая существует).
			void				unset_watching_directory(void);


	#ifdef MLIB_ENABLE_INOTIFY
		private:
			/// Производит все необходимые действия по обработки события
			/// inotify.
			void		process_event(const inotify_event* event, const std::string& watching_directory, const std::string& event_file_name);


		public:
			void 		operator()(void);
	#endif
};



#ifdef MLIB_ENABLE_INOTIFY
	Fs_watcher::Implementation::Implementation(void)
	:
		watch_fd(-1),
		thread(NULL)
	{
		this->fd = inotify_init();

		if(this->fd < 0)
			MLIB_E(__("Can't create an inotify instance: %1.", strerror(errno)));
	}



	Fs_watcher::Implementation::~Implementation(void)
	{
		unset_watching_directory();

		if(close(this->fd))
			MLIB_SW(__("Error while closing an inotify instance: %1.", strerror(errno)));
	}
#endif



void Fs_watcher::Implementation::clear(void)
{
	#ifdef MLIB_ENABLE_INOTIFY
		boost::mutex::scoped_lock lock(this->mutex);
		this->new_files = std::queue<std::string>();
	#endif
}



sigc::connection Fs_watcher::Implementation::connect(const sigc::slot<void>& slot)
{
	return this->new_file_signal.connect(slot);
}



bool Fs_watcher::Implementation::get(std::string* file_path)
{
	#ifdef MLIB_ENABLE_INOTIFY
		boost::mutex::scoped_lock lock(this->mutex);

		if(this->new_files.empty())
			return false;
		else
		{
			*file_path = this->new_files.front();
			new_files.pop();
			return true;
		}
	#else
		return false;
	#endif
}



std::string Fs_watcher::Implementation::get_watching_directory(void)
{
	#ifdef MLIB_ENABLE_INOTIFY
		boost::mutex::scoped_lock lock(this->mutex);
		return this->watching_directory;
	#else
		return "";
	#endif
}



#ifdef MLIB_ENABLE_INOTIFY
	void Fs_watcher::Implementation::process_event(const inotify_event* event, const std::string& watching_directory, const std::string& event_file_name)
	{
		bool interesting = false;
		std::string file_name = event_file_name;

		// Определяем, интересует ли нас это событие -->
			// В директории был создан файл
			if( event->mask & IN_CREATE )
			{
				MLIB_A(file_name != "");

				MLIB_D(_C(
					"Gotten inotify event: File '%1' in directory '%2' has been created.",
					file_name, watching_directory
				));

				try
				{
					m::sys::Stat file_stat;
					file_stat = m::sys::unix_lstat(Path(watching_directory) / file_name);

					// На только что созданные файлы не
					// обращаем внимания - ждем, пока в них
					// запишут все данные и закроют. Исключение
					// делаем только для тех объектов, которые
					// создаются мгновенно.
					if(!file_stat.is_reg() || file_stat.nlink > 1)
						interesting = true;
				}
				catch(m::Exception& e)
				{
					MLIB_D(_C(
						"Can't stat new file '%1' in directory '%2': %3.",
						file_name, watching_directory, EE(e)
					));
				}
			}
			// В директорию был перемещен файл или файл, открытый на запись, был
			// закрыт.
			else if( event->mask & (IN_CLOSE_WRITE | IN_MOVED_TO) )
			{
				MLIB_A(file_name != "");

				MLIB_D(_C(
					"Gotten inotify event: File '%1' in directory '%2' has been gotten.",
					file_name, watching_directory
				));

				interesting = true;
			}
			// Сама директория была перемещена
			else if( event->mask & (IN_MOVE_SELF | IN_DELETE_SELF) )
			{
				MLIB_D(_C(
					"Gotten inotify event: Directory '%1' has been deleted or moved.",
					watching_directory
				));

				interesting = true;
				file_name = "";
			}
		// Определяем, интересует ли нас это событие <--

		if(interesting)
		{
			// Помещаем файл в очередь -->
			{
				boost::mutex::scoped_lock lock(this->mutex);

				if(file_name == "")
					this->new_files.push("");
				else
				{
					this->new_files.push(
						Path(watching_directory) / file_name
					);
				}
			}
			// Помещаем файл в очередь <--

			// Оповещаем всех интересующихся о появлении нового
			// события.
			this->new_file_signal();
		}
		else
			MLIB_D("This event is not interesting for us.");
	}
#endif



void Fs_watcher::Implementation::set_watching_directory(const std::string& directory)
{
	MLIB_D(_C("Setting new watching directory '%1'...", directory));

	#ifdef MLIB_ENABLE_INOTIFY
		// Сначала сбрасываем текущую директорию
		this->unset_watching_directory();
		this->clear();

		// Устанавливаем новую директорию -->
		{
			int new_watch;

			new_watch = inotify_add_watch(
				this->fd, U2L(directory).c_str(),
				IN_CREATE | IN_CLOSE_WRITE | IN_MOVED_TO |
				IN_MOVE_SELF | IN_DELETE_SELF
			);

			if(new_watch < 0)
				M_THROW(strerror(errno));

			this->watching_directory = directory;
			this->watch_fd = new_watch;
		}
		// Устанавливаем новую директорию <--

		// Запускаем поток
		this->thread = new boost::thread(boost::ref(*this));
	#else
		M_THROW(_("Program has been compiled without inotify support."));
	#endif
}



void Fs_watcher::Implementation::unset_watching_directory(void)
{
	#ifdef MLIB_ENABLE_INOTIFY
		{
			boost::mutex::scoped_lock lock(this->mutex);

			if(this->watching_directory == "")
				return;

			MLIB_D(_C("Unsetting watching directory '%1'...", this->watching_directory));
		}

		if(inotify_rm_watch(this->fd, this->watch_fd))
			MLIB_E(__("Error while removing an inotify watch instance: %1.", strerror(errno)));
		this->watch_fd = -1;

		this->connection.post();
		this->thread->join();
		delete this->thread;

		this->watching_directory = "";
			
		MLIB_D("Watching directory has been unsetted.");
	#endif
}



#ifdef MLIB_ENABLE_INOTIFY
	void Fs_watcher::Implementation::operator()(void)
	{
		std::string watching_directory;
		char events_buf[sizeof(struct inotify_event) + m::sys::FILE_PATH_MAX_SIZE];

		{
			boost::mutex::scoped_lock lock(this->mutex);
			watching_directory = this->watching_directory;
		}

		while(1)
		{
			// В дескрипторе появились какие-то данные
			if(connection.wait_for_with_owning(this->fd, true))
			{
				MLIB_D("Getting inotify event(s)...");

				{
					char* cur_ptr = events_buf;
					char* end_ptr = events_buf;

					try
					{
						end_ptr += m::sys::unix_read(
							this->fd, &events_buf, sizeof events_buf
						);

						MLIB_D(_C("Readed %1 bytes of inotify event.", end_ptr - cur_ptr));
					}
					catch(m::Exception& e)
					{
						MLIB_E(__("Can't read data from inotify descriptor: %1.", EE(e)));
					}


					while(cur_ptr < end_ptr)
					{
						struct inotify_event* event;
						std::string file_name;

						// event -->
							event = reinterpret_cast<struct inotify_event*>(cur_ptr);
							cur_ptr += sizeof(struct inotify_event);

							if(cur_ptr > end_ptr)
								MLIB_E("Gotten invalid inotify event.");
						// event <--

						// file_name -->
							if(event->len)
							{
								if(cur_ptr + event->len > end_ptr)
									MLIB_E("Gotten invalid inotify event.");

								// event->len указывает размер буфера, который
								// может быть не полон. Поэтому, нулевой символ
								// может быть в нем где-то ранее, но на всякий
								// случай ставим свой, если его там нет.
								cur_ptr[event->len - 1] = '\0';

								file_name = L2U(cur_ptr);
								cur_ptr += event->len;
							}
						// file_name <--

						// Обрабатываем полученное событие
						this->process_event(event, watching_directory, file_name);
					}
				}
			}
			// Получен сигнал, останавливающий поток
			else
				return;
		}
	}
#endif



Fs_watcher::Fs_watcher(void)
:
	impl(new Implementation)
{
}



Fs_watcher::~Fs_watcher(void)
{
	delete this->impl;
}



void Fs_watcher::clear(void)
{
	return this->impl->clear();
}



sigc::connection Fs_watcher::connect(const sigc::slot<void>& slot)
{
	return this->impl->connect(slot);
}



bool Fs_watcher::get(std::string* file_path)
{
	return this->impl->get(file_path);
}



std::string Fs_watcher::get_watching_directory(void)
{
	return this->impl->get_watching_directory();
}



void Fs_watcher::set_watching_directory(const std::string& directory)
{
	this->impl->set_watching_directory(directory);
}



void Fs_watcher::unset_watching_directory(void)
{
	this->impl->unset_watching_directory();
}

}

