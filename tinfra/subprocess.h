//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_subprocess_h_included
#define tinfra_subprocess_h_included

#include "tinfra/platform.h"

#include "tinfra/stream.h"
#include "tinfra/runtime.h"
#include "tinfra/tstring.h"

#include <memory>
#include <map>
#include <string>

namespace tinfra {


class subprocess {
public:
    // create default platform implementation
    static std::auto_ptr<subprocess> create();

    enum pipe_mode {
        INHERIT,
        REDIRECT, 
        NONE
    };
    subprocess() : 
        stdin_mode(INHERIT), 
        stdout_mode(INHERIT), 
        stderr_mode(INHERIT), 
        redirect_stderr(false), 
        working_dir("."),
        signal_information(0)
    {
    }
    void set_working_dir(std::string const& dir ) { working_dir = dir; }
    // TODO: add environment setting
    
    void set_redirect_stderr(bool r = true) { redirect_stderr = r; }
    
    void set_stdin_mode(pipe_mode pm)   { stdin_mode = pm; }
    void set_stdout_mode(pipe_mode pm)  { stdout_mode = pm; }
    void set_stderr_mode(pipe_mode pm)  { stderr_mode = pm; }
    
    int      get_exit_signal() { return signal_information; }
    
    virtual ~subprocess() {}
    
    virtual void     set_environment(environment_t const& e) = 0;
    virtual void     start(const char* command) = 0;
    virtual void     start(std::vector<std::string> const& args) = 0;
    
    virtual void     wait() = 0;
    virtual void     detach() = 0;
    virtual int      get_exit_code() = 0;
    	     
    
    virtual void     terminate() = 0;
    virtual void     kill() = 0;
    
    virtual tinfra::output_stream*  get_stdin() = 0;
    virtual tinfra::input_stream*   get_stdout() = 0;
    virtual tinfra::input_stream*   get_stderr() = 0;
    
    virtual intptr_t get_native_handle() = 0;
    
protected:
    pipe_mode stdin_mode;
    pipe_mode stdout_mode;
    pipe_mode stderr_mode;

    bool redirect_stderr;

    std::string working_dir;
    int signal_information;
};

// deprecated
subprocess* create_subprocess();

// TODO: start_detached& capture_command are members of ::tinfra
// they should include "process, program, subprocess" in name
// or become member of subprocess ?

/// Capture command output.
/// $(command) or `command` subsitute
/// TODO: create stream counterpart
std::string capture_command(std::string const& command);

/// Capture command output.
/// same as capture_command but overrides program environment
///
std::string capture_command(std::string const& command, environment_t const& env);

/// Start a process, don't wait for finalization.
///
/// Throws on error.
void start_detached(tstring const& command);

/// Start a process, don't wait for finalization.
///
/// Version that overrides subprocess
/// Throws on error.
void start_detached(tstring const& command, environment_t const& env);

} // end of namespace tinfra

#endif // #ifndef tinfra_subprocess_h_included
