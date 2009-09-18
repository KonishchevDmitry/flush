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


#include <boost/tuple/tuple.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <glibmm/main.h>


namespace m { namespace gtk {


namespace Dispatcher_aux {


	class Emiter: private Virtual, private boost::noncopyable
	{
		public:
			virtual void emit(void) const = 0;
	};


	template <class Signal>
	class Emiter0: public Emiter
	{
		public:
			Emiter0(Signal& signal)
			: signal(signal) {}

		private:
			Signal&	signal;

		public:
			void emit(void) const
			{ this->signal(); }
	};


	template <class Signal, class T1>
	class Emiter1: public Emiter
	{
		private:
			typedef boost::tuple<T1> Args;

		public:
			Emiter1(Signal& signal, T1& arg1)
			: signal(signal), args(arg1) { }

		private:
			Signal&	signal;
			Args	args;

		public:
			void emit(void) const
			{ this->signal( this->args.template get<0>() ); }
	};


	template <class Signal, class T1, class T2>
	class Emiter2: public Emiter
	{
		private:
			typedef boost::tuple<T1, T2> Args;

		public:
			Emiter2(Signal& signal, T1& arg1, T2& arg2)
			: signal(signal), args(arg1, arg2) { }

		private:
			Signal&	signal;
			Args	args;

		public:
			void emit(void) const
			{ this->signal( this->args.template get<0>(), this->args.template get<1>() ); }
	};


}



template <class T> template <class Fun>
typename Dispatcher<T>::Connection Dispatcher<T>::connect(Fun fun)
{
	return this->signal.connect(fun);
}



template <class T>
void Dispatcher<T>::emit(void)
{
	using namespace Dispatcher_aux;

	this->schedule_emission(Emiter_ptr(
		new Emiter0<Signal>(this->signal) ));
}



template <class T> template <class A1>
void Dispatcher<T>::emit(A1 arg1)
{
	using namespace Dispatcher_aux;

	this->schedule_emission(Emiter_ptr(
		new Emiter1<Signal, typename boost::remove_reference<A1>::type>(
				this->signal, arg1 )));
}



template <class T> template <class A1, class A2>
void Dispatcher<T>::emit(A1 arg1, A2 arg2)
{
	using namespace Dispatcher_aux;

	this->schedule_emission(Emiter_ptr(
		new Emiter2<
			Signal,
			typename boost::remove_reference<A1>::type,
			typename boost::remove_reference<A2>::type
		>( this->signal, arg1, arg2 )
	));
}



template <class T>
void Dispatcher<T>::operator()(void)
{
	this->emit();
}



template <class T> template <class A1>
void Dispatcher<T>::operator()(A1 arg1)
{
	this->emit(arg1);
}



template <class T> template <class A1, class A2>
void Dispatcher<T>::operator()(A1 arg1, A2 arg2)
{
	this->emit(arg1, arg2);
}


}}

