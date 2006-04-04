#ifndef PTHREAD_H
	#define PTHREAD_H
	
	typedef void *(*thread_function)(void *)
	
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
	