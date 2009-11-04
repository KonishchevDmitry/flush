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


#include "dispatcher.hpp"


namespace m { namespace gtk {


namespace Dispatcher_aux {


class Emission_handler
{
	public:
		Emission_handler(void): connected(true) {}
	
	public:
		bool connected;
};


Dispatcher_base::Dispatcher_base(void)
:
	handler(NULL)
{
}



Dispatcher_base::~Dispatcher_base(void)
{
	// Если в данный момент запущен поток для порождения сигналов, уведомляем
	// его о том, что Dispatcher больше не существует. Об освобождении памяти
	// для объекта позаботится поток.
	if(this->handler)
		this->handler->connected = false;
}



bool Dispatcher_base::emission_cb(Emission_handler* handler)
{
	{
		m::gtk::Scoped_enter lock;
		
		while(handler->connected && !this->emiters.empty())
		{
			Emiter_ptr emiter = this->emiters.front();
			this->emiters.pop();

			// Тут может произойти все, что угодно вплоть до того, что
			// запустится новый Main loop, который один или несколько раз
			// выйдет/войдет в критическую секцию GTK.
			emiter->emit();
		}

		// Уведомляем Dispatcher о том, что поток завершил свою работу
		if(handler->connected)
			this->handler = NULL;
	}

	delete handler;
	return false;
}



void Dispatcher_base::schedule_emission(Emiter_ptr emiter)
{
	this->emiters.push(emiter);

	// Если в данный момент нет запущенного g_idle
	if(!this->handler)
	{
		this->handler = new Emission_handler;

		Glib::signal_idle().connect(
			sigc::bind<Emission_handler*>(sigc::mem_fun(*this, &Dispatcher_base::emission_cb), this->handler),
			Glib::PRIORITY_DEFAULT
		);
	}
}


}



}}

