#ifndef __tinfra_exception_h_
#define __tinfra_exception_h_

#include <exception>
#include <string>
#include <vector>

namespace tinfra {

struct stackentry {
    void*	address;
    std::string symbol;
};

typedef std::vector<stackentry> stacktrace_t;

void populate_stacktrace(stacktrace_t& dest,int ignore_stacks=0);

class generic_exception: public std::exception {
public:
    generic_exception(std::string const& message);
    virtual ~generic_exception() throw() {}
    
    virtual const char* what() const throw() { return _message.c_str(); }

    stacktrace_t const& stacktrace() const { return _stacktrace; }

protected:
    std::string   _message;
    stacktrace_t  _stacktrace;
};
} // end of namespace tinfra

#endif
