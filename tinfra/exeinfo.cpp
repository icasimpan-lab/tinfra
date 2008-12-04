//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <istream>
#include <cstdio>
#include <cstring>
#include <cctype>

#include "tinfra/exeinfo.h"
namespace tinfra {

static void trim(char* str)
{	
	size_t len = std::strlen(str);
	while( std::isspace(str[len-1]) && len > 0) {
		str[len-1] = '\0';
		len--;
	}
}

void read_symbol_map(std::istream& input, symbol_table_visitor visitor)
{
    char linebuf[1024];
    
    while( input.getline(linebuf, sizeof(linebuf)) ) {
        trim(linebuf);
        intptr_t address = 0;
        char type = 0;
        char name_buf[1024];
        char filename_buf[1024];
        int  line_number = 0;
        //FIXME - don't work for symbols with spaces in names
		int address_tmp;
        int result = std::sscanf(linebuf, "%08x %c %s\t%s:%i", &address_tmp, &type, name_buf, filename_buf, &line_number);
		address = address_tmp;
        symbol_info current_symbol;
        current_symbol.address = 0;
        current_symbol.name = 0;
        current_symbol.file_name = 0;
        current_symbol.line_number = 0;
        if( result == 0 ) continue;
        current_symbol.address = address;
        if( result >= 3 )
            current_symbol.name = name_buf;
        if( result >= 4 )
            current_symbol.file_name = filename_buf;
        if( result >= 5 )
            current_symbol.line_number = line_number;
        visitor(current_symbol);
    }
}

static std::string exepath = "";

std::string get_exepath()
{
    if( exepath.size() == 0 ) {
        // TODO: write some OS-dependent code here
		return "";
    } else {
        return exepath;
    }
}
void set_exepath(std::string const& path)
{    
    exepath = path;
}

} // end namespace tinfra