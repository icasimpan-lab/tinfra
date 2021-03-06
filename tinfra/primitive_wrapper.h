//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_primitive_wrapper_h_included
#define tinfra_primitive_wrapper_h_included

#include <iosfwd> // for std::ostream

namespace tinfra {
namespace primitive_wrapper {
    
//
// types defined in RFC
//
template <typename T>
class integer_wrapper {
    T _v;
    
public:
    integer_wrapper() {}
    integer_wrapper(T v): _v(v) {}

    
    integer_wrapper& operator=(T v) { _v = v; return *this;}
    
    operator T const&() const { return _v; }
    operator T&() { return _v; }
};

template <typename T> 
struct really_integer_for {
    typedef T type;
};

template<> struct really_integer_for<char> { typedef int type; };
template<> struct really_integer_for<signed char> { typedef int type; };
template<> struct really_integer_for<unsigned char> { typedef unsigned int type; };

#define PRIMITIVE_INTEGER_WRAPPER(basetype, name)                 \
    typedef integer_wrapper<basetype> name;                       \
    inline std::ostream& operator <<(std::ostream& s, name v) {       \
        return s << static_cast<really_integer_for<basetype>::type >(v); \
    }

PRIMITIVE_INTEGER_WRAPPER(signed   char,      int8);
PRIMITIVE_INTEGER_WRAPPER(unsigned char,      uint8);
PRIMITIVE_INTEGER_WRAPPER(signed   short,     int16);
PRIMITIVE_INTEGER_WRAPPER(unsigned short,     uint16);
PRIMITIVE_INTEGER_WRAPPER(signed   int,       int32);
PRIMITIVE_INTEGER_WRAPPER(unsigned int,       uint32);
PRIMITIVE_INTEGER_WRAPPER(signed   long long, int64);
PRIMITIVE_INTEGER_WRAPPER(unsigned long long, uint64);

}} // end namespace tinfra::primitive_wrapper

#endif // tinfra_primitive_wrapper_h_included
