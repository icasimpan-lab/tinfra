#include "json.h" // we implement this

#include "tinfra/variant.h"
#include "tinfra/memory_stream.h"
#include "tinfra/lex.h"
#include "tinfra/fmt.h"
#include "tinfra/string.h"
#include "tinfra/trace.h"

#include <ostream>
// impl
namespace tinfra {

std::ostream& operator <<(std::ostream& s, json_token::token_type tt)
{
    switch( tt ) {
    case json_token::OBJECT_BEGIN: s << "object begin '{'"; break;
    case json_token::OBJECT_END: s << "object end '}'"; break;
    case json_token::ARRAY_BEGIN: s << "array begin '['"; break;
    case json_token::ARRAY_END: s << "array end ']'"; break;
    case json_token::COMMA: s << "comma ','"; break;
    case json_token::COLON: s << "colon ':'"; break;
    case json_token::STRING: s << "string"; break;
    case json_token::INTEGER: s << "integer"; break;
    case json_token::DOUBLE: s << "double"; break;
    case json_token::TOK_TRUE: s << "true"; break;
    case json_token::TOK_FALSE: s << "false"; break;
    case json_token::TOK_NULL: s << "null"; break;
    }
    return s;
}

tinfra::module_tracer json_parser_tracer(tinfra::tinfra_tracer, "json_parser");
class json_parser {
    json_lexer& lexer;
    variant&    root;
    
    json_token  current;
    bool        finished;
public:
    json_parser(json_lexer& l, variant& target):
        lexer(l),
        root(target),
        finished(false)
    {
        next();
    }

    bool next() {
        if( finished )
            return false;
        this->finished = !this->lexer.fetch_next(this->current);
        TINFRA_TRACE(json_parser_tracer, "readed token " << this->current.type << " finished=" << finished);
        return !this->finished;
    }
    
    void next_or_fail(const char* message) 
    {
        if( !next() ) {
            fail(message);
        }
    }
    
    void fail(std::string const& message)
    {
        TINFRA_TRACE(json_parser_tracer, "failure: " << message);
        throw std::runtime_error(tsprintf("json_parser: %s", message));
    }
    void expect(json_token::token_type tt) {
        if( finished ) {
            fail(tsprintf("expecting %s, but end of input reached",
                                              tt));
        }
        if( this->current.type != tt ) {
            fail(tsprintf("expecting %s, but found %s",
                 tt, this->current.type));
        }
    }
    void parse()
    {
        parse_node(root);
        if( ! finished ) {
            fail("expected EOF after last object");
        }
    }
    void parse_node(variant& dest)
    {
        if( finished )
            return;
        switch( current.type ) {
        case json_token::OBJECT_BEGIN:
            dest = variant::dict();
            parse_object(dest);
            break;
        case json_token::ARRAY_BEGIN:
            dest = variant::array();
            parse_array(dest);
            break;
        case json_token::STRING:
            dest.set_string(current.value.str());
            next();
            break;
        case json_token::INTEGER:
            {
                variant::integer_type value = tinfra::from_string<variant::integer_type>(current.value.str());
                dest.set_integer(value);
            }
            next();
            break;
        case json_token::DOUBLE:
            {
                double value = tinfra::from_string<double>(current.value.str());
                dest.set_double(value);
            }
            next();
            break;
        default:
            fail(tsprintf("expected value but %s found", current.type));
        }
    }
    
    void parse_object(variant& dict)
    {
        expect(json_token::OBJECT_BEGIN);
        next_or_fail("expected } or \"key\":value when parsing object but EOF found"); 
        
        if( current.type == json_token::OBJECT_END ) {
            // short circuit for {}
            next();
            return;
        }
        
        while( true ) {
            // 'foo'
            expect(json_token::STRING);
            std::string key(current.value.str());
            next();
            // 'foo' :
            expect(json_token::COLON);
            next_or_fail("expected value after ':'");
            {
                // 'foo' : ANYTHING
                variant tmp;
                parse_node(tmp);
                
                using std::swap;
                swap(dict[key],tmp);
            }
            
            // now decide: next or end
            if( current.type == json_token::COMMA ) {
                // , -> continue
                next();
                continue;
            }
            if( current.type == json_token::OBJECT_END ) {
                // } -> break
                next();
                break;
            }
            fail("expected ',' or '}' after value when parsing dictionary");
        }
    }
    
    void parse_array(variant& array)
    {
        expect(json_token::ARRAY_BEGIN);
        next_or_fail("expected ] or value when parsing array but EOF found"); 
        
        if( current.type == json_token::ARRAY_END ) {
            // short circuit for []
            next();
            return;
        }
        
        while( true ) {
            {
                // ANYTHING
                variant tmp;
                parse_node(tmp);
                
                using std::swap;
                swap(array[array.size()],tmp);
            }
            
            // now decide: next or end
            if( current.type == json_token::COMMA ) {
                // , -> continue
                next_or_fail("expected value after ','");
                continue;
            }
            if( current.type == json_token::ARRAY_END ) {
                // ] -> break
                next();
                break;
            }
            fail("expected ',' or ']' after value when parsing array");
        }
    }
};
variant json_parse(tstring const& s)
{
    memory_input_stream stream(s.data(), s.size(),USE_BUFFER);
    return json_parse(stream);
}

variant json_parse(tinfra::input_stream& in)
{
    json_lexer lexer(in);
    variant result;
    json_parser parser(lexer, result);
    
    parser.parse();
    return result;
}

tinfra::module_tracer json_renderer_tracer(tinfra::tinfra_tracer, "json_renderer");

json_renderer::json_renderer(tinfra::output_stream& out, json_encoding enc): 
    out(out),
    enc(enc) 
{}

void json_renderer::object_begin() {
    this->out.write("{",1);
}
void json_renderer::object_end() {
    this->out.write("}",1);
}
void json_renderer::array_begin() {
    this->out.write("[",1);
}
void json_renderer::array_end() {
    this->out.write("]",1);
}
void json_renderer::comma() {
    this->out.write(",",1);
}
void json_renderer::colon() {
    this->out.write(":",1);
}
void json_renderer::string(tstring const& str)
{
    std::string escaped = escape_c(str);
    this->out.write("\"",1);
    this->out.write(escaped);
    this->out.write("\"",1);
}
void json_renderer::integer(variant::integer_type v)
{
    std::string formatted = tinfra::to_string(v);
    this->out.write(formatted);
}
void json_renderer::double_(double v)
{
    std::string formatted = tinfra::to_string(v);
    this->out.write(formatted);
}
void json_renderer::boolean(bool v)
{
    if( v ) {
        this->out.write("true");
    } else {
        this->out.write("false");
    }
}
void json_renderer::none()
{
    this->out.write("nil");
}

//
// json_writer
//

json_writer::json_writer(json_renderer& renderer):
    renderer(renderer),
    need_separator(false)
{
}

json_writer::~json_writer()
{
    while(!this->stack.empty()) 
        this->end();
}

void json_writer::begin_object()
{
    this->stack.push(OBJECT);
    this->renderer.object_begin();
    need_separator = false;
}

void json_writer::named_begin_object(tstring const& name)
{
    TINFRA_ASSERT(this->current_type() == OBJECT);
    if( this->need_separator ) {
        this->renderer.comma();
    }
    this->renderer.string(name);
    this->renderer.colon();

    this->begin_object();
}

void json_writer::begin_array()
{
    this->stack.push(ARRAY);
    this->renderer.array_begin();
    need_separator = false;
}

void json_writer::named_begin_array(tstring const& name)
{
    TINFRA_ASSERT(this->current_type() == OBJECT);
    if( this->need_separator ) {
        this->renderer.comma();
    }
    this->renderer.string(name);
    this->renderer.colon();

    this->begin_array();
}

void json_writer::end_object()
{
    TINFRA_ASSERT(!this->stack.empty());
    TINFRA_ASSERT(this->current_type() == OBJECT);

    this->renderer.object_end();
    this->stack.pop();
    this->need_separator = true;
}
void json_writer::end_array()
{
    TINFRA_ASSERT(!this->stack.empty());
    TINFRA_ASSERT(this->current_type() == ARRAY);

    this->renderer.array_end();
    this->stack.pop();
    this->need_separator = true;
}
void json_writer::end()
{
    TINFRA_ASSERT(!this->stack.empty());
    if( this->current_type() == OBJECT ) {
        this->end_object();
    } else {
        this->end_array();
    }
}

//  a value, valid only in array context or after name
void json_writer::value_impl(variant const& v)
{
    if( v.is_dict() ) {
        variant::dict_type const& dict = v.get_dict();
        this->value_impl(dict);
    } else if( v.is_array() ) {
        variant::array_type const& array = v.get_array();
        this->value_impl(array);
    } else if( v.is_string() ) {
        this->value_impl(tstring(v.get_string()));
    } else if( v.is_integer() ) {
        this->value_impl(v.get_integer());
    } else if ( v.is_double() ) {
        this->value_impl(v.get_double());
    } else if ( v.is_none() ) {
        this->value_impl();
    }
}
void json_writer::value_impl(tstring const& value)
{
    this->renderer.string(value);
}
void json_writer::value_impl(variant::integer_type const& value)
{
    this->renderer.integer(value);
}

void json_writer::value_impl(int const& value)
{
    this->renderer.integer(value);
}

void json_writer::value_impl(double value)
{
    this->renderer.double_(value);
}
void json_writer::value_impl(bool value)
{
    this->renderer.boolean(value);
}
void json_writer::value_impl() // none/nil
{
    this->renderer.none();
}

void        json_write(variant const& v, tinfra::output_stream& out, json_encoding encoding)
{
    json_renderer renderer(out, encoding);
    json_writer writer(renderer);
    writer.value(v);
}

std::string json_write(variant const& v, json_encoding encoding)
{
    std::string result;
    tinfra::memory_output_stream stream(result);
    
    json_write(v, stream, encoding);
    //stream->flush();
    return result;
}

//
// json_lexer
//

tinfra::module_tracer json_lexer_tracer(tinfra::tinfra_tracer, "json_lexer");

struct json_lexer::internal_data {
    int   current;
    bool  finished;
    
    input_stream* input;
    json_encoding input_encoding;
    enum {
        BUFFER_LEN = 10
    };
    char*   buffer_start;
    char*   buffer_end;
    char    buffer[BUFFER_LEN];
    
    std::string last_token;
    
    size_t    bytes_in_buffer() {
        return buffer_end - buffer_start;
    }
    
    bool fill_buffer(size_t required_size_at_least) {
        const size_t current_len_at_start = this->bytes_in_buffer();
        
        if( current_len_at_start >= required_size_at_least )
            return true;
        if( finished ) {
            return false;
        }
        
        TINFRA_ASSERT(required_size_at_least < BUFFER_LEN);
        if( buffer_start > buffer ) {
            memmove(buffer, buffer_start, current_len_at_start);
            buffer_start = buffer;
            buffer_end   = buffer+current_len_at_start;
        }
        const size_t len_required = required_size_at_least - current_len_at_start;
        int r = read_for_sure(buffer_end, len_required);
        this->buffer_end += r;
        
        if( r == 0 && bytes_in_buffer() == 0 ) {
            this->finished = true;
        }
        return bytes_in_buffer() >= required_size_at_least;
    }
    
    int read_for_sure(void* buf, size_t len)
    {
        char* buf2 = static_cast<char*>(buf);
        size_t readed = 0;
        while( len > 0 ) {
            int r = this->input->read(buf2, len);
            if( r == 0 ) {
                return readed;
            }
            readed += r;
            len -= r;
            buf2 += r;
        }
        return readed;
    }
    
    bool next() {
        switch( this->input_encoding ) {
        case UTF8:
            return next_utf8();
        default:
            throw std::runtime_error(tsprintf("json_lexer: encoding %s not implemented", (int)this->input_encoding));
        }
    }
    
    bool next_utf8() {
        // ok, this is fake, we just return byte by byte
        if( !fill_buffer(1) )
            return false;
        this->current = * (this->buffer_start);
        this->buffer_start += 1;
        return true;
    }
    
    void detect_encoding()
    {
        int r =  this->read_for_sure(this->buffer, 4);
        this->buffer_start = buffer;
        this->buffer_end   = buffer + r;
        
        switch( r ) {
        case 0: // 0-byte JSON,
            this->finished = true;
            break;
        case 1: 
            // only 1-byte JSON, so it can be actually only 1-digit integer
            this->input_encoding = UTF8;
            break;
        case 2:
        case 3:
            // only 2- or 3-byte JSON, so it can be 1 UTF-8 or invalid UTF16
            if( this->buffer[0] == 0 ) {
                this->input_encoding = UTF16_BE;
            } else if( this->buffer[1] == 0 ) {
                this->input_encoding = UTF16_BE;
            } else {
                this->input_encoding = UTF8;
            }
            break;
        case 4:
            if( this->buffer[0] == 0 ) { // 00 ... 
                if (this->buffer[1] == 0 ) { // 00 00 ...
                    this->input_encoding = UTF32_BE;
                } else { // 00 xx ...
                    this->input_encoding = UTF16_BE;
                }
            } else { // xx ...
                if (this->buffer[1] == 0 ) { // xx 00 ...
                    if( this->buffer[2] == 0 ) { // xx 00 00 ..
                        this->input_encoding = UTF32_LE;
                    } else { // xx 00 xx ...
                        this->input_encoding = UTF16_LE;
                    }
                } else { // xx xx ...
                    this->input_encoding = UTF8;
                }
            }
            break;
        }
    }
    void fail(std::string const& message)
    {
        TINFRA_TRACE(json_lexer_tracer, "failure: " << message);
        throw std::runtime_error(tsprintf("json_lexer: %s", message));
    }
    int parse_hexdigit(int c) {
        switch( c ) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'a': case 'A': return 10;
        case 'b': case 'B': return 11;
        case 'c': case 'C': return 12;
        case 'd': case 'D': return 13;
        case 'e': case 'E': return 14;
        case 'f': case 'F': return 15;
        }
        return 0;
    }
    void consume_string()
        // assuming that current == ' or "
        // parse string and put it into this->last_token
    {
        last_token = "";
        TINFRA_ASSERT(current == '"');
        next(); // skip the "
        while( !finished ) {
            switch(this->current) {
            case '"': // just break
                next();
                return;
            case '\\':
                if( !next() ) {
                    fail("'\\' at end of input");
                }
                switch(this->current) {
                case '"':  last_token.append(1, '"'); break;
                case '\\': last_token.append(1, '\\'); break;
                case '/':  last_token.append(1, '/'); break;
                case 'b':  last_token.append(1, '\b'); break;
                case 'f':  last_token.append(1, '\f'); break;
                case 'n':  last_token.append(1, '\n'); break;
                case 'r':  last_token.append(1, '\r'); break;
                case 't':  last_token.append(1, '\t'); break;
                case 'u':
                    // TBD. parse unicode
                    {
                        int result_char = 0;
                        for(int i = 0; i < 4; ++i) {
                            if( !next() ) {
                                fail("invalid '\\uXXXX' expression, expecting 4 hex digits but EOF reached");
                            }
                            if( !isxdigit(this->current)) {
                                fail("invalid '\\uXXXX' expression, expecting 4 hex digits, non-hex digit found");
                            }
                            int n = parse_hexdigit(this->current);
                            result_char = (result_char << 8) | n;
                        }
                        if( result_char > 127) fail("we don't support anything plain old ASCII");
                        last_token.append(1, (char)result_char);
                    }
                    break;
                default:
                    fail("unknown escape character");
                }
                next();
                break;
            default:
                if( this->current > 127) fail("we don't support anything plain old ASCII");
                last_token.append(1, (char)this->current);
                next();
                break;
            }
        } // end while
        fail("unterminated string constant (expected \")");
    }
    
    json_token::token_type consume_number()
        // assuming that current is first digit of number
        // parse numer and put it into this->last_token
        // and return token type (DOUBLE | INTEGER)
    {
        json_token::token_type token_type = json_token::INTEGER;
        last_token = "";
        while( !finished ) {
            switch( this->current ) {
            case 'e': case 'E':
            case '.':
                token_type = json_token::DOUBLE;
                last_token.append(1, (char)this->current);
                next();
                break;
            case '-':
            case '+':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': // numbers: integer or double
                last_token.append(1, (char)this->current);
                next();
                break;
            default: // any other character means end of token
                return token_type;
            }
        }
        return token_type;
    }
    
    void consume_keyword(const char* keyword) {
        const char* ic = keyword+1;
        while( *ic ) {
            if( !next() ) {
                fail(tsprintf("bad keyword, expected %s but EOF found", keyword));
            }
            if( this->current != *ic ) {
                fail(tsprintf("bad keyword, expected %s", keyword));
            }
            ic++;
        }
        TINFRA_TRACE(json_lexer_tracer, "readed keyword " << keyword);
        next();
    }
};
json_lexer::json_lexer(tinfra::input_stream& s):
    self(new internal_data())
{
    self->input = &s;
    self->current = -1;
    self->finished = 0;
    self->buffer_start = self->buffer;
    self->buffer_end = self->buffer;
    
    self->detect_encoding();
    self->next();
}
json_lexer::~json_lexer()
{
}

bool json_lexer::fetch_next(json_token& tok)
{
    while (! self->finished ) {
        switch( self->current ) {
        case '{':
            tok.type = json_token::OBJECT_BEGIN;
            self->next();
            TINFRA_TRACE(json_lexer_tracer, "readed OBJECT_BEGIN");
            return true;
        case '}':
            tok.type = json_token::OBJECT_END;
            self->next();
            TINFRA_TRACE(json_lexer_tracer, "readed OBJECT_END");
            return true;
        case '[':
            tok.type = json_token::ARRAY_BEGIN;
            self->next();
            TINFRA_TRACE(json_lexer_tracer, "readed ARRAY_BEGIN");
            return true;
        case ']':
            tok.type = json_token::ARRAY_END;
            self->next();
            TINFRA_TRACE(json_lexer_tracer, "readed ARRAY_END");
            return true;
        case ':': // 
            tok.type = json_token::COLON;
            self->next();
            TINFRA_TRACE(json_lexer_tracer, "readed COLON");
            return true;
        case ',': // COMMA
            tok.type = json_token::COMMA;
            self->next();
            TINFRA_TRACE(json_lexer_tracer, "readed COMMA");
            return true;
        case '"':  // "string"
                   // NOTE, shall we support ' ??, RFC says that only " starts strings
            self->consume_string();
            tok.type  = json_token::STRING;
            tok.value = self->last_token;
            TINFRA_TRACE(json_lexer_tracer, "readed STRING (" << self->last_token <<")");
            return true;
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': // numbers: integer or double
            tok.type  = self->consume_number();
            tok.value = self->last_token;
            TINFRA_TRACE(json_lexer_tracer, "readed " <<
                                            (tok.type == json_token::INTEGER ? "INTEGER": "DOUBLE") <<
                                            " (" << self->last_token <<")");
            return true;
        case 't': // keywords true
            self->consume_keyword("true");
            tok.type = json_token::TOK_TRUE;
            TINFRA_TRACE(json_lexer_tracer, "readed TOK_TRUE");
            return true;
        case 'f': // keywords: false
            self->consume_keyword("false");
            tok.type = json_token::TOK_FALSE;
            TINFRA_TRACE(json_lexer_tracer, "readed TOK_FALSE");
            return true;
        case 'n': // keywords: TOK_NULL
            self->consume_keyword("null");
            tok.type = json_token::TOK_NULL;
            TINFRA_TRACE(json_lexer_tracer, "readed NULL");
            return true;
        case ' ': case '\t': case '\r': case '\n': // whitespace
            self->next();
            continue;
        default:
            self->fail(tsprintf("unknown input %s", self->current)); 
        }
    }
    return false;
}

} // end namespace tinfra
