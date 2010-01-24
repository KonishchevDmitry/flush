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


// Предоставляет объекты-мьютексы и удобный интерфейс для их блокировки.


#ifndef HEADER_MLIB_MUTEX
#define HEADER_MLIB_MUTEX

#if M_BOOST_GET_VERSION() >= M_GET_VERSION(1, 35, 0)
	#define MLIB_HAS_SHARED_MUTEX 1
#endif

	#include <boost/thread/mutex.hpp>
	#include <boost/thread/recursive_mutex.hpp>
#if MLIB_HAS_SHARED_MUTEX
	#include <boost/thread/shared_mutex.hpp>
#endif


namespace m {


	typedef boost::mutex Mutex;
#if MLIB_HAS_SHARED_MUTEX
	typedef boost::shared_mutex Shared_mutex;
#endif
	typedef boost::recursive_mutex Recursive_mutex;



class Scoped_lock
{
	private:
		enum Mutex_type {
			TYPE_ORDINARY,
		#if MLIB_HAS_SHARED_MUTEX
			TYPE_SHARED,
		#endif
			TYPE_RECURSIVE
		};


	public:
		explicit Scoped_lock(Mutex* mutex);
	#if MLIB_HAS_SHARED_MUTEX
		explicit Scoped_lock(Shared_mutex* mutex);
	#endif
		explicit Scoped_lock(Recursive_mutex* mutex);

		~Scoped_lock(void);


	private:
		Mutex_type	type;
		void*		mutex;
};



#if MLIB_HAS_SHARED_MUTEX
	class Scoped_shared_lock
	{
		public:
			explicit Scoped_shared_lock(Shared_mutex* mutex);
			~Scoped_shared_lock(void);

		private:
			Shared_mutex&	mutex;
	};
#endif



namespace aliases
{
	using m::Mutex;
#if MLIB_HAS_SHARED_MUTEX
	using m::Shared_mutex;
#endif
	using m::Recursive_mutex;

	using m::Scoped_lock;
#if MLIB_HAS_SHARED_MUTEX
	using m::Scoped_shared_lock;
#endif
}


}

#include "mutex.hh"

#endif

