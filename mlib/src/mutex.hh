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


namespace m {


inline Scoped_lock::Scoped_lock(Mutex* mutex)
:
	type(TYPE_ORDINARY),
	mutex(mutex)
{
	mutex->lock();
}



inline Scoped_lock::Scoped_lock(Recursive_mutex* mutex)
:
	type(TYPE_RECURSIVE),
	mutex(mutex)
{
	mutex->lock();
}



#if MLIB_HAS_SHARED_MUTEX
	inline Scoped_lock::Scoped_lock(Shared_mutex* mutex)
	:
		type(TYPE_SHARED),
		mutex(mutex)
	{
		mutex->lock();
	}
#endif



inline Scoped_lock::~Scoped_lock(void)
{
	switch(this->type)
	{
		case TYPE_ORDINARY:
			reinterpret_cast<Mutex*>(this->mutex)->unlock();
			break;

	#if MLIB_HAS_SHARED_MUTEX
		case TYPE_SHARED:
			reinterpret_cast<Shared_mutex*>(this->mutex)->unlock();
			break;
	#endif

		case TYPE_RECURSIVE:
			reinterpret_cast<Recursive_mutex*>(this->mutex)->unlock();
			break;
	}
}



inline Scoped_shared_lock::Scoped_shared_lock(Shared_mutex* mutex)
:
	mutex(*mutex)
{
	mutex->lock();
}



inline Scoped_shared_lock::~Scoped_shared_lock(void)
{
	this->mutex.unlock();
}


}

