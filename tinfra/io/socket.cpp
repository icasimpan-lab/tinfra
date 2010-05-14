//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/fmt.h"
#include "tinfra/os_common.h"

#include "tinfra/string.h" // debug only

#include "tinfra/win32.h"
#include "tinfra/trace.h"
#include <stdexcept>

#ifdef _WIN32
#include <winsock.h>

#define TS_WINSOCK

typedef int socklen_t;

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

#define TS_BSD
#endif

using namespace std;

namespace tinfra {
namespace io {
namespace socket {

/*
class stream {
    virtual ~stream() {}
    enum {
        start,
        end,
        current
    } seek_origin;        
    void close() = 0;
    int seek(int pos, seek_origin origin = start) = 0;
    int read(char* dest, int size) = 0;
    int write(const char* data, int size) = 0;
};
*/

#ifdef TS_WINSOCK
typedef SOCKET socket_type;
#else
typedef int    socket_type;
#endif

static const socket_type invalid_socket = static_cast<socket_type>(-1);

static int  close_socket_nothrow(socket_type socket);
static void close_socket(socket_type socket);
static void throw_socket_error(const char* message);

// TODO this log facility should be tinfra-wide
#ifdef _DEBUG
static void L(const std::string& msg)
{
    //std::cerr << "socket: " << escape_c(msg) << std::endl;
}
#else
#define L(a) (void)0
#endif

class socketstream: public stream {
    socket_type socket_;
public:
    socketstream(socket_type socket): socket_(socket) {}
    ~socketstream() {
        if( socket_ != invalid_socket ) {            
            if( close_socket_nothrow(socket_) == -1 ) {
                // TODO: add silent failures reporting
                // int err = socket_get_last_error();
                // tinfra::silent_failure(fmt("socket close failed: %i" % blabla )
            }
        }
    }
    void close() {        
        socket_type tmp(socket_);
        socket_ = invalid_socket;
        close_socket(tmp);
    }
    
    int seek(int, stream::seek_origin)
    {
        throw io_exception("sockets don't support seek()");
    }
    
    int read(char* data, int size)
    {
        L(fmt("%i: reading ...") % socket_);
        int result = ::recv(socket_, data ,size, 0);
        L(fmt("%i: readed %i '%s'") % socket_ % result % std::string(data,result));
        if( result == -1 ) {
            throw_socket_error("unable to read from socket");
        }
        return result;
    }
    
    int write(const char* data, int size)
    {
        L(fmt("%i: send '%s'") % socket_ % std::string(data,size));
        int result = ::send(socket_, data, size, 0);
        L(fmt("%i: sent %i") % socket_ % result);
        if( result == -1 ) {
            throw_socket_error("unable to write to socket");
        }
        return result;
    }
    
    void sync() 
    {
        // TODO: are sockets synchronized by default ? check it for unix/winsock
    }
    
    intptr_t native() const 
    {
        return socket_;
    }
    void release() 
    {
        socket_ = invalid_socket;
    }
    
    socket_type get_socket() const { return socket_; }
};

static void throw_socket_error(const char* message);
static void throw_socket_error(int error_code, const char* message);

static void ensure_socket_initialized()
{
#ifdef TS_WINSOCK
    static bool winsock_initialized = false;
    if( winsock_initialized ) return;
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(1, 1);

    err = WSAStartup(wVersionRequested, &wsaData);

    if (err != 0)
	throw_socket_error(err, "unable to initialize WinSock subsystem");

    if ( LOBYTE( wsaData.wVersion ) != 1 ||
	   HIBYTE( wsaData.wVersion ) != 1 ) {
        WSACleanup();
        err = WSAVERNOTSUPPORTED;
        throw_socket_error(err, "unsupported WinSock version");
    }
    winsock_initialized = true;
#endif
}

static int  socket_get_last_error()
{
#ifdef TS_WINSOCK
    return WSAGetLastError();
#else
    return errno;
#endif
}

static bool error_means_blocked(int error_code) {
#ifdef TS_WINSOCK
    if( error_code == WSAEWOULDBLOCK )
        return true;
#else
    if( error_code == EAGAIN || error_code == EINPROGRESS )
        return true;
#endif
    return false;
}

static void throw_socket_error(int error_code, const char* message)
{
#ifdef TS_WINSOCK
    tinfra::win32::throw_system_error(error_code, message);
#else
    throw_errno_error(errno, message);
#endif
}

static void throw_socket_error(const char* message)
{
    throw_socket_error(socket_get_last_error(), message);
}

static int close_socket_nothrow(socket_type socket)
{
#ifdef TS_WINSOCK
    return ::closesocket(socket);
#else
    return ::close(socket);
#endif    
}

static void close_socket(socket_type socket)
{
    int rc = close_socket_nothrow(socket);
    if( rc == -1 ) 
        throw_socket_error("unable to close socket");
}

static socket_type create_socket()
{
    ensure_socket_initialized();
    socket_type result = ::socket(AF_INET,SOCK_STREAM,0);
    if( result == invalid_socket ) 
        throw_socket_error("socket creation failed");
    return result;
}
#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

#if !defined(TS_WINSOCK) && !defined(HAVE_HSTRERROR)
static const char* hstrerror(int error_code)
{
	return "host not found";
}
#endif

static void get_inet_address(const char* address, int rport, struct sockaddr_in* sa)
{
    ensure_socket_initialized();
    if( address == 0 ) throw std::invalid_argument("null address pointer");
    
    std::memset(sa,0,sizeof(*sa));
    sa->sin_family = AF_INET;
    sa->sin_port = htons((short)rport);

    ::in_addr     ia;
    unsigned long ian =  ::inet_addr(address);
    ia.s_addr = ian;
    if( ian == INADDR_NONE ) {
        ::hostent*    ha;
        ha = ::gethostbyname(address);
        if( ha == NULL ) {
#ifdef TS_WINSOCK
            throw_socket_error(fmt("unable to resolve '%s'") % address);
#else
            std::string message = fmt("unable to resolve '%s': %s") % address % hstrerror(h_errno);
            switch( h_errno ) {
            case HOST_NOT_FOUND:
            case NO_ADDRESS:
            // TODO: check on uix machine: NO_DATA should be also domain error
            // case NO_DATA: 
                throw std::domain_error(message);
            default:
                throw std::runtime_error(message);
            }
#endif
        }            
    	std::memcpy(&sa->sin_addr, ha->h_addr, ha->h_length);
    } else {
        /* found with inet_addr or inet_aton */
        std::memcpy(&sa->sin_addr,&ia,sizeof(ia));
    }
    sa->sin_port = htons((short)rport);
}

stream* open_client_socket(char const* address, int port, int flags)
{   
    const bool non_blocking = (flags & NON_BLOCK) == NON_BLOCK ;
    ::sockaddr_in sock_addr;
    
    get_inet_address(address, port, &sock_addr);
    
    socket_type s = create_socket();
     
    if( non_blocking ) {
	    set_blocking(s, false);
    }
    
    if( ::connect(s, (struct sockaddr*)&sock_addr,sizeof(sock_addr)) != 0 ) {
        int error_code = socket_get_last_error();
	if( !non_blocking && !error_means_blocked(error_code) ) {
	    close_socket_nothrow(s);
	    throw_socket_error(error_code, fmt("unable to connect to '%s:%i'") % address % port);
	}
    }
    return new socketstream(s);
}

stream* open_server_socket(char const* listening_host, int port)
{
    ::sockaddr_in sock_addr;
    std::memset(&sock_addr,0,sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    if( listening_host && *listening_host ) {
        get_inet_address(listening_host, port, &sock_addr);
    } else {
        listening_host = "0.0.0.0";
        sock_addr.sin_port = htons((short) port);
    }
    
    socket_type s = create_socket();
    
    {
        int r = 1;
        if( ::setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)(void*)&r, sizeof(r)) ) {
            // TODO: it should be warning
            TINFRA_LOG_ERROR("unable to set SO_REUSEADDR=1 on socket");
        }
    }
    if( ::bind(s,(struct sockaddr*)&sock_addr, sizeof(sock_addr)) != 0 ) {
        int error_code = socket_get_last_error();
        close_socket_nothrow(s);
        throw_socket_error(error_code, fmt("bind to '%s:%i' failed") % listening_host % port );
    }

    if( ::listen(s,5) != 0 ) {
        int error_code = socket_get_last_error();
        close_socket_nothrow(s);
        throw_socket_error(error_code, "listen failed");
    }
    
    return new socketstream(s);
}

std::string get_peer_address_string(sockaddr_in const& address)
{
    char buf[64] = "0.0.0.0";
#ifdef HAVE_INET_NTOP
    if( inet_ntop(AF_INET, &address.sin_addr, buf, sizeof(buf)) == 0 ) {
        return "<unknown>";
    }
#elif defined TS_WINSOCK
    strncpy( buf, inet_ntoa(address.sin_addr), sizeof(buf));    
#endif
    return tinfra::fmt("%s:%i") % buf % ntohs(address.sin_port);
}

stream* accept_client_connection(stream* server_socket, std::string* peer_address)
{
    socketstream* socket = dynamic_cast<socketstream*>(server_socket);
    if( !socket ) 
        throw std::invalid_argument("accept: not a socketstream");
    
    socklen_t addr_size = sizeof(sockaddr_in);
    sockaddr_in client_address;
    socket_type accept_sock = ::accept(socket->get_socket(), (struct sockaddr*)&client_address, &addr_size );
    
    if( accept_sock == invalid_socket ) {
        int error_code = socket_get_last_error();
	if( error_means_blocked(error_code) )
	    return 0;
        else
	    throw_socket_error(error_code, "accept failed");
    }
    
    if( peer_address )
        *peer_address = get_peer_address_string(client_address);
    return new socketstream(accept_sock);
}

void set_blocking(intptr_t socket_, bool blocking)
{
    socket_type socket = static_cast<socket_type>(socket_);
    
#ifdef TS_WINSOCK
    unsigned long block = blocking ? 0 : 1;
    if( ioctlsocket(socket, FIONBIO, &block) < 0 )
        throw_socket_error("set_blocking: ioctlsocket(FIONBIO) failed");
#else
    int flags = fcntl( socket, F_GETFL );
    if( flags < 0 )
        throw_socket_error("set_blocking: fcntl(F_GETFL) failed");
    if( !blocking )
        flags |= O_NONBLOCK;
    else
        flags &= ~(O_NONBLOCK);
    if( fcntl( socket, F_SETFL, flags ) < 0 )
        throw_socket_error("set_blocking: fcntl(F_SETFL) failed");
#endif
    
}

} } } //end namespace tinfra::io::socket

