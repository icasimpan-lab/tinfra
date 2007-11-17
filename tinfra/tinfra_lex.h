#ifndef __tinfra_lex_h__
#define __tinfra_lex_h__
#include <sstream>

#include "tinfra.h"

namespace tinfra {

class bad_lexical_cast: public std::bad_cast {
    std::string _what;
public:
    bad_lexical_cast(const char* src, const char* dest) {
        _what = string("cannot cast from '") + string(src) + string("' to '") + string(dest) + "'";
    }
    virtual ~bad_lexical_cast() throw() {}
        
    const char* what() const throw() {
        return _what.c_str();
    }
};
namespace detail {
	template <typename T>
	struct DefaultLexicalInterpreter {
		static void to_string(T const& v, std::string& dest) { 
			std::ostringstream fmt;
			fmt << v; // TODO: catch IO failures
			dest = fmt.str();
			//std::cerr << "to_string<" << TypeTraits<T>::name() << ">(" << v << ") -> " << dest.c_str() << std::endl;
		}
		static void from_string(const char* v, T& dest) { 
			std::istringstream in(v);
			in >> dest; // TODO: catch IO, format failures
			
			//std::cerr << "from_string<" << TypeTraits<T>::name() << ">(" << v << ") -> " << dest << std::endl;
		}
	};
};

// default implementation to catch all default casts
template <typename T>
struct LexicalInterpreter {
	static void to_string(T const& v, std::string& r) {
	    //std::cerr << "WARNING! to_string<" << TypeTraits<T>::name() << ">(" << v << ") -> \"\"" << std::endl;
	    r = "<no conversion>";
	}
	static void from_string(const char* v, T& t) {
	    std::cerr << "WARNING! from_string<" << TypeTraits<T>::name() << ">(" << v << ") -> NO OOP" << std::endl;
	    t = T();
	}
};

// default implementation provided by default stream bases string IO
template<> struct LexicalInterpreter<int>: public detail::DefaultLexicalInterpreter<int> {};
template<> struct LexicalInterpreter<unsigned int>: public detail::DefaultLexicalInterpreter<unsigned int> {};

template<> struct LexicalInterpreter<char>: public detail::DefaultLexicalInterpreter<char> {};
template<> struct LexicalInterpreter<signed char>: public detail::DefaultLexicalInterpreter<signed char> {};
template<> struct LexicalInterpreter<unsigned char>: public detail::DefaultLexicalInterpreter<unsigned char> {};

template<> struct LexicalInterpreter<short>: public detail::DefaultLexicalInterpreter<short> {};
template<> struct LexicalInterpreter<unsigned short>: public detail::DefaultLexicalInterpreter<unsigned short> {};

template<> struct LexicalInterpreter<long>: public detail::DefaultLexicalInterpreter<long> {};
template<> struct LexicalInterpreter<unsigned long>: public detail::DefaultLexicalInterpreter<unsigned long> {};
    
template<> struct LexicalInterpreter<float>: public detail::DefaultLexicalInterpreter<float> {};    
template<> struct LexicalInterpreter<double>: public detail::DefaultLexicalInterpreter<double> {};

// strings are can be casted with no-op
template<> 
struct LexicalInterpreter<std::string> {
	static void to_string(std::string const& v, std::string& dest) {
		dest = v;
	}
	static void from_string(const char* v, std::string& dest) {
		dest = v;
	}
};

template<typename T, int N> 
struct LexicalInterpreter<T[N]> {
	static void to_string(T v[N], std::string& dest) {
		dest = v;
	}
	static void from_string(const char* v, char dest[N]) {
		if( ::strlen(v) <= N-1 ) {
		    ::strcpy(dest,v);
		} else {
		    throw bad_lexical_cast(TypeTraits<const char*>::name(), TypeTraits<char[N]>::name());
		}
	}
};

template<> 
struct LexicalInterpreter<Symbol> {
	static void to_string(Symbol const& v, std::string& dest) {		
	    std::stringstream a;
	    a << v.c_str() << "(" << v.getId() << ")";
	    dest = a.str();
	    //dest = v.getName();
	}
	static void from_string(const char* v, Symbol& dest) {	    
	    dest = Symbol::get(v);
	    std::cerr << "IN(" << v << ") -> symbol " << dest.c_str() << "(" << dest.getId() << ")" << std::endl;
	}
};
//
//
//

namespace detail {    
	class LexicalSetter {
		Symbol      _field;
		char const* _value;
	public:		
		LexicalSetter(Symbol field,char const* value): _field(field), _value(value) {}
		
		template <typename F>
		void operator ()(Symbol const& symbol, F& v) {
			if( symbol == _field ) LexicalInterpreter<F>::from_string(_value, v);
		}
	    
	};

	class LexicalGetter {
	public:
		Symbol       _field;
		std::string& _dest;		
	public:
		LexicalGetter(Symbol field, std::string& dest): _field(field), _dest(dest) {}
		
		template <class F>
		void operator ()(Symbol const& symbol, F const& v) {
			if( symbol == _field ) LexicalInterpreter<F>::to_string(v, _dest);
		}    
	};
} // end of details

///
/// lexical get & set
///

template<typename T>
void lexical_set(T& obj, Symbol field, char const* value)
{
	detail::LexicalSetter setter(field,value);
	mutate(obj, setter);
}
template<typename T>
void lexical_set(T& obj, Symbol field, std::string const& value)
{
	detail::LexicalSetter setter(field,value.c_str());
	mutate(obj, setter);
}

template<typename T>
void lexical_get(T const& obj, Symbol field, std::string& dest)
{
	detail::LexicalGetter getter(field,dest);
	process(obj, getter);
}

template<typename T>
std::string lexical_get(T const& obj, Symbol field)
{
	std::string dest;
	detail::LexicalGetter getter(field,dest);
	process(obj, getter);
	return dest;
}

template<typename F>
void from_string(char const* str, F& dest) {
        LexicalInterpreter<F>::from_string(str, dest);
}


template <typename F>
void to_string(F const& value, std::string& dest) {
	LexicalInterpreter<F>::to_string(value, dest);
}

} // nemaspace tinfra

#endif // __tinfra_lex_h__
