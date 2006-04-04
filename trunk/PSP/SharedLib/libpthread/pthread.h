/*
	pthread library for the PSP
	Copyright (C) 2006  Rafael Cabezas a.k.a. Raf

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef PTHREAD_H
	#define PTHREAD_H
	
	typedef void (*thread_function)(void *);
	
	
	/** Revise: */
	typedef int pthread_t;
	typedef int pthread_attr_t;

	/* Create a thread */
	int pthread_create(pthread_t *thread_id, const pthread_attr_t *attributes, thread_function tf, void *arguments);
	
	/* pthreads terminate when they return, or if they call: */
	int pthread_exit(void *status);
	
	///int pthread_join(pthread_t thread, void **status_ptr);
	
	/* get thread id from within thread */
	///pthread_t pthread_self();
	
	/* to compare thread ids */
	///int pthread_equal(pthread_t t1, pthread_t t2);
	
	/** Mutexes */
	///int pthread_mutex_init, unlock, trylock, destroy
	
#endif
