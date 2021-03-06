//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tstring.h"

#include "tinfra/assert.h"

#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <ostream>

#include <cstdlib>
#include <cassert>


namespace tinfra {

const tstring::size_type tstring::npos = ~(size_type)0;

int tstring::cmp(tstring const& other) const {
    size_t common_length = std::min(size(),other.size()); // WINDOWS min macro workaround
    
    int r = std::memcmp(data(), other.data(), common_length);
    if( r != 0 ) 
        return r;
    if( tstring::size() > other.size() )
        return -1;
    else if( tstring::size() < other.size() )
        return 1;
    else
        return 0;
}

tstring tstring::substr(size_type pos, size_type n) const
{
    assert(pos <= size());
    const size_t len = std::min(size()-pos, n);
    const size_t last_character_pos = pos+len;
    bool sub_is_null_terminated = 
           ( last_character_pos == size() && this->is_null_terminated())
        || ( last_character_pos <  size() && data()[last_character_pos] == '\0' );
    return tstring(data() + pos, len, sub_is_null_terminated);
}

tstring::size_type
tstring::find(char_type const* s, size_type pos, size_type n) const
{    
	if( size() == 0 ) {
		if( n == 0 )
			return 0;
		else
			return npos;
	}

    if( pos > this->size() ) {
        return npos;
    }

    tstring const other = tstring(s, n, false);
    const_iterator result = std::search(
        begin()+pos,
        end(),
        other.begin(),
        other.end());
    if( result == end() )
        return npos;
    else
        return result - begin(); 
}

// find first of
tstring::size_type 
tstring::find_first_of(char_type const* s, size_type pos, size_type n) const
{
    if( pos > this->size() ) {
        return npos;
    }
    if( tstring::size() > 0 ) {
        for( const_iterator i = begin()+pos; i != end(); ++i ) {
            if( std::memchr(s, *i, n) != 0 ) // 'not in S' so return
                return i - begin();
        }
    }
    return npos;
}
tstring::size_type 
tstring::find_first_of(char_type c, size_type pos) const
{
    if( pos > this->size() ) {
        return npos;
    }
    if( tstring::size() > 0 ) {
        for( const_iterator i = begin()+pos; i != end(); ++i ) {
            if( *i == c ) // '== C' so return
                return i - begin();
        }
    }
    return npos;
}

// find first not of

tstring::size_type
tstring::find_first_not_of(char_type const* s, size_type pos, size_type n) const
{
    if( pos > this->size() ) {
        return npos;
    }
    if( tstring::size() > 0 ) {
        for( const_iterator i = begin()+pos; i != end(); ++i ) {
            if( std::memchr(s, *i, n) == 0 ) // 'not in S' so return
                return i - begin();
        }
    }
    return npos;
}

tstring::size_type
tstring::find_first_not_of(char_type c, size_type pos) const
{
    if( pos > this->size() ) {
        return npos;
    }
    if( tstring::size() > 0 ) {
        for( const_iterator i = begin()+pos; i != end(); ++i ) {
            if( *i != c )
                return i - begin();
        }
    }
    return npos;
}

template <typename Predicate>
tstring::size_type
tstring_find_last(tstring const& subject, Predicate p, size_t pos)
{
	const size_t npos = tstring::npos;

	if( subject.size() == 0 ) 
		return npos;

	if( pos != npos && pos > subject.size() )
		pos = subject.size()-1;

	TINFRA_ASSERT(pos == npos || pos < subject.size() );

	const  tstring::const_iterator begin = subject.begin();
	tstring::const_iterator i = 
			(pos == npos) ? subject.end()-1
		                  : subject.begin() + pos;

	do {
        if( p(*i)  ) // predicate true, then found
            return i - begin;
	} while( i-- != begin );
	return npos;
}
// find last of



struct last_of_str_predicate {
	const char* pattern;
	size_t      pattern_len;

	bool operator()(char c) {
		return std::memchr(this->pattern, c, this->pattern_len) != 0;
	}
};

tstring::size_type
tstring::find_last_of(char_type const* s, size_type pos, size_type n) const
{
	last_of_str_predicate pred;
	pred.pattern = s;
	pred.pattern_len = (n == npos) ? std::strlen(s) : n;
	return tstring_find_last(*this, pred, pos);
}

struct last_of_char_predicate {
	char expected;
	bool operator()(char c) {
		return (c == expected);
	}
};

tstring::size_type
tstring::find_last_of(char_type c, size_type pos) const
{
    last_of_char_predicate pred;
	pred.expected = c;
	return tstring_find_last(*this, pred, pos);
}

// find last not of
struct last_not_of_str_predicate {
	const char* pattern;
	size_t      pattern_len;

	bool operator()(char c) {
		return std::memchr(this->pattern, c, this->pattern_len) == 0;
	}
};

tstring::size_type
tstring::find_last_not_of(char_type const* s, size_type pos, size_type n) const
{
	last_not_of_str_predicate pred;
	pred.pattern = s;
	pred.pattern_len = (n == npos) ? std::strlen(s) : n;
	return tstring_find_last(*this, pred, pos);
}

struct last_not_of_char_predicate {
	char expected;
	bool operator()(char c) {
		return (c != expected);
	}
};

tstring::size_type
tstring::find_last_not_of(char_type c, size_type pos) const
{
    last_not_of_char_predicate pred;
	pred.expected = c;
	return tstring_find_last(*this, pred, pos);
}
    

std::ostream& operator<<(std::ostream& out, tstring const& s)
{
    return out.write(s.data(), s.size());
}

const char* tstring::temporary_alloc(string_pool& pool, tstring const& s)
{
    return pool.create(s);
}

const char* string_pool::create(tstring const& src)
{
    strings.push_back(0);
    size_t len = src.size();
    char* result = reinterpret_cast<char*>( std::malloc(len+1) );
    strings[strings.size()-1] = result;
    
    std::memcpy(result, src.data(), len);
    result[len] = 0;
    return result;
}

void string_pool::clear()
{
    for( size_t i = 0; i < strings.size(); ++i ) {
        std::free(strings[i]);
        strings[i] = 0;
    }
    strings.clear();
}
string_pool::string_pool(size_t)
{
}

string_pool::~string_pool()
{
    clear();
}

} // end namespace tinfra

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:


