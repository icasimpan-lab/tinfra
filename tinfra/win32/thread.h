//
// Copyright (C) 2008,2009  Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

//
// win32/thread.h
//   win32 based implementation of threads
//   STILL NOT READY

#ifndef __tinfra_win32_thread_h__
#define __tinfra_win32_thread_h__

#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define _WIN32_WINNT 0x0500 // Windows 2000
#include <windows.h>

namespace tinfra {
namespace thread {

class mutex {
public:    
    mutex();
    ~mutex();

    void lock();
    void unlock();

    CRITICAL_SECTION* get_native() { return &mutex_; };
private:
    CRITICAL_SECTION mutex_;
};

class condition {    
public:
    typedef void* handle_type;

    condition();
    ~condition();
    
    void signal();
    void broadcast();
    void wait(mutex& mutex);

    //CONDITION_VARIABLE* get_native() { return &cond_; }
private:
    mutex            internal_lock;
    unsigned long    current_generation;

    unsigned long    signal_count;
    HANDLE           signal_sem;
    unsigned long    signal_generation;

    unsigned long    waiters_count;

    // broadcast related
    bool             was_broadcast;
    HANDLE           broadcast_ended;
};

class thread {
public:
    typedef HANDLE handle_type;

    explicit thread(handle_type thread, DWORD thread_id_);
    ~thread();

    static thread current();
    
    static void sleep(long milliseconds);
    
    typedef void* (thread_entry)(void*);

    static thread start( Runnable& runnable);
    
    /// Start a detached thread
    /// runnable will be deleted after thread end
    static void start_detached( Runnable* runnable);    
    
    static thread start( thread_entry entry, void* param );
    static void   start_detached( thread_entry entry, void* param );
    
    void* join();
    
    size_t to_number() const;
    
    HANDLE native_handle() const { return thread_handle_; }
    DWORD  native_id() const { return thread_id_; }
    
private:
    handle_type thread_handle_;
    DWORD       thread_id_;
};

} } // end namespace tinfra::thread

#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

