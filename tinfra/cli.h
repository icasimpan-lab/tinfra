#ifndef tinfra_cli_h_included
#define tinfra_cli_h_included

#include "option.h"
#include "tstring.h"

#include <vector>

namespace tinfra {

extern tinfra::option_switch opt_help;
    
int cli_main(int argc, char** argv, int (*program_main)(tstring const& program_name, std::vector<tinfra::tstring>& args));


} // end namespace tinfra

#endif // tinfra_cli_h_included