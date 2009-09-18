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


// Предоставляет объект, аналогичный Glib::Dispatcher, предназначенный для
// передачи сигналов GTK из других потоков.
//
// Объект должен использоваться только из GTK потока или из потока,
// вызвавшего функцию gdk_threads_enter().
//
//
// Обоснование:
//
// Зачастую необходимо порождать какие-либо сигналы GTK из других потоков. При
// этом очень важно, чтобы обработчик сигнала выполнялся не в том потоке, из
// которого он порождается, т. к. если данный обработчик вызовет, к примеру,
// функцию gtk_dialog_run(), то она приостановит выполнение данного потока, что
// в большинстве случаев нежелательно.
//
// В принципе, для этого можно использовать Glib::Dispatcher, но он не
// позволяет передавать сигналу какие-либо аргументы, что очень не удобно.
// Поэтому был написан данный класс.


#ifndef HEADER_MLIB_GTK_DISPATCHER
#define HEADER_MLIB_GTK_DISPATCHER

#include <queue>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals.hpp>

#include <mlib/gtk/main.hpp>

#include "dispatcher.hxx"


namespace m { namespace gtk {


namespace Dispatcher_aux {

class Emiter;
class Emission_handler;

typedef boost::shared_ptr<Emiter> Emiter_ptr;



class Dispatcher_base: private boost::noncopyable
{
	protected:
		Dispatcher_base(void);
		~Dispatcher_base(void);


	private:
		/// handler запущенного в данный момент потока для порождения сигналов.
		Emission_handler*		handler;

		/// Очередь сигналов, ожидающих передачи их в GTK.
		std::queue<Emiter_ptr>	emiters;


	protected:
		/// Добавляет сигнал в очередь запланированных сигналов.
		void		schedule_emission(Emiter_ptr emiter);

	private:
		/// Поток для порождения сигналов GTK.
		bool		emission_cb(Emission_handler* handler);
};

}



template <class T>
class Dispatcher: public Dispatcher_aux::Dispatcher_base
{
	public:
		typedef Dispatcher_connection	Connection;

	private:
		typedef boost::signal<T>		Signal;


	private:
		typedef Dispatcher_aux::Emiter				Emiter;
		typedef Dispatcher_aux::Emiter_ptr			Emiter_ptr;
		typedef Dispatcher_aux::Emission_handler	Emission_handler;


	private:
		Signal		signal;


	public:
		template <class Fun>
		Connection	connect(Fun fun);

		void		emit(void);

		template <class A1>
		void		emit(A1 arg1);

		template <class A1, class A2>
		void		emit(A1 arg1, A2 arg2);


	public:
		void		operator()(void);

		template <class A1>
		void		operator()(A1 arg1);

		template <class A1, class A2>
		void		operator()(A1 arg1, A2 arg2);
};


}}

#include "dispatcher.hh"

#endif

