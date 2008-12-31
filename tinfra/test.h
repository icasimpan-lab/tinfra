//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_test_h__
#define __tinfra_test_h__

#include <string>

namespace tinfra {
namespace test {

class TempTestLocation {
public:
    explicit TempTestLocation(std::string const& name = "");
    ~TempTestLocation();
    
    std::string getPath() const;
        
    static void setTestResourcesDir(std::string const& x);
private:
    void init();
    std::string name_;
    std::string orig_pwd_;
    std::string tmp_path_;
};

void user_wait(const char* prompt = 0);

/// Test driver
///
/// Use this as your main function if building
/// unittester. It invokes UnitTest++ unittests.
/// 
/// If argument list is empty then it invokes all
/// unittests, if not. It invokes only tests
/// that are on argument list.
///
///
/// Test invocation is reported on stderr.
/// On win32 it's also reported to system debugger using OutputDebugString.
///
/// Example
///
/// If used then UnitTest++ library must be linked with executable.
///
/// Sample usage:
/// @code
///    int importer_main(int argc, char** argv)
///    {
///    #ifdef BUILD_UNITTEST
///        if( argc>1 && strcmp(argv[1], "--tests") )
///            return tinfra::test::test_main(argc,argv);
///    #endif
///        ...
///        normal processing
///    }
///  @endcode
///  @return 0 if all tests have passed, 1 if any of tests failed

int test_main(int argc, char** argv);

} } // end namespace tinfra::test

#endif
