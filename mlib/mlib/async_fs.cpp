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


#include <queue>

#include <boost/ref.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <glibmm/dispatcher.h>

#include <mlib/gtk/main.hpp>

#include <mlib/fs.hpp>
#include <mlib/main.hpp>
#include <mlib/misc.hpp>

#include "async_fs.hpp"


namespace m { namespace async_fs {

/// Обеспечивает асинхронную работу с файловой системой.
/// Предполагается, что объект будет глобальным, а, следовательно, уничтожаться
/// будет только при закрытии программы, когда никто уже точно не будет с ним
/// взаимодействовать.
/// Поэтому в случае изменения требований его будет необходимо изменить.
class Async_fs
{
	public:
		class Task
		{
			public:
				enum Type { NONE, COPY, COPY_FILES, REMOVE, REMOVE_FILES_WITH_EMPTY_DIRS, REMOVE_IF_EXISTS };


			public:
				Task(void)
				:
					type(NONE)
				{
				}

				Task
				(
					const Task_id& id,
					const Type& type,
					const std::string& group,
					const std::string& error_title,
					const std::string& error_string,
					const std::vector<std::string>& args
				)
				:
					id(id),
					type(type),
					group(group),
					error_title(error_title),
					error_string(error_string),
					args(args)
				{
				}


			public:
				/// Уникальный идентификатор задачи.
				Task_id						id;

				/// Тип операции.
				Type						type;

				/// Группа, к которой относится данная задача.
				std::string					group;

				/// Заголовок для ошибки, если операция завершится
				/// неудачей.
				std::string					error_title;

				/// Строка, которая будет предварять сообщение об
				/// ошибке, если операция завершится неудачей.
				std::string					error_string;

				/// Аргументы, которые необходимо передать функции,
				/// осуществляющей необходимые операции с файлами.
				std::vector<std::string>	args;
		};


	public:
		Async_fs(void);
		~Async_fs(void);


	private:
		/// Определяет, запущен ли уже поток или нет.
		bool						started;

		/// Для синхронизации потоков.
		boost::mutex				mutex;

		/// Список ожидающих завершения всех задач.
		std::queue<m::Semaphore*>	waitings;


		/// Определяет приостановлена ли в данный момент работа асинхронной
		/// файловой системы.
		bool						paused;

		/// Сигнал, который будет породжаться в момент остановки
		/// асинхронной файловой системы по асинхронному запросу.
		Glib::Dispatcher			paused_signal;


	#ifdef MLIB_ASYNC_FS_GLIB_SIGNALS
		/// Сигнал, используемый для запуска функций внутри
		/// Glib'овского Main loop'а.
		Glib::Dispatcher			task_finished;

		/// Сигнал, к которому сторонние функции привязвывают
		/// свои обработчики.
		sigc::signal<void, Task_id>	task_finished_signal;
	#endif


		/// Список задач, стоящих в очереди.
		std::queue<Task>			tasks;

	#ifdef MLIB_ASYNC_FS_GLIB_SIGNALS
		/// Идентификаторы завершенных задач
		std::queue<Task_id>			finished_tasks;
	#endif

		/// Идентификатор последней добавленной задачи.
		Task_id						last_task_id;


	public:
		/// Добавляет новую задачу.
		/// @param error_title - заголовок для ошибки, если операция
		/// завершится неудачей.
		/// @param error_string - строка, которая будет предварять
		/// сообщение об ошибке, если операция завершится неудачей.
		Task_id							add_task(const Task::Type& type, const std::string& group, const std::string& error_title, const std::string& error_string, const std::vector<std::string>& args);

		/// Устанавливает сигнал, который будет породжаться в момент остановки
		/// асинхронной файловой системы по асинхронному запросу.
		Glib::Dispatcher&				get_paused_signal(void);

		/// Посылает запрос на приостановку работы асинхронной файловой системы.
		/// Как только система будет приостановлена, сгенерируется сигнал.
		/// @return - true, если в данный момент выполняется задание для группы group.
		bool							pause_and_get_group_status(const std::string& group);

		/// Снимает систему с паузы.
		/// @return - true, если есть еще задачи и запустился поток для их обработки.
		bool							resume(void);

	#ifdef MLIB_ASYNC_FS_GLIB_SIGNALS
		/// Возвращает сигнал, генерируемый при завершении задачи, к которому можно
		/// привязать обработчик.
		sigc::signal<void, Task_id>&	signal(void);
	#endif

	private:
	#ifdef MLIB_ASYNC_FS_GLIB_SIGNALS
		/// Производит все необходимые действия по завершению
		/// обработки задачи и уведомлению об этом всех заинтересованных
		/// объектов.
		void							finish_task(Task_id id);

		/// Обработчик сигнала на завершение задачи.
		/// Запускается диспетчером внутри
		/// Glib'овского Main loop'а.
		void							on_task_finished_callback(void);
	#endif

		/// Запускает поток.
		void							start(void);

		/// Блокирует выполнение текущего потока, пока не завершится
		/// выполнение всех поставленных задач и потока, который их
		/// выполняет или пока система не будет приостановлена.
		void							wait(void);


	public:
		void 	operator()(void);
};



namespace
{
	/// Обеспечивает асинхронную работу с файловой системой.
	Async_fs ASYNC_FS;
}



// Async_fs -->
	Async_fs::Async_fs(void)
	:
		started(false),
		paused(false),
		last_task_id(0)
	{
		#ifdef MLIB_ASYNC_FS_GLIB_SIGNALS
			// Чтобы функция запускалась внутри Glib'овского Main loop'а.
			this->task_finished.connect(sigc::mem_fun(*this, &Async_fs::on_task_finished_callback));
		#endif
	}



	Async_fs::~Async_fs(void)
	{
		MLIB_D("Closing asynchronous filesystem. Waiting for tasks finishing...");

		this->wait();

		{
			// Освобождение семафора не дает гарантии, что
			// поток завершен. Возможно, он в данный момент
			// освобождает другие семафоры. Поэтому имеет
			// смысл еще раз захватить мьютекс.
			boost::mutex::scoped_lock lock(this->mutex);

			if(!this->tasks.empty())
				MLIB_W(_("Asynchronous filesystem finished it's work, but not all tasks are processed."));
		}

		MLIB_D("Asynchronous filesystem's tasks has been finished.");
	}



	Task_id Async_fs::add_task(const Task::Type& type, const std::string& group, const std::string& error_title, const std::string& error_string, const std::vector<std::string>& args)
	{
		Task_id id;
		bool is_start = false;

		MLIB_D(_C("Adding new async fs task: [%1] [%2]...", type, group));

		{
			boost::mutex::scoped_lock lock(this->mutex);

			id = ++this->last_task_id;
			this->tasks.push(Task(id, type, group, error_title, error_string, args));

			if(!this->started)
			{
				if(!this->paused)
				{
					is_start = true;
					this->started = true;
				}
				else
					MLIB_D("Task has been added, but async fs is paused.");
			}
		}

		if(is_start)
			this->start();

		return id;
	}



#ifdef MLIB_ASYNC_FS_GLIB_SIGNALS
	void Async_fs::finish_task(Task_id id)
	{
		{
			boost::mutex::scoped_lock lock(this->mutex);
			this->finished_tasks.push(id);
		}
		this->task_finished();
	}
#endif



	Glib::Dispatcher& Async_fs::get_paused_signal(void)
	{
		return this->paused_signal;
	}



#ifdef MLIB_ASYNC_FS_GLIB_SIGNALS
	void Async_fs::on_task_finished_callback(void)
	{
		Task_id id;

		{
			boost::mutex::scoped_lock lock(this->mutex);
			id = this->finished_tasks.front();
			this->finished_tasks.pop();
		}

		{
		#ifdef MLIB_ENABLE_GTK
			m::gtk::Scoped_enter gtk_lock;
		#endif
			this->task_finished_signal(id);
		}
	}
#endif



	bool Async_fs::pause_and_get_group_status(const std::string& group)
	{
		boost::mutex::scoped_lock lock(this->mutex);

		this->paused = true;

		if(this->started && !this->tasks.empty())
			return this->tasks.front().group == group;
		else
			return false;
	}



	bool Async_fs::resume(void)
	{
		{
			boost::mutex::scoped_lock lock(this->mutex);

			if(this->paused)
			{
				this->paused = false;

				if(this->started)
					return true;
				else
				{
					if(this->tasks.empty())
						return false;
					else
					{
						// Запускаем поток
						this->started = true;
					}
				}
			}
			else
				return this->started;
		}

		this->start();

		return true;
	}



#ifdef MLIB_ASYNC_FS_GLIB_SIGNALS
	sigc::signal<void, Task_id>& Async_fs::signal(void)
	{
		return this->task_finished_signal;
	}
#endif



	void Async_fs::start(void)
	{
		MLIB_D("Starting async fs thread...");
		boost::thread(boost::ref(*this));
	}



	void Async_fs::wait(void)
	{
		m::Semaphore wait_semaphore;

		{
			boost::mutex::scoped_lock lock(this->mutex);

			if(!this->started)
				return;

			this->waitings.push(&wait_semaphore);
		}

		// Ожидаем, пока все задачи будут завершены.
		wait_semaphore.wait();
	}



	void Async_fs::operator()(void)
	{
		Task task;

		while(1)
		{
			{
				boost::mutex::scoped_lock lock(this->mutex);

				if(this->tasks.empty() || this->paused)
				{
					// Если больше не осталось задач, или систему необходимо
					// приостановить, то завершаем выполнение потока, извещая
					// всех, кому это интересно.

					this->started = false;

					// Сообщаем, что система приостановлена
					if(this->paused)
						this->paused_signal();

					while(!this->waitings.empty())
					{
						m::Semaphore* wait_semaphore = this->waitings.front();
						wait_semaphore->post();
						this->waitings.pop();
					}

					return;
				}
				else
					task = this->tasks.front();
			}

			MLIB_D("Processing new async fs task...");

			try
			{
				switch(task.type)
				{
					case Task::COPY:
						fs::cp(task.args[0], task.args[1]);
						break;

					case Task::COPY_FILES:
					{
						std::string src_root = task.args[0];
						task.args.erase(task.args.begin());

						std::string dest_root = task.args[0];
						task.args.erase(task.args.begin());

						fs::copy_files(src_root, dest_root, task.args);
					}
					break;

					case Task::REMOVE:
						fs::rm(task.args[0]);
						break;

					case Task::REMOVE_FILES_WITH_EMPTY_DIRS:
					{
						std::string root = task.args[0];
						task.args.erase(task.args.begin());
						fs::rm_files_with_empty_dirs(root, task.args);
					}
					break;

					case Task::REMOVE_IF_EXISTS:
						fs::rm_if_exists(task.args[0]);
						break;

					case Task::NONE:
					default:
						MLIB_LE();
						break;
				}
			}
			catch(Exception& e)
			{
				if(!task.error_string.empty())
					task.error_string += " ";

				MLIB_W(task.error_title, task.error_string + EE(e));
			}

			#ifdef MLIB_ASYNC_FS_GLIB_SIGNALS
				this->finish_task(task.id);
			#endif

			// Удаляем обработанную задачу из очереди
			{
				boost::mutex::scoped_lock lock(this->mutex);
				this->tasks.pop();
			}

			MLIB_D("Async fs task has been processed.");
		}
	}
// Async_fs <--



Task_id copy_files(const std::string& group, const std::string& src_root, const std::string& dest_root, const std::vector<std::string>& files, const std::string& error_title, const std::string& error_string)
{
	std::vector<std::string> args(files);
	args.insert(args.begin(), dest_root);
	args.insert(args.begin(), src_root);
	return ASYNC_FS.add_task(Async_fs::Task::COPY_FILES, group, error_title, error_string, args);
}



Task_id cp(const std::string& group, const std::string& from, const std::string& to, const std::string& error_title, const std::string& error_string)
{
	std::vector<std::string> args;

	args.push_back(from);
	args.push_back(to);

	return ASYNC_FS.add_task(Async_fs::Task::COPY, group, error_title, error_string, args);
}



Glib::Dispatcher& get_paused_signal(void)
{
	return ASYNC_FS.get_paused_signal();
}



bool pause_and_get_group_status(const std::string& group)
{
	return ASYNC_FS.pause_and_get_group_status(group);
}



bool resume(void)
{
	return ASYNC_FS.resume();
}



Task_id rm(const std::string& group, const std::string& path, const std::string& error_title, const std::string& error_string)
{
	std::vector<std::string> args(1, path);
	return ASYNC_FS.add_task(Async_fs::Task::REMOVE, group, error_title, error_string, args);
}



Task_id rm_files_with_empty_dirs(const std::string& group, const std::string& root, const std::vector<std::string>& files, const std::string& error_title, const std::string& error_string)
{
	std::vector<std::string> args(files);
	args.insert(args.begin(), root);
	return ASYNC_FS.add_task(Async_fs::Task::REMOVE_FILES_WITH_EMPTY_DIRS, group, error_title, error_string, args);
}



Task_id rm_if_exists(const std::string& group, const std::string& path, const std::string& error_title, const std::string& error_string)
{
	std::vector<std::string> args(1, path);
	return ASYNC_FS.add_task(Async_fs::Task::REMOVE_IF_EXISTS, group, error_title, error_string, args);
}



#ifdef MLIB_ASYNC_FS_GLIB_SIGNALS
	sigc::signal<void, Task_id>& signal(void)
	{
		return ASYNC_FS.signal();
	}
#endif


}}

