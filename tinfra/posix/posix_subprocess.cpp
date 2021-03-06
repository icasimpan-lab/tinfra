//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "../platform.h"
#ifdef TINFRA_POSIX

#include <sys/types.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <stdio.h>

#include "tinfra/posix/posix_stream.h"
#include "tinfra/fmt.h"
#include "tinfra/path.h"
#include "tinfra/subprocess.h"
#include "tinfra/os_common.h"
#include "tinfra/trace.h"
#include "tinfra/logger.h"
#include "tinfra/runtime.h" // for test_interrupt
extern "C" char** environ;

namespace tinfra {

namespace posix {

using tinfra::input_stream;
using tinfra::output_stream;

enum {
    MAX_NUMBER_OF_FILES = NOFILE // defined in <sys/param.h> on linux 
};

class fd_holder {
    int fd;
public:
    explicit fd_holder(int pfd): fd(pfd) {}
        
    ~fd_holder() { 
        if( fd != -1 )
            ::close(fd);
    }
    
    fd_holder& operator =(int pfd) { this->fd = pfd; return *this; }
    
    operator int () const { return fd; }
};


char** make_system_environment(environment_t const& e)
{
    char** result = new char*[e.size()+1];
    int idx = 0;
    for(environment_t::const_iterator ie = e.begin(); ie != e.end(); ++ie ) {
        std::string const& name = ie->first;
        std::string const& value = ie->second;
        const size_t string_size = name.size()+value.size()+1;
        char* ev = new char[string_size+1];
        memcpy(ev, name.data(), name.size());
        ev[name.size()] = '=';
        memcpy(ev + name.size()+1 , value.data(), value.size());
        ev[string_size] = 0;
        result[idx++] = ev;
    }
    result[idx] = 0;
    return result;
}

int execute_process(std::vector<std::string> const& args, environment_t const* e)
{
    char** raw_args;
    raw_args = new char*[args.size()+1];
    {
        int idx = 0;
        for(std::vector<std::string>::const_iterator i=args.begin(); i != args.end(); ++i ) {
            raw_args[idx++] = const_cast<char*>(i->c_str());
        }
        raw_args[idx] = 0;
    }
    if( !e ) {
        execvp(raw_args[0], raw_args);
    } else {
        char** env = make_system_environment(*e);
        std::string executable = tinfra::path::search_executable(raw_args[0]);
        if( executable.size() == 0 ) {
            delete[] env;
            //std::cerr << "tinfra::unable to find executable for " << raw_args[0] << "\n";
            TINFRA_LOG_ERROR(fmt("subprocess start failed: unable to find executable for '%s'") % raw_args[0]);
            return 127;
        }
        execve(executable.c_str(), raw_args, env);
    }
    TINFRA_LOG_ERROR(fmt("subprocess start failed: exec failed %s %s ...") % raw_args[0] % raw_args[1]);
    return 126;
}
//
// subprocess support for posix
//

struct posix_subprocess: public subprocess {
    input_stream*  soutput;
    output_stream* sinput;
    input_stream*  serror;
    
    int pid;
    
    int exit_code;
    environment_t env;
    bool          env_set;
    
    posix_subprocess() :
        soutput(0),
        sinput(0),
        serror(0),
        pid(-1),
        exit_code(-1),
        env_set(false)
    {
    }
    
    ~posix_subprocess() {
        delete sinput;
        delete soutput;
        if( soutput != serror )
            delete serror;
    }
    virtual void     set_environment(environment_t const& e)
    {
        env_set = true;
        env = e;
    }
    
    virtual void     wait() {
        if( pid == -1 ) 
            return;
        while( true ) {
            int exit_code_raw;
            int tpid = ::waitpid(pid, &exit_code_raw, 0);
            
            if( tpid < 0 &&  errno == EINTR ) {
                tinfra::test_interrupt();
                continue;
            }
            if( tpid < 0 )
                throw_errno_error(errno, "waitpid failed");
            // now convert wait exit_code to process exit code as in wait(2)
            // manual
            if(  WIFSIGNALED(exit_code_raw) ) {
                exit_code = EXIT_FAILURE;
                this->signal_information = WTERMSIG(exit_code_raw);
            } else if( WIFEXITED(exit_code_raw) ) {
                exit_code = WEXITSTATUS(exit_code_raw);
            } else {
                exit_code = EXIT_FAILURE;
            }
            //std::cerr << "PSP: waitpid(" << pid << ") returned " << exit_code << std::endl;            
            return;
        }
    }
    
    virtual void     detach()
    {
        exit_code = -1;
        pid = -1;
    }
    
    virtual int      get_exit_code() {        
        return exit_code;
    }
    
    virtual void     terminate() {
        if ( ::kill(pid, SIGTERM) < 0 ) 
            throw_errno_error(errno, fmt("kill(%i,SIGTERM)") % pid);
    }
    
    virtual void     kill() { 
        if ( ::kill(pid, SIGKILL) < 0 ) 
            throw_errno_error(errno, fmt("kill(%i,SIGKILL)") % pid);
    }
    
    virtual output_stream*  get_stdin()  { return sinput;  }
    virtual input_stream*   get_stdout() { return soutput; }
    virtual input_stream*   get_stderr() { return serror; }
    
    virtual intptr_t get_native_handle() { return pid; }
    
    void start(const char* command) {
        std::vector<std::string> args;
        args.push_back("/bin/sh");
        args.push_back("-c");
        args.push_back(command);
        start(args);
    }
    
    void start(std::vector<std::string> const& args)
    {
        fd_holder out_here(-1);    // writing HERE -> child
        fd_holder out_remote(-1);  // reading CHILD <- here
        
        fd_holder in_here(-1);     // reading HERE <- child
        fd_holder in_remote(-1);   // writing CHILD -> here
        
        fd_holder err_here(-1);    // reading HERE <- child (diagnostic)
        fd_holder err_remote(-1);  // writing CHILD -> here (diagnostic)
                
        const bool fread =  (stdout_mode == REDIRECT);
        const bool fwrite = (stdin_mode == REDIRECT);
        const bool ferr  =  (stderr_mode == REDIRECT && !redirect_stderr);
        
        sinput = 0;
        serror = 0;
        soutput = 0;
        
        if( fwrite ) {            
            int boo[2];
            if ( pipe(boo) < 0 ) 
                throw_errno_error(errno, "pipe failed");
            out_remote = boo[0];
            out_here = boo[1];
        }
        if( fread ) {
            int boo[2];
            if ( pipe(boo) < 0 ) 
                throw_errno_error(errno, "pipe failed");
            
            in_here   = boo[0];
            in_remote = boo[1];
        }
        if( ferr ) {
            int boo[2];
            if ( pipe(boo) < 0 ) 
                throw_errno_error(errno, "pipe failed");
            
            err_here   = boo[0];
            err_remote = boo[1];
        }
        
        pid = ::fork();
        if( pid == -1 ) 
            throw_errno_error(errno, "fork failed");
            
        if( pid == 0 ) {
            // child part
            {
                sigset_t mask;
                sigemptyset(&mask);
                if( sigprocmask(SIG_SETMASK, &mask, NULL) < 0 ) {
                    TINFRA_LOG_ERROR(fmt("warning: unable to reset signal mask in child process") % errno_to_string(errno));
                }
            }
            if( fwrite ) {
                int a = ::dup(out_remote);
                ::dup2(a,0);
                ::close(a);
                
                // detach from tty
                //
                // TODO: decide if we should really detach from tty - and how?
                //	always/flag/never? connection with detached flag on woe32 ?
                
                // now reasoing is following:
                //   if we're redirecting stdin of subprocess then we wanw
                //   it to read from us not TTY  ==> always detach if stdin == REDIRECT
		// on openindiana, ioctl(/dev/tty, TIOCNOTTY) fails, so ignore
		// but i don't know why :/
#ifndef sun		
                int tty_fd = open("/dev/tty", O_RDWR);
                if (tty_fd >= 0) {
                    int ret = ioctl(tty_fd, TIOCNOTTY, 0);
                    if (ret == -1) {
                        perror("ioctl  (disabling tty)");
			TINFRA_LOG_ERROR(fmt("warning: detaching from tty failed: ioctl(/dev/tty,TIOCNOTTY) returned errno: %s") % errno_to_string(errno));
		    }
                    close(tty_fd);
                }
                else {
		    TINFRA_LOG_ERROR(fmt("warning: detaching from tty failed: open(/dev/tty) returned errno: %s)") % errno_to_string(errno));
                }
#endif
            } else if( stdin_mode == NONE) { 
                ::close(0);
                ::open("/dev/null",O_RDONLY);
            }
            if( fread ) {
                int a = ::dup(in_remote);
                ::dup2(a,1);
                ::close(a);
            } else if( stdout_mode == NONE) { 
                ::close(1);
                ::open("/dev/null",O_WRONLY);
            }
            
            if( ferr ) {
                int a = ::dup(err_remote);
                ::dup2(a,2);
                ::close(a);
            } else if (redirect_stderr ) {
                ::dup2(1, 2);
            } else if( stderr_mode == NONE) { 
                ::close(2);
                ::open("/dev/null",O_WRONLY);
            }

            for( int fd = 3; fd < MAX_NUMBER_OF_FILES; ++fd ) 
                ::close(fd);
            
            if( ::chdir(working_dir.c_str()) < 0 ) {
                TINFRA_LOG_ERROR(fmt("subprocess start: unable to chdir to '%s'") % working_dir);
                ::exit(127);
            }
            int result = execute_process(args, env_set ? &env : 0);  
            _exit(result);
        } else {
            if( fwrite ) {
                sinput = new tinfra::posix::native_output_stream(out_here, true);
                out_here = -1;
            }
            
            if( fread ) {
                soutput = new tinfra::posix::native_input_stream(in_here, true);
                in_here = -1;
            }
            
            if( ferr ) {
                serror = new tinfra::posix::native_input_stream(err_here, true);
                err_here = -1;
            } else if( redirect_stderr ) {
                serror = soutput;
            } 
        }
    }
};

} // end namespace posix

std::auto_ptr<subprocess> subprocess::create()
{
    return std::auto_ptr<subprocess>(new posix::posix_subprocess());
}


} // end namespace tinfra

#endif // TINFRA_POSIX

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++


