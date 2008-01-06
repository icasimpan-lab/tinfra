#ifndef __tinfra_thread_h__
#define __tinfra_thread_h__

#include <vector>
#include <pthread.h>

namespace tinfra {

class Mutex {
    pthread_mutex_t mutex_;
public:
    
    typedef pthread_mutex_t handle_type;

    Mutex() { ::pthread_mutex_init(&mutex_, 0); }
    ~Mutex() { ::pthread_mutex_destroy(&mutex_); }

    void lock() { 
        ::pthread_mutex_lock(&mutex_); 
    }
    void unlock() { 
        ::pthread_mutex_unlock(&mutex_); 
    }
    pthread_mutex_t& getMutex() { return mutex_; }
};

class Condition {
    pthread_cond_t cond_;
public:
    typedef pthread_cond_t handle_type;

    Condition()  { ::pthread_cond_init(&cond_, 0); }
    ~Condition() { ::pthread_cond_destroy(&cond_); }
    
    void signal()    { 
        ::pthread_cond_signal(&cond_); 
    }
    void broadcast() {
        ::pthread_cond_broadcast(&cond_); 
    }
    void wait(Mutex::handle_type* mutex) { 
        ::pthread_cond_wait(&cond_, mutex );
    }
    void wait(Mutex& mutex) { 
        wait( & mutex.getMutex() );
    }
};

class Thread {
    pthread_t thread_;
public:
    explicit Thread(pthread_t thread): thread_(thread) {}
    static Thread current() { return Thread(::pthread_self()); }
    
    typedef void* (thread_entry)(void*);
    
    static Thread start( thread_entry entry, void* param );
    static Thread start_detached( thread_entry entry, void* param );
    
    void* join();
};

class ThreadSet {
    std::vector<Thread> threads_;

public:
    ~ThreadSet();

    Thread start(Thread::thread_entry entry, void* param);

    template <typename T>
        Thread start(void* (*entry)(T), T param) {
            return start(static_cast<Thread::thread_entry>(entry), (void*)param);
        }
    
    void   add(Thread t);

    /// Join all threads
    /// 
    /// Join all threads in the ThreadSet in reverse order reverse to
    /// addition/creation. Successfully joined threads are removed
    /// from ThreadSet.
    /// result vector pointer may be null if result is to be ignored.
    void join(std::vector<void*>* result = 0);
    
};
};

#endif