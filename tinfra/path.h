//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_path_h_included
#define tinfra_path_h_included

#include <string>
#include "tinfra/tstring.h"

namespace tinfra { 
namespace path {

/// join path components
///
/// Create string representing PATH joined from several PATH
/// components.
///
std::string join(tstring const& p1, tstring const& p2 = "", tstring const& p3 = "", tstring const& p4 = "" );

bool is_executable(tstring const& name);
std::string basename(tstring const& name);

std::string dirname(tstring const& name);

std::string tmppath(const char* prefix = 0, const char* tmpdir = 0);

bool has_extension(tstring const& filename);
std::string extension(tstring const& filename);
std::string remove_extension(tstring const& filename);
std::string remove_all_extensions(tstring const& filename);

bool is_absolute(tstring const& filename);
//std::string sanitize(tstring const& path);

std::string search_executable(tstring const& filename, tstring const& path);
std::string search_executable(tstring const& filename);

} } // end namespace tinfra::path

#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

