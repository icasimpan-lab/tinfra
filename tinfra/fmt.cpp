//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/fmt.h"
#include <string>
#include <cctype>
#include <iterator>
#include <algorithm>

namespace tinfra {

struct fmt_command {
    char  command;
    int   width;
    int   precision;
    char  fill;
};
    
template <typename T>
size_t process_fmt(std::string fmt, size_t start, fmt_command& result, T& output)
{
    std::string::const_iterator istart = fmt.begin() + start;
    std::string::const_iterator c = istart;
    
    const std::string::const_iterator end = fmt.end();
    
    result.command = '?';
    result.width = 0;
    result.precision = 0;
    result.fill = ' ';
    //std::cout << "pf: " << fmt << " at " << start << std::endl;
    enum {
        FREE_TEXT,
        BEGIN_MATCH,
        PADDING_GIVEN,
    } state = FREE_TEXT;
    bool in_precision = false;
    while( c != end )  switch( state ) {
    case FREE_TEXT:
        if( *c == '%' ) {
            if( c+1 == end ) 
                throw format_exception("bad format: '%' at the end");
            state = BEGIN_MATCH;
            in_precision = false;
            output.append(istart, c);
            ++c;
        } else {
            ++c;
        }
        continue;
    case BEGIN_MATCH:
        if( *c == '%' ) {
            ++c;
            istart = c;
            output.append(1,'%');
            state = FREE_TEXT;
            continue; // %% sequence is an escape
        }
        if( *c == '0' ) {
            result.fill = '0';
            state = PADDING_GIVEN;
            ++c;
            continue;
        }
    case PADDING_GIVEN:
        if( *c == '.' )  {
            in_precision = true;
            ++c;
            continue;
        }
        if( std::isdigit(*c) ) {
            if( in_precision ) {
                result.precision = result.precision*10 + (*c - '0');
            } else {
                result.width = result.width*10 + (*c - '0');
            }
            ++c;
            continue;
        }
        if( std::isalpha(*c) ) {
            result.command = *c;
            ++c;
            state = FREE_TEXT;
            return c - fmt.begin();
        }
        throw format_exception("basic_fmt: bad format character");
    }
    //std::cout << "pf: " << -1 << std::endl;
    output.append(istart, end);
    return (size_t)-1;
}

class ostream_string_output {
    std::ostream& out;
    
public:
    ostream_string_output(std::ostream& out): out(out) {}
        
    void append(int n, char c)
    {
        for( int i = 0; i < n; ++i ) {
            out << c;
        }
    }
    void append(std::string::const_iterator begin, std::string::const_iterator end)
    {
        /*if( end != begin ) {
            const char* pb = & (*begin);
            size_t size = distance(begin,end);
            out.write(pb, size);
        }*/
        std::copy(begin, end, std::ostream_iterator<char>(out));
    }
};

///
/// basic_fmt
///
void basic_fmt::reset() {
    pos_ = 0;
}
size_t basic_fmt::check_command(std::ostream& formatter)
{
    fmt_command cmd;
    ostream_string_output output(formatter);
    
    size_t cmd_pos = process_fmt(this->fmt_, this->pos_, cmd, output);
    if( cmd_pos == (size_t)-1 ) 
        throw format_exception("basic_fmt: too many actual arguments");
    if( cmd.fill )
        formatter.fill(cmd.fill);
    if( cmd.width )
        formatter.width(cmd.width);
    if( cmd.precision )
        formatter.precision(cmd.precision);
    
    switch( cmd.command ) {
    case 's':
    case 'i':
    case 'd':
        std::dec(formatter);
        break;
    case 'x':
        std::hex(formatter);
        break;
    default:
        throw format_exception(fmt("basic_fmt: bad format command %s") % cmd.command);
    }
    return cmd_pos;
}

void basic_fmt::realize()
{
    fmt_command cmd;
    std::ostream formatter(this->buf_);
    ostream_string_output output(formatter);
    if( process_fmt(this->fmt_, this->pos_, cmd, output) != (size_t)-1 ) 
        throw format_exception("basic_fmt: not all arguments realized");
    
    this->pos_ = fmt_.size();
}


std::ostream& operator << (std::ostream& out, stringbuf_fmt& fmt)
{
    return out << fmt.str();
}

} // end namespace tinfra
