//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/multitype_map.h"
#include <string>

#include <unittest++/UnitTest++.h>


using namespace std;

#define CHECK_VALUE(m, k, T, v)       \
        m.put<T>(k,v);                \
        CHECK(m.contains<T>(k));      \
        CHECK_EQUAL(v, m.get<T>(k) )  

SUITE(tinfra) {
TEST(multitype_map)
{
    tinfra::multitype_map<string> m;
    
    CHECK_VALUE(m, "a", int, 5);
    CHECK_VALUE(m, "b", double, 2.4);
    CHECK_VALUE(m, "c", string, "Z");
    
    CHECK( m.begin<int>() != m.end<int>() );
    CHECK( m.begin<double>() != m.end<double>() );
    CHECK( m.begin<string>() != m.end<string>() );
    
    CHECK(  m.contains_type<int>() );
    CHECK( !m.contains_type<short>() );
    
    CHECK(  m.contains_type(typeid(int)) );
    CHECK( !m.contains_type(typeid(short)) );
    
    m.clear();
    CHECK( !m.contains<int>("a") );
    CHECK( !m.contains<double>("a") );
    CHECK( !m.contains<string>("Z") );
    
    CHECK( m.begin<int>() == m.end<int>() );
    CHECK( m.begin<double>() == m.end<double>() );
    CHECK( m.begin<string>() == m.end<string>() );
}
}