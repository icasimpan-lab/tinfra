//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/shared_ptr.h"
#include "tinfra/thread.h"

#include "tinfra/test.h" // for test infra

SUITE(tinfra) {
    using tinfra::shared_ptr;
    
    struct X
    {
        shared_ptr<X> next;
    };

    TEST(shared_ptr_implicit_reference)
    {
        shared_ptr<X> p(new X);
        p->next = shared_ptr<X>(new X);
        p = p->next;
        //CHECK(false);
        CHECK( p->next.get() == 0 );
    }
    
    
    void the_swap(shared_ptr<X>& a, shared_ptr<X>& b)
    {
        using std::swap;
        TINFRA_GLOBAL_TRACE("hello");
        swap(a,b);
    }
    TEST(shared_ptr_swap)
    {
        shared_ptr<X> p(new X);
        shared_ptr<X> a;
        
        the_swap(a,p);
    }
    // TODO: this test rquires helgrind for statuc/dynamic
    // analysis of thread interlavinga

    void* fun(void* _a)
    {
	shared_ptr<int>* a = reinterpret_cast<shared_ptr<int>* >(_a);

	*( a->get()) = 2;
	delete a;
	return 0;
    }
    TEST(shared_ptr_thread_passing)
    {
	tinfra::thread::thread_set ts;
	shared_ptr<int> x;
	{
	    shared_ptr<int> a(new int(6));

	    shared_ptr<int>* b = new shared_ptr<int>(a);
	    ts.start(&fun, b);
	    x = a;
	}

	ts.join();
	CHECK_EQUAL(2, * (x.get()) );
    }
    
    struct FOO {
        
        FOO() { instance_count++; }
        ~FOO() { instance_count--; }
        static int instance_count;
    };
    
    int FOO::instance_count = 0;
    TEST(shared_ptr_construct_from_auto_ptr)
    {
        CHECK_EQUAL(0, FOO::instance_count);
        std::auto_ptr<FOO> foo_orig(new FOO());
        CHECK_EQUAL(1, FOO::instance_count);
        {
            tinfra::shared_ptr<FOO> foo_shared(foo_orig); // should take away reference from foo_orig
            
            CHECK(foo_orig.get() == 0);
            CHECK_EQUAL(1, FOO::instance_count);
        }
        
        CHECK_EQUAL(0, FOO::instance_count);
    }
}
