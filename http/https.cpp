//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <stdexcept>
#include <string>

#include <iostream>
#include <sstream>

#include "tinfra/cmd.h"
#include "tinfra/runtime.h"

#include "tinfra/aio.h"
#include "tinfra/aio_net.h"
#include "tinfra/connection_handler.h"
#include "tinfra/trace.h"

std::string fake_response;
static void build_fake_response()
{
    std::string fake_str = "abcdefgh\r\n";
    unsigned size = 10000000;
    fake_response.reserve(size + fake_str.size());
    for(unsigned i = 0; i < size/fake_str.size(); i++ ) {
            fake_response.append(fake_str);
    }
}

const int DEFAULT_PORT = 10456;

using tinfra::aio::connection_handler;
using tinfra::aio::generic_connection_handler;
using tinfra::generic_factory_impl2;
using tinfra::aio::dispatcher;
using tinfra::aio::listener;
using tinfra::aio::stream;

class http_request_connection_handler: public generic_connection_handler, tinfra::aio::listener {
public:
	typedef generic_factory_impl2<http_request_connection_handler, connection_handler::factory_type> 
		factory_type;
		
	http_request_connection_handler(std::auto_ptr<tinfra::aio::stream> stream, 
	                                std::string const& client_address)
		: generic_connection_handler(stream, client_address)
	{
		TINFRA_TRACE_MSG("http_request_connection_handler created");
	}
	
	~http_request_connection_handler()
	{
		TINFRA_TRACE_MSG("http_request_connection_handler destroyed");
	}
	
	listener& get_aio_listener() { return *this; }
	
	virtual void event(dispatcher& d, stream* c, int event)
	{
		
		if( event == dispatcher::READ ) {
			//TINFRA_TRACE_MSG("http_request_connection_handler: READ event");
			char x;
			if( c->read(&x, 1) == 0 ) {
				// clean eof
				TINFRA_TRACE_MSG("http_request_connection_handler: clean EOF");
				d.remove(c);
			} else {
			}
		} else {
			TINFRA_TRACE_MSG("http_request_connection_handler: WRITE event");
		}
	}
	
	virtual void failure(dispatcher& d, stream* c, int error)
	{
		TINFRA_TRACE_MSG("http_request_connection_handler: failure");
	}
	
	virtual void removed(dispatcher& d, stream* c)
	{
		TINFRA_TRACE_MSG("http_request_connection_handler: removed from dispatching");
	}
};

int listen(int port)
{

	using tinfra::io::stream;
	using std::auto_ptr;
	using tinfra::aio::connection_handler_aio_adapter;
	using tinfra::aio::dispatcher;

	auto_ptr<stream> listen_stream = tinfra::aio::create_service_stream("", port);
	
	http_request_connection_handler::factory_type connection_handler_factory;
	
	connection_handler_aio_adapter cl(connection_handler_factory);
	
	auto_ptr<dispatcher> dispatcher = tinfra::aio::dispatcher::create();
	
	dispatcher->add( listen_stream.get(), &cl, dispatcher::READ) ;
	
	bool should_continue = true;
	
	while( should_continue ) {
		dispatcher->step();
	
		tinfra::test_interrupt();
	}
	return 0;
}



int https_main(int argc, char** argv)
{
	tinfra::set_interrupt_policy(tinfra::DEFERRED_SIGNAL);
	return listen(DEFAULT_PORT);
}

TINFRA_MAIN(https_main);


